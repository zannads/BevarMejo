#pragma once

#include <chrono>
#include <filesystem>
namespace fsys = std::filesystem;
#include <iostream>
#include <string>
#include <vector>

#include <pagmo/problem.hpp>

namespace bevarmejo {

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

} // namespace bevarmejo
