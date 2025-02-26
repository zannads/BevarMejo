import os 
import json
import re

import pandas as pd

def get_release_version(problem_version):
    """
    Given a problem version (as int or string), returns the appropriate release version
    based on the version compatibility ranges.
    
    Args:
        problem_version: The version number of the problem file, either as int (e.g., 250200) 
                        or string (e.g., 'v25.02.0')
        
    Returns:
        str: The release version that can run this problem in format 'release/XX.XX.XX'
    """
    # Convert string version to integer if needed
    if isinstance(problem_version, str) and problem_version.startswith('v'):
        # Remove 'v' prefix and split by dots
        version_parts = problem_version[1:].split('.')
        
        # Convert to integer format (YYMMDD)
        major = int(version_parts[0]) * 10000
        minor = int(version_parts[1]) * 100
        patch = int(version_parts[2])
        int_version = major + minor + patch
    else:
        int_version = problem_version
    
    # Determine the compatible release version using integer comparison
    if int_version < 230600:
        raise ValueError(f"Problem version {problem_version} is not compatible with any release version.")
    elif int_version < 240600:
        result_version = 240400
    elif int_version < 241100:
        result_version = 240600
    elif int_version < 241200:
        result_version = 241100
    elif int_version < 250200:
        result_version = 241200
    elif int_version <= 250200: # Latest version
        result_version = 250200
    else:
        raise ValueError(f"Problem version {problem_version} is not compatible with any release version.")
        
    # Convert integer version to formatted string
    major = result_version // 10000
    minor = (result_version % 10000) // 100
    patch = result_version % 100
    
    return f"releases/{major}.{minor}.{patch}"

# An experiment is a dictionary with the keys as in the JSON output files.
# However, we add some cached information to speed up the access to the data.
# For example, the fitness vector as a pandas dataframe.
class Experiment:
    def __init__(self, experiment_namefile: str, verbose=False):
        self.experiment_namefile = experiment_namefile
        self.data = self.__load_experiment(experiment_namefile, verbose) # JSON with the complete information

        # Cached data
        self.__generations = None # Multi-index Pandas Series with the reported generations.
        self.__timestamps = None # Multi-index Pandas Series with the timestamps of the reported generations.
        self.__fevals = None # Multi-index Pandas Series with the number of function evaluations of the reported generations.
        self.__fvs = None # Multi-index Pandas DataFrame with the fitness vectors of the individuals.
        self.__dvs = None # Multi-index Pandas DataFrame with the decision vectors of the individuals.
        self.__ids = None # Multi-index Pandas Series with the ids of the individuals.

    @property
    def name(self) -> str:
        return self.data['name']
    
    @property
    def folder(self) -> str:
        return self.data['folder']
    
    @property
    def beme_version(self) -> str:
        if 'software' in self.data and 'bemelib_version' in self.data['software']:
            return self.data['software']['bemelib_version']
        
        return "v25.02.0"

    @property
    def islands(self) -> dict:
        return self.data['archipelago']['islands']
    
    def island(self, island_name: str) -> dict:
        return self.data['archipelago']['islands'][island_name]
    
    @property
    def generations(self) -> pd.Series:
        if self.__generations is None:
            # Cache the generations in a multi-index pandas series.
            # Index: island, generation_index
            # Values: generation (absolute number, i.e., algorithm.parameters.generations * generation_index)

            index_list = []
            value_list = []
            
            for island_name, island in self.islands.items():
                report_generation = island['algorithm']['parameters']['generations']
                for generation_index, _ in enumerate(island['generations']):
                    index_list.append((island_name, generation_index))
                    value_list.append(report_generation * generation_index)

            # Convert to MultiIndex Series
            multi_index = pd.MultiIndex.from_tuples(index_list, names=['island', 'generation_index'])
            self.__generations = pd.Series(value_list, index=multi_index, name='generation', dtype='int')

        return self.__generations
    
    @property
    def timestamps(self) -> pd.Series:
        if self.__timestamps is None:
            # Cache the timestamps in a multi-index pandas series.
            # Index: island, generation 
            # Values: timestamp of the generation ('current_time' field)

            # Collect data
            index_list = []
            values = []

            for island_name, island in self.islands.items():
                for generation_index, generation in enumerate(island['generations']):
                    index_list.append((island_name, generation_index))
                    values.append(generation['current_time'])
            
            # Convert to MultiIndex Series
            multi_index = pd.MultiIndex.from_tuples(index_list, names=['island', 'generation'])
            self.__timestamps = pd.Series(values, index=multi_index, dtype='datetime64[ns]', name='current_time')

        return self.__timestamps
    
    @property
    def fitness_evaluations(self) -> pd.Series:
        if self.__fevals is None:
            # Cache the fitness evaluations in a multi-index pandas series.
            # Index: island, generation
            # Values: number of fitness evaluations of the generation

            index_list = []
            values = []

            for island_name, island in self.islands.items():
                for generation_index, generation in enumerate(island['generations']):
                    index_list.append((island_name, generation_index))
                    values.append(generation['fitness_evaluations'])
            
            # Convert to MultiIndex Series
            multi_index = pd.MultiIndex.from_tuples(index_list, names=['island', 'generation'])
            self.__fevals = pd.Series(values, index=multi_index, name='fitness_evaluations', dtype='int')

        return self.__fevals
    
    @property
    def fitness_vectors(self) -> pd.DataFrame:
        if self.__fvs is None:
            # Cache the fitness vectors in a multi-index pandas dataframe.
            # Index: island, generation, individual
            # Columns: fitness vector components

            index_list = []
            values = []

            for island_name, island in self.islands.items():
                for generation_index, generation in enumerate(island['generations']):
                    gen_value = self.generations[(island_name, generation_index)]
                    for individual_index, individual in enumerate(generation['individuals']):
                        index_list.append((island_name, gen_value, individual_index))
                        values.append(individual['fitness_vector'])

            # Convert to MultiIndex DataFrame
            multi_index = pd.MultiIndex.from_tuples(index_list, names=['island', 'generation', 'individual'])
            self.__fvs = pd.DataFrame(values, index=multi_index, columns=[f'fitness_value_{i+1}' for i in range(len(values[0]))])

        return self.__fvs
    
    @property
    def decision_vectors(self) -> pd.DataFrame:
        if self.__dvs is None:
            # Cache the decision vectors in a multi-index pandas dataframe.
            # Index: island, generation, individual
            # Columns: decision vector components

            index_list = []
            values = []

            for island_name, island in self.islands.items():
                for generation_index, generation in enumerate(island['generations']):
                    gen_value = self.generations[(island_name, generation_index)]
                    for individual_index, individual in enumerate(generation['individuals']):
                        index_list.append((island_name, gen_value, individual_index))
                        values.append(individual['decision_vector'])

            # Convert to MultiIndex DataFrame
            multi_index = pd.MultiIndex.from_tuples(index_list, names=['island', 'generation', 'individual'])
            self.__dvs = pd.DataFrame(values, index=multi_index, columns=[f'decision_variable_{i+1}' for i in range(len(values[0]))])

        return self.__dvs
    
    @property
    def ids(self) -> pd.Series:
        if self.__ids is None:
            # Cache the ids in a multi-index pandas series.
            # Index: island, generation, individual
            # Values: id of the individual

            index_list = []
            values = []

            for island_name, island in self.islands.items():
                for generation_index, generation in enumerate(island['generations']):
                    for individual_index, individual in enumerate(generation['individuals']):
                        index_list.append((island_name, generation_index, individual_index))
                        values.append(individual['id'])
            
            # Convert to MultiIndex Series
            multi_index = pd.MultiIndex.from_tuples(index_list, names=['island', 'generation', 'individual'])
            self.__ids = pd.Series(values, index=multi_index, name='id', dtype='int')

        return self.__ids
    
    def individual(self, island_name: str, generation_index: int, individual_index: int) -> dict:
        return self.islands[island_name]['generations'][generation_index]['individuals'][individual_index]  

    def __load_experiment(self, experiment_namefile: str, verbose=False) -> dict:
        """
        Load the results of an experiment from a json file.
        """
        # Check if the file exists
        if not os.path.exists(experiment_namefile):
            raise FileNotFoundError(f"File {experiment_namefile} does not exist.")

        # The name of the experiment is included after the name prefix (bemeexp__) and
        # before the extension of the file.
        # The experiment folder is outside the output folder, so I need to go two levels up.

        experiment_name = re.match(r"bemeexp__(.*)", os.path.basename(experiment_namefile)).group(1)
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

        islands = { }
        for island_relpath in experiment_results['archipelago']['islands']:
            # The island name is what remains after removing the prefix, the experiment
            # name and before the extensions.
            island_name = os.path.splitext(os.path.basename(island_relpath))[0].split('__')[-1]

            with open( os.path.join(experiment_folder, 'output', island_relpath), 'r') as file:
                islands[island_name] = json.load(file)

            if verbose:
                print(f"Results of island {island_relpath} loaded successfully.")

        experiment_results['archipelago']['islands'] = islands   

        # Convert the current_time field to a datetime object
        for island_name, island in experiment_results['archipelago']['islands'].items():
            for generation in island['generations']:
                generation['current_time'] = pd.to_datetime(generation['current_time'])

        if verbose:
            print(f"Results of experiment {experiment_name} loaded successfully.")

        experiment_results['name'] = experiment_name
        # However I have a relative path to were I am runnign the script, so I need to join it before saving it
        experiment_folder = os.path.realpath(experiment_folder)
        if experiment_folder.startswith(os.path.expanduser("~")):
            experiment_folder = experiment_folder.replace(os.path.expanduser("~"), "~", 1)

        experiment_results['folder'] = experiment_folder

        return experiment_results

    def save_simulation_settings_file(self, individual_coord: tuple, save_in_folder: str =".tmp") -> str:
        # I need to create a file with:
        # - the individual dv
        # - the individual fv (optional)
        # - the individual id (optional)
        # - print message (optional)
        # - version of bemelib used (optional)
        # - user defined problem settings
        # - lookup paths (optional)

        individual = self.individual(individual_coord[0], individual_coord[1], individual_coord[2])

        # Based on the island of the individual, the UDP settings could be slightly different.
        # For example, different islands may have been run with different water demand profiles.
        # So I extract the UDP settings from the island file.
        udp_sett = self.island(individual_coord[0])['problem']

        simu_sett= {
            "decision_vector": individual['decision_vector'],
            "fitness_vector": individual['fitness_vector'],
            "id": individual['id'],
            "print": "",
            "bemelib_version": self.beme_version,
            "problem": udp_sett,
            "lookup_paths": [
                os.path.expanduser(self.folder)
            ]
        }    
        id = simu_sett['id']
        full_path = os.path.join(save_in_folder, f'bemesim__{id}.json')
        with open(f'{full_path}', 'w') as file:
            json.dump(simu_sett, file)

        return full_path

def load_experiments(experiment_folder: str, verbose=False) -> dict:
    """
    Load all the experiments in a folder.
    Multi-experiments folder is supported look into folders.
    """
    # If the folder is non existing or non a directory, raise an error
    if not os.path.exists(experiment_folder) or not os.path.isdir(experiment_folder):
        raise FileNotFoundError(f"Folder {experiment_folder} does not exist.")
    
    experiments = { }
    # If the folder is an experiment folder (so contains the input data, the
    # optimisation settings file and the output folder), collect all the experiments
    # files in the output folder and run the load_experiment function on each of them.
    # Otherwise, if the folder contains instead multiple experiment folders, run
    # recursively this function on each of them and append the results to the dict.

    if 'output' in os.listdir(experiment_folder):
        # list the experiment files (bemexp__*.json) in the output folder
        experiment_files = [f for f in os.listdir(os.path.join(experiment_folder, 'output')) if f.startswith('bemeexp__') and f.endswith('.json')]
        
        if verbose:
            print(f"Loading {len(experiment_files)} experiments in folder {experiment_folder}...")
            print(" ")

        for experiment_file in experiment_files:
            experiment_namefile = os.path.join(experiment_folder, 'output', experiment_file)
            experiment = Experiment(experiment_namefile, verbose) # Load the experiment
            experiments[experiment.name] = experiment

            print(experiment.fitness_vectors)
            print(experiment.fitness_vectors.shape)
            print(experiment.fitness_vectors.columns)
            print(experiment.fitness_vectors.index)
            print(experiment.fitness_vectors.dtypes)
            print(experiment.fitness_vectors.groupby(['island', 'generation']).last().to_numpy())
    else:
        # We are in a folder of folders
        for folder in os.listdir(experiment_folder):
            if os.path.isdir(folder):
                if verbose:
                    print(f"Loading experiments in folder {folder}...")
                    print(" ")

                experiments.update(load_experiments(os.path.join(experiment_folder, folder), verbose))

    return experiments

if __name__ == "__main__":
    # TODO: Add a command line interface to load the experiments and extract the dataframes
    pass