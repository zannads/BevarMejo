#pragma once

#include <chrono>
#include <filesystem>
namespace fsys = std::filesystem;
#include <iostream>
#include <string>
#include <vector>

#include <pagmo/problem.hpp>

#include "bevarmejo/io/streams.hpp"
#include "bevarmejo/problems/anytown.hpp"

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

    bool write_files() const
    {
        // No need to save anything (everything went well by definition).
        if (!save_inp)
            return true;

        bevarmejo::io::stream_out(std::cout, "Thanks for using BeMe-Sim, saving the inp file...\n");
        try
        {
            if ( p.is<bevarmejo::anytown::Problem>() )
            {
                p.extract<bevarmejo::anytown::Problem>()->save_solution(dvs, std::to_string(id) + ".inp");
                return true;
            }
            else 
            {
                bevarmejo::io::stream_out(std::cerr, "The problem is not supported for saving the inp file.\n");
                return false;
            }
        }
        catch (const std::exception& e)
        {
            bevarmejo::io::stream_out(std::cerr, "An error happend while saving the inp file:\n", e.what(), "\n" );
            return false;
        }
    }

}; // struct Simulation

} // namespace bevarmejo
