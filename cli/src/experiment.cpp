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

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "bevarmejo/io.hpp"
#include "bevarmejo/labels.hpp"
#include "bevarmejo/pagmo_helpers/udc_help.hpp"
#include "bevarmejo/pagmo_helpers/containers_help.hpp"
#include "bevarmejo/pagmo_helpers/algorithms/nsga2_help.hpp"

#include "Anytown/prob_anytown.hpp"
#include "Anytown/rehab/prob_at_reh_f1.hpp"
#include "Anytown/mixed/prob_at_mix_f1.hpp"
#include "Anytown/operations/prob_at_ope_f1.hpp"

#include "Hanoi/problem_hanoi_biobj.hpp"

#include "experiment.hpp"

namespace bevarmejo {
    
namespace fsys = std::filesystem;

void Experiment::build(const ExperimentSettings &settings) {
    m_name = settings.name;
    m_folder = settings.folder;

    //TODO: compose based on the settings
    json jnsga2{ {label::__report_gen_sh, settings.jinput[label::__typconfig][label::__population][label::__report_gen_sh].get<unsigned int>() } };
    pagmo::algorithm algo{ bevarmejo::Nsga2(jnsga2) };

    // Construct a pagmo::problem for ANYTOWN model
    pagmo::problem p{};
    auto probname = settings.jinput[label::__typconfig][label::__problem_sh][label::__name].get<std::string>();
    auto pparams = settings.jinput[label::__typconfig][label::__problem_sh][label::__params];
    if ( probname == bevarmejo::anytown::rehab::f1::name) {
        p = bevarmejo::anytown::rehab::f1::Problem(pparams, settings.lookup_paths);
    }
    else if ( probname == bevarmejo::anytown::mixed::f1::name) {
        p = bevarmejo::anytown::mixed::f1::Problem(pparams, settings.lookup_paths);
    }
    else if (probname == bevarmejo::anytown::operations::f1::name) {
        p = bevarmejo::anytown::operations::f1::Problem(pparams, settings.lookup_paths);
    }
    else if ( probname == bevarmejo::hanoi::fbiobj::name) {
        p = bevarmejo::hanoi::fbiobj::Problem(pparams, settings.lookup_paths);
    }
    else {
        throw std::runtime_error("The problem name is not recognized.");
    }
        
    // and instantiate population
    pagmo::population pop{ std::move(p), settings.jinput[label::__typconfig][label::__population][label::__size].get<unsigned int>() };

    m_archipelago.push_back(algo, pop);
    _seed_ = pop.get_seed();
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
        auto jtopology = io::json::static_part_to_json(m_archipelago.get_topology());
        if ( !jtopology.empty() ) jarchipelago.update(jtopology);
    }

    // 2. Load the runtime data of each island (final population already in) and
    //    add the static part of the island (i.e., common parameters between the
    //    generations) and save the file.
    auto [saved_islands, errors] = save_final_results();

    jarchipelago[to_kebab_case(label::__islands)] = json::array();
    for (auto& s_island : saved_islands){
        jarchipelago[to_kebab_case(label::__islands)].push_back(s_island);
    }
    if (!errors.empty())
        jarchipelago[to_kebab_case(label::__errors)] = errors;

    // 3. Save the file
    json jout = {
        {to_kebab_case(label::__system), jsys},
        {to_kebab_case(label::__archi), jarchipelago}
    };
    ofs << jout.dump(4);
    ofs.close();
}

fsys::path Experiment::main_filename() const {
    std::string complete_filename = label::__beme_prefix+m_name+label::__beme_suffix;
    
    return output_folder()/complete_filename;
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
            io::stream_out(oss, e.what());
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
        auto jisland = io::json::static_part_to_json(isl);
        if ( !jisland.empty() ) jstat.update(jisland);
    }


    // 2.2.2. The User Defined Algorithm infos
    {   // see pattern above 2.2.1.
        auto jalgo = io::json::static_part_to_json(isl.get_algorithm());
        if ( !jalgo.empty() ) jstat.update(jalgo);
    }

    // 2.2.3. The User Defined Problem infos
    // see pattern above 2.2.1.
    jstat[to_kebab_case(label::__problem)] = {{ to_kebab_case(label::__name), isl.get_population().get_problem().get_name() }};
    if ( !isl.get_population().get_problem().get_extra_info().empty() )
        jstat[to_kebab_case(label::__problem)][to_kebab_case(label::__extra_info)] = isl.get_population().get_problem().get_extra_info();
    
    // 2.2.4. The User Defined Replacement Policy infos
    {   // see pattern above 2.2.1.
        auto jrpolicy = io::json::static_part_to_json(isl.get_r_policy());
        if ( !jrpolicy.empty() ) jstat.update(jrpolicy);
    }    

    // 2.2.5. The User Defined Selection Policy infos
    {   // see pattern above 2.2.1.
        auto jspolicy = io::json::static_part_to_json(isl.get_s_policy());
        if ( !jspolicy.empty() ) jstat.update(jspolicy);
    }
    
    // 2.3. Save the file
    json& jout = jstat;
    jout[to_kebab_case(label::__generations)] = jdyn[to_kebab_case(label::__generations)];

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
        {to_kebab_case(label::__fevals), pop.get_problem().get_fevals()},
        {to_kebab_case(label::__currtime), currtime_str}
    };
    
    // 2.2 Mandatory info, the population's individuals
    auto population_ids = pop.get_ID();
    auto pop_dvs        = pop.get_x();
    auto pop_fitnesses  = pop.get_f();
    for (auto individual = 0u; individual<pop.size(); ++individual){
        jpop[to_kebab_case(label::__individuals)].push_back({
            {to_kebab_case(label::__id), population_ids[individual]},
            {to_kebab_case(label::__dv), pop_dvs[individual]},
            {to_kebab_case(label::__fv), pop_fitnesses[individual]}
        });
    }

    // 2.3 Optional info: Gradient evals, Hessian evals, dynamic info of the Algotithm, Problem, UDRP, UDSP
    // TODO: for now it is empty, but I will add it in the future, e.g. see below 
    if (pop.get_problem().get_gevals() > 0)
        jpop[to_kebab_case(label::__gevals)] = pop.get_problem().get_gevals();
    if (pop.get_problem().get_hevals() > 0)
        jpop[to_kebab_case(label::__hevals)] = pop.get_problem().get_hevals();

    j[to_kebab_case(label::__generations)].push_back(jpop);

    // Save the file
    std::ofstream ofs(filename);
    if (!ofs.is_open())
        return false;
    ofs << j.dump(4);
    ofs.close();

    return true;
}


fsys::path Experiment::runtime_file() {
    std::string outtemp_filename = label::__beme_prefix+m_name+"__"+std::to_string(_seed_)+".json";

    return output_folder()/outtemp_filename;
}

} // namespace bevarmejo
