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
namespace fsys = std::filesystem;
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <pagmo/algorithm.hpp>
#include <pagmo/population.hpp>

#include <nlohmann/json.hpp>
using json_o = nlohmann::json;

#include "bevarmejo/bemexcept.hpp"
#include "bevarmejo/io/labels.hpp"
#include "bevarmejo/io/streams.hpp"
#include "bevarmejo/io/keys/beme.hpp"
#include "bevarmejo/io/keys/bemeexp.hpp"
#include "bevarmejo/io/keys/bemeopt.hpp"

#include "bevarmejo/factories.hpp"
#include "bevarmejo/library_metadata.hpp"

#include "bevarmejo/pagmo_helpers/containers_serializers.hpp"
#include "bevarmejo/pagmo_helpers/containers_help.hpp"
#include "bevarmejo/pagmo_helpers/algorithms/nsga2_help.hpp"

#include "Anytown/prob_anytown.hpp"
#include "Hanoi/problem_hanoi_biobj.hpp"

#include "experiment.hpp"

namespace bevarmejo {
    
namespace fsys = std::filesystem;

void Experiment::build(const ExperimentSettings &settings) {
    m_name = settings.name;
    m_folder = settings.folder;

    const json_o &jconfig = io::json::extract(io::key::typconfig).from(settings.jinput);

    assert(io::key::algorithm.exists_in(jconfig));
    const json_o &jalgo = io::json::extract(io::key::algorithm).from(jconfig);

    const json_o &jpop = io::json::extract(io::key::population).from(jconfig);
    
    std::string sname = io::json::extract(io::key::name).from(jalgo).get<std::string>();
    assert(sname == "nsga2");

    json_o jparams{};
    if( !io::key::params.exists_in(jalgo) )
        jparams[io::key::repgen[0]] = io::json::extract(io::key::repgen).from(jpop);
    
    pagmo::algorithm algo{ bevarmejo::Nsga2(jparams) };

    // Construct a pagmo::problem
    const json_o &jprob = io::json::extract(io::key::problem).from(jconfig);
    sname = io::json::extract(io::key::name).from(jprob).get<std::string>();
    jparams = json_o{};
    if (io::key::params.exists_in(jprob))
        jparams = io::json::extract(io::key::params).from(jprob);
    
    pagmo::problem p{ bevarmejo::build_problem(sname, jparams , settings.lookup_paths) };
        
    // and instantiate population
    pagmo::population pop{ std::move(p), io::json::extract(io::key::size).from(jpop).get<unsigned int>() };

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

    json_o jsys;
    // example machine, OS etc ... 
    json_o jsoft;
    jsoft[io::key::beme_version()] = VersionManager::library().version().str();

    json_o jarchipelago; 
    {
        auto jtopology = io::json::static_descr(m_archipelago.get_topology());
        if ( !jtopology.empty() ) jarchipelago.update(jtopology);
    }

    // 2. Load the runtime data of each island (final population already in) and
    //    add the static part of the island (i.e., common parameters between the
    //    generations) and save the file.
    auto [saved_islands, errors] = save_final_results();

    jarchipelago[io::key::islands()] = json_o::array();
    for (auto& s_island : saved_islands){
        jarchipelago[io::key::islands()].push_back(s_island);
    }
    if (!errors.empty())
        jarchipelago[io::key::errors()] = errors;

    // 3. Save the file
    json_o jout = {
        {io::key::system(), jsys},
        {io::key::archi(), jarchipelago},
        {io::key::software(), jsoft}
    };
    ofs << jout.dump(4);
    ofs.close();
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
    while (isl_it != m_archipelago.end() && isl_fn_it != m_islands_filenames.end())
    {
        try
        {
            save_final_result(*isl_it, *isl_fn_it);
            
            saved_islands.push_back(fsys::relative(*isl_fn_it, output_folder()));
        }
        catch (std::runtime_error& e)
        {
            io::stream_out(oss, e.what());
        }

        ++isl_it;
        ++isl_fn_it;
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
    json_o jdyn;
    if (!ifs.is_open()) {
        // Critical error, I could not find the data of the island, no point on
        // continuing
        throw std::runtime_error("Could not find the runtime data of the island, i.e. file "+filename.string()+"\n");
    }
    ifs >> jdyn;
    ifs.close();

    // Loading was successfully, the dynamic part of the results is now in jdyn

    // 2.2. Add the static part of the island
    json_o jstat;
    // 2.2.1. The User Defined Island infos 
    {   // reporting::static_part_to_json calls the correct transformation to 
        // json for the static part of the object (here the island). The same 
        // exist for the dynamic part, but it may be deleted for some type of
        // objects, e.g. the island. 
        // Internally, static_part_to_json calls the correct method based on the 
        // UD class hold by the pagmo container. It uses is() and extract().
        // TODO: when the container is defaulted return empty json, for now print everything
        auto jisland = io::json::static_descr(isl);
        if ( !jisland.empty() ) jstat.update(jisland);
    }


    // 2.2.2. The User Defined Algorithm infos
    {   // see pattern above 2.2.1.
        auto jalgo = io::json::static_descr(isl.get_algorithm());
        if ( !jalgo.empty() ) jstat.update(jalgo);
    }

    // 2.2.3. The User Defined Problem infos
    {   // see pattern above 2.2.1.
        auto jprob = io::json::static_descr(isl.get_population().get_problem());
        if ( !jprob.empty() ) jstat.update(jprob);
    }
    
    
    // 2.2.4. The User Defined Replacement Policy infos
    {   // see pattern above 2.2.1.
        auto jrpolicy = io::json::static_descr(isl.get_r_policy());
        if ( !jrpolicy.empty() ) jstat.update(jrpolicy);
    }    

    // 2.2.5. The User Defined Selection Policy infos
    {   // see pattern above 2.2.1.
        auto jspolicy = io::json::static_descr(isl.get_s_policy());
        if ( !jspolicy.empty() ) jstat.update(jspolicy);
    }
    
    // 2.3. Save the file
    json_o& jout = jstat;
    jout[io::key::generations()] = jdyn[io::key::generations()];

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
    json_o j;
    if (ifs.is_open()  ) {
        if (!ifs.eof() && json_o::accept(ifs)) {
            // I most likely consumed the stream already with accept so I have 
            // to go back to the beginning
            ifs.seekg(0, std::ios::beg);
            ifs >> j;
        }
        ifs.close();
    }

    // 2. Add the info of each population
    pagmo::population pop = isl.get_population();
    // 2.1 Mandatory info: time, fitness evaulations 
    json_o jcgen = {
        {io::key::fevals(), pop.get_problem().get_fevals()},
        {io::key::ctime(), currtime_str},
        {io::key::individuals(), json_o::array()}
    };
    
    // 2.2 Mandatory info, the population's individuals
    json_o &jinds = jcgen[io::key::individuals()];
    auto population_ids = pop.get_ID();
    auto pop_dvs        = pop.get_x();
    auto pop_fitnesses  = pop.get_f();
    for (auto individual = 0u; individual<pop.size(); ++individual){
        jinds.push_back({
            {io::key::id(), population_ids[individual]},
            {io::key::dv(), pop_dvs[individual]},
            {io::key::fv(), pop_fitnesses[individual]}
        });
    }

    // 2.3 Optional info: Gradient evals, Hessian evals, dynamic info of the Algotithm, Problem, UDRP, UDSP
    // TODO: for now it is empty, but I will add it in the future, e.g. see below 
    if (pop.get_problem().get_gevals() > 0)
        jcgen[io::key::gevals()] = pop.get_problem().get_gevals();
    if (pop.get_problem().get_hevals() > 0)
        jcgen[io::key::hevals()] = pop.get_problem().get_hevals();

    {  // see pattern above 2.2.1. in save_final_result
        auto jalgo_dyn = io::json::dynamic_descr(isl.get_algorithm());
        if (!jalgo_dyn.empty()) jcgen.update(jalgo_dyn);
    }

    {   // see pattern above 2.2.1. in save_final_result
        auto jprob_dyn = io::json::dynamic_descr(pop.get_problem());
        if (!jprob_dyn.empty()) jcgen.update(jprob_dyn);
    }
    

    j[io::key::generations()].push_back(jcgen);

    // Save the file
    std::ofstream ofs(filename);
    if (!ofs.is_open())
        return false;
    ofs << j.dump(4);
    ofs.close();

    return true;
}


const std::string& Experiment::name() const {
    return m_name;
}

const fsys::path& Experiment::folder() const {
    return m_folder;
}

const fsys::path Experiment::output_folder() const {
    return m_folder/io::other::bemeexp_out_folder;
}

fsys::path Experiment::main_filename() const {
    std::string temp = (
        io::other::bemeexp_prefix+
        io::other::beme_filenames_separator+
        m_name+
        io::other::bemeexp_exp_suffix+".json"
    );
    
    return output_folder()/temp;
}

fsys::path Experiment::runtime_file() {
    std::string temp = (
        io::other::bemeexp_prefix+
        io::other::beme_filenames_separator+
        m_name+
        io::other::beme_filenames_separator+
        std::to_string(_seed_)+
        io::other::bemeexp_isl_suffix+".json"
    );
        
    return output_folder()/temp;
}

} // namespace bevarmejo
