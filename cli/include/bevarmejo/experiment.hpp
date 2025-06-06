//
//  experiment.hpp
//  bemelib_classes
//
//  Created by Dennis Zanutto on 06/07/23.
//

#ifndef BEMELIB_EXPERIMENT_HPP
#define BEMELIB_EXPERIMENT_HPP

#include <iostream>
#include <string>
#include <vector>

#include <pagmo/archipelago.hpp>
#include <pagmo/island.hpp>

#include "bevarmejo/io/json.hpp"
#include "bevarmejo/io/fsys.hpp"

namespace bevarmejo
{

class Experiment final
{
/*----------------------------------------------------------------------------*/
/*---------------------------- Member types ----------------------------------*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*---------------------------- Member objects --------------------------------*/
/*----------------------------------------------------------------------------*/
private:
    // The folder where the experiment is stored or going to be saved.
    fsys::path m__root_folder;
    // The full path to the settings file.
    fsys::path m__settings_file;
    // Additional lookup paths for the internal files.
    std::vector<fsys::path> m__lookup_paths;


    // Name of the experiment.
    std::string m__name;
    // Archipelago to run the experiment, container for all the objects.
    pagmo::archipelago m__archipelago;
    // Names for runtime and final results files save for each island.
    std::vector<std::string> m__islands_names;
    
    struct Settings {
        unsigned int n_evolves{1}; // Number of generations to evolve. At least 1.
        bool outf_indent{true}; // Enable indentation in the output files.
        unsigned int outf_indent_val{4}; // Indentation value for the output files. (Valid for JSON)
    } m__settings;

/*----------------------------------------------------------------------------*/
/*--------------------------- Member functions -------------------------------*/
/*----------------------------------------------------------------------------*/

// (constructor)
public:
    Experiment() = default;
    Experiment(const Experiment& other) = default;
    Experiment(Experiment&& other) = default;
    Experiment(const fsys::path &settings_file);
private:
    // Prepare the main experiment file.
    void prepare_exp_file() const;
    // Prepare the runtime files for the islands.
    void prepare_isl_files() const;
    // Build the experiment from the input file.
    void build(const Json &jinput);
    // Build the archipelago from the input file.
    void build_islands(const Json &typconfig, const Json &specs=Json{}, const std::size_t rand_starts=1);
    // Build an island from the input file.
    void build_island(const Json &config);

// (destructor)
public:
    ~Experiment() = default;
private:
    // Move and format the runtime files to the final files.
    void finalise_isl_files() const;
    // Finalise the experiment file and delete the runtime files.
    void finalise_exp_file() const;
    
// operator=
public:
    Experiment& operator=(const Experiment& other) = default;
    Experiment& operator=(Experiment&& other) = default;

// Element access
private:
    // The folder where the output files are stored.
    fsys::path output_folder() const;

    // The main file of the experiment, tracking info about all the islands.
    fsys::path exp_filename() const;

    // The file for the island, tracking the runtime data.
    fsys::path isl_filename(const std::size_t island_idx=0, bool runtime=false) const;

// Methods
public:
    static Experiment parse(int argc, char* argv[]);

    void pre_run_tasks();

    void run();

    void post_run_tasks();

private:
    // Freeze the runtime data of the island to the main experiment file.
    void freeze_isl_runtime_data(Json &jout, const pagmo::island &isl) const;
    
    // Append the runtime data of the island to the main experiment file.
    void append_isl_runtime_data(const pagmo::island &isl, const fsys::path &isl_filen) const;
    
}; // class Experiment

} // namespace bevarmejo

#endif /* BEMELIB_EXPERIMENT_HPP */
