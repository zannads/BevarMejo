import os 
import json
import re

import pandas as pd

def load_experiment_results(experiment_namefile: str, verbose=False) -> dict:
    """
    Load the results of an experiment from a json file.
    """
    # Check if the file exists
    if not os.path.exists(experiment_namefile):
        raise FileNotFoundError(f"File {experiment_namefile} does not exist.")

    # Before version 2024.10.0:
    """
    # The name of the experiment is included in the name of the file between the
    # name prefix (bemeopt__) and the name suffix (__exp). You can find it 
    # between the first '__' and the last '__' in the name of the file. 
    # First need to remove the path and the extension of the file.
    """
    # After version 2024.10.0:
    """
    # The name of the experiment is included after the name prefix (bemeexp__) and
    # before the name suffix (.exp), which comes before the extension of the file.
    # The experiment folder is one level outside the full file path.
    """
    if os.path.basename(experiment_namefile).startswith("bemeopt__"):
        experiment_name = re.match(r"bemeopt__(.*)__exp", os.path.basename(experiment_namefile)).group(1)
    else : # assume bemeexp__
        experiment_name = re.match(r"bemeexp__(.*)\.exp", os.path.basename(experiment_namefile)).group(1)
    
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
    for island in experiment_results['archipelago']['islands']:
        for generation in island['generations']:
            generation['current-time'] = pd.to_datetime(generation['current-time'])


    if verbose:
        print(f"Results of experiment {experiment_name} loaded successfully.")

    experiment_results['name'] = experiment_name
    # However I have a relative path to were I am runnign the script, so I need to join it before saving it
    experiment_folder = os.path.realpath(experiment_folder)
    if experiment_folder.startswith(os.path.expanduser("~")):
        experiment_folder = experiment_folder.replace(os.path.expanduser("~"), "~", 1)

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

class IndividualCoordinates:
    def __init__(self, island_idx, generation_idx, individual_idx):
        self.island_idx = island_idx
        self.generation_idx = generation_idx
        self.individual_idx = individual_idx

# Extract the individual information from the experiment results dictionary
def extract_individual_from_coordinates(experiment_results: dict, ind_coord: IndividualCoordinates) -> dict:
    """
    Extract the information of an individual from the results of an experiment.

    Inputs:
    - experiment_results: dictionary with the results of the experiment
    - individual_idx: coordinates of the individual in the experiment results

    Outputs:
    - individual: dictionary with the information of the individual
    """    
    return experiment_results['archipelago']['islands'][ind_coord.island_idx]['generations'][ind_coord.generation_idx]['individuals'][ind_coord.individual_idx]

def list_all_final_individuals(experiment_results: dict, coordinate=True) -> list:
    """
    List the coordinates of all the final individuals in the experiment results.
    """
    inds = []
    for island_idx, island in enumerate(experiment_results['archipelago']['islands']):
        for ind_idx, _ in enumerate(island['generations'][-1]['individuals']):
            if coordinate:
                inds.append(IndividualCoordinates(island_idx, -1, ind_idx))
            else:
                inds.append(experiment_results['archipelago']['islands'][island_idx]['generations'][-1]['individuals'][ind_idx])
    return inds

# Create a Simulation Settings file for beme-sim
def save_simulation_settings(exp, individual_coord, save_in_folder =".tmp") -> str:
    # I need to create a file with:
    # - the individual dv
    # - the individual fv (optional)
    # - the individual id (optional)
    # - print message (optional)
    # - version of bemelib used (optional)
    # - user defined problem settings
    # - lookup paths (optional)

    individual = extract_individual_from_coordinates(exp, individual_coord)
    individual_island = exp['archipelago']['islands'][individual_coord.island_idx]
    bemelib_version = "v24.10.0"
    if 'bemelib-version' in exp['software']:
        bemelib_version = exp['software']['bemelib-version']
    
    # Based on the island of the individual, the UDP settings could be slightly different.
    # For example, different islands may have been run with different water demand profiles.
    # So I extract the UDP settings from the island file. If this is empty, or 
    # the version is before 24.10.0, I use the UDP settings from the optimisation settings file.
    if bemelib_version < "v24.10.0" or 'problem' not in individual_island:
        # Open the optimisation settings file 
        with open(os.path.expanduser(os.path.join(exp['folder'], 'bemeopt__settings.json')), 'r') as file:
            opt_sett = json.load(file)
        udp_sett = opt_sett['Typical configuration']['UDP']
    else:
        udp_sett = individual_island['problem']

    simu_sett= {
        "decision-vector": individual['decision-vector'],
        "fitness-vector": individual['fitness-vector'],
        "id": individual['id'],
        "print": "",
        "bemelib-version": bemelib_version,
        "UDP": udp_sett,
        "lookup-paths": [
            os.path.expanduser(exp['folder'])
        ]
    }    
    id = simu_sett['id']
    with open(f'{save_in_folder}/bemesim__{id}__settings.json', 'w') as file:
        json.dump(simu_sett, file)

    return id

if __name__ == "__main__":
    if len(os.sys.argv) > 1:
        load_experiment_results(os.sys.argv[1], verbose=True)
    else:
        print("To load the results of an experiment, pass an experiment file as an argument to the script.")