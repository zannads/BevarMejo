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

#include "bevarmejo/io/json.hpp"
#include "bevarmejo/io/keys/beme.hpp"
#include "bevarmejo/io/keys/bemeexp.hpp"
#include "bevarmejo/io/keys/bemeopt.hpp"
#include "bevarmejo/io/labels.hpp"
#include "bevarmejo/io/streams.hpp"

#include "bevarmejo/utility/exceptions.hpp"
#include "bevarmejo/utility/metadata.hpp"
#include "bevarmejo/utility/string.hpp"

#include "bevarmejo/utility/pagmo/serializers/json/containers.hpp"

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
    beme_throw_if(!file.is_open(), std::runtime_error,
        "Failed to open settings file.",
        io::other::settings_file + settings_file.string());

    std::string file_contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // For now, I only implement the JSON file format.
    Json jinput;
    if (settings_file.extension() == io::other::ext__json) 
    {   
        try
        {
            jinput = Json::parse(file_contents);
        }
        catch (const std::exception& e)
        {
            beme_throw(std::runtime_error,
                "Impossible to create the experiment.",
                "Failed to parse settings file as JSON.",
                e.what(),
                io::other::settings_file + settings_file.string());
        }
    }
    else
    {
        beme_throw(std::runtime_error,
            "Impossible to create the experiment.",
            "The file format of the settings file is not supported.",
            "Format: ", settings_file.extension(),
            io::other::settings_file + settings_file.string());
    }
    // TODO: if format is different, convert to JSON

    // TODO: Apply the key value pairs passed from command line

    // Once you have the final object call the build function.
    build(jinput);
}

void Experiment::build(const Json &jinput)
{
    // Upload the settings or the fields that can change the behavior of the construction of the experiment (e..g, lookup paths)
    if (io::key::settings.exists_in(jinput)) {
        const Json &jsettings = jinput.at(io::key::settings.as_in(jinput));
        
        // AliasedKey outf_pretty, aka "Output file enable indent".
        // When boolean, false deactives the indent, true activates it with a default value of 4.
        // When numeric, it is automatically true and the value is the number of spaces to indent (can be 0).
        if (io::key::outf_pretty.exists_in(jsettings))
        {
            const auto& joutf_pretty = jsettings.at(io::key::outf_pretty.as_in(jsettings));
            if (joutf_pretty.is_boolean())
            {
                m__settings.outf_indent = joutf_pretty.get<bool>();
                // m__settings.outf_indent_val = default value;
            }
            else if (joutf_pretty.is_number())
            {
                m__settings.outf_indent = true;
                m__settings.outf_indent_val = joutf_pretty.get<unsigned int>();
            }
            else
            {
                // TODO: log the error
            }
        }
        // If non existing, the default is true with a value of 4.
    }

    if (io::key::lookup_paths.exists_in(jinput))
    {
        const Json &jpaths = jinput.at(io::key::lookup_paths.as_in(jinput));

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

    // as of now name is a mandatory field
    check_mandatory_field(io::key::name, jinput);
    m__name = jinput.at(io::key::name.as_in(jinput)).get<std::string>();

    auto typconfig = jinput.value(io::key::typconfig.as_in(jinput), Json{});
    auto specs = jinput.value(io::key::specs.as_in(jinput), Json{});
    auto rand_starts = jinput.value(io::key::rand_starts.as_in(jinput), std::size_t{1});

    build_islands(typconfig, specs, rand_starts);

    if (!fsys::exists(output_folder()))
        fsys::create_directory(output_folder());

    prepare_isl_files();

    prepare_exp_file();

}

void Experiment::build_island(const Json &config)
{
    // Construct a pagmo::population
    // Population, its size and the generations are mandatory. 
    // Seed, report gen are optional. 
    check_mandatory_field(io::key::population, config);
    const auto& jpop = config.at(io::key::population.as_in(config));

    check_mandatory_field(io::key::generations, jpop);
    const auto& genz = jpop.at(io::key::generations.as_in(jpop));

    check_mandatory_field(io::key::algorithm, config);
    Json jalgo = config.at(io::key::algorithm.as_in(config));

    // the algorithms need to know how many generations to report because the island calls the evolve method n times
    // until n*__report_gen > __generations, default = __generations
    //  m__settings.n_evolves = 1 unless the user specifies it
    auto repgenz = jpop.value(io::key::repgen.as_in(jpop), Json{});

    // If it is not present, I retrieve it from the [algorithm][params][generations] field.
    // And if it is not present even there, it means it is the same as the generations.
    if (repgenz.empty() && io::key::params.exists_in(jalgo))
    {
        const auto& jalgo_p = jalgo.at(io::key::params.as_in(jalgo));
        repgenz = jalgo_p.value(io::key::generations.as_in(jalgo_p), genz);
    }

    // TODO: this should be island specifics
    m__settings.n_evolves = ceil(genz.get<double>()/repgenz.get<double>()); // get double instead of unsigned int to force non integer division

    // We have established the number of evolutions and the number of generations
    // for the report gen. Now overwrite the generations in the parameters of the algorithm
    if ( !io::key::params.exists_in(jalgo) )
    {
        jalgo[io::key::params()] = Json{};
    }
    jalgo.at(io::key::params.as_in(jalgo))[io::key::generations()] = repgenz;
    
    pagmo::algorithm algo = jalgo.get<pagmo::algorithm>();

    // Construct a pagmo::problem
    check_mandatory_field(io::key::problem, config);
    Json jprob = config.at(io::key::problem.as_in(config));
    
    // Update it with the extra paths for the lookup
    jprob[io::key::lookup_paths()] = m__lookup_paths;

    auto p = jprob.get<pagmo::problem>();
    
    // Now that I have everything I can build the population and then the island
    check_mandatory_field(io::key::size, jpop);
    pagmo::population pop{ std::move(p), jpop.at(io::key::size.as_in(jpop)).get<unsigned int>() };

    // Create and track the island
    m__archipelago.push_back(algo, pop); 

    // The name should be built from the string and extracting the placeholders (e.g., ${seed})
    m__islands_names.push_back(std::to_string(pop.get_seed()));
}

void Experiment::build_islands(const Json &typconfig, const Json &specs, const std::size_t rand_starts)
{
    // At least one between typconfig and specs should be present
    // Random starts must be at least 1
    // For each spec, merge the typconfig with the spec.
        // For each random start, build the island

    beme_throw_if(typconfig.empty() && specs.empty(), std::runtime_error,
        "Impossible to create the islands.",
        "No configuration for the islands.",
        "At least one between the typical configuration and the specializations must be present.");

    beme_throw_if(rand_starts < 1, std::runtime_error,
        "Impossible to create the islands.",
        "Invalid number of random starts.",
        "The number of random starts must be at least 1.",
        "\tNumber of random starts : ", rand_starts);

    // Spec is valid if it is empty, an object, or an array of objects
    beme_throw_if(!specs.empty() && !specs.is_object() && !specs.is_array(), std::runtime_error,
        "Impossible to create the islands.",
        "Invalid specializations.",
        "The specializations must be an object (also 'null') or an array of objects.");
    
    // If it is null or an object, I only have one specialisation (default typeconfig)
    std::size_t n_specs = (specs.empty() || specs.is_object()) ? 1 : specs.size();

    for (std::size_t i = 0; i < n_specs; ++i)
    {
        Json config = typconfig;

        if (!specs.empty())
        {
            if (specs.is_object())
                config.update(specs);
            else
                config.update(specs[i]);
        }
        
        for (std::size_t j = 0; j < rand_starts; ++j)
            build_island(config);
    }
}

void Experiment::run() {

    // This is where the magic happens, for now I deal with only one island, but
    // in the future I will have to deal with multiple islands and the archipelago

    // Everything is already in the archipelago, so I just have to call evolve n times
    // and save the results between each call and at the beginning for the init
    // population.

    // This should be a loop over the islands or a simple archipelago.evolve(n_generations)
    for (auto n = 0; n < m__settings.n_evolves; ++n) {
        for (auto i = 0; i < m__archipelago.size(); ++i) 
        {
            auto& island = *(m__archipelago.begin() + i);
            island.evolve(1);
        }

        for (auto i = 0; i < m__archipelago.size(); ++i) 
        {
            auto& island = *(m__archipelago.begin() + i);
            island.wait();
            append_isl_runtime_data(island, isl_filename(i, /*runtime=*/ true));
            // TODO: deal when the population has not been saved correctly
        }
    }

    // First, make sure that all the islands data have been moved from the runtime
    // files to the final files.
    finalise_isl_files();

    // Then, finalise the experiment file and delete the runtime files.
    finalise_exp_file();
}

fsys::path Experiment::output_folder() const
{
    return m__root_folder/io::other::dir__beme_out;
}

fsys::path Experiment::exp_filename() const
{
    std::string temp = (
        io::other::pre__beme_exp+
        io::other::sep__beme_filenames+
        m__name+
        io::other::ext__json
    );
    
    return output_folder()/temp;
}

fsys::path Experiment::isl_filename(std::size_t island_idx, bool runtime) const
{
    std::string suffix = runtime ? io::other::ext__jsonl : io::other::ext__json;
    std::string temp = (
        io::other::pre__beme_isl+
        io::other::sep__beme_filenames+
        m__name+
        io::other::sep__beme_filenames+
        m__islands_names.at(island_idx)+
        suffix
    );
        
    return output_folder()/temp;
}

void Experiment::prepare_isl_files() const 
{
    for (std::size_t i = 0; i < m__archipelago.size(); ++i)
    {
        auto& isl = *(m__archipelago.begin() + i);

        // Add the static information about the island. The dynamic ones will be appended.
        Json jstat{};
        jstat.update( Json{{io::key::island(), isl}} );
        jstat.update( Json{{io::key::algorithm(), isl.get_algorithm()}} );
        jstat.update( Json{{io::key::problem(), isl.get_population().get_problem()}} );
        jstat.update( Json{{io::key::r_policy(), isl.get_r_policy()}} );
        jstat.update( Json{{io::key::s_policy(), isl.get_s_policy()}} );

        // Add the intial population and the initial dynamic parameters of the objects. 
        Json& jout = jstat;
        jout[io::key::generations()] = Json::array();

        Json jcurr_isl_status;
        freeze_isl_runtime_data(jcurr_isl_status, isl);

        jout[io::key::generations()].push_back(jcurr_isl_status);

        std::ofstream ofs(isl_filename(i, /*runtime=*/ true), std::ios::out);
        beme_throw_if(!ofs.is_open(), std::runtime_error,
            "Failed to create the runtime file for the island.",
            "Could not open the runtime file for the island.",
            "File : ", isl_filename(i, /*runtime=*/ true).string());
            
        ofs << jout.dump() << std::endl; // No value in dump so that it is a single line (JSONL)
        ofs.close();
    }
}

void Experiment::prepare_exp_file() const {

    std::string currtime = bevarmejo::now_as_str();

    std::ofstream ofs(exp_filename());
    beme_throw_if(!ofs.is_open(), std::runtime_error,
        "Failed to create the experiment file.",
        "Could not open the experiment file.",
        "File : ", exp_filename().string());

    // Add the information about the experiment.
    
    Json jsys;
    // example machine, OS etc ... 
    Json jsoft;
    jsoft[io::key::beme_version()] = bevarmejo::version_str;

    Json jarchipelago;
    jarchipelago[io::key::topology()] = m__archipelago.get_topology();

    // Save the relative name of the islands.
    jarchipelago[io::key::islands()] = Json::array();
    for (auto i = 0; i < m__archipelago.size(); ++i)
        jarchipelago[io::key::islands()].push_back(isl_filename(i, /*runtime=*/ true).filename().string());

    // Save the file
    Json jout = {
        {io::key::system(), jsys},
        {io::key::archi(), jarchipelago},
        {io::key::software(), jsoft},
        {io::key::t0(), currtime},
        {io::key::tend(), nullptr}
    };

    if (m__settings.outf_indent)
        ofs << jout.dump(m__settings.outf_indent_val) << std::endl;
    else
        ofs << jout.dump() << std::endl;
    ofs.close();
}

void Experiment::freeze_isl_runtime_data(Json &jout, const pagmo::island &isl) const
{
    std::string currtime = bevarmejo::now_as_str();

    const pagmo::population pop = isl.get_population();
    // 2.1 Mandatory info: time, fitness evaulations 
    Json jcgen = {
        {io::key::fevals(), pop.get_problem().get_fevals()},
        {io::key::ctime(), currtime},
        {io::key::individuals(), Json::array()}
    };

    // 2.2 Mandatory info, the population's individuals
    Json &jinds = jcgen[io::key::individuals()];
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
    /*
    As of version 25.01.0, the dynamic information is not saved in the JSON file 
    anymore.
    The dynamic information was useful only for the UDP with an internal optimization
    of the controls of the problem, which is the only type of algorithm not supported
    anymore.


    auto append_dynamic_info = [](Json &jdyn, auto pagmo_container) {
        auto jinfo = io::json::dynamic_descr(pagmo_container);
        if ( !jinfo.empty() ) jdyn.update(jinfo);
    };

    append_dynamic_info(jcgen, isl.get_algorithm());
    append_dynamic_info(jcgen, pop.get_problem());
    */

    jout = std::move(jcgen);
}

void Experiment::append_isl_runtime_data(const pagmo::island &isl, const fsys::path &isl_filen) const
{
    // Open the file, append the new data, close the file.
    Json jcurr_isl_status;
    freeze_isl_runtime_data(jcurr_isl_status, isl);
    
    std::ofstream ofs(isl_filen, std::ios::app);
    beme_throw_if(!ofs.is_open(), std::runtime_error,
        "Impossible to append the runtime data for the island.",
        "Could not open the runtime file for the island.",
        "File : ", isl_filen.string());

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
        beme_throw_if(!rnt_file.is_open(), std::runtime_error,
            "Impossible to finalise the runtime file for the island.",
            "Could not open the runtime file for the island.",
            "File : ", isl_filename(i, /*runtime=*/ true).string());

        
        if (rnt_file.peek() == std::ifstream::traits_type::eof())
        {
            rnt_file.close();
            beme_throw_if(true, std::runtime_error,
                "Impossible to finalise the runtime file for the island.",
                "The runtime file for the island is empty.",
                "File : ", isl_filename(i, /*runtime=*/ true).string());
        }

        // Prepare the json to combine all the data.
        // The data was saved in JSONL format, so I have to read it line by line.
        // The first one is the static data, all the others are the dynamic data
        // and we just need to append them to the array of the "generations" key.
        Json jdata;
        std::string line;
        while (line.empty() && !rnt_file.eof())
            std::getline(rnt_file, line);
        
        // Just check that the "generations" key is present (we trust that the file is well formatted).
        jdata = Json::parse(line);
        beme_throw_if(!io::key::generations.exists_in(jdata), std::runtime_error,
            "Impossible to finalise the runtime file for the island.",
            "The runtime file for the island does not contain the generations key.",
            "File : ", isl_filename(i, /*runtime=*/ true).string());

        // Now, append the dynamic data to the generations key.
        while (!rnt_file.eof())
        {
            // Read the file line by line and append it to the final file.
            std::getline(rnt_file, line);
            if (line.empty()) continue;

            // We trust that the file is well formatted, so we just append the line to the generations key.
            jdata[io::key::generations()].push_back(Json::parse(line));    
        }

        rnt_file.close();
        
        // Save the final file.
        std::ofstream ofs(isl_filename(i, /*runtime=*/ false));
        beme_throw_if(!ofs.is_open(), std::runtime_error,
            "Impossible to finalise the runtime file for the island.",
            "Could not create the final file for the island.",
            "File : ", isl_filename(i, /*runtime=*/ false).string());

        if (m__settings.outf_indent)
            ofs << jdata.dump(m__settings.outf_indent_val) << std::endl;
        else
            ofs << jdata.dump() << std::endl;
        ofs.close();
    }

    return;
}

void Experiment::finalise_exp_file() const
{
    // Load the experiment file, check that all the islands have been saved correctly and only
    // save those that did
    // append the final time.

    std::fstream exp_file(exp_filename());
    beme_throw_if(!exp_file.is_open(), std::runtime_error,
        "Impossible to finalise the experiment file.",
        "Could not open the experiment file.",
        "File : ", exp_filename().string());
        
    if (exp_file.peek() == std::fstream::traits_type::eof())
    {
        exp_file.close();
        beme_throw(std::runtime_error,
            "Impossible to finalise the experiment file.",
            "The experiment file is empty.",
            "File : ", exp_filename().string());
    }

    Json jdata = Json::parse(exp_file);
    exp_file.close();

    if (!io::key::archi.exists_in(jdata))
    {
        exp_file.close();
        beme_throw(std::runtime_error,
            "Impossible to finalise the experiment file.",
            "The experiment file does not contain the archipelago key.",
            "File : ", exp_filename().string());
    }

    Json &jarchi = jdata.at(io::key::archi.as_in(jdata));

    if (!io::key::islands.exists_in(jarchi))
    {
        exp_file.close();
        beme_throw(std::runtime_error,
            "Impossible to finalise the experiment file.",
            "The archipelago key does not contain the islands key.",
            "File : ", exp_filename().string());
    }

    Json &jislands = jarchi.at(io::key::islands.as_in(jarchi));
    Json jislands_old = jislands;
    jislands = Json::array();

    for (auto i = 0; i < m__archipelago.size(); ++i)
    {
        auto file = isl_filename(i, /*runtime=*/ true);
        if (!fsys::exists(file) || !fsys::is_regular_file(file))
            continue; // The file has not been saved correctly, so I remove the name from the list.

        // Open and check it is not empty
        std::ifstream isl_file(file);
        if (!isl_file.is_open())
            continue;

        if (isl_file.peek() == std::ifstream::traits_type::eof())
        {
            isl_file.close();
            continue;
        }

        // Ok, it is fine. Close and append the final name
        isl_file.close();
        jislands.push_back(isl_filename(i).filename().string());

        // Delete the runtime file
        fsys::remove(file);
    }

    // Append the final time
    jdata[io::key::tend()] = bevarmejo::now_as_str();

    // Save the final file
    exp_file.open(exp_filename(), std::ios::out);
    beme_throw_if(!exp_file.is_open(), std::runtime_error,
        "Impossible to finalise the experiment file.",
        "Could not create the final experiment file.",
        "File : ", exp_filename().string());

    if (m__settings.outf_indent)
        exp_file << jdata.dump(m__settings.outf_indent_val) << std::endl;
    else
        exp_file << jdata.dump() << std::endl;
    exp_file.close();
}

} // namespace bevarmejo
