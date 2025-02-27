#pragma once

#include <chrono>
#include <filesystem>
namespace fsys = std::filesystem;
#include <iostream>
#include <string>
#include <vector>

#include <pagmo/problem.hpp>

namespace bevarmejo
{

class Simulator final
{
/*----------------------------------------------------------------------------*/
/*---------------------------- Member types ----------------------------------*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*---------------------------- Member objects --------------------------------*/
/*----------------------------------------------------------------------------*/
// (common utility objects)
private:
    // The folder where the experiment is stored or going to be saved.
    fsys::path m__root_folder;
    // The full path to the settings file
    fsys::path m__settings_file;
    // Additional lookup paths for the internal files
    std::vector<fsys::path> m__lookup_paths;

// (mandatory inputs)
private:
    // Decision variables to assign the problem (mandatory input)
    std::vector<double> m__dvs;
    // pagmo::problem containing the UDP loaded from the settings file (mandatory input)
    pagmo::problem m__p;

// (optional inputs)
private:
    // Fitness of the decision variables (optional input, defaults to nothing, used to check the results)
    std::vector<double> m__fvs;
    // ID of the decision vector to be used (optional input, defaults to 0, used to print in stdout)
    unsigned long long m__id;
    // Additional print out information after the simulation (optional input, defaults to nothing)
    std::string m__extra_message;
    // Version requested for the simulation (optional input, defaults to last)
    std::string m__version;

// (outputs)
private:
    // Fitness of the decision variables (output)
    std::vector<double> m__res;
    // Starting time of the simulation
    std::chrono::high_resolution_clock::time_point m__start_time;
    // Ending time of the simulation
    std::chrono::high_resolution_clock::time_point m__end_time;

// (flags) temporary callable methods will be added from the input parser.
    // Flag to save the inp file (useful for running the simulations directly from EPANET)
    bool m__save_inp;
    bool m__save_res;

/*----------------------------------------------------------------------------*/
/*--------------------------- Member functions -------------------------------*/
/*----------------------------------------------------------------------------*/
// (constructor)
public:
    Simulator() = default;
    Simulator(const Simulator& other) = default;
    Simulator(Simulator&& other) = default;
    Simulator(const fsys::path &settings_file);

// (destructor)
public:
    ~Simulator() = default;

// operator=
public:
    Simulator& operator=(const Simulator& other) = default;
    Simulator& operator=(Simulator&& other) = default;

// Element access
public:
    void save_inp(bool save_inp);
    void save_res(bool save_res);

public:
    static Simulator parse(int argc, char* argv[]);

    void pre_run_tasks();

    void run();

    void post_run_tasks();

}; // struct Simulator

} // namespace bevarmejo
