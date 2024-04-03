// 
//  main_anytown_test.cpp
//	anytown original test
//
//  Created by Dennis Zanutto on 11/07/23.

#include <iostream>
#include <string>
#include <stdexcept>

#include "bevarmejo/io.hpp"
#include "bevarmejo/parsers.hpp"

#include "bevarmejo/experiment.hpp"

int main(int argc, char* argv[]) {   
    
    // 1. Parse the inputs, ideally I could change anything and should perform checks.
    // argv[1] the settings file (it also implicitly defines the experiment folder unless copy flag is active)
    // TODO: argv[2, ...] optional flags 
    bevarmejo::ExperimentSettings settings;
    try {
        settings = bevarmejo::parse_optimization_settings(argc, argv);
    }
    catch (const std::exception& e) {
        bevarmejo::io::stream_out(std::cerr, "An error happend while parsing the CLI inputs:\n", e.what(), "\n" );
        return 1;
    }

    // Create an experiment object to handle results and settings
    bevarmejo::Experiment experiment{ };

    try {
        experiment.build( settings );

        experiment.run( settings.n_evolve );
    }
    catch (const std::exception& e) {
        bevarmejo::io::stream_out(std::cerr, "An error happend while building or running the experiment:\n", e.what(), "\n" );
        experiment.save_outcome();
        return 1;
    }

    experiment.save_outcome();
    
    return 0;
}
