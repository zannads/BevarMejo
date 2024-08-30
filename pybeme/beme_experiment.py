import os 
import json

import pandas as pd

def load_experiment_results(experiment_namefile: str, verbose=False) -> dict:
    """
    Load the results of an experiment from a json file.
    """
    # The name of the experiment is included in the name of the file between the
    # name prefix and the name suffix. You can find it between the first '__' and
    # the last '__' in the name of the file. First need to remove the path 
    # and the extension of the file.
    experiment_name = os.path.basename(experiment_namefile).split('__')[1:-1]
    experiment_name = '__'.join(experiment_name)
    # Experiment folder is one level outside the full file path
    experiment_folder = os.path.dirname(os.path.dirname(experiment_namefile))

    if verbose:
        print(f"Loading results of experiment {experiment_name} from file {experiment_namefile}...")

    # Now in the island key there is a list of filenames, each one containing the results
    # of the experiment for a different island. We load the results of each island and store
    # them in the dictionary.
    with open(experiment_namefile, 'r') as file:
        experiment_results = json.load(file)

    if verbose:
        print(f"Starting to load results of {len(experiment_results['archipelago']['islands'])} islands...")

    island_results = [ ]
    island_names = { }
    for island_relpath in experiment_results['archipelago']['islands']:
        # Island name is after the prefix and after the experiment name
        island_name = os.path.splitext(os.path.basename(island_relpath))[0]
        island_name = island_name.replace("opt__", "").split("__")[-1]
        # if island_relpath is absolute path, just use it, otherwise join it with experiment_folder and output
        if not os.path.isabs(island_relpath):
            island_relpath = os.path.join(experiment_folder, 'output', island_relpath)

        with open( island_relpath, 'r') as file:
            island_results.append( json.load(file) )
        island_names[island_name] = island_results[-1] # reference to the same object

        if verbose:
            print(f"Results of island {island_relpath} loaded successfully.")

    experiment_results['archipelago']['islands'] = island_results   # access by index
    experiment_results['archipelago']['island'] = island_names      # access by name

    # TODO: convert the current-time in each generation to a datetime object

    if verbose:
        print(f"Results of experiment {experiment_name} loaded successfully.")

    experiment_results['name'] = experiment_name
    experiment_results['folder'] = experiment_folder

    return experiment_results

def extract_dataframe(experiment_results: dict) -> pd.DataFrame:
    """
    Extract the results of an experiment in the form of a multiindex pandas dataframe.
    """

    # the index is made by: island, generation (fitness-vector/population), individual
    # the columns are the fitness values
    
    # Extract the island names
    island_names = list(experiment_results['archipelago']['island'].keys())
    
    # Initialize an empty list to store the dataframes for each island
    island_dataframes = []
    
    # Iterate over each island
    for island_name in island_names:
        # Extract the island results
        island_results = experiment_results['archipelago']['island'][island_name]
        
        # Initialize an empty list to store the data for each generation
        generation_data = []
        
        # Iterate over each generation
        for generation in island_results['generations']:
            # Extract the fitness vector for each individual in the generation
            fitness_vectors = [individual['fitness-vector'] for individual in generation['individuals']]
            
            # Create a dataframe for the generation with fitness vectors as columns
            generation_df = pd.DataFrame(fitness_vectors)
            
            # Set the column names as the fitness vector components
            generation_df.columns = [f'component_{i+1}' for i in range(len(fitness_vectors[0]))]
            
            # Add the generation dataframe to the list
            generation_data.append(generation_df)
        
        # Concatenate all the generation dataframes into a single dataframe for the island
        island_df = pd.concat(generation_data, keys=range(len(generation_data)), names=['generation'])
        
        # Add the island dataframe to the list
        island_dataframes.append(island_df)
    
    # Concatenate all the island dataframes into a single dataframe
    return pd.concat(island_dataframes, keys=island_names, names=['island'])

# Create a Simulation Settings file for beme-sim
def save_simulation_settings(opt_settings_file, exp, individual_idx, save_in_folder =".tmp") -> str:
    # I need to create a file with:
    # - the individual dv
    # - the individual fv (optional)
    # - the individual id (optional)
    # - print message (optional)
    # - version of bemelib used (optional)
    # - user defined problem settings
    # - lookup paths (optional)

    # First load the settings from the optimisation settings file (for the user defined problem)
    # Second use the results of the exp and the bemelib version
    with open(opt_settings_file, 'r') as file:
        opt_sett = json.load(file)

    all_individuals = [ind for island in exp['archipelago']['islands'] for ind in island['generations'][-1]['individuals'] ]

    individual = all_individuals[individual_idx]
    bemelib_version = "v24.08.0"
    if 'bemelib-version' in exp:
        bemelib_version = exp['bemelib-version']
    
    udp_sett = opt_sett['Typical configuration']['UDP']
    # TODO: based on the island of the individual, the UDP settings could be slightly different
    # for example. different islands, were run with different water demand profiles

    simu_sett= {
        "decision-vector": individual['decision-vector'],
        "fitness-vector": individual['fitness-vector'],
        "id": individual['id'],
        "print": "",
        "bemelib-version": bemelib_version,
        "UDP": udp_sett,
        "lookup-paths": [
            exp['folder']
        ]
    }    
    
    with open(f'{save_in_folder}/bemesim__{individual['id']}__settings.json', 'w') as file:
        json.dump(simu_sett, file)

    return simu_sett['id']

if __name__ == "__main__":
    if len(os.sys.argv) > 1:
        load_experiment_results(os.sys.argv[1], verbose=True)
    else:
        print("To load the results of an experiment, pass an experiment file as an argument to the script.")