import json
import os
import sys

from refactor_json_keys_case import recursive_refactor, kebab_case_to_snake_case

if __name__ == '__main__':
    """
    This script is used to convert the configuration files and the results files
    of an experiment before version 24.06.01 to the new format.
    Mainly three changes need to be applied:
    - All the keys in the same format (Sentence case).
    - the bemeopt file needs that the parameters of the problems are in the new format:
        - no nested dictionaries, ['WDS']['inp'] -> ['WDS inp'] and ['WDS']['UDEGs'] -> ['WDS UDEGs']
        - 'Available diameters' becomes two separate keys: 'Existing pipe options' and 'New pipe options'
        - 'Tank costs', 'Existing pipe options', 'New pipe options' are fixed.
        - only 4 subnetets ['WDS UDEGs'] remaain and are fixed. 
    - Add random starts key with the number equal to the number of islands.

    - The problem in each island is usually null and I will overwrite it with the problem in the optimisation settings file.

    - Finally, the experiment files' keys are changed to kebab case.
    """
    if len(sys.argv) < 2:
        print("Usage: python modernize_pre_v240601_exp.py <experiment_directory>")
        sys.exit(1)

    exp_folder = sys.argv[1]
    exp_name = exp_folder.split('/')[-1]

    # Create a copy folder with the name: exp_folder + "-modern"
    modern_folder = exp_folder + "-modern"
    if not os.path.exists(modern_folder):
        os.makedirs(modern_folder)
    else:
        print(f"Folder {modern_folder} already exists. Please remove it before running this script. Exiting...")
        os.sys.exit(1)

    # Load the experiment file (name bemeopt__settings.json)
    settings_file = os.path.join(exp_folder, 'bemeopt__settings.json')
    with open(settings_file, 'r') as file:
        settings_file = json.load(file)

    # Modify the problem in the experiment files
    wds_parameters = settings_file['Typical configuration']['UDP']['Parameters'].pop('WDS', {})
    settings_file['Typical configuration']['UDP']['Parameters']['WDS inp'] = wds_parameters.get('inp', '')

    settings_file['Typical configuration']['UDP']['Parameters']['WDS UDEGs'] = {
                    "city_pipes": ["2", "3", "4", "27", "28", "29", "30", "31", "32", "33", "34", "35", "37", "38", "41"],
                    "existing_pipes": ["1", "2", "3", "4", "5", "6", "7", "8", "9", "11", "12", "17", "18", "19", "20", "21", "22", "23", "24", "26", "27", "28", "29", "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "40", "41"],
                    "new_pipes": ["110", "113", "114", "115", "116", "125"],
                    "possible_tank_locations": ["1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "15", "16", "18", "19"]
                }

    # Add the new keys 'Existing pipe options' and 'New pipe options'
    settings_file['Typical configuration']['UDP']['Parameters']['Existing pipe options'] = [
                    {
                        "diameter__in": 6,
                        "cost_dup_city__per_ft": 26.2,
                        "cost_dup_resi__per_ft": 14.2,
                        "cost_clean_city__per_ft": 17.0,
                        "cost_clean_resi__per_ft": 12.0
                    },
                    {
                        "diameter__in": 8,
                        "cost_dup_city__per_ft": 27.8,
                        "cost_dup_resi__per_ft": 19.8,
                        "cost_clean_city__per_ft": 17.0,
                        "cost_clean_resi__per_ft": 12.0
                    },
                    {
                        "diameter__in": 10,
                        "cost_dup_city__per_ft": 34.1,
                        "cost_dup_resi__per_ft": 25.1,
                        "cost_clean_city__per_ft": 17.0,
                        "cost_clean_resi__per_ft": 12.0
                    },
                    {
                        "diameter__in": 12,
                        "cost_dup_city__per_ft": 41.4,
                        "cost_dup_resi__per_ft": 32.4,
                        "cost_clean_city__per_ft": 17.0,
                        "cost_clean_resi__per_ft": 13.0
                    },
                    {
                        "diameter__in": 14,
                        "cost_dup_city__per_ft": 50.2,
                        "cost_dup_resi__per_ft": 40.2,
                        "cost_clean_city__per_ft": 18.2,
                        "cost_clean_resi__per_ft": 14.2
                    },
                    {
                        "diameter__in": 16,
                        "cost_dup_city__per_ft": 58.5,
                        "cost_dup_resi__per_ft": 48.5,
                        "cost_clean_city__per_ft": 19.8,
                        "cost_clean_resi__per_ft": 15.5
                    },
                    {
                        "diameter__in": 18,
                        "cost_dup_city__per_ft": 66.2,
                        "cost_dup_resi__per_ft": 57.2,
                        "cost_clean_city__per_ft": 21.6,
                        "cost_clean_resi__per_ft": 17.1
                    },
                    {
                        "diameter__in": 20,
                        "cost_dup_city__per_ft": 76.8,
                        "cost_dup_resi__per_ft": 66.8,
                        "cost_clean_city__per_ft": 23.5,
                        "cost_clean_resi__per_ft": 20.2
                    },
                    {
                        "diameter__in": 24,
                        "cost_dup_city__per_ft": 109.2,
                        "cost_dup_resi__per_ft": 85.5,
                        "cost_clean_city__per_ft": 30.1,
                        "cost_clean_resi__per_ft": 1000000
                    },
                    {
                        "diameter__in": 30,
                        "cost_dup_city__per_ft": 142.5,
                        "cost_dup_resi__per_ft": 116.1,
                        "cost_clean_city__per_ft": 41.3,
                        "cost_clean_resi__per_ft": 1000000
                    }
                ]
    
    settings_file['Typical configuration']['UDP']['Parameters']['New pipe options'] = [
                    {"diameter__in": 6, "cost__per_ft": 12.8},
                    {"diameter__in": 8, "cost__per_ft": 17.8},
                    {"diameter__in": 10, "cost__per_ft": 22.5},
                    {"diameter__in": 12, "cost__per_ft": 29.2},
                    {"diameter__in": 14, "cost__per_ft": 36.2},
                    {"diameter__in": 16, "cost__per_ft": 43.6},
                    {"diameter__in": 18, "cost__per_ft": 51.5},
                    {"diameter__in": 20, "cost__per_ft": 60.1},
                    {"diameter__in": 24, "cost__per_ft": 77.0},
                    {"diameter__in": 30, "cost__per_ft": 105.5}
                ]

    settings_file['Typical configuration']['UDP']['Parameters']['Tank costs'] = [
        {"volume__gal": 50000, "cost": 115000},
        {"volume__gal": 100000, "cost": 145000},
        {"volume__gal": 250000, "cost": 325000},
        {"volume__gal": 500000, "cost": 425000},
        {"volume__gal": 1000000, "cost": 600000}
    ]

    # Remove the old "Alternative diameters" key
    settings_file['Typical configuration']['UDP']['Parameters'].pop('Available diameters', None)

    # Add the random starts key (number of files in the output folder minus 1 for the main experiment file)
    settings_file['Random starts'] = (len([name for name in os.listdir(os.path.join(exp_folder, 'output')) if os.path.isfile(os.path.join(exp_folder, 'output', name)) and not name.endswith('.DS_Store')]) - 1)

    # Save the modified JSON in the copy folder with the new name 'bemeopt__exp_name'
    with open(os.path.join(modern_folder, exp_name+'.json'), 'w') as file:
        json.dump(settings_file, file, indent=4)

    # Copy in the folder the only file that the configuration file depends on, the 'WDS inp' file.
    wds_inp_file = os.path.join(exp_folder, settings_file['Typical configuration']['UDP']['Parameters']['WDS inp'])
    os.system(f"cp {wds_inp_file} {modern_folder}/")

    # Now for each file in the output folder, I will change the problem in the json object of the islands.

    # make the output directory 
    output_folder = os.path.join(modern_folder, 'output')
    os.makedirs(output_folder)

    # copy the **__exp file with the new name 'bemexp__${name}.json'
    new_exp_name = exp_name.replace('bemeopt__', 'bemeexp__')+".json"
    os.system(f"cp {os.path.join(exp_folder, 'output', exp_name+'__exp.json')} {output_folder}/{new_exp_name}")
    
    # Open it so that we can modify the name of the islands and the keys
    # Keys will be converted from kebab-case to snake_case, but since they differ
    # only for multi word keys, only these keys will be changed.
    with open(os.path.join(output_folder, new_exp_name), 'r') as file:
        exp = json.load(file)

    for i, isl_relpath in enumerate(exp['archipelago']['islands']):
        print(isl_relpath)

        with open(os.path.join(exp_folder, 'output', isl_relpath), 'r') as file:
            island = json.load(file)

        # Only for algorithm: remove extra-info and add its content to parameters
        # rename NSGA_II to nsga2
        island['algorithm']['name'] = "nsga2"
        ef = island['algorithm'].pop('extra-info', {})
        # split the extra-info string in the parameters, use the \n as separator
        # then each subtoken (should be Generations: number and Verboisty: number) should be split by ':'
        # and the first token should be the key (in snkae_case and the second the value)
        for token in ef.split('\n')[0:2]:
            key, value = token.split(':')
            island['algorithm']['parameters'][key.strip().replace(' ', '_').lower()] = int(value.strip())

        # For algorithm, island, replacement-policy and selection-policy
        # name -> type
        # ['name'] -> "pagmo::snake_case_name"
        def fix_containers_name(container):
            container['type'] = f"pagmo::{container.pop('name').replace(' ', '_').lower()}"
            return container
        cs = ['algorithm', 'island', 'replacement-policy', 'selection-policy']
        for c in cs:
            island[c] = fix_containers_name(island[c])

        # Generations, algorithm, island, replacement-policy, selection-policy
        # should recursively refactor the keys to snake_case
        cs = ['generations', 'algorithm', 'island', 'replacement-policy', 'selection-policy']
        for c in cs:
            island[c.replace('-', '_')] = recursive_refactor(island.pop(c), kebab_case_to_snake_case)

        # Finally, override the problem in the island with the problem in the settings file
        island['problem'] = {
            'type': settings_file['Typical configuration']['UDP']['Name'],
            'parameters': {
                'wds_inp': settings_file['Typical configuration']['UDP']['Parameters']['WDS inp'],
                'wds_udegs': settings_file['Typical configuration']['UDP']['Parameters']['WDS UDEGs'],
                'existing_pipe_options': settings_file['Typical configuration']['UDP']['Parameters']['Existing pipe options'],
                'new_pipe_options': settings_file['Typical configuration']['UDP']['Parameters']['New pipe options'],
                'tank_costs': settings_file['Typical configuration']['UDP']['Parameters']['Tank costs']
            }
        }
        if 'Operations' in settings_file['Typical configuration']['UDP']['Parameters']:
            island['problem']['parameters']['operations'] = settings_file['Typical configuration']['UDP']['Parameters']['Operations']

        # Some other keys may need to be fixed. But I will change them as I encounter them.

        # Reorder the keys in the alpahbetical order like the c++ library wold do
        new_relpathname = isl_relpath.replace('opt', 'isl')
        with open(os.path.join(output_folder, new_relpathname), 'w') as file:
            json.dump(island, file, indent=4, sort_keys=True)

        exp['archipelago']['islands'][i] = new_relpathname

    # Convert also all the other keys in the experiment file to snake_case
    exp['archipelago']['topology'] = {'type': "pagmo::unconnected"}
    exp['software']['bemelib_version'] = exp['software'].pop('bemelib-version')

    # Save the modified experiment file in the output folder
    with open(os.path.join(output_folder, new_exp_name), 'w') as file:
        json.dump(exp, file, indent=4)

    print("The experiment files have been successfully converted to the new format.")
    print(f"See the new folder: {modern_folder}")
