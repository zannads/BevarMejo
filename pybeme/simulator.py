import os 
import json
import subprocess
import sys

import numpy as np

import wntr

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

class Simulator:
    # I need to create a dict with:
    # - the individual dv
    # - user defined problem settings
    # - the individual fv (optional)
    # - the individual id (optional)
    # - print message (optional)
    # - version of bemelib used (optional)
    # - lookup paths (optional)
    def __init__(self,
                 # Mandatory positional arguments
                 decision_vector: list,
                 problem: dict,

                # Optional keyword arguments
                    fitness_vector: list = None,
                    id: int = 0,
                    print_message: str = "",
                    bemelib_version: str = "v25.02.0",
                    lookup_paths: list = None):
        
        self.data = {
            "decision_vector": decision_vector,
            "problem": problem,
            "id": id,
            "fitness_vector": fitness_vector,
            "print": print_message,
            "bemelib_version": bemelib_version,
            "lookup_paths": lookup_paths
        }
        
    def save(self, directory: str = ".tmp") -> str:
        
        full_path = os.path.join(directory, f'bemesim__{self.data["id"]}.json')
        with open(f'{full_path}', 'w') as file:
            json.dump(self.data, file)

        return full_path
    
    def run(self, cli_flags: str = "") -> np.array:
        # Run the c++ simulator from command line
        
        # Save the data to a file that the cli will read
        simu_filepath = self.save()

        # Get the release version to run the correct executable
        release_version = get_release_version(self.data["bemelib_version"])

        # Prepare the command to run the simulator
        beme_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) # The root folder of the project is outside the pybeme folder
        release_dir = os.path.join(beme_dir, 'builds', release_version)
        command = f'{release_dir}/cli/beme-sim {simu_filepath} {cli_flags} --savefv'

        # Run the command
        simre = subprocess.run(command, shell=True, check=False, capture_output=True, text=True)
        print("Standard output:")
        print(simre.stdout)
        print("Standard error:")
        print(simre.stderr)
        print("Return code:")
        print(simre.returncode)

        # Upload the resulting fitness vector and delete the temporary file
        with open(f"{self.data['id']}.fv.json", 'r') as file:
            fv = json.load(file)
        os.remove(f"{self.data['id']}.fv.json")

        self.result = np.array(fv)
        return fv

    def wntr_networks(self) -> list:
        # Return the water network models
        # Run with the saveinp flag
        # List the inp files with the id in the name
        # Load them one on one and return a list of wntr.epanet.io.WaterNetworkModel objects

        self.run("--saveinp")

        inp_files = [f for f in os.listdir() if f.startswith(f"{self.data['id']}") and f.endswith(".inp")]

        networks = []
        for inp_file in inp_files:
            wn = wntr.network.WaterNetworkModel(inp_file)
            networks.append(wn)
            os.remove(inp_file)

        self.networks = networks

        return self.networks
        