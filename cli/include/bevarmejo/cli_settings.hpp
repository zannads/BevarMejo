#pragma once 

#include <filesystem>
namespace fsys = std::filesystem;
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "bevarmejo/utility/except.hpp"
#include "bevarmejo/io/streams.hpp"

namespace bevarmejo {

struct ExperimentSettings 
{
    fsys::path settings_file{}; // The full path to the settings file.
    
    bool resume{false};     // Flag for resuming the experiment.

    bool deep_copy{false};  // Flag to perform a deep copy of the experiment folder.

    using key_value = std::pair<std::string, std::string>;
    std::vector<key_value> keys_override; // Keys and their values to override in the settings file.
};

struct SimulationSettings 
{
    fsys::path settings_file; // The full path to the settings file.

    bool save_inp{false}; // Flag to save the inp file (useful for running the simulations directly from EPANET).

    using key_value = std::pair<std::string, std::string>;
    std::vector<key_value> keys_override; // Keys and their values to override in the settings file.    
};

} // namespace bevarmejo
