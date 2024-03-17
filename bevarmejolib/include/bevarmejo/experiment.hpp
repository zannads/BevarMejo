//
//  experiment.hpp
//  bemelib_classes
//
//  Created by Dennis Zanutto on 06/07/23.
//

#ifndef BEMELIB_EXPERIMENT_HPP
#define BEMELIB_EXPERIMENT_HPP

#include <chrono>
#include <iostream>
#include <filesystem>
#include <string>

#include <pagmo/archipelago.hpp>
#include <pagmo/island.hpp>
#include <pagmo/algorithm.hpp>
#include <pagmo/population.hpp>
#include <pagmo/problem.hpp>


#include <pugixml.hpp>

#include <nlohmann/json.hpp>

#include "bevarmejo/io.hpp"
#include "bevarmejo/pagmo_helpers.hpp"

namespace bevarmejo {

namespace fsys = std::filesystem;

class Experiment{
protected:
    fsys::path _root_experiment_folder_;
    fsys::path _settings_filename_;

    pugi::xml_document _settings_;
    // For now I will modify only the seed, but in the future I will add more options
    unsigned int _seed_{0};
    // Ideally, here I change all the settings of the algorithm and the model
    

private: 
    // Name of the experiment
    std::string m_name;
    // Archipelago to run the experiment
    pagmo::archipelago m_archipelago;
    // Filenames for runtime and final results saving for each island
    std::vector<fsys::path> m_islands_filenames;
    // TODO: add the settings for each independent island

    // Flag for resuming the experiment.
    bool m_resume{false};
    // Flag to perform a deep copy of the experiment folder.
    bool m_deep_copy{false};

public:
    /* Constructors */
    // Default constructor
    Experiment() = default;
    // Starting from path to root folder and filename
    Experiment(std::string a_name, 
            fsys::path experiment_folder,
               unsigned int seed,
               fsys::path settings_filename = "beme_settings.xml");
    
    /* Setters and getters */
    const std::string& get_name() const;
    
    fsys::path input_dir() const ;
    fsys::path output_dir() const ;
    fsys::path settings_file();
    fsys::path runtime_file();

    pugi::xml_node algorithm_settings() const;
    // TODO: overload for multiple algorithms

    pugi::xml_node model_settings() const;
    // TODO :overload for multiple models



/*--- Methods ---*/

    // Construct the experiment from the settings 
    // TEMP: algo and pop, in the future some struct with the settings
    void build(pagmo::algorithm &algo, pagmo::population &pop);

    // Run the experiment
    // TEMP: the inputs are not necessary, everything will be loaded from the
    // archipelago inside the class but for now let's use the objects constructed
    // in the main
    void run(unsigned int n_generations);

    // void finalize_with_error ?

    // Save the outcome of the experiment
    void save_outcome();

    // Helper to standardize the output filename
    fsys::path main_filename() const;

private:
    // Save the final results of all the islands in the archipelago independently
    std::pair<std::vector<std::string>, std::string> save_final_results() const;

    // Save the final result of a single island (called by save_final_results())
    fsys::path save_final_result(const pagmo::island &isl, const fsys::path& filename) const;

    // Save the runtime result of a single island (called by run())
    bool save_runtime_result(const pagmo::island &isl, const fsys::path& filename) const;
    
}; // class Experiment

} // namespace bevarmejo

#endif /* BEMELIB_EXPERIMENT_HPP */
