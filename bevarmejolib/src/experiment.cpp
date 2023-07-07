//
//  experiment.cpp
//  bemelib_classes
//
//  Created by Dennis Zanutto on 06/07/23.
//

#include <chrono>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "pagmo/algorithm.hpp"
#include "pagmo/population.hpp"

#include "io.hpp"

#include "experiment.hpp"

namespace bevarmejo {
// Default constructor on .hpp

namespace fsys = std::filesystem;
// Constructor from path to root folder (preferred)
Experiment::Experiment(fsys::path experiment_folder,
                       fsys::path settings_filename) :
    _root_experiment_folder_(experiment_folder),
    _settings_filename_(settings_filename) {
        
    // save initial time of the experiment
    const auto now = std::chrono::system_clock::now();
    _start_time_ = std::chrono::system_clock::to_time_t(now);
    
    // check that the root folder exist
    if (!fsys::exists(_root_experiment_folder_) ||
        !fsys::is_directory(_root_experiment_folder_) ){
        throw std::runtime_error("\nExperiment folder not located\nMake sure it exists.\n");
    }
    
    // check that every folder inside exist as it should, otherwise fix it
    if (!is_compliant()){
        std::cout <<"Root experiment folder has not the required structure.\nTaking care of it.";
        make_compliant();
    }
    
    // look for the settings file
    if (!fsys::exists(settings_file()) || !fsys::is_regular_file(settings_file())){
        std::string error_message{"\nSettings file ("};
        error_message.append(settings_file().string());
        error_message.append(") not found or not a file\n");
        throw std::runtime_error(error_message);
    }
    
}
/* Methods */
//Check complaiance of root folder
bool Experiment::is_compliant(){
    // Check if the required subdirectories exist
    if (!fsys::exists(input_dir()) || !fsys::is_directory(input_dir()) || !fsys::exists(output_dir()) || !fsys::is_directory(output_dir()) || !fsys::exists(runtime_dir()) || !fsys::is_directory(runtime_dir())) {
      return false;
    }
    return true;
}

// Make root folder complaiant
void Experiment::make_compliant(){
    fsys::create_directory(input_dir());
    fsys::create_directory(output_dir());
    fsys::create_directory(runtime_dir());
}

void Experiment::finished(){
    const auto now = std::chrono::system_clock::now();
    _end_time_ = std::chrono::system_clock::to_time_t(now);
}

void Experiment::save_final_result(pagmo::population &pop, pagmo::algorithm &algo){
    
    //TODO create your own name
    fsys::path output_filename = output_dir()/"test.out";
    
    // open and create the file
    std::ofstream ofs(output_filename);
    if (!ofs.is_open())
        return; // how can I redirect this to cout?
    
    std::cout <<"\nWriting results on:\n\t" <<output_filename.string() <<std::endl;
    
    // 0. write the header (system and experiment info)
    stream_param(ofs, "Experiment:", get_name());
    stream_param(ofs, get_extra_info(), "");
    
    // 1. Algorithm
    stream_param(ofs, "Algorithm: ", algo.get_name());
    stream_param(ofs, "", algo.get_extra_info());
    ofs <<std::endl;
    
    // 2. Model Problem
    stream_param(ofs, "Problem name: ", pop.get_problem().get_name());
    stream_param(ofs, "", pop.get_problem().get_extra_info());
    
    ofs <<std::endl;
    
    // 3. Population
    stream_param(ofs, "Population:", "");
    stream_param(ofs, "\tFitness evaluations: ", pop.get_problem().get_fevals());
    auto last_individual = pop.size();
    stream_param(ofs, "\tPopulation size:\t ", last_individual);
    stream_param(ofs, "\tSeed:\t\t\t\t ", pop.get_seed());
    
    auto population_ids = pop.get_ID();
    auto pop_dvs        = pop.get_x();
    auto pop_fitnesses  = pop.get_f();
    
    //stream_param(ofs, "List of individuals: ", "");
    for (auto individual = 0u; individual<last_individual; ++individual){
        stream_param(ofs, "#", individual);
        stream_param(ofs, "\tID: ", population_ids[individual]);
        stream_param(ofs, "\tDecision vector: ", pop_dvs[individual]);
        stream_param(ofs, "\tFitness vector: ", pop_fitnesses[individual]);
    }
}

/* Setters and getters */
std::string Experiment::get_name(){
    std::ostringstream oss;
    oss << _name_;
    return oss.str();
}

std::string Experiment::get_extra_info(){
    // TODO extend
    std::ostringstream oss;
    oss << _user_custom_info_ <<'\n';
    stream_param(oss, "\tOptimisation begun: ", ctime(&_start_time_));
    stream_param(oss, "\tOptimisation ended: ", ctime(&_end_time_  ));
    return oss.str();
}

fsys::path Experiment::input_dir(){
    return _root_experiment_folder_/"input";
}

fsys::path Experiment::output_dir(){
    return _root_experiment_folder_/"output";
}

fsys::path Experiment::runtime_dir(){
    return _root_experiment_folder_/"runtime";
}

fsys::path Experiment::settings_file(){
    return input_dir()/_settings_filename_;
}
}
