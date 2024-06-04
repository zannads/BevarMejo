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

    if (argc < 3) {
        throw std::invalid_argument("Not enough arguments.\nUsage: " + std::string(argv[0]) + " <problem_settings_file> <decision_variables_file> [flags]");
    }
    Simulation simu;

    // 1. Parse the problem settings file
    try {
        // 1.0 make sure it exists 
        std::filesystem::path filepath(argv[1]);
        if (!std::filesystem::exists(filepath)) {
            throw std::invalid_argument("Settings file does not exist: " + filepath.string());
        }
        if (!std::filesystem::is_regular_file(filepath)) {
            throw std::invalid_argument("Settings file is not a regular file: " + filepath.string());
        }
        // Settings file is fine, save its full path and the folder
        simu.settings_file = filepath;
        simu.folder = filepath.parent_path();
        // settings folder is also the first lookup path
        simu.lookup_paths.push_back(simu.folder);

        // 1.1 read it 
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open settings file: " + filepath.string());
        }
        if (file.peek() == std::ifstream::traits_type::eof()) {
            file.close();
            throw std::runtime_error("Settings file is empty: " + filepath.string());
        }

        std::string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        // 1.2 parse it
        json jproblem = json::parse(file_content);

        // 1.3 Check the settings file has the required fields
        // name, if params not there pass empty -> most likely will throw exception the constructor, 
        if (!jproblem.contains(label::__name)) {
            throw std::runtime_error("Settings file does not specifies the name of the User Defined Problem\n");
        }
        if (!jproblem.contains(label::__params)) {
            jproblem[label::__params] = json::object();
        }

        // 1.4 check the settings file has the optional fields
        if (jproblem.contains(label::__paths)) {
            for (const auto& path : jproblem[label::__paths]) {
                // Check that is actually a string and an existing directory
                std::filesystem::path p(path.get<std::string>());
                if (std::filesystem::exists(p) && std::filesystem::is_directory(p)) {
                    simu.lookup_paths.push_back(p);
                }
                else {
                    std::cerr << "Path in the settings file is not a valid directory: " << p.string() << std::endl;
                }
            }
        }

        // 1.5 build the problem
        simu.p = std::move(build_problem( jproblem, simu.lookup_paths ));
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse problem settings file: " + simu.settings_file.string() + "\n" + e.what());
    }
    
    // 2. Parse the decision variables file
    // 2.0 add the cwd to the lookup path for the decision variables file as it may be a rel path 
    simu.lookup_paths.push_back(std::filesystem::current_path());
    try {
        // 2.1 make sure it exists
        // locate the file
        auto filepath = bevarmejo::io::locate_file(std::filesystem::path{argv[2]}, simu.lookup_paths);
        if (!filepath) {
            throw std::invalid_argument("Decision variables file not found: " + std::string(argv[2]));
        }
        // Decision variables file is fine, save its full path
        simu.dv_file = filepath.value();
        // 2.2 read it
        std::ifstream file(simu.dv_file);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open decision variables file: " + simu.dv_file.string());
        }
        if (file.peek() == std::ifstream::traits_type::eof()) {
            file.close();
            throw std::runtime_error("Decision variables file is empty: " + simu.dv_file.string());
        }

        std::string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        // 2.3 parse it
        json j = json::parse(file_content);
        // the file keys can be normal case or kebab case
        std::vector<std::string> keys = {label::__dv, label::__id, "Print"};
        for (auto& key : keys) {
            // if found normal just skip
            // othwersie check for kebab, if found re-add as normal key
            // else throw key not dofund 
            if (j.contains(key)) {
                continue;
            }
            else if (j.contains(bevarmejo::to_kebab_case(key))) {
                j[key] = j[bevarmejo::to_kebab_case(key)]; // TODO: check if you can simply change the string instead of copyingthe object 
            }
            else {
                throw std::runtime_error("Key "+key+ " not found in the decision variables file.\n" );
            }
        }
        simu.dvs            = j[label::__dv].get<std::vector<double>>();
        simu.id             = j[label::__id].get<unsigned long long>();
        simu.extra_message  = j["Print"].get<std::string>();

    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse decision variables file: " + simu.dv_file.string() + "\n" + e.what());
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