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

#include "bevarmejo/bemexcept.hpp"
#include "bevarmejo/io/fsys_helpers.hpp"
#include "bevarmejo/io/streams.hpp"

#include "bevarmejo/io/keys/beme.hpp"
#include "bevarmejo/io/keys/bemeexp.hpp"
#include "bevarmejo/io/keys/bemeopt.hpp"
#include "bevarmejo/io/keys/bemesim.hpp"

#include "bevarmejo/utils/string_manip.hpp"

#include "bevarmejo/library_metadata.hpp"
#include "bevarmejo/factories.hpp"

#include "parsers.hpp"

namespace bevarmejo {

namespace io {
namespace log {
namespace nname {
static const std::string exp = "experiment::"; // "experiment::"
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




ExperimentSettings parse_optimization_settings(int argc, char* argv[]) {
    if (argc < 2) 
        __format_and_throw<std::invalid_argument, bevarmejo::FunctionError>(io::log::nname::exp+io::log::fname::parse,
            io::log::mex::parse_error,
            io::log::mex::nearg,
            io::log::mex::usage_start+std::string(argv[0])+io::log::mex::usage_end);
    
    fsys::path settings_file(argv[1]);
    if (!fsys::exists(settings_file))
        __format_and_throw<std::invalid_argument, bevarmejo::FunctionError>(io::log::nname::exp+io::log::fname::parse,
            io::log::mex::parse_error,
            "Settings file does not exist.",
            io::other::settings_file+settings_file.string());

    if (!fsys::is_regular_file(settings_file)) 
        __format_and_throw<std::invalid_argument, bevarmejo::FunctionError>(io::log::nname::exp+io::log::fname::parse,
            io::log::mex::parse_error,
            "Settings file is not a regular file.",
            io::other::settings_file+settings_file.string());

    ExperimentSettings settings;
    // Settings file is fine, save its full path and the folder
    settings.settings_file = settings_file;
    settings.folder = settings_file.parent_path();
    // settings folder is also the first lookup path
    settings.lookup_paths.push_back(settings.folder);

    // 2. Now actually parse the settings file
    std::ifstream file(settings_file);
    if (!file.is_open())
        __format_and_throw<std::runtime_error, bevarmejo::FunctionError>(io::log::nname::exp+io::log::fname::parse,
            io::log::mex::parse_error,
            "Failed to open settings file.",
            io::other::settings_file+settings_file.string());

    if (file.peek() == std::ifstream::traits_type::eof()) {
        file.close();

        __format_and_throw<std::runtime_error, bevarmejo::FunctionError>(io::log::nname::exp+io::log::fname::parse,
            io::log::mex::parse_error,
            "Settings file is empty.",
            io::other::settings_file+settings_file.string());
    }

    std::string file_contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    try {
        settings.jinput = json_o::parse(file_contents);
    } catch (const std::exception& e) {
        __format_and_throw<std::runtime_error, bevarmejo::FunctionError>(io::log::nname::exp+io::log::fname::parse,
            io::log::mex::parse_error,
            "Failed to parse settings file as JSON.",
            io::other::settings_file+settings_file.string()+"\n"+e.what());
    }

    // 3. Check the settings file has the required fields
    auto check_mandatory_field = [](const io::key::Key &key, const json_o &j) {
        if (key.exists_in(j)) {
            return;
        }

        __format_and_throw<std::runtime_error, bevarmejo::FunctionError>(io::log::nname::exp+io::log::fname::parse,
            io::log::mex::parse_error,
            "Settings file does not contain a mandatory field.",
            "Missing field : "+key[0]
        );
    };

    // as of now name is a mandatory field
    check_mandatory_field(io::key::name, settings.jinput);
    settings.name = io::json::extract(io::key::name).from(settings.jinput).get<std::string>();

    // as of now typconfig is a mandatory field
    check_mandatory_field(io::key::typconfig, settings.jinput);
    auto& typconfig = io::json::extract(io::key::typconfig).from(settings.jinput);
    // TODO: ALGO
    // TODO: PROBLEM

    // population, its size and the generations are mandatory. 
    // Seed, report gen are optional. 
    check_mandatory_field(io::key::population, typconfig);
    auto& pop = io::json::extract(io::key::population).from(typconfig);

    check_mandatory_field(io::key::size, pop);
    check_mandatory_field(io::key::generations, pop);
    const auto& genz = io::json::extract(io::key::generations).from(pop);

    // the algorithms need to know how many generations to report because the island calls the evolve method n times
    // until n*__report_gen > __generations, default = __generations
    if (!io::key::repgen.exists_in(pop)) 
        pop[io::key::repgen()] = pop[io::key::generations()];
    const auto &repgenz = io::json::extract(io::key::repgen).from(pop);
    
    // ceil(__generations/__report_gen) = n_evolve
    settings.n_evolve = ceil(genz.get<double>()/repgenz.get<double>()); // get double instead of unsigned int to force non integer division

    // 4. Check the settings file has the optional fields
    if (io::key::lookup_paths.exists_in(settings.jinput)) {
        json_o paths = io::json::extract(io::key::lookup_paths).from(settings.jinput);

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
                    settings.lookup_paths.push_back(p);
                } else {
                    std::cerr << "Path in the settings file is not a valid directory: " << p.string() << std::endl;
                }
            }
        }
    }

    // TODO: other flags

    // Last look up path is the current directory
    settings.lookup_paths.push_back(fsys::current_path());

    return settings;
}

namespace sim {

Simulation parse(int argc, char *argv[]) {

    if (argc < 2) 
        __format_and_throw<std::invalid_argument, bevarmejo::FunctionError>(io::log::nname::sim+io::log::fname::parse,
            io::log::mex::parse_error,
            io::log::mex::nearg,
            io::log::mex::usage_start+std::string(argv[0])+io::log::mex::usage_end);
    
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
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open simulation settings file: " + simu.settings_file.string());
        }
        if (file.peek() == std::ifstream::traits_type::eof()) {
            file.close();
            throw std::runtime_error("Simulation settings file is empty: " + simu.settings_file.string());
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
            VersionManager::user().set(io::json::extract(io::key::beme_version).from(j));

        // 1.3.2 mandatory keys first: dv, udp
         auto check_mandatory_field = [](const io::key::Key &key, const json_o &j) {
            if (key.exists_in(j)) {
                return;
            }

            __format_and_throw<std::runtime_error, bevarmejo::FunctionError>(io::log::nname::exp+io::log::fname::parse,
                io::log::mex::parse_error,
                "Settings file does not contain a mandatory field.",
                "Missing field : "+key[0]
            );
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
    catch (const std::exception& e) {
        __format_and_throw<std::runtime_error, bevarmejo::FunctionError>(io::log::nname::sim+io::log::fname::parse,
            io::log::mex::parse_error,
            "Failed to parse the simulation settings file.",
            "File: " + simu.settings_file.string() + "\n" + e.what()
        ); 
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