# Anytown dashboard to visualize the results of the optimization and perform simulations
import os
os.chdir(os.path.dirname(os.path.abspath(__file__)))
import sys
sys.path.append('..')
from dash import Dash, html, dcc, callback, Output, Input, dash_table
import plotly.express as px
import plotly.graph_objects as go
import plotly.subplots as subplots
import pandas as pd
import numpy as np
import matplotlib      
matplotlib.use('agg')
import matplotlib.pyplot as plt
import base64
from io import BytesIO
import subprocess

import warnings
warnings.filterwarnings('ignore')

from pybeme.beme_experiment import load_experiment, load_experiments, save_simulation_settings, list_all_final_individuals, extract_individual_from_coordinates
from pybeme.reference_set import naive_pareto_front

import wntr

def create_dashboard_layout(experiments_names):
    return html.Div([
        html.H1("Anytown Dashboard", style={'text-align': 'center'}),
        html.Div([
            html.Label("Select the experiments to visualize"),
            dcc.Checklist(
                id='experiment_checklist',
                options=[{'label': expname, 'value': expname} for expname in experiments_names],
                value=[],
                inline=True
            )
        ]),
        dcc.Graph(id='pareto-f'),
        html.Div([
            html.H2("Simulation of the solution", style={'text-align': 'center'}),
            dcc.Dropdown(
                id='simproblem', 
                options=[{'label': expname, 'value': expname} for expname in experiments_names],
                value=[]
            ),
            dcc.Input(
                id='simsolutionidx',
                debounce=True,
                placeholder='Insert the index of the solution to simulate',
                type='number'
            )
        ]),
        html.Div([
            html.Img(id='network', src='')
        ]),
        dcc.Graph(id='pattern'),
        dcc.Graph(id='simresults')
    ])

def setup_callbacks(app, experiments, beme_folder):
    colors = px.colors.qualitative.Dark2

    @app.callback(
        Output('pareto-f', 'figure'),
        Input('experiment_checklist', 'value')
    )
    def update_pareto_f(exps):
        fig=go.Figure()

        def format_layout(fig):
            xlims = [-.5e6, 20.5e6]
            ylims = [-0.05, 0.705]
            xaxis_title_text='Cost [$]'
            yaxis_title_text='Reliability Index [-]'
            axis_settings = dict(
                showline=True,
                showgrid=True,
                linewidth=1,
                linecolor='grey',
                zerolinecolor='black',
                gridcolor='lightgrey',
                automargin=True
            )
            
            fig.update_layout(
                title='Pareto Fronts',
                xaxis_title='Cost [$]',
                yaxis_title='Reliability Index [-]',
                legend_title='Experiments',
                plot_bgcolor='white',
                paper_bgcolor='white',
                xaxis=dict(range=xlims, **axis_settings),
                yaxis=dict(range=ylims, **axis_settings)
            )
            return fig
        
        if not exps:
            return format_layout(fig)
        
        for e, expname, in enumerate(exps):
            # Extract the fitness vector for all final individuals across all islands
            final_individuals = list_all_final_individuals(experiments[expname], coordinate=False)
            # for each island in islands and individual in individual extract the fitness-vector and put it in a numpy array
            fitness = np.array([ind['fitness_vector'] for ind in final_individuals])

            #fitness.append([9.97392e+06, -0.200667])
            # I want full color the best pareto front of each solution and a lighter color for the rest, which are still pareto fronts but for the individual islands
            # Also, I need to make transparent solutions in the best pareto front but that are not feasible (i.e. reliability index < 0)
            pf = naive_pareto_front(fitness, f__indexes=True)
            pf = pf[fitness[pf,1] <= -0.1] # only feasible solutions (I should do a simulation but this will do)
            
            fig.add_trace(go.Scatter(x=fitness[pf,0], y=-fitness[pf,1], mode='markers', marker=dict(size=12, symbol='circle', color=colors[e]),  
                                    showlegend=True, name=expname,
                                    customdata=np.array(pf, dtype=str),
                                    hovertemplate='Cost: %{x:2.2f} <br> Reliability Index: %{y:.2f} <extra>%{customdata}</extra>'
                                    ) )
            
        return format_layout(fig)

    @app.callback(
        Output(component_id='network', component_property='src'),
        Output('pattern', 'figure'),
        Output('simresults', 'figure'),
        Input('simproblem', 'value'),
        Input('simsolutionidx', 'value')
    )
    def update_sim(expname, final_individuals_idx):
        if (expname == None) or (final_individuals_idx == None):
            return "", go.Figure(), go.Figure()

        final_indv_coord = list_all_final_individuals(experiments[expname], coordinate=True)[final_individuals_idx]
        individual_id = extract_individual_from_coordinates(experiments[expname], final_indv_coord)['id']
        save_simulation_settings(experiments[expname], final_indv_coord)
        
        # run the simulation of beme to save the inp file from shell
        command=f'../builds/VSCode/Debug/cli/beme-sim .tmp/bemesim__{individual_id}.json --saveinp'
        simre = subprocess.run(command, shell=True, check=False, capture_output=True, text=True)
        print("Standard Output:\n", simre.stdout)
        print("Standard Error:\n", simre.stderr)

        subprocess.run('mv *.inp .tmp/', shell=True, check=True)
    
        net = wntr.epanet.io.WaterNetworkModel(f'.tmp/{individual_id}.inp')
        #net.options.time.hydraulic_timestep = 1*60*60
        #net.options.time.report_timestep = 60*60
        net.options.time.report_timestep = net.options.time.hydraulic_timestep
        net.options.hydraulic.demand_model = 'PDD'
        sim = wntr.sim.EpanetSimulator(net)
        # How to render it on dash?? https://plotly.com/blog/dash-matplotlib/
        fig_net_matplotlib=plt.figure()
        net_matplotlib_axes=wntr.graphics.plot_network(net, title='Network', node_labels=True, link_labels=True, ax=fig_net_matplotlib.gca())
        buf=BytesIO()
        fig_net_matplotlib.savefig(buf, format='png')
        # fig_net_matplotlib.savefig('.tmp/network.png', format='png')
        buf.seek(0) # rewind the file
        # Embed the result in the html output.
        fig_data = base64.b64encode(buf.read()).decode("ascii").replace("\n", "")
        fig_bar_matplotlib = f'data:image/png;base64,{fig_data}'

        # Plot the patterns 
        fig_pattern = go.Figure()
        cumpattern=np.zeros(24)
        for _, pump in net.pumps():
            pattern = net.get_pattern(pump.speed_pattern_name)
            cumpattern+=pattern.multipliers[0:24]

        t=np.array(np.arange(0, 24, 1)).reshape(-1, 1)
        t=np.hstack([t,t+1]).flatten()
        d=np.array(net.get_pattern(net.pattern_name_list[0]).multipliers).reshape(-1, 1)
        d=np.hstack([d,d]).flatten()
        p=cumpattern.reshape(-1, 1)
        p=np.hstack([p,p]).flatten()

        fig_pattern.add_trace(go.Scatter(x=t, y=d, name='Demand Pattern',
                                        mode='lines', line=dict(color='blue'),
                                        hovertemplate='Time: %{(x/3600+18)/24:.2f} <br> D-M: %{y:.2f}'))
        fig_pattern.add_trace(go.Scatter(x=t, y=p, name='# Pumps running',
                                        mode='lines',  line=dict(color='red')))
        tlab=['18:00', '23:00', '04:00', '09:00', '14:00']
        fig_pattern.update_xaxes(title='Time of the day [h]', range=[0, 24], showline=True, showgrid=True, linewidth=1, linecolor='grey', zerolinecolor='black', gridcolor='lightgrey', tickvals=np.array(np.arange(0,24,5)), ticktext=tlab)
        fig_pattern.update_yaxes(title='Multipliers', range=[0, 3.1], showline=True, showgrid=True, linewidth=1, linecolor='grey', zerolinecolor='black', gridcolor='lightgrey')   

        fig_res=subplots.make_subplots(rows=2, cols=1, shared_xaxes=True, vertical_spacing=0.1, subplot_titles=('Pressure [m]', 'Flow [m3/s]'))
        nodes=['41', '42', 'T0', 'T1']
        sim_res=sim.run_sim()
        t=np.array(sim_res.node['head'].index).reshape(-1, 1)
        tt=np.hstack([t,t+net.options.time.hydraulic_timestep]).flatten()
        for n, node in enumerate(nodes):
            if node not in sim_res.node['head'].columns:
                continue
            h=np.array(sim_res.node['head'][node]).reshape(-1, 1)
            hh=np.hstack([h,h]).flatten()
            q=np.array(sim_res.node['demand'][node]).reshape(-1, 1)
            qq=np.hstack([q,q]).flatten()
            fig_res.add_trace(go.Scatter(
                x=t.flatten(), y=h.flatten(), mode='lines', name=node, showlegend=True, legendgroup=node,
                line=dict(color=colors[n])
            ), row=1, col=1)
            fig_res.add_trace(go.Scatter(
                x=tt, y=qq, mode='lines', name=node+'(q)', showlegend=False, legendgroup=node,
                line=dict(color=colors[n])
            ), row=2, col=1)

        fig_res.update_xaxes(title='Time [h]', range=[0, 24*3600], showline=True, showgrid=True, linewidth=1, linecolor='grey', zerolinecolor='black', gridcolor='lightgrey', tickvals=np.array(np.arange(0,24*3600,5*3600)), ticktext=tlab, row=2, col=1)
        fig_res.update_yaxes(title='Pressure [m]', showline=True, showgrid=True, linewidth=1, linecolor='grey', zerolinecolor='black', gridcolor='lightgrey', row=1, col=1)
        fig_res.update_yaxes(title='Flow [m3/s]', showline=True, showgrid=True, linewidth=1, linecolor='grey', zerolinecolor='black', gridcolor='lightgrey', row=2, col=1)
        return fig_bar_matplotlib, fig_pattern, fig_res

def initialize_app(experiments_names):
    app = Dash('Anytown Dashboard')
    app.layout = create_dashboard_layout(experiments_names)
    return app

def main():
    if len(sys.argv) < 2:
        print("Please provide the experiments directory as first argument")
        sys.exit(1)
        
    # Get experiments directory from first argument
    experiments_dir = sys.argv[1]
    
    # Set working directory
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    
    # Load experiments
    experiments = load_experiments(experiments_dir, verbose=True)
    if not experiments:
        print("No experiments found!")
        sys.exit(1)

    # Initialize and configure the dashboard
    app = initialize_app(list(experiments.keys()))
    setup_callbacks(app, experiments, "../")  # Using default beme folder
    
    # Run the server
    app.run(debug=True, port=8050)  # Using default port and debug settings

if __name__ == '__main__':
    main()