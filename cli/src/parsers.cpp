#include <cmath>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <pagmo/problem.hpp>

#include "bevarmejo/io.hpp"
#include "bevarmejo/labels.hpp"
#include "bevarmejo/library_metadata.hpp"
#include "bevarmejo/factories.hpp"

#include "parsers.hpp"

namespace bevarmejo {

ExperimentSettings parse_optimization_settings(int argc, char* argv[]) {
    if (argc < 2) {
        throw std::invalid_argument("Not enough arguments.\nUsage: " + std::string(argv[0]) + " <settings_file> [flags]");
    }
    std::filesystem::path settings_file(argv[1]);
    if (!std::filesystem::exists(settings_file)) {
        throw std::invalid_argument("Settings file does not exist: " + settings_file.string());
    }
    if (!std::filesystem::is_regular_file(settings_file)) {
        throw std::invalid_argument("Settings file is not a regular file: " + settings_file.string());
    }

    ExperimentSettings settings;
    // Settings file is fine, save its full path and the folder
    settings.settings_file = settings_file;
    settings.folder = settings_file.parent_path();
    // settings folder is also the first lookup path
    settings.lookup_paths.push_back(settings.folder);

    // 2. Now actually parse the settings file
    std::ifstream file(settings_file);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open settings file: " + settings_file.string());
    }
    if (file.peek() == std::ifstream::traits_type::eof()) {
        file.close();
        throw std::runtime_error("Settings file is empty: " + settings_file.string());
    }

    std::string file_contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    try {
        settings.jinput = json::parse(file_contents);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse settings file as JSON: " + settings_file.string() + "\n" + e.what());
    }

    // 3. Check the settings file has the required fields
    // as of now name is a mandatory field
    if (!settings.jinput.contains(label::__name)) {
        throw std::runtime_error("Settings file does not contain the mandatory field: " + label::__name);
    }
    settings.name = settings.jinput[label::__name];
    // typical configuration
    if (!settings.jinput.contains(label::__typconfig)) {
        throw std::runtime_error("Settings file does not contain the mandatory field: " + label::__typconfig);
    }
    // TODO: ALGO
    // TODO: PROBLEM

    // population, its size and the generations are mandatory. 
    // Seed, report gen are optional. 
    if (!settings.jinput[label::__typconfig].contains(label::__population)) {
        throw std::runtime_error("Settings file does not contain the mandatory field: " + label::__population);
    }
    if (!settings.jinput[label::__typconfig][label::__population].contains(label::__size)) {
        throw std::runtime_error("Settings file does not contain the mandatory field: " + label::__size);
    }
    if (!settings.jinput[label::__typconfig][label::__population].contains(label::__generations)) {
        throw std::runtime_error("Settings file does not contain the mandatory field: " + label::__generations);
    }
    if (!settings.jinput[label::__typconfig][label::__population].contains(label::__report_gen_sh)) {
        // the algorithms need to know how many generations to report because the island calls the evolve method n times
        // until n*__report_gen > __generations, default = __generations
        settings.jinput[label::__typconfig][label::__population][label::__report_gen_sh] = settings.jinput[label::__typconfig][label::__population][label::__generations];
    }
    // ceil(__generations/__report_gen) = n_evolve
    settings.n_evolve = ceil(settings.jinput[label::__typconfig][label::__population][label::__generations].get<unsigned int>()/
                            settings.jinput[label::__typconfig][label::__population][label::__report_gen_sh].get<double>()); // get double instead of unsigned int to force non integer division
    

    // 4. Check the settings file has the optional fields
    if (settings.jinput.contains(label::__paths)) {
        // TODO: Check it is actually an array
       // for (const auto& path : settings.jinput[label::__paths]) {
            // TODO: Check if it exists
            // TODO: Check if it is a directory
            // TODO: Check if absolute or relative path
      //  }
    }

    // TODO: other flags

    // Last look up path is the current directory
    settings.lookup_paths.push_back(std::filesystem::current_path());

    return settings;
}

namespace sim {

Simulation parse(int argc, char *argv[]) {

    if (argc < 2) {
        throw std::invalid_argument("Not enough arguments.\nUsage: " + std::string(argv[0]) + " <simulation_settings_file> [flags]");
    }
    Simulation simu;

    // 1. Parse the settings file
    // 1.0 add the cwd to the lookup path for the settings file as it may be a rel path 
    simu.lookup_paths.push_back(std::filesystem::current_path());
    
    try {
        // 1.1 make sure it exists
        // locate the file
        auto filepath = bevarmejo::io::locate_file(std::filesystem::path{argv[1]}, simu.lookup_paths);
        if (!filepath) {
            throw std::invalid_argument("Simulation settings file not found: " + std::string(argv[1]));
        }

        // Simulation settings file is fine, save its full path
        simu.settings_file = filepath.value();
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
        json j = json::parse(file_content);

        // 1.3.1 Optional keys that may change the behavior of the simulation
        if (j.contains(label::__paths)) {
            for (const auto& path : j[label::__paths]) {
                // Check that is actually a string and an existing directory
                std::filesystem::path p(path.get<std::string>());
                if (std::filesystem::exists(p) && std::filesystem::is_directory(p)) {
                    simu.lookup_paths.push_back(p);
                }
                else {
                    std::cerr << "Path in the simulation settings file is not a valid directory: " << p.string() << std::endl;
                }
            }
        }
        // it could be for a old version of a library
        if (j.contains(bevarmejo::to_kebab_case(label::__version)) )
            j[label::__version] = j[bevarmejo::to_kebab_case(label::__version)];
        
        if (j.contains(label::__version)) {
            auto ud_version = j[label::__version].get<std::string>();
            VersionManager::instance().set(ud_version);
        }

        // 1.3.2 mandatory keys first: dv, udp
        // the file keys can be normal case or kebab case
        std::vector<std::string> keys = {label::__dv, label::__problem_sh};
        for (auto& key : keys) {
            // if found normal just skip
            // othwersie check for kebab, if found re-add as normal key
            // else throw key not dofund 
            if (j.contains(key)) {
                continue;
            }
            else if (j.contains(bevarmejo::to_kebab_case(key))) {
                j[key] = j[bevarmejo::to_kebab_case(key)];
                j.erase(bevarmejo::to_kebab_case(key));
            }
            else {
                throw std::runtime_error("Key "+key+ " not found in the simulation settings file.\n" );
            }
        }
        simu.dvs            = j[label::__dv].get<std::vector<double>>();

        json jproblem = j[label::__problem_sh];

        // 1.3 Check the settings file has the required fields
        // name, if params not there pass empty -> most likely will throw exception the constructor, 
        if (!jproblem.contains(label::__name)) {
            throw std::runtime_error("Simulation settings file does not specifies the name of the User Defined Problem\n");
        }
        if (!jproblem.contains(label::__params)) {
            jproblem[label::__params] = json::object();
        }

        // 1.5 build the problem
        simu.p = std::move(build_problem( jproblem, simu.lookup_paths ));

        // 1.6 optional keys that don't change the behavior of the simulation
        if (j.contains(label::__fv)) {
            simu.fvs = j[label::__fv].get<std::vector<double>>();
        }
        else if (j.contains(bevarmejo::to_kebab_case(label::__fv))) {
            simu.fvs = j[bevarmejo::to_kebab_case(label::__fv)].get<std::vector<double>>();
        }

        if (j.contains(label::__id)) {
            simu.id = j[label::__id].get<unsigned long long>();
        }
        else if (j.contains(bevarmejo::to_kebab_case(label::__id))) {
            simu.id = j[bevarmejo::to_kebab_case(label::__id)].get<unsigned long long>();
        }

        if (j.contains("Print")) {
            simu.extra_message = j["Print"].get<std::string>();
        }
        else if (j.contains("print")) {
            simu.extra_message = j["print"].get<std::string>();
        }
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse the simulation settings file: " + simu.settings_file.string() + "\n" + e.what());
    }

    // 3. Check the flags
    // 3.1 save inp file
    simu.save_inp = false;
    for (int i = 3; i < argc; ++i) {
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