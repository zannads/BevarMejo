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

namespace io {
namespace log {
namespace nname {
static const std::string opt = "optimisation::"; // "optimisation::"
}
namespace cname {
static const std::string Experiment = "Experiment"; // "Experiment"
}
namespace fname {
static const std::string parse = "build"; // "parse"
}
namespace mex {
static const std::string parse_error = "Error parsing the settings file."; // "Error parsing the settings file."

static const std::string unsupported_format = "The file format of the settings file is not supported."; // "The file format of the settings file is not supported."
}
} // namespace log

namespace other {
static const std::string settings_file = "Settings file : "; // "Settings file : "
}
} // namespace io
    
Experiment::Experiment(const fsys::path &settings_file) : 
    m__settings_file(settings_file),
    m__root_folder(settings_file.parent_path()),
    m__lookup_paths({settings_file.parent_path()})
{
    // Check the extension, and based on that open the file, parse it based on
    // the file structure (JSON, YAML, XML, etc). 
    // Apply the key value pairs passed from command line
    // Once you have the final object call the build function.

    std::ifstream file(settings_file);
    if (!file.is_open())
        __format_and_throw<std::runtime_error, bevarmejo::ClassError>(io::log::nname::opt+io::log::cname::Experiment, io::log::cname::Experiment,
            io::log::mex::parse_error,
            "Failed to open settings file.",
            io::other::settings_file+settings_file.string());

    if (file.peek() == std::ifstream::traits_type::eof()) {
        file.close();

        __format_and_throw<std::runtime_error, bevarmejo::ClassError>(io::log::nname::opt+io::log::cname::Experiment, io::log::cname::Experiment,
            io::log::mex::parse_error,
            "Settings file is empty.",
            io::other::settings_file+settings_file.string());
    }

    std::string file_contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // For now, I only implement the JSON file format.
    json_o jinput;
    if (settings_file.extension() == ".json") 
    {   
        try
        {
            jinput = json_o::parse(file_contents);
        }
        catch (const std::exception& e)
        {
            __format_and_throw<std::runtime_error, bevarmejo::ClassError>(io::log::nname::opt+io::log::cname::Experiment, io::log::cname::Experiment,
                io::log::mex::parse_error,
                "Failed to parse settings file as JSON.",
                io::other::settings_file+settings_file.string()+"\n"+e.what());
        }
    }
    else
    {
        __format_and_throw<std::runtime_error, bevarmejo::ClassError>(io::log::nname::opt+io::log::cname::Experiment, io::log::cname::Experiment,
            io::log::mex::unsupported_format,
            "The file format of the settings file is not supported.",
            io::other::settings_file+settings_file.string());
    }
    // TODO: if format is different, convert to JSON

    // TODO: Apply the key value pairs passed from command line

    // Once you have the final object call the build function.
    build(jinput);
}

void Experiment::build(const json_o &jinput)
{
    // Upload the settings or the fields that can change the behavior of the construction of the experiment (e..g, lookup paths)
    if (io::key::settings.exists_in(jinput)) {
        const json_o &jsettings = io::json::extract(io::key::settings).from(jinput);

        if (io::key::out_file_format.exists_in(jsettings))
        {
            // TODO: set the output file format
        }

        if (io::key::out_key_style.exists_in(jsettings))
        {
            try
            {
                io::key::Key::set_out_style(io::json::extract(io::key::out_key_style).from(jsettings).get<std::string>());
            }
            catch (const std::exception &e)
            {
               // TODO: log the error
            }
        }
    }

    if (io::key::lookup_paths.exists_in(jinput))
    {
        const json_o &jpaths = io::json::extract(io::key::lookup_paths).from(jinput);

        auto add_if_valid_path = [](const fsys::path &p, std::vector<fsys::path> &lookup_paths) {
            if (fsys::exists(p) && fsys::is_directory(p))
                lookup_paths.push_back(p);
            else
            {
                // TODO: log the error
            }
                
        };

        if (jpaths.is_string())
        {
            add_if_valid_path(jpaths.get<fsys::path>(), m__lookup_paths);
        }
        else if (jpaths.is_array()) 
        {
            for (const auto& path : jpaths) 
                add_if_valid_path(path.get<fsys::path>(), m__lookup_paths);
        }
        else
        {
            //TODO: log the error 
        }
    }
    // Final lookup path is always the current directory.
    m__lookup_paths.push_back(fsys::current_path());

    // Check that the settings file has the required fields
    auto check_mandatory_field = [](const io::key::Key &key, const json_o &j) {
        if (key.exists_in(j)) {
            return;
        }

        __format_and_throw<std::runtime_error, bevarmejo::FunctionError>(io::log::nname::opt+io::log::fname::parse,
            io::log::mex::parse_error,
            "Settings file does not contain a mandatory field.",
            "Missing field : "+key[0]
        );
    };

    // as of now name is a mandatory field
    check_mandatory_field(io::key::name, jinput);
    m__name = io::json::extract(io::key::name).from(jinput).get<std::string>();

    // as of now typconfig is a mandatory field
    check_mandatory_field(io::key::typconfig, jinput);
    const auto& typconfig = io::json::extract(io::key::typconfig).from(jinput);

    // I should construct multiple islands combining typconfig and specializations
    // I will build an island using an algorithm and a population.
    
    // Construct a pagmo::population
    // Population, its size and the generations are mandatory. 
    // Seed, report gen are optional. 
    check_mandatory_field(io::key::population, typconfig);
    const auto& jpop = io::json::extract(io::key::population).from(typconfig);

    check_mandatory_field(io::key::size, jpop);
    check_mandatory_field(io::key::generations, jpop);
    const auto& genz = io::json::extract(io::key::generations).from(jpop);

    // the algorithms need to know how many generations to report because the island calls the evolve method n times
    // until n*__report_gen > __generations, default = __generations
    //  m__settings.n_evolves = 1 unless the user specifies it
    if (io::key::repgen.exists_in(jpop)) {
        const auto &repgenz = io::json::extract(io::key::repgen).from(jpop);

        m__settings.n_evolves = ceil(genz.get<double>()/repgenz.get<double>()); // get double instead of unsigned int to force non integer division
    }

    // Constuct the pagmo::algorithm
    check_mandatory_field(io::key::algorithm, typconfig);
    const json_o &jalgo = io::json::extract(io::key::algorithm).from(typconfig);

    std::string sname = io::json::extract(io::key::name).from(jalgo).get<std::string>();

    json_o jparams{};
    if( io::key::params.exists_in(jalgo) )
        jparams = io::json::extract(io::key::params).from(jalgo);
    // add the report gen to the algorithm params
    if (io::key::repgen.exists_in(jpop))
        jparams[io::key::repgen[0]] = io::json::extract(io::key::repgen).from(jpop);
    else 
        jparams[io::key::repgen[0]] = genz;
    
    assert(sname == "nsga2");
    pagmo::algorithm algo{ bevarmejo::Nsga2(jparams) };

    // Construct a pagmo::problem
    const json_o &jprob = io::json::extract(io::key::problem).from(typconfig);

    sname = io::json::extract(io::key::name).from(jprob).get<std::string>();

    jparams = json_o{};
    if (io::key::params.exists_in(jprob))
        jparams = io::json::extract(io::key::params).from(jprob);
    
    pagmo::problem p{ bevarmejo::build_problem(sname, jparams , m__lookup_paths) };
        
    // Now that I have everything I can build the population and then the island
    pagmo::population pop{ std::move(p), io::json::extract(io::key::size).from(jpop).get<unsigned int>() };

    m__archipelago.push_back(algo, pop); // Create the island

    // Track the info of the island.
    // This should build the name based on the name in typconfig and the parameters of the island.
    m__islands_names.push_back(std::to_string(pop.get_seed()));

    if (!fsys::exists(output_folder()))
        fsys::create_directory(output_folder());

    prepare_isl_files();

    prepare_exp_file();

}


void Experiment::run() {

    // This is where the magic happens, for now I deal with only one island, but
    // in the future I will have to deal with multiple islands and the archipelago

    // Everything is already in the archipelago, so I just have to call evolve n times
    // and save the results between each call and at the beginning for the init
    // population.

    // This should be a loop over the islands or a simple archipelago.evolve(n_generations)
    for (auto n = 0; n < m__settings.n_evolves; ++n) {
        for (auto island : m__archipelago) {
            
            island.evolve(1);
            
            island.wait();
            append_isl_runtime_data(0);
            // TODO: deal when the population has not been saved correctly
        }
    }

    // First, make sure that all the islands data have been moved from the runtime
    // files to the final files.
    finalise_isl_files();

    // Then, finalise the experiment file and delete the runtime files.
    finalise_exp_file();
}

fsys::path Experiment::output_folder() const {
    return m__root_folder/io::other::bemeexp_out_folder;
}

fsys::path Experiment::exp_filename() const {
    std::string temp = (
        io::other::bemeexp_prefix+
        io::other::beme_filenames_separator+
        m__name+
        io::other::bemeexp_exp_suffix+".json"
    );
    
    return output_folder()/temp;
}

fsys::path Experiment::isl_filename(std::size_t island_idx, bool runtime) const {
    std::string suffix = runtime ? io::other::bemeexp_rnt_suffix : io::other::bemeexp_isl_suffix;
    std::string temp = (
        io::other::bemeexp_prefix+
        io::other::beme_filenames_separator+
        m__name+
        io::other::beme_filenames_separator+
        m__islands_names.at(island_idx)+
        suffix+".json"
    );
        
    return output_folder()/temp;
}

void Experiment::prepare_isl_files() const 
{
    for (std::size_t i = 0; i < m__archipelago.size(); ++i)
    {
        auto isl = *(m__archipelago.begin() + i);

        // Add the static information about the island. The dynamic ones will be appended.
        json_o jstat;
        // reporting::static_part_to_json calls the correct transformation to 
        // json for the static part of the object (here the island). The same 
        // exist for the dynamic part, but it may be deleted for some type of
        // objects, e.g. the island. 
        // Internally, static_part_to_json calls the correct method based on the 
        // UD class hold by the pagmo container. It uses is() and extract().
        auto append_static_info = [](json_o &jstat, auto pagmo_container) {
            auto jinfo = io::json::static_descr(pagmo_container);
            if ( !jinfo.empty() ) jstat.update(jinfo);
        }; 

        append_static_info(jstat, isl);
        append_static_info(jstat, isl.get_algorithm());
        append_static_info(jstat, isl.get_population().get_problem());
        append_static_info(jstat, isl.get_r_policy());
        append_static_info(jstat, isl.get_s_policy());

        // Add the intial population and the initial dynamic parameters of the objects. 
        json_o& jout = jstat;
        jout[io::key::generations()] = json_o::array();

        json_o jcurr_isl_status;
        freeze_isl_runtime_data(i, jcurr_isl_status);

        jout[io::key::generations()].push_back(jcurr_isl_status);

        std::ofstream ofs(isl_filename(i, /*runtime=*/ true), std::ios::out);
        if (!ofs.is_open())
            __format_and_throw<std::runtime_error, bevarmejo::ClassError>(io::log::nname::opt+io::log::cname::Experiment, "prepare_isl_files",
                "Could not create the runtime file for the island.",
                "File : "+isl_filename(i, /*runtime=*/ true).string());
            
        ofs << jout.dump() << std::endl; // No value in dump so that it is a single line (JSONL)
        ofs.close();
    }
}

void Experiment::prepare_exp_file() const {

    std::ofstream ofs(exp_filename());
    if (!ofs.is_open())
        __format_and_throw<std::runtime_error, bevarmejo::ClassError>(io::log::nname::opt+io::log::cname::Experiment, "prepare_exp_file",
            "Could not create the experiment file.",
            "File : "+exp_filename().string());
    
    // Add the information about the experiment.
    
    json_o jsys;
    // example machine, OS etc ... 
    json_o jsoft;
    jsoft[io::key::beme_version()] = VersionManager::library().version().str();

    json_o jarchipelago; 
    {
        auto jtopology = io::json::static_descr(m__archipelago.get_topology());
        if ( !jtopology.empty() ) jarchipelago.update(jtopology);
    }

    // Save the relative name of the islands.
    jarchipelago[io::key::islands()] = json_o::array();
    for (auto i = 0; i < m__archipelago.size(); ++i)
        jarchipelago[io::key::islands()].push_back(isl_filename(i, /*runtime=*/ true).filename().string());

    // 3. Save the file
    json_o jout = {
        {io::key::system(), jsys},
        {io::key::archi(), jarchipelago},
        {io::key::software(), jsoft}
    };
    ofs << jout.dump(4) << std::endl;
    ofs.close();
}

void Experiment::freeze_isl_runtime_data(std::size_t island_idx, json_o &jout) const
{
    auto currtime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::put_time(std::localtime(&currtime), "%c"); // Convert to string without the final newline
    std::string currtime_str = ss.str();

    auto isl = *(m__archipelago.begin() + island_idx);
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
    if (pop.get_problem().get_gevals() > 0)
        jcgen[io::key::gevals()] = pop.get_problem().get_gevals();
    if (pop.get_problem().get_hevals() > 0)
        jcgen[io::key::hevals()] = pop.get_problem().get_hevals();

    // Same as for append_static_info, but for the dynamic part
    auto append_dynamic_info = [](json_o &jdyn, auto pagmo_container) {
        auto jinfo = io::json::dynamic_descr(pagmo_container);
        if ( !jinfo.empty() ) jdyn.update(jinfo);
    };

    append_dynamic_info(jcgen, isl.get_algorithm());
    append_dynamic_info(jcgen, pop.get_problem());

    jout = std::move(jcgen);
}

void Experiment::append_isl_runtime_data(std::size_t island_idx) const
{
    // Open the file, append the new data, close the file.
    json_o jcurr_isl_status;
    freeze_isl_runtime_data(island_idx, jcurr_isl_status);
    
    std::ofstream ofs(isl_filename(island_idx, /*runtime=*/ true), std::ios::app);
    if (!ofs.is_open())
        __format_and_throw<std::runtime_error, bevarmejo::ClassError>(io::log::nname::opt+io::log::cname::Experiment, "prepare_isl_files",
            "Could not create the runtime file for the island.",
            "File : "+isl_filename(island_idx, /*runtime=*/ true).string());
        
    ofs << jcurr_isl_status.dump() << std::endl; // No value in dump so that it is a single line (JSONL)
    ofs.close();
}

void Experiment::finalise_isl_files() const
{
    // For each island, upload the file and move it to the final file, but well formatted.
    for (std::size_t i = 0; i < m__archipelago.size(); ++i)
    {
        auto isl = *(m__archipelago.begin() + i);

        std::ifstream rnt_file(isl_filename(i, /*runtime=*/ true));

        if (!rnt_file.is_open())
            __format_and_throw<std::runtime_error, bevarmejo::ClassError>(io::log::nname::opt+io::log::cname::Experiment, "finalise_isl_files",
                "Could not open the runtime file for the island.",
                "File : "+isl_filename(i, /*runtime=*/ true).string());
        
        if (rnt_file.peek() == std::ifstream::traits_type::eof())
        {
            rnt_file.close();
            __format_and_throw<std::runtime_error, bevarmejo::ClassError>(io::log::nname::opt+io::log::cname::Experiment, "finalise_isl_files",
                "The runtime file for the island is empty.",
                "File : "+isl_filename(i, /*runtime=*/ true).string());
        }

        // Prepare the json to combine all the data.
        // The data was saved in JSONL format, so I have to read it line by line.
        // The first one is the static data, all the others are the dynamic data
        // and we just need to append them to the array of the "generations" key.
        json_o jdata;
        std::string line;
        while (line.empty() && !rnt_file.eof())
            std::getline(rnt_file, line);
        
        // Just check that the "generations" key is present (we trust that the file is well formatted).
        jdata = json_o::parse(line);
        if (!io::key::generations.exists_in(jdata))
        {
            rnt_file.close();
            __format_and_throw<std::runtime_error, bevarmejo::ClassError>(io::log::nname::opt+io::log::cname::Experiment, "finalise_isl_files",
                "The runtime file for the island does not contain the generations key.",
                "File : "+isl_filename(i, /*runtime=*/ true).string());
        }

        // Now, append the dynamic data to the generations key.
        while (!rnt_file.eof())
        {
            // Read the file line by line and append it to the final file.
            std::getline(rnt_file, line);
            if (line.empty()) continue;

            // We trust that the file is well formatted, so we just append the line to the generations key.
            jdata[io::key::generations()].push_back(json_o::parse(line));    
        }

        rnt_file.close();
        
        // Save the final file.
        std::ofstream ofs(isl_filename(i, /*runtime=*/ false));
        if (!ofs.is_open())
            __format_and_throw<std::runtime_error, bevarmejo::ClassError>(io::log::nname::opt+io::log::cname::Experiment, "finalise_isl_files",
                "Could not create the final file for the island.",
                "File : "+isl_filename(i, /*runtime=*/ false).string());

        ofs << jdata.dump(4) << std::endl;
        ofs.close();
    }

    return;
}

void Experiment::finalise_exp_file() const
{
    // Maybe load the time or check that all the islands have been saved correctly.
    return;
}



} // namespace bevarmejo
