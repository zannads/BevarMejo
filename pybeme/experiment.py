import copy
import os 
import json
import re

import numpy as np
import pandas as pd
# If pygmo is available we can have extra utilities about the pareto fronts and hypervolumes
try:
    import pygmo as pg
    pygmo_available = True
except ImportError:
    pygmo_available = False

from pybeme.simulator import Simulator

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
        self.__nadirs = None # Multi-index Pandas Series with the nadir point of each island.

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
        
        return "v25.06.02"

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
    
    @property
    def final_fitness_vectors(self) -> pd.DataFrame:
        # Return the fitness vector of the last population of each island
        return self.fitness_vectors.groupby(['island', 'individual']).last()
    
    @property
    def final_decision_vectors(self) -> pd.DataFrame:
        # Return the decision vector of the last population of each island
        return self.decision_vectors.groupby(['island', 'individual']).last()

    @property
    def nadir_points(self) -> pd.DataFrame:
        """
        Compute the nadir point for each island.
        
        The nadir point represents the worst objective values found across all individuals
        and generations for each island. It serves as a reference point for hypervolume
        calculations and multi-objective optimization analysis.
        
        Since each island may solve different problems (different objective functions),
        each island gets its own nadir point computed from its specific fitness vectors.
        
        Returns:
            pd.DataFrame: DataFrame with islands as index and nadir point components as columns.
                        Each row contains the maximum (worst) values for each objective
                        found in that island across all generations and individuals.
        """
        if self.__nadirs is None:
            nadirs = {}
            fvs = self.fitness_vectors

            for island, individuals in fvs.groupby('island'):
                # Compute nadir point as maximum values across all objectives for this island
                # (We assume it's a minimization problem as in pagmo/pygmo)
                nadirs[island] = np.max(individuals.to_numpy(), axis=0)

            self.__nadirs = pd.DataFrame.from_dict(nadirs, orient='index')

        return self.__nadirs

    if pygmo_available:
        def hypervolumes(self, ref_point) -> pd.DataFrame:
            """
            Compute hypervolume for each island and generation combination.
            
            Args:
                ref_point: Reference point for hypervolume calculation. Can be:
                        - A single list/array: same reference point for all islands
                        - A dict: {island_name: ref_point_array} for per-island reference points
                        - A pandas DataFrame: with islands as index and ref point components as columns
            
            Returns:
                pd.DataFrame: Multi-index DataFrame with index ['island', 'generation'] and 'hypervolume' column
            """
            fvs = self.fitness_vectors
        
            # Handle different ref_point input formats
            if isinstance(ref_point, dict):
                # Dictionary format: {island_name: ref_point_array}
                ref_points = ref_point
            elif isinstance(ref_point, pd.DataFrame):
                # DataFrame format: convert to dictionary
                ref_points = {island: row.values for island, row in ref_point.iterrows()}
            else:
                # Single ref_point for all islands
                ref_points = {}
                for island in fvs.index.get_level_values('island').unique():
                    ref_points[island] = ref_point
            
            # Lists to store index tuples and hypervolume values for the final multi-index dataframe
            index_list = []
            hypervolume_values = []
            
            # Group by island and generation
            for (island, generation), group in fvs.groupby(['island', 'generation']):
                
                fitness_array = group.to_numpy()
                
                # Get reference point for this island
                if island not in ref_points:
                    raise ValueError(f"No reference point provided for island '{island}'")
                
                island_ref_point = ref_points[island]

                # Remove points where ANY objective is worse than (greater than) the reference point 
                fitness_array = fitness_array[np.all(fitness_array <= island_ref_point, axis=1)]

                if len(fitness_array) == 0:
                    index_list.append((island, generation))
                    hypervolume_values.append(0.0)
                    continue

                # Compute hypervolume with the island-specific reference point
                hv = pg.hypervolume(fitness_array)
                hypervolume_value = hv.compute(island_ref_point)
                
                index_list.append((island, generation))
                hypervolume_values.append(hypervolume_value)
            
            multi_index = pd.MultiIndex.from_tuples(index_list, names=['island', 'generation'])
            return pd.DataFrame({'hypervolume': hypervolume_values}, index=multi_index)

    def individual(self, island_name: str, individual_index: int, generation_index: int = None, generation: int = None ) -> dict:
        if generation_index is None:
            # Find the generation index from the generations series
            generation_index = np.argmax(self.generations.to_numpy() == generation)

        return self.islands[island_name]['generations'][generation_index]['individuals'][individual_index]
    
    def simulator(self, individual_coord: tuple) -> Simulator:
        
        # You must extract the problem from the island of the individual, because
        # each island can potentially have a different problem (or same problem with different settings).
        individual = self.individual(island_name=individual_coord[0],
                                     generation=individual_coord[1],
                                     individual_index=individual_coord[2])
        individual_udp = self.island(individual_coord[0])['problem']

        simr = Simulator(
            decision_vector= individual['decision_vector'],
            problem= individual_udp,
            # Optional arguments
            fitness_vector= individual['fitness_vector'],
            id= individual['id'],
            print_message= "",
            bemelib_version= self.beme_version,
            lookup_paths= [os.path.expanduser(self.folder)]
        )
        return copy.deepcopy(simr)

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

        experiment_name = re.match(r"bemeexp__(.*)\..*", os.path.basename(experiment_namefile)).group(1)
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

    else:
        # We are in a folder of folders
        for folder in os.listdir(experiment_folder):
            if os.path.isdir(os.path.join(experiment_folder, folder)):
                if verbose:
                    print(f"Loading experiments in folder {folder}...")
                    print(" ")

                experiments.update(load_experiments(os.path.join(experiment_folder, folder), verbose))

    return experiments

if __name__ == "__main__":
    # TODO: Add a command line interface to load the experiments and extract the dataframes
    pass