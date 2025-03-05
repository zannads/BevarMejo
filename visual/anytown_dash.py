# Anytown dashboard to visualize the results of the optimization and perform simulations
# Run this script from the root directory of the project and provide a folder with the experiments as first argument
# Example: python visual/anytown_dash.py experiments_dir
# Export the root directory of the project to PYTHONPATH to be able to import the pybeme package
# Command: export PYTHONPATH=$PYTHONPATH:/Users/zannads/repos/BevarMejo
import os
import sys
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

import pygmo as pg
from epyt import epanet

import warnings
warnings.filterwarnings('ignore')

from pybeme.beme_experiment import load_experiments


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
        dcc.Graph(id='pareto-f', style={'height': '900px'}),
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
        dcc.Graph(id='simresults', style={'height': '1000px'})
    ])

def setup_callbacks(app, experiments):
    beme_dir =  os.path.dirname(os.path.dirname(os.path.abspath(__file__))) # The root folder of the project is outside the visual folder
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
            final_fvs = experiments[expname].final_fitness_vectors.to_numpy()

            # I want full color the best pareto front of each solution and a lighter color for the rest, which are still pareto fronts but for the individual islands
            # Also, I need to make transparent solutions in the best pareto front but that are not feasible (i.e. reliability index < 0)
            pf = pg.non_dominated_front_2d(final_fvs)
            pf = pf[final_fvs[pf,1] <= -0.1] # only feasible solutions (I should do a simulation but this will do)
            
            fig.add_trace(go.Scatter(x=final_fvs[pf,0], y=-final_fvs[pf,1], mode='markers', marker=dict(size=12, symbol='circle', color=colors[e]),  
                                    showlegend=True, name=expname,
                                    customdata=np.array(pf, dtype=str),
                                    hovertemplate='Cost: %{x:2.2f} <br> Reliability Index: %{y:.2f} <extra>%{customdata}</extra>'
                                    ) )
            
        return format_layout(fig)

    @app.callback(
        Output(component_id='network', component_property='src'),
        Output('simresults', 'figure'),
        Input('simproblem', 'value'),
        Input('simsolutionidx', 'value')
    )
    def update_sim(expname, final_individuals_idx):
        if (expname == None) or (final_individuals_idx == None):
            return "", go.Figure()

        # Create a temporary directory to save the simulation settings and its results
        tmp_dir = f'{beme_dir}/.tmp'
        if not os.path.exists(tmp_dir):
            os.makedirs(tmp_dir)

        # Extract the coordinates of the final individual, simulate, get the wntr objects and plot the results
        final_indv_coord = experiments[expname].final_fitness_vectors.index[final_individuals_idx]
        final_indv_coord = (final_indv_coord[0], # island name
                            experiments[expname].generations[final_indv_coord[0]].to_numpy()[-1], # last generation of the island
                            final_indv_coord[1]) # individual index

        net = experiments[expname].simulator(final_indv_coord).epanet_networks()[0]
        
        # How to render it on dash?? https://plotly.com/blog/dash-matplotlib/
        fig_net_matplotlib=net.plot(title='Network', nodesID=True, linksID=True)
        buf=BytesIO()
        fig_net_matplotlib.savefig(buf, format='png')
        # fig_net_matplotlib.savefig('.tmp/network.png', format='png')
        buf.seek(0) # rewind the file
        # Embed the result in the html output.
        fig_data = base64.b64encode(buf.read()).decode("ascii").replace("\n", "")
        fig_bar_matplotlib = f'data:image/png;base64,{fig_data}'

        # Prepare the plot for the results
        fig_res=subplots.make_subplots(rows=3, cols=1, shared_xaxes=True, vertical_spacing=0.1, subplot_titles=('Patterns', 'Pressure [m]', 'Flow [m3/s]'))

        # Run the simulation
        hres = net.getComputedHydraulicTimeSeries()

        # Extract and setup the time.
        time=np.array(hres.Time)
        

        h_ts = net.getTimeHydraulicStep()
        p_ts = net.getTimePatternStep()

        # First plot: patterns
        patterns = net.getPattern()
        cumpattern=np.zeros(patterns.shape[1])
        for i in range(net.getLinkPumpCount()):
            pattern_idx = net.getLinkPumpPatternIndex(i)-1
            cumpattern+=patterns[pattern_idx, :]
        
        p=np.zeros(len(time))
        d=np.zeros(len(time))
        demand_pattern_idx = 0
        for i, t in enumerate(time):
            time_idx_in_pattern = t // p_ts % patterns.shape[1]
            p[i]= cumpattern[ time_idx_in_pattern ]
            d[i]= patterns[ demand_pattern_idx, time_idx_in_pattern]

        # Repeat the values to plot the steps
        tt=time.reshape(-1, 1)
        tt=np.hstack([tt[0:-1],tt[1:]]).flatten()
        p = np.repeat(p, 2)
        d = np.repeat(d, 2) 

        fig_res.add_trace(go.Scatter(x=tt, y=d, name='Demand Pattern',
                                        mode='lines', line=dict(color='blue'),
                                        # hovertemplate='Time: %{(x/3600+18)/24:.2f} <br> D-M: %{y:.2f}'),
        ), row=1, col=1)
        fig_res.add_trace(go.Scatter(x=tt, y=p, name='# Pumps running',
                                        mode='lines',  line=dict(color='red')),
                                        row=1, col=1)
        
        # Second and third plots: pressure and flow
        nodes2plot=['41', '42', 'T0', 'T1'
               # '1', '2', '3', '4', '5', '6', '7', '8', '9', '10', 
               # '11', '12', '13', '14', '15', '16', '17', '18', '19', '20',
               # '21', '22'
               ]
        net_nodes = net.getNodeNameID()
        for n, node2plot in enumerate(nodes2plot):
            if node2plot not in net_nodes:
                continue
            
            h=np.array(hres.Head[:, net_nodes.index(node2plot)])*0.3048
            q=np.array(hres.Demand[:, net_nodes.index(node2plot)]).reshape(-1, 1)*0.06309019640344
            qq=np.hstack([q,q]).flatten()
            fig_res.add_trace(go.Scatter(
                x=time, y=h, mode='lines', name=node2plot, showlegend=True, legendgroup=node2plot,
                line=dict(color=colors[n % len(colors)])
            ), row=2, col=1)
            fig_res.add_trace(go.Scatter(
                x=tt, y=qq, mode='lines', name=node2plot+'(q)', showlegend=False, legendgroup=node2plot,
                line=dict(color=colors[n % len(colors)])
            ), row=3, col=1)

        tlab=['18:00', '23:00', '04:00', '09:00', '14:00']
        fig_res.update_xaxes(title='Time [h]', range=[0, 24*3600], showline=True, showgrid=True, linewidth=1, linecolor='grey', zerolinecolor='black', gridcolor='lightgrey', tickvals=np.array(np.arange(0,24*3600,5*3600)), ticktext=tlab, row=3, col=1)
        fig_res.update_yaxes(title='Multipliers', range=[0, 3.1], showline=True, showgrid=True, linewidth=1, linecolor='grey', zerolinecolor='black', gridcolor='lightgrey', row=1, col=1)   
        fig_res.update_yaxes(title='Head [m]', showline=True, showgrid=True, linewidth=1, linecolor='grey', zerolinecolor='black', gridcolor='lightgrey', row=2, col=1)
        fig_res.update_yaxes(title='Flow [L/s]', showline=True, showgrid=True, linewidth=1, linecolor='grey', zerolinecolor='black', gridcolor='lightgrey', row=3, col=1)
        
        return fig_bar_matplotlib, fig_res
        

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
    
    # Load experiments
    experiments = load_experiments(experiments_dir, verbose=False)
    if not experiments:
        print("No experiments found!")
        sys.exit(1)

    # Initialize and configure the dashboard
    app = initialize_app(sorted(list(experiments.keys())))
    setup_callbacks(app, experiments)
    
    # Run the server
    app.run(debug=True, port=8050)  # Using default port and debug settings

if __name__ == '__main__':
    main()