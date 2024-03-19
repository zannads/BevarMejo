#ifndef BEVARMEJOLIB__CLI_PARSER_HPP
#define BEVARMEJOLIB__CLI_PARSER_HPP

#include <iostream>
#include <string>
#include <filesystem>
#include <utility>

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
    // TODO: Add additional flags 
};

ExperimentSettings parse_optimization_settings(int argc, char* argv[]);

} // namespace bevarmejo

#endif // BEVARMEJOLIB__CLI_PARSER_HPP
