#ifndef BEVARMEJOLIB__CLI_PARSER_HPP
#define BEVARMEJOLIB__CLI_PARSER_HPP

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <pagmo/problem.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace bevarmejo {

struct ExperimentSettings {
    // JSON containing the settings uploaded from the file
    json jinput;
    // The folder where the experiment is stored or going to be saved
    std::filesystem::path folder;
    // The full path to the settings file
    std::filesystem::path settings_file;
    // Name of the experiment
    std::string name;
    // Additional lookup paths for the internal files
    std::vector<std::filesystem::path> lookup_paths;
    // Number of times it is necessary to call evolve to reach the desired number of generations
    unsigned int n_evolve;
    // TODO: Add additional flags 
};

namespace sim {
struct Simulation {
    // pagmo::problem containing the UDP loaded from the settings file
    pagmo::problem p;
    // The folder where the experiment is stored or going to be saved
    std::filesystem::path folder;
    // The full path to the settings file
    std::filesystem::path settings_file;
    // Additional lookup paths for the internal files
    std::vector<std::filesystem::path> lookup_paths;
    // Decision variables to assign the problem
    std::vector<double> dvs;
    // The full path to the decision variables file
    std::filesystem::path dv_file;
    // ID of the decision vector to be used
    unsigned long long id;
    // Additional print out information after the simulation
    std::string extra_message;
    // Starting time of the simulation
    std::chrono::high_resolution_clock::time_point start;
    // Ending time of the simulation
    std::chrono::high_resolution_clock::time_point end;
}; // struct Simulation
} // namespace sim



ExperimentSettings parse_optimization_settings(int argc, char* argv[]);

namespace sim {
Simulation parse(int argc, char* argv[]);
} // namespace sim

} // namespace bevarmejo

#endif // BEVARMEJOLIB__CLI_PARSER_HPP
