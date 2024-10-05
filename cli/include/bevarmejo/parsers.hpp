#ifndef BEVARMEJOLIB__CLI_PARSER_HPP
#define BEVARMEJOLIB__CLI_PARSER_HPP

#include <chrono>
#include <filesystem>
namespace fsys = std::filesystem;
#include <iostream>
#include <string>
#include <vector>

#include <pagmo/problem.hpp>

#include <nlohmann/json.hpp>
using json_o = nlohmann::json;

namespace bevarmejo {

struct ExperimentSettings {
    // JSON containing the settings uploaded from the file
    json_o jinput;
    // The folder where the experiment is stored or going to be saved
    fsys::path folder;
    // The full path to the settings file
    fsys::path settings_file;
    // Name of the experiment
    std::string name;
    // Additional lookup paths for the internal files
    std::vector<fsys::path> lookup_paths;
    // Number of times it is necessary to call evolve to reach the desired number of generations
    unsigned int n_evolve;
    // TODO: Add additional flags 
};

namespace sim {
struct Simulation {
    // The full path to the settings file
    fsys::path settings_file;
    // Decision variables to assign the problem (mandatory input)
    std::vector<double> dvs;
    // Fitness of the decision variables (optional input, defaults to nothing, used to check the results)
    std::vector<double> fvs;
    // ID of the decision vector to be used (optional input, defaults to 0, used to print in stdout)
    unsigned long long id;
    // Additional print out information after the simulation (optional input, defaults to nothing)
    std::string extra_message;
    // Version requested for the simulation (optional input, defaults to last)
    std::string version;
    // pagmo::problem containing the UDP loaded from the settings file (mandatory input)
    pagmo::problem p;
    // The folder where the experiment is stored or going to be saved
    fsys::path folder;
    // Additional lookup paths for the internal files
    std::vector<fsys::path> lookup_paths;
    // Starting time of the simulation
    std::chrono::high_resolution_clock::time_point start;
    // Ending time of the simulation
    std::chrono::high_resolution_clock::time_point end;
    // Flag to save the inp file (useful for running the simulations directly from EPANET)
    bool save_inp;
}; // struct Simulation
} // namespace sim



ExperimentSettings parse_optimization_settings(int argc, char* argv[]);

namespace sim {
Simulation parse(int argc, char* argv[]);
} // namespace sim

} // namespace bevarmejo

#endif // BEVARMEJOLIB__CLI_PARSER_HPP
