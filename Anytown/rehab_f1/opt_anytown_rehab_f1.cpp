// 
//  main_anytown_test.cpp
//	anytown original test
//
//  Created by Dennis Zanutto on 11/07/23.

#include <iostream>
#include <string>
#include <filesystem>
#include <utility>

#include <pagmo/problem.hpp>
#include <pagmo/types.hpp>
#include <pagmo/population.hpp>
#include <pagmo/algorithm.hpp>
#include <pagmo/algorithms/nsga2.hpp>

#include "bevarmejo/experiment.hpp"
#include "bevarmejo/pagmo_helpers/algorithms/nsga2_help.hpp"
#include "bevarmejo/io.hpp"
#include "bevarmejo/parsers.hpp"

#include "model_anytown.hpp"

int main(int argc, char* argv[]) {   
    
    // 1. Parse the inputs, ideally I could change anything and should perform checks.
    // argv[1] the settings file (it also implicitly defines the experiment folder unless copy flag is active)
    // TODO: argv[2, ...] optional flags 
    bevarmejo::ExperimentSettings settings;
    try {
        settings = bevarmejo::parse_optimization_settings(argc, argv);
    }
    catch (const std::exception& e) {
        std::cerr << "An error happend while parsing the CLI inputs:\n" << e.what() << std::endl;
        return 1;
    }

    // Create an experiment object to handle results and settings
    bevarmejo::Experiment experiment{ };

    try {
        experiment.build(settings);

        // Construct a pagmo::algorithm for NSGA2
        // Should check if it is "NSGA2"
        // For now i know I don't pass anything, just put the Report NFE that works as the algo number of generations
        json jnsga2{ {"Report gen", settings.jinput["Typical configuration"]["Population"]["Report gen"].get<unsigned int>() } };
        pagmo::algorithm algo{ bevarmejo::Nsga2(jnsga2) };

        // Construct a pagmo::problem for ANYTOWN model
        pagmo::problem p{ bevarmejo::ModelAnytown(settings.jinput["Typical configuration"]["UDP"]["Parameters"], settings.lookup_paths) };

        // and instantiate population
        pagmo::population pop{ std::move(p), settings.jinput["Typical configuration"]["Population"]["Size"].get<unsigned int>() };

        experiment.build(algo, pop);

        experiment.run(
            ceil(settings.jinput["Typical configuration"]["Population"]["Generations"].get<unsigned int>()/
            settings.jinput["Typical configuration"]["Population"]["Report gen"].get<unsigned int>())
            );
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        experiment.save_outcome();
        return 1;
    }

    experiment.save_outcome();
    
    return 0;
}
