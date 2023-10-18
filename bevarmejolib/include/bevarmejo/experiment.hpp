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

#include "pagmo/algorithm.hpp"
#include "pagmo/population.hpp"

#include "pugixml.hpp"

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
    
    std::time_t _start_time_{0}; // start time is always init within the constuctor
    std::time_t _end_time_{0}; // call finished method
    
    // For now they are here I will think what to do with them and how to handle the construction in the future.
    std::string _name_;
    std::string _user_custom_info_;
    
    
public:
    /* Constructors */
    // Default constructor
    Experiment(){ };
    // Starting from path to root folder and filename
    Experiment(fsys::path experiment_folder,
               unsigned int seed,
               fsys::path settings_filename = "beme_settings.xml");
    
    /* Methods */
    
    // Check complaiance of root folder
    bool is_compliant();
    // Make root folder complaiant
    void make_compliant();
    
    // Save end time of the experiment
    void finished();
    
    
    void save_final_result(pagmo::population &pop, pagmo::algorithm &algo);
    
    /* Setters and getters */
    void set_name(std::string name);
    std::string get_name();
    std::string get_extra_info();
    //void set_settings_filename(std::string& filename);
    
    fsys::path input_dir();
    fsys::path output_dir();
    fsys::path runtime_dir();
    fsys::path settings_file();
    fsys::path output_file();

    pugi::xml_node algorithm_settings() const;
    // TODO: overload for multiple algorithms

    pugi::xml_node model_settings() const;
    // TODO :overload for multiple models
};
}

#endif /* BEMELIB_EXPERIMENT_HPP */
