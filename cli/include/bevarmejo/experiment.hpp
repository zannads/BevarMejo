//
//  experiment.hpp
//  bemelib_classes
//
//  Created by Dennis Zanutto on 06/07/23.
//

#ifndef BEMELIB_EXPERIMENT_HPP
#define BEMELIB_EXPERIMENT_HPP

#include <iostream>
#include <filesystem>
namespace fsys = std::filesystem;
#include <string>
#include <vector>

#include <pagmo/archipelago.hpp>

#include <nlohmann/json.hpp>
using json_o = nlohmann::json;

namespace bevarmejo {

class Experiment final {

private:
    // The folder where the experiment is stored or going to be saved.
    fsys::path m__root_folder;
    // The full path to the settings file.
    fsys::path m__settings_file;
    // Name of the experiment.
    std::string m__name;
    // Additional lookup paths for the internal files.
    std::vector<fsys::path> m__lookup_paths;
    
    // Archipelago to run the experiment, container for all the objects.
    pagmo::archipelago m__archipelago;
    // Names for runtime and final results files save for each island.
    std::vector<std::string> m__islands_names;
    
    struct Settings {
        unsigned int n_evolves{1}; // Number of generations to evolve. At least 1.
    } m__settings;

public:
    /* Constructors and co all defaulted */
    Experiment() = default;
    Experiment(const Experiment& other) = default;
    Experiment(Experiment&& other) = default;
    Experiment& operator=(const Experiment& other) = default;
    Experiment& operator=(Experiment&& other) = default;
    ~Experiment() = default;

    Experiment(const fsys::path &settings_file);
private:
    void build(const json_o &jinput);

/*--- Methods ---*/
public:
    void run();

private:

    // Helpers for tracking data and the output files.
    fsys::path output_folder() const; // The folder where the output files are stored.
    fsys::path exp_filename() const; // The main file of the experiment, tracking info about all the islands.
    fsys::path isl_filename(std::size_t island_idx=0, bool runtime=false) const; // The file for the island, tracking the runtime data.

    void prepare_isl_files() const; // Prepare the runtime files for the islands.
    void prepare_exp_file() const; // Prepare the main experiment file.

    void freeze_isl_runtime_data(std::size_t island_idx, json_o &jdyn) const; // Freeze the runtime data of the island to the main experiment file.
    void append_isl_runtime_data(std::size_t island_idx) const; // Append the runtime data of the island to the main experiment file.

    void finalise_isl_files() const; // Move and format the runtime files to the final files.
    void finalise_exp_file() const; // Finalise the experiment file and delete the runtime files.
    
}; // class Experiment

} // namespace bevarmejo

#endif /* BEMELIB_EXPERIMENT_HPP */
