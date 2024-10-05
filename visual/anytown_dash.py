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

import warnings
warnings.filterwarnings('ignore')

from pybeme.beme_experiment import load_experiment_results, save_simulation_settings, list_all_final_individuals, extract_individual_from_coordinates
from pybeme.reference_set import naive_pareto_front

import wntr 

def load_experiments_from_data() -> dict:
    # Load all the names of the experiments in the results folder in a list
    exp_dirs = [d for d in os.listdir('../data') if os.path.isdir(os.path.join('../data', d))]
    # print(exp_dirs)

    # List of the experiments to always remove
    exps2rem=["bemeopt__nsga2__hanoi_fbiobj", "bemeopt__nsga2__anytown_opers_f1__exp01", "bemeopt__nsga2__anytown_mixed_f1__exp01"]
    exp_dirs=[expname for expname in exp_dirs if expname not in exps2rem] 
    exp_dirs.sort()

    # Load the results of the experiments in a dictionary
    experiments={}
    for expdir in exp_dirs:
        # We have two different behaviours for before and after version 2024.10.0
        # Starts with after, if it fails, then it is before
        exp_files = [f for f in os.listdir(os.path.join('../data', expdir, 'output')) if os.path.splitext(f)[0].startswith('bemeexp__') and os.path.splitext(f)[0].endswith('.exp')]
        if len(exp_files) == 0:
            exp_files = [f for f in os.listdir(os.path.join('../data', expdir, 'output')) if os.path.splitext(f)[0].startswith('bemeopt__') and os.path.splitext(f)[0].endswith('__exp')]
        if len(exp_files) == 0:
            print(f'No experiment files found in {expdir}')
            continue
        for expfile in exp_files:
            try:
                exp = load_experiment_results(os.path.join('../data', expdir, 'output', expfile))
                experiments[exp['name']] = exp
            except Exception as e:
                print('Error loading experiment:', expfile)
                print('Exception:', e)
                continue
            
    return experiments

experiments = load_experiments_from_data()
exps_shortnames= {
    'nsga2__anytown_mixed_f1__exp02': "Integrated-1h-bug",
    'nsga2__anytown_mixed_f1__exp03': "Integrated-1h-fix",
    'nsga2__anytown_mixed_f1__exp04': "Integrated-30m-fix",
    'nsga2__anytown_mixed_f1__exp05': "Integrated-15m-fix",
    'nsga2__anytown_2ph_f1__nsga2inside': "2Ph-1h-bug",
    'nsga2__anytown_rehab_f1__exp03': "Design-1h-bug-wrongSiew",
    'nsga2__anytown_rehab_f1__median': "Design-1h-bug-median",
    'nsga2__anytown_rehab_f1__fullpower': "Design-1h-bug-fullpower",
    'nsga2__anytown_rehab_f1__exp04': "Design-1h-fix-wrongSiew",
    'nsga2__anytown_rehab_f1__exp05': "Design-1h-fix-Siew",
    'nsga2__anytown_rehab_f1__exp06': "Design-30m-fix-Siew",
    'nsga2__anytown_rehab_f1__exp07': "Design-15m-fix-Siew",
    '016': 'Design-15min-f2',
    '017': 'Integrated-15min-f2'
}

def at_dash_layout(experiments_names):

    return html.Div([
        html.H1("Anytown Dashboard", style={'text-align': 'center'}),
        html.Div([
            # Checkboxes to select the experiments
            html.Label("Select the experiments to visualize"),
            dcc.Checklist(
                id='experiment_checklist',
                options=[{'label': expname, 'value': expname} for expname in experiments_names],
                value=[],
                inline=True
            )
        ]),
        # Graphs to visualize the results
        dcc.Graph(id='pareto-f'),
        html.Div([
            html.H2("Simulation of the solution", style={'text-align': 'center'}),
            dcc.Dropdown(id='simproblem', options=[{'label': expname, 'value': expname} for expname in experiments_names], value=[]),
            dcc.Input(id='simsolutionidx', debounce=True, placeholder='Insert the index of the solution to simulate', type='number')
        ]),
        # Plots for epanet network and epanet pumps pattern
        html.Div([
            html.Img(id='network', src='')
        ]),
        dcc.Graph(id='pattern'),
        dcc.Graph(id='simresults')
    ])

colors = px.colors.qualitative.Dark2

app = Dash('Anytown Dashboard')
app.layout = at_dash_layout(list(experiments.keys()))

@app.callback(
    Output('pareto-f', 'figure'),
    Input('experiment_checklist', 'value')
)
def update_pareto_f(exps):
    fig=go.Figure()
    def fxl(fig):
        xlims = [-.5e6, 20.5e6]
        ylims = [-0.05, 0.705]
        xaxis_title_text='Cost [$]'
        yaxis_title_text='Reliability Index [-]'
        fig.update_layout(
            title='Pareto Fronts', xaxis_title='Cost [$]', yaxis_title='Reliability Index [-]', legend_title='Experiments', 
            plot_bgcolor='white',
            paper_bgcolor='white',
            xaxis=dict(
                title=xaxis_title_text,
                range=xlims,
                automargin=True,
                showline=True,
                showgrid=True,
                linewidth=1,
                linecolor='grey',
                zerolinecolor='black',
                gridcolor='lightgrey'
            ),
            yaxis=dict(
                title=yaxis_title_text,
                range=ylims,
                automargin=True,
                showline=True,
                showgrid=True,
                linewidth=1,
                linecolor='grey',
                zerolinecolor='black',
                gridcolor='lightgrey'
            ),
                    )
        return fig
    if (len(exps)<1):
        return fxl(fig)
    
    for e, expname, in enumerate(exps):
        # Extract the fitness vector for all final individuals across all islands
        final_individuals = list_all_final_individuals(experiments[expname], coordinate=False)
        # for each island in islands and individual in individual extract the fitness-vector and put it in a numpy array
        fitness = np.array([ind['fitness-vector'] for ind in final_individuals])

        #fitness.append([9.97392e+06, -0.200667])
        # I want full color the best pareto front of each solution and a lighter color for the rest, which are still pareto fronts but for the individual islands
        # Also, I need to make transparent solutions in the best pareto front but that are not feasible (i.e. reliability index < 0)
        pf = naive_pareto_front(fitness, f__indexes=True)
        pf = pf[fitness[pf,1] <= -0.1] # only feasible solutions (I should do a simulation but this will do)
        
        fig.add_trace(go.Scatter(x=fitness[pf,0], y=-fitness[pf,1], mode='markers', marker=dict(size=12, symbol='circle', color=colors[e]),  
                                showlegend=True, name=exps_shortnames[expname],
                                customdata=np.array(pf, dtype=str),
                                hovertemplate='Cost: %{x:2.2f} <br> Reliability Index: %{y:.2f} <extra>%{customdata}</extra>'
                                ) )
        
    return fxl(fig)

import subprocess
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

    beme_folder='../'

    # After version 2024.10.0, the optimisation settings file can have different names and in the same folder there can be multiple files
    # But they always start with "bemeopt__". Prefereably they are called "bemeopt__${expname}.json", but if they have a
    # different name, I need to open every optimisation settings file and make sure it is the right one
    final_indv_coord = list_all_final_individuals(experiments[expname], coordinate=True)[final_individuals_idx]
    individual_id = extract_individual_from_coordinates(experiments[expname], final_indv_coord)['id']
    save_simulation_settings(experiments[expname], final_indv_coord)
    
    # run the simulation of beme to save the inp file from shell
    command=f'{beme_folder}build-vs-code/cli/beme-sim .tmp/bemesim__{individual_id}__settings.json --saveinp'
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

if __name__ == '__main__':
    app.run(debug=True, port=8050)