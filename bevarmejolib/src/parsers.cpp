#include <iostream>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "bevarmejo/io.hpp"

#include "parsers.hpp"

namespace bevarmejo {

ExperimentSettings parse_optimization_settings(int argc, char* argv[]) {
    if (argc < 2) {
        throw std::invalid_argument("Usage: " + std::string(argv[0]) + " <settings_file> [flags]");
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

} // namespace bevarmejo