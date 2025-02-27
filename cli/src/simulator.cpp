#include <cmath>
#include <iostream>
#include <filesystem>
namespace fsys = std::filesystem;
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <pagmo/problem.hpp>

#include "bevarmejo/io/fsys.hpp"
#include "bevarmejo/io/json.hpp"
#include "bevarmejo/io/keys/beme.hpp"
#include "bevarmejo/io/keys/bemeexp.hpp"
#include "bevarmejo/io/keys/bemeopt.hpp"
#include "bevarmejo/io/keys/bemesim.hpp"

#include "bevarmejo/io/streams.hpp"

#include "bevarmejo/utility/exceptions.hpp"
#include "bevarmejo/utility/string.hpp"
#include "bevarmejo/utility/metadata.hpp"
#include "bevarmejo/utility/pagmo/serializers/json/containers.hpp"

#include "simulator.hpp"

namespace bevarmejo {

using Paths = std::vector<fsys::path>;

namespace io {
namespace log {
namespace nname {
static const std::string opt = "optimisation::"; // "optimisation::"
static const std::string sim = "simulation::"; // "simulation::"
}
namespace fname {
static const std::string parse = "parse"; // "parse"
}
namespace mex {
static const std::string parse_error = "Error parsing the settings file."; // "Error parsing the settings file."

static const std::string nearg = "Not enough arguments."; // "Not enough arguments."
static const std::string usage_start = "Usage: "; // "Usage: "
static const std::string usage_end = " <settings_file> [flags]"; // " <settings_file> [flags]"
}
} // namespace log

namespace other {
static const std::string settings_file = "Settings file : "; // "Settings file : "
}
} // namespace io

Simulator::Simulator(const fsys::path& settings_file) :
    m__settings_file(settings_file),
    m__root_folder(settings_file.parent_path()),
    m__lookup_paths({settings_file.parent_path(), fsys::current_path()})

{
    // Check the extension, and based on that open the file, parse it based on
    // the file structure (JSON, YAML, XML, etc). 
    // Apply the key value pairs passed from command line
    // Once you have the final object call the build function.
    
    std::ifstream file(settings_file);
    beme_throw_if(!file.is_open(), std::runtime_error,
        "Failed to create the simulator.",
        "Failed to open the settings file.",
        io::other::settings_file + settings_file.string());

    if (file.peek() == std::ifstream::traits_type::eof())
    {
        file.close();
        beme_throw(std::runtime_error,
            "Failed to create the simulator.",
            "The settings file is empty.",
            io::other::settings_file + settings_file.string());
    }

    std::string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    Json jinput;
    if (settings_file.extension() == io::other::ext__json) 
    {   
        try
        {
            jinput = Json::parse(file_content);
        }
        catch (const std::exception& e)
        {
            beme_throw(std::runtime_error,
                "Failed to create the simulator.",
                "Failed to parse the settings file as JSON.",
                e.what(),
                io::other::settings_file + settings_file.string());
        }
    }
    else
    {
        beme_throw(std::runtime_error,
            "Impossible to create the simulator.",
            "The file format of the settings file is not supported.",
            "Format: ", settings_file.extension(),
            io::other::settings_file + settings_file.string());
    }

    try
    {
        // 1.3.1 Optional keys that may change the behavior of the simulation
        auto paths = jinput.value(io::key::lookup_paths.as_in(jinput), Json{});
        
        if (paths != nullptr && paths.is_string())
        {
            Json jpath = Json::array();
            jpath.push_back(paths.get<std::string>());
            paths = std::move(jpath);
        }

        for (const auto& path : paths)
        {
			auto p = path.get<fsys::path>();

            if (fsys::exists(p) && fsys::is_directory(p))
            {
                m__lookup_paths.push_back(p);
            }
            else
            {
                std::cerr << "Path in the simulation settings file is not a valid directory: " << p.string() << std::endl;
            }
        }

        if(io::key::beme_version.exists_in(jinput))
        {
            auto user_v_str = jinput.at(io::key::beme_version.as_in(jinput)).get<std::string>();
            if (!is_valid_version(user_v_str))
            {
                io::stream_out(std::cerr,
                    "The requested version is not valid. To use this version, recompile the library with the -DPROJECT_VERSION=YY.MM.PP flag.\n",
                    "Requested version: ", user_v_str, "\n",
                    "Valid versions for this executable: [", min_version_str, ", ", version_str, "].\n");
                std::abort();
            }
        }

        // 1.3.2 mandatory keys first: dv, udp
        check_mandatory_field(io::key::dv, jinput);
        check_mandatory_field(io::key::problem, jinput);

        m__dvs = jinput.at(io::key::dv.as_in(jinput)).get<std::vector<double>>();;

        // 1.5 build the problem
        Json jproblem = jinput.at(io::key::problem.as_in(jinput));
        jproblem[io::key::lookup_paths()] = m__lookup_paths;
        m__p = jproblem.get<pagmo::problem>();

        // 1.6 optional keys that don't change the behavior of the simulation
        m__fvs = jinput.value(io::key::fv.as_in(jinput), std::vector<double>{});

        m__id = jinput.value(io::key::id.as_in(jinput), 0ull);

        m__extra_message = jinput.value(io::key::print.as_in(jinput), std::string{});
    }
    catch (const std::exception& e)
    {
        beme_throw(std::runtime_error,
            "Failed to parse the simulation settings file.",
            e.what(),
            "File: ", m__settings_file.string());
    }
}

void Simulator::save_inp(bool save_inp)
{
    m__save_inp = save_inp;
}

Simulator Simulator::parse(int argc, char *argv[])
{
    beme_throw_if(argc < 2, std::invalid_argument,
        "Error parsing the command line arguments.",
        "Not enough arguments.",
        "Usage: beme-sim <settings_file> [flags]");

    // Add the cwd to the lookup path for the settings file as it may be a rel path
    std::vector<fsys::path> lookup_paths;
    lookup_paths.push_back(fsys::current_path());

    auto settings_file = bevarmejo::io::locate_file(fsys::path{argv[1]}, lookup_paths);

    auto simulator = Simulator(settings_file);

    // 3. Check the flags
    // 3.1 save inp file
    for (int i = 2; i < argc; ++i) {
        if (std::string(argv[i]) == "--saveinp") {
            simulator.save_inp(true);
        }
    }
    // TODO: check the other flags, and transform this in something assigning tasks.

    return std::move(simulator);
}

void Simulator::pre_run_tasks()
{
    return;
}

void Simulator::run()
{
    try
    {   
        m__start_time = std::chrono::high_resolution_clock::now();
        m__res = m__p.fitness(m__dvs);
    }
    catch(const std::exception& e)
    {
        m__end_time = std::chrono::high_resolution_clock::now();
        bevarmejo::io::stream_out(std::cerr, "An error happend while evaluating the decision vector:\n", e.what(), "\n" );
        return;
    }
    
    m__end_time = std::chrono::high_resolution_clock::now();
}

void Simulator::post_run_tasks()
{
    bevarmejo::io::stream_out(std::cout, 
        "Element with ID ", m__id,
        " evaluated with Fitness vector : ", m__res,
        " in ", std::chrono::duration_cast<std::chrono::milliseconds>(m__end_time - m__start_time).count(), " ms\n");

    if (!m__extra_message.empty())
        bevarmejo::io::stream_out(std::cout, m__extra_message, "\n");

    bool success = true;
    if (!m__fvs.empty() && m__fvs.size() == m__res.size())
    {
       
        for (size_t i = 0; i < m__fvs.size(); ++i)
        {
            if ( std::abs(m__fvs[i] - m__res[i]) > std::numeric_limits<double>::epsilon() )
            {
                bevarmejo::io::stream_out(std::cerr, "Mismatch between the fitness vector provided and the one returned by the problem simulation.\n",
                    std::setprecision(16),
                    "Index: ", i, "\n",
                    "Expected: ", m__fvs[i], "\n",
                    "Returned: ", m__res[i], "\n");

                success = false;
            }
        }
    }

    // No need to save anything (everything went well by definition).
    if (!m__save_inp)
        return;

    bevarmejo::io::stream_out(std::cout, "Thanks for using BeMe-Sim, saving the inp file...\n");
    try
    {
        if ( m__p.is<bevarmejo::anytown::Problem>() )
        {
            m__p.extract<bevarmejo::anytown::Problem>()->save_solution(m__dvs, std::to_string(m__id) + ".inp");
            return;
        }
        else 
        {
            beme_throw(std::runtime_error,
                "Impossible to save the inp file.",
                "The problem is not of the type anytown::Problem.",
                "Problem type: ", m__p.get_name());
        }
    }
    catch (const std::exception& e)
    {
        bevarmejo::io::stream_out(std::cerr, "An error happend while saving the inp file:\n", e.what(), "\n" );
        return;
    }
}

} // namespace bevarmejo
