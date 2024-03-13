#include <iostream>
#include <string>
#include <filesystem>
#include <utility>

#include <pagmo/problem.hpp>
#include <pagmo/types.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "bevarmejo/experiment.hpp"
#include "bevarmejo/io.hpp"

#include "model_anytown.hpp"

int main(int argc, char* argv[]) {   
    
    // Parse the inputs, ideally I could change anything and should perform checks. 
    // For now the structure will be: -p path/to/experiment/folder
    //                                -dv path/to/dv.json
    std::filesystem::path experiment_folder(argv[2]);
    
    // Create an experiment object to handle results and settings
    bevarmejo::Experiment experiment(experiment_folder, 0);
    experiment.set_name("anytown_nsga2");

    // Construct a pagmo::problem for ANYTOWN model
    pagmo::problem p{ bevarmejo::ModelAnytown(experiment.input_dir(), experiment.model_settings()) };

    // Load the decision vector from file
    std::filesystem::path dv_file(argv[4]);
    if (!std::filesystem::exists(dv_file) || !std::filesystem::is_regular_file(dv_file) ) {
        bevarmejo::stream_out(std::cout, "Decision vector file not found or not a file: ", dv_file, "\n");
        return 1;
    }
        
    std::ifstream dv_stream(dv_file);
    if (!dv_stream.is_open()) {
        bevarmejo::stream_out(std::cout, "Could not open decision vector file: ", dv_file, "\n");
        return 1;
    }

    json j;
    dv_stream >> j;
    dv_stream >> j;
    dv_stream.close();

    // Convert the decision vector to a pagmo::vector_double
    std::vector<double> dvs = j["Decision vector"].get<std::vector<double>>();

    // Get the time to measure the elapsed time 
    auto start = std::chrono::high_resolution_clock::now();
    
    auto res = p.fitness(dvs);

    auto end = std::chrono::high_resolution_clock::now();

    bevarmejo::stream_out(std::cout, 
        "Element with ID ", j["ID"], 
        " evaluated with Fitness vector : ", res, 
        " in ", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(), "ms\n");
    
    return 0;
}
