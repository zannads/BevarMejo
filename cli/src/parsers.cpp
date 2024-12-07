#include <cmath>
#include <iostream>
#include <filesystem>
namespace fsys = std::filesystem;
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>
using json_o = nlohmann::json;

#include <pagmo/problem.hpp>

#include "bevarmejo/utility/bemexcept.hpp"
#include "bevarmejo/io/fsys.hpp"
#include "bevarmejo/io/streams.hpp"

#include "bevarmejo/io/keys/beme.hpp"
#include "bevarmejo/io/keys/bemeexp.hpp"
#include "bevarmejo/io/keys/bemeopt.hpp"
#include "bevarmejo/io/keys/bemesim.hpp"

#include "bevarmejo/utility/string_manip.hpp"

#include "bevarmejo/library_metadata.hpp"
#include "bevarmejo/factories.hpp"

#include "bevarmejo/cli_settings.hpp"
#include "bevarmejo/simulation.hpp"

#include "parsers.hpp"

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

namespace opt {

ExperimentSettings  parse(int argc, char* argv[])
{
    beme_throw_if(argc < 2, std::invalid_argument,
        "Error parsing the command line arguments.",
        "Not enough arguments.",
        "Usage: ", argv[0], " <settings_file> [flags]");

    ExperimentSettings exp_settings{};

    // Add the cwd to the lookup path for the settings file as it may be a rel path
    Paths lookup_paths{fsys::current_path()};

    exp_settings.settings_file = bevarmejo::io::locate_file(fsys::path{argv[1]}, lookup_paths);

    // TODO: parse all the flags and set the values in the exp_settings

    // TODO: parse all the key value pairs that are passed as experiment flags

    return exp_settings;
}

} // namespace opt

namespace sim {

bevarmejo::Simulation parse(int argc, char *argv[])
{
    beme_throw_if(argc < 2, std::invalid_argument,
        "Error parsing the command line arguments.",
        "Not enough arguments.",
        "Usage: ", argv[0], " <settings_file> [flags]");
    
    Simulation simu;

    // 1. Parse the settings file
    // 1.0 add the cwd to the lookup path for the settings file as it may be a rel path 
    simu.lookup_paths.push_back(fsys::current_path());
    
    try {
        // 1.1 Locate in the system and save the full path of the settings file and other  info
        simu.settings_file = bevarmejo::io::locate_file(fsys::path{argv[1]}, simu.lookup_paths);
        simu.folder = simu.settings_file.parent_path();
        simu.lookup_paths.push_back(simu.folder);

        // 1.2 read it
        std::ifstream file(simu.settings_file);
        beme_throw_if(!file.is_open(), std::runtime_error,
            "Error parsing the command line arguments.",
            "Failed to open simulation settings file.",
            "File: ", simu.settings_file.string());

        if (file.peek() == std::ifstream::traits_type::eof())
        {
            file.close();
            beme_throw(std::runtime_error,
                "Simulation settings file is empty.",
                "The file is empty.",
                "File: ", simu.settings_file.string());
        }

        std::string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        // 1.3 parse it
        json_o j = json_o::parse(file_content);

        // 1.3.1 Optional keys that may change the behavior of the simulation
        if(io::key::lookup_paths.exists_in(j)) {
            json_o paths = io::json::extract(io::key::lookup_paths).from(j);

            if (paths != nullptr) {
                // Paths could be a string or an array of strings. In both case we need to check if they are directories 

                if (paths.is_string()) {
                    json_o jpath = json_o::array();
                    jpath.push_back(paths.get<std::string>());
                    paths = jpath;
                }

                for (const auto& path : paths) {
                    fsys::path p{path};

                    if (fsys::exists(p) && fsys::is_directory(p)) {
                        simu.lookup_paths.push_back(p);
                    } else {
                        std::cerr << "Path in the simulation settings file is not a valid directory: " << p.string() << std::endl;
                    }
                }
            }
        }

        if(io::key::beme_version.exists_in(j)) 
            VersionManager::user().set_user_v(io::json::extract(io::key::beme_version).from(j).get<std::string>());

        // 1.3.2 mandatory keys first: dv, udp
        auto check_mandatory_field = [](const io::key::Key &key, const json_o &j){
            if (key.exists_in(j)) {
                return;
            }

            beme_throw(std::runtime_error,
                "Error parsing the simulation settings file.",
                "Settings file does not contain a mandatory field.",
                "Missing field : ", key[0]);
        };

        check_mandatory_field(io::key::dv, j);
        check_mandatory_field(io::key::problem, j);

        simu.dvs = io::json::extract(io::key::dv).from(j).get<std::vector<double>>();

        const json_o &jproblem = io::json::extract(io::key::problem).from(j);
        check_mandatory_field(io::key::name, jproblem);
        check_mandatory_field(io::key::params, jproblem);

        // 1.5 build the problem
        simu.p = build_problem(
            io::json::extract(io::key::name).from(jproblem).get<std::string>(), 
            io::json::extract(io::key::params).from(jproblem),
            simu.lookup_paths
        );

        // 1.6 optional keys that don't change the behavior of the simulation
        if(io::key::fv.exists_in(j))
            simu.fvs = io::json::extract(io::key::fv).from(j).get<std::vector<double>>();
        
        if(io::key::id.exists_in(j))
            simu.id = io::json::extract(io::key::id).from(j).get<unsigned long long>();
        
        if(io::key::print.exists_in(j)) 
            simu.extra_message = io::json::extract(io::key::print).from(j).get<std::string>();

    }
    catch (const std::exception& e)
    {
        beme_throw(std::runtime_error,
            "Failed to parse the simulation settings file.",
            e.what(),
            "File: ", simu.settings_file.string());
    }

    // 3. Check the flags
    // 3.1 save inp file
    simu.save_inp = false;
    for (int i = 2; i < argc; ++i) {
        if (std::string(argv[i]) == "--saveinp") {
            simu.save_inp = true;
        }
    }
    // TODO: check the other flags

    // last last thing because I assume then after this it starts the simulation
    simu.start = std::chrono::high_resolution_clock::now();
    return simu;
}

} // namespace sim
} // namespace bevarmejo