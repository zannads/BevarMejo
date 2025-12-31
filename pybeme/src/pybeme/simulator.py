import os 
import json
import subprocess
import sys
from typing import Optional

import numpy as np

try:
    import wntr
    wntr_available = True
except ImportError:
    wntr_available = False

try:
    from epyt import epanet
    epyt_available = True
except ImportError:
    epyt_available = False

from .utility import formulations_conversions as fc
from .utility.bemelib_versions import get_bemelib_installed_releases, get_working_bemelib_release
from ._config import get_beme_project_path

class Simulator:
    # I need to create a dict with:
    # - the individual dv
    # - user defined problem settings
    # - the individual fv (optional)
    # - the individual id (optional)
    # - print message (optional)
    # - version of bemelib used (optional)
    # - lookup paths (optional)
    def __init__(
        self,
        # Mandatory positional arguments
        decision_vector: list,
        problem: dict,

        # Optional keyword arguments
        fitness_vector: Optional[list] = None,
        id: int = 0,
        print_message: str = "",
        bemelib_version: Optional[str] = None,
        lookup_paths: Optional[list] = None
    ):
        
        # default bemelib version is the max of the latest
        if bemelib_version is None:
            all_releases = get_bemelib_installed_releases()
            bemelib_version =  str(max(all_releases, key=lambda rel: rel.version).version)

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

        if not os.path.exists(directory):
            os.makedirs(directory)
        
        full_path = os.path.join(directory, f'bemesim__{self.data["id"]}.json')
        with open(f'{full_path}', 'w') as file:
            json.dump(self.data, file)

        return full_path
    
    def run(self, cli_flags: str = "") -> np.array:
        # Run the c++ simulator from command line
        
        # Save the data to a file that the cli will read
        simu_filepath = self.save()

        # Get the release version to run the correct executable
        release_info = get_working_bemelib_release(self.data["bemelib_version"])

        command = f'{release_info.full_path}/cli/beme-sim {simu_filepath} {cli_flags} --savefv --savemetrics'

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

        # Same thing for the metrics file
        metrics_file = f"{self.data['id']}.mtr.json"
        if os.path.exists(metrics_file):
            with open(metrics_file, 'r') as file:
                metrics = json.load(file)
            os.remove(metrics_file)
            self.metrics = metrics
        else:
            self.metrics = {}

        return self.result
    
    def save_inps(self, directory: str = ".tmp") -> list:
        # Save the network models to inp files
        os.makedirs(directory, exist_ok=True)
        
        # Run the simulator with the saveinp flag
        self.run("--saveinp")

        # List the inp files with the id in the name
        inp_files = [f for f in os.listdir() if f.startswith(f"{self.data['id']}") and f.endswith(".inp")]

        # Move the inp files to the specified directory
        for i, inp_file in enumerate(inp_files):
            os.rename(inp_file, os.path.join(directory, inp_file))
            inp_files[i] = os.path.join(directory, inp_file)

        return inp_files

    if wntr_available:
        def wntr_networks(self) -> list:
            # Return the water network models
            # Run with the saveinp flag
            # List the inp files with the id in the name
            # Load them one on one and return a list of wntr.epanet.io.WaterNetworkModel objects

            inp_files = self.save_inps()
            
            networks = []
            for inp_file in inp_files:
                wnet = wntr.network.WaterNetworkModel(inp_file)
                networks.append(wnet)
                os.remove(inp_file)

            self.networks = networks

            return self.networks
        
    if epyt_available:
        def epanet_networks(self, remove_files: bool = True) -> list:
            # Return the water network models
            # Run with the saveinp flag
            # List the inp files

            inp_files = self.save_inps()

            # Get the release version to run the correct executable
            release_info = get_working_bemelib_release(self.data["bemelib_version"])

            networks = []
            for inp_file in inp_files:
                enet = epanet(inp_file, version=2.3, ph=True, loadfile=True,
                              customlib=os.path.join(get_beme_project_path(), "bemelib", "extern", "EPANET.beme", str(release_info.epanet_version)[1:], "build", "lib", "libepanet2.dylib"),
                              display_msg=True, display_warnings=True)
                networks.append(enet)
                if remove_files:
                    os.remove(inp_file)

            self.networks = networks

            return self.networks
            
    def convert_to_formulation(self, formulation: str) -> "Simulator":

        suite = self.data['problem']['type'].split('::')[0]
        assert suite == 'bevarmejo'

        problem = self.data['problem']['type'].split('::')[1]

        if problem == 'anytown':
            # We use a numerical convention here
            f = int(formulation)
            fc.anytown__from_fx_to_fy(self, f)

        elif problem == 'anytown_systol25':
            # just pass the name along
            fc.anytown_systol25__from_a_to_b(self, formulation)

        else:
            raise ValueError(f"Unsupported problem type: {problem}")

