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
#include <string>
#include <utility>

#include <pagmo/algorithm.hpp>
#include <pagmo/population.hpp>

#include <pugixml.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "bevarmejo/io.hpp"
#include "bevarmejo/pagmo_helpers.hpp"

#include "experiment.hpp"

namespace bevarmejo {
// Default constructor on .hpp

namespace fsys = std::filesystem;
// Constructor from path to root folder (preferred)
Experiment::Experiment(std::string a_name,
                       fsys::path experiment_folder,
                       unsigned int seed,
                       fsys::path settings_filename) :
    m_name(a_name),
    _root_experiment_folder_(experiment_folder),
    _seed_(seed),
    _settings_filename_(settings_filename) {
    
    // check that the root folder exist
    if (!fsys::exists(_root_experiment_folder_) ||
        !fsys::is_directory(_root_experiment_folder_) ){
        throw std::runtime_error("\nExperiment folder "+_root_experiment_folder_.string()+" not located\nMake sure it exists.\n");
    }
    
    // check that the output folder exist, otherwise create it
    if (!fsys::exists(output_dir()))
        fsys::create_directory(output_dir());
    
    
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

void Experiment::build(pagmo::algorithm &algo, pagmo::population &pop) {
    // TEMP: the set and the inputs of this function will be removed once the 
    // islands are created in the build method. The pop and algo will be created 
    // there.
    m_archipelago.push_back(algo, pop);
    m_islands_filenames.push_back(runtime_file());
    // TODO: the name is based on the input files when multiple islands are used

    // Are we resuming or not? if not, clear or create the runtime file
    if (!m_resume) {
        std::ofstream ofs(runtime_file());
        if (!ofs.is_open())
            throw std::runtime_error("Could not create the runtime file " + runtime_file().string() + "\n");
        ofs.close();
    }
}

void Experiment::run(unsigned int n_generations) {

    // This is where the magic happens, for now I deal with only one island, but
    // in the future I will have to deal with multiple islands and the archipelago

    // Everything is already in the archipelago, so I just have to call evolve n times
    // and save the results between each call and at the beginning for the init
    // population.

    // This should be a loop over the islands or a simple archipelago.evolve(n_generations)
   {
        auto island = *m_archipelago.begin();
        auto island_filename = *m_islands_filenames.begin();
    
        // 1. Save the initial population
        save_runtime_result( island, island_filename );

        // 2. Evolve n times
        for (unsigned int i = 0; i < n_generations; ++i) {
            // 2.1. Evolve
            island.evolve(1);
            // 2.2. Save the population
            // TODO: deal when the population has not been saved correctly
            island.wait();
            save_runtime_result( island, island_filename );
        }
    }
}

void Experiment::save_outcome()
{

    // 1. Create the main file which then references the other files.
    std::ofstream ofs(main_filename());
    if (!ofs.is_open())
        return; // TODO: critical error! I will loose the data on the archipelago

    json jsys;
    // example machine, OS etc ... 

    json jarchipelago; 
    {
        auto jtopology = reporting::static_part_to_json(m_archipelago.get_topology());
        if ( !jtopology.empty() ) jarchipelago.update(jtopology);
    }

    // 2. Load the runtime data of each island (final population already in) and
    //    add the static part of the island (i.e., common parameters between the
    //    generations) and save the file.
    auto [saved_islands, errors] = save_final_results();

    jarchipelago[label::__islands] = json::array();
    for (auto& s_island : saved_islands){
        jarchipelago[label::__islands].push_back(s_island);
    }
    if (!errors.empty())
        jarchipelago[label::__errors] = errors;

    // 3. Save the file
    json jout = {
        {label::__system, jsys},
        {label::__archi, jarchipelago}
    };
    ofs << jout.dump(4);
    ofs.close();
}

fsys::path Experiment::main_filename() const {
    fsys::path outtemp_filename = output_dir()/m_name;
    outtemp_filename += label::__beme_suffix;
    return outtemp_filename;
}

std::pair<std::vector<std::string>, std::string> Experiment::save_final_results() const {
    
    // possibilities to use multiple threads as the files are independent (?)

    // 0. Allocate the space for the vector of filenames strings and for the errors

    // Vector of strings to store the filenames of the saved islands
    std::vector<std::string> saved_islands;
    saved_islands.reserve(m_archipelago.size());

    // To store the errors
    std::ostringstream oss;

    // 1. For each island, save. If it fails log the error
    assert(m_archipelago.size() == m_islands_filenames.size());
    auto isl_it = m_archipelago.begin();
    auto isl_fn_it = m_islands_filenames.begin();
    for (; isl_it != m_archipelago.end(); ++isl_it, ++isl_fn_it){

        try {
            saved_islands.push_back(
                save_final_result(*isl_it, *isl_fn_it).string()
            );
        } catch (std::runtime_error& e) {
            stream_out(oss, e.what());
        }
    }

    return std::make_pair(saved_islands, oss.str());
}

fsys::path Experiment::save_final_result(const pagmo::island& isl, const fsys::path& filename) const {
    // 2. Load the runtime data of each island (final population already in) and
    //    add the static part of the island (i.e., common parameters between the
    //    generations) and save the file.
    
    // The final population has already been added to the file the last time 
    // the evolve method was called. Here we only add the static part of the 
    // island and save the file.

    // 2.1. Load the runtime data of this island
    std::ifstream ifs(filename);
    json jdyn;
    if (!ifs.is_open()) {
        // Critical error, I could not find the data of the island, no point on
        // continuing
        throw std::runtime_error("Could not find the runtime data of the island, i.e. file "+filename.string()+"\n");
    }
    ifs >> jdyn;
    ifs.close();

    // Loading was successfully, the dynamic part of the results is now in jdyn

    // 2.2. Add the static part of the island
    json jstat;
    // 2.2.1. The User Defined Island infos 
    {   // reporting::static_part_to_json calls the correct transformation to 
        // json for the static part of the object (here the island). The same 
        // exist for the dynamic part, but it may be deleted for some type of
        // objects, e.g. the island. 
        // Internally, static_part_to_json calls the correct method based on the 
        // UD class hold by the pagmo container. It uses is() and extract().
        // TODO: when the container is defaulted return empty json, for now print everything
        auto jisland = reporting::static_part_to_json(isl);
        if ( !jisland.empty() ) jstat.update(jisland);
    }


    // 2.2.2. The User Defined Algorithm infos
    {   // see pattern above 2.2.1.
        auto jalgo = reporting::static_part_to_json(isl.get_algorithm());
        if ( !jalgo.empty() ) jstat.update(jalgo);
    }

    // 2.2.3. The User Defined Problem infos
    // see pattern above 2.2.1.
    jstat[label::__problem] = {{ label::__name, isl.get_population().get_problem().get_name() }};
    if ( !isl.get_population().get_problem().get_extra_info().empty() )
        jstat[label::__problem][label::__extra_info] = isl.get_population().get_problem().get_extra_info();
    
    // 2.2.4. The User Defined Replacement Policy infos
    {   // see pattern above 2.2.1.
        auto jrpolicy = reporting::static_part_to_json(isl.get_r_policy());
        if ( !jrpolicy.empty() ) jstat.update(jrpolicy);
    }    

    // 2.2.5. The User Defined Selection Policy infos
    {   // see pattern above 2.2.1.
        auto jspolicy = reporting::static_part_to_json(isl.get_s_policy());
        if ( !jspolicy.empty() ) jstat.update(jspolicy);
    }
    
    // 2.3. Save the file
    json& jout = jstat;
    jout[label::__generations] = jdyn[label::__generations];

    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
        // Critical error, I could not save the data of the island, no point on
        // continuing
        throw std::runtime_error("Could not save the final results of the island, i.e. file " + filename.string() + "\n");
    }
    ofs << jout.dump(4);
    ofs.close();

    // Everything went well, I can return the filename assuming it was saved.
    return filename;
}

bool Experiment::save_runtime_result(const pagmo::island &isl, const fsys::path &filename) const {
    
    // Assumption file exist already and it is well formatted
   
    // 0. Get the current time first to not be influenced by the time it takes to load/save the file
    auto currtime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::put_time(std::localtime(&currtime), "%c"); // Convert to string without the final newline
    std::string currtime_str = ss.str();

    // 1. Load the file to see what was there before
    std::ifstream ifs(filename);
    json j;
    if (ifs.is_open()  ) {
        if (!ifs.eof() && json::accept(ifs)) {
            // I most likely consumed the stream already with accept so I have 
            // to go back to the beginning
            ifs.seekg(0, std::ios::beg);
            ifs >> j;
        }
        ifs.close();
    }

    json jpop;
    pagmo::population pop = isl.get_population();

    // 2. Add the info of each population
    // 2.1 Mandatory info: time, fitness evaulations 
    jpop = {
        {label::__fevals, pop.get_problem().get_fevals()},
        {label::__currtime, currtime_str}
    };
    
    // 2.2 Mandatory info, the population's individuals
    auto population_ids = pop.get_ID();
    auto pop_dvs        = pop.get_x();
    auto pop_fitnesses  = pop.get_f();
    for (auto individual = 0u; individual<pop.size(); ++individual){
        jpop[label::__individuals].push_back({
            {label::__id, population_ids[individual]},
            {label::__dv, pop_dvs[individual]},
            {label::__fv, pop_fitnesses[individual]}
        });
    }

    // 2.3 Optional info: Gradient evals, Hessian evals, dynamic info of the Algotithm, Problem, UDRP, UDSP
    // TODO: for now it is empty, but I will add it in the future, e.g. see below 
    if (pop.get_problem().get_gevals() > 0)
        jpop[label::__gevals] = pop.get_problem().get_gevals();
    if (pop.get_problem().get_hevals() > 0)
        jpop[label::__hevals] = pop.get_problem().get_hevals();

    j[label::__generations].push_back(jpop);

    // Save the file
    std::ofstream ofs(filename);
    if (!ofs.is_open())
        return false;
    ofs << j.dump(4);
    ofs.close();

    return true;
}



/* Setters and getters */
const std::string& Experiment::get_name() const {
    return m_name;
}

fsys::path Experiment::output_dir() const {
    return _root_experiment_folder_/"output";
}

fsys::path Experiment::input_dir() const {
    return _root_experiment_folder_/"input";
}

fsys::path Experiment::settings_file(){
    return input_dir()/_settings_filename_;
}

fsys::path Experiment::runtime_file() {
    fsys::path outtemp_filename = output_dir()/m_name;
    outtemp_filename += "__";
    outtemp_filename += std::to_string(_seed_);
    outtemp_filename += ".json";
    return outtemp_filename;
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
