//
//  experiment.cpp
//  bemelib_classes
//
//  Created by Dennis Zanutto on 06/07/23.
//

#include <chrono>
#include <iomanip>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "pagmo/algorithm.hpp"
#include "pagmo/population.hpp"

#include "pugixml.hpp"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "bevarmejo/io.hpp"

#include "experiment.hpp"

namespace bevarmejo {
// Default constructor on .hpp

namespace fsys = std::filesystem;
// Constructor from path to root folder (preferred)
Experiment::Experiment(fsys::path experiment_folder,
                       unsigned int seed,
                       fsys::path settings_filename) :
    _root_experiment_folder_(experiment_folder),
    _seed_(seed),
    _settings_filename_(settings_filename) {
        
    // save initial time of the experiment
    const auto now = std::chrono::system_clock::now();
    _start_time_ = std::chrono::system_clock::to_time_t(now);
    
    // check that the root folder exist
    if (!fsys::exists(_root_experiment_folder_) ||
        !fsys::is_directory(_root_experiment_folder_) ){
        throw std::runtime_error("\nExperiment folder "+_root_experiment_folder_.string()+" not located\nMake sure it exists.\n");
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
    
    // upload settings file 
    pugi::xml_parse_result result = _settings_.load_file(settings_file().c_str());

    if (result.status != pugi::status_ok) {
        throw std::runtime_error(result.description());
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

void Experiment::save_runtime_result(pagmo::population &pop) {
    // If it does not exist, create the file. 
    // If it exist, load it, add the new info and then save it again.
   
    auto currtime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::put_time(std::localtime(&currtime), "%c"); // Convert to string without the final newline
    std::string currtime_str = ss.str();

    std::ifstream ifs(runtime_file());
    json j;
    if (ifs.is_open()) {
        ifs >> j;
        ifs.close();
    }

    json jpop;

    // Add the info of each population
    jpop = {
        {"Evaluations", pop.get_problem().get_fevals()},
        {"Current time", currtime_str},
        {"Population size", pop.size()},
        {"Seed", pop.get_seed()}
    };
    
    // Add the info of each individual
    auto population_ids = pop.get_ID();
    auto pop_dvs        = pop.get_x();
    auto pop_fitnesses  = pop.get_f();
    for (auto individual = 0u; individual<pop.size(); ++individual){
        jpop["Individuals"].push_back({
            {"ID", population_ids[individual]},
            {"Decision vector", pop_dvs[individual]},
            {"Fitness vector", pop_fitnesses[individual]}
        });
    }

    j["Generations"].push_back(jpop);

    // Save the file
    std::ofstream ofs(runtime_file());
    ofs << j.dump(4);
}

void Experiment::save_final_result(pagmo::population &pop, pagmo::algorithm &algo)
{

    // open and create the file
    std::ofstream ofs(output_file());
    if (!ofs.is_open())
        return; // how can I redirect this to cout?
    
    std::cout <<"\nWriting results on:\n\t" << output_file().string() << std::endl;
    
    // 0. write the header (system and experiment info)
    stream_param(ofs, "Experiment", get_name());
    stream_out(ofs, get_extra_info());
    
    // 1. Algorithm
    stream_param(ofs, "Algorithm", algo.get_name());
    stream_out(ofs, algo.get_extra_info(), '\n', '\n');
    
    // 2. Model Problem
    stream_param(ofs, "Problem name", pop.get_problem().get_name());
    stream_out(ofs, pop.get_problem().get_extra_info(), '\n', '\n');
    
    // 3. Population
    stream_out(ofs, "Population\n");
    stream_param(ofs, "\tFitness evaluations", pop.get_problem().get_fevals());
    auto last_individual = pop.size();
    stream_param(ofs, "\tPopulation size", last_individual);
    stream_param(ofs, "\tSeed", pop.get_seed());
    
    auto population_ids = pop.get_ID();
    auto pop_dvs        = pop.get_x();
    auto pop_fitnesses  = pop.get_f();
    
    //stream_param(ofs, "List of individuals: ", "");
    for (auto individual = 0u; individual<last_individual; ++individual){
        stream_out(ofs, "#", individual, '\n');
        stream_param(ofs, "\tID", population_ids[individual]);
        stream_param(ofs, "\tDecision vector", pop_dvs[individual]);
        stream_param(ofs, "\tFitness vector", pop_fitnesses[individual]);
    }
}

void Experiment::set_name(std::string name)
{
	_name_ = name;
}

/* Setters and getters */
std::string Experiment::get_name(){
    return _name_;
}

std::string Experiment::get_extra_info(){
    // TODO extend
    std::ostringstream oss;
    stream_out(oss, _user_custom_info_, '\n');
    stream_param(oss, "\tOptimisation begun", ctime(&_start_time_));
    stream_param(oss, "\tOptimisation ended", ctime(&_end_time_  ));
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

fsys::path Experiment::runtime_file() {
    fsys::path outtemp_filename = output_dir()/_name_;
    outtemp_filename += "_";
    outtemp_filename += std::to_string(_seed_);
    outtemp_filename += ".json";
    return outtemp_filename;
}

fsys::path Experiment::output_file()
{
    fsys::path output_filename = output_dir()/_name_;
    output_filename += "_";
    output_filename += std::to_string(_seed_);
    output_filename += ".out";
    return output_filename;
}
pugi::xml_node Experiment::algorithm_settings() const
{
    pugi::xml_node algorithm_settings_node = _settings_.child("optProblem").child("optAlgorithm").first_child();
    if (algorithm_settings_node.empty())
		throw std::runtime_error("\nNo algorithm settings found in the settings file\n");
    return algorithm_settings_node;
}

pugi::xml_node Experiment::model_settings() const
{
    pugi::xml_node model_settings_node = _settings_.child("optProblem").child("systemModel").first_child();
    if (model_settings_node.empty())
        throw std::runtime_error("\nNo model settings found in the settings file\n");
	return model_settings_node;
}

} // namespace bevarmejo
