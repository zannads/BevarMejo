// 
//  main_anytown_test.cpp
//	anytown original test
//
//  Created by Dennis Zanutto on 11/07/23.

#include <iostream>

#include "bevarmejo/cli_settings.hpp"
#include "bevarmejo/bemexcept.hpp"
#include "bevarmejo/experiment.hpp"
#include "bevarmejo/parsers.hpp"

int main(int argc, char* argv[]) {   
    
    // 1. Parse the inputs, ideally I could change anything and should perform checks.
    // argv[1] the settings file (it also implicitly defines the experiment folder unless copy flag is active)
    bevarmejo::ExperimentSettings settings;
    try {
        settings = bevarmejo::opt::parse(argc, argv);
    }
    catch (const std::exception& e) {
        bevarmejo::io::stream_out(std::cerr, "An error happend while parsing the CLI inputs:\n", e.what(), "\n" );
        return 1;
    }
    bevarmejo::Experiment experiment(settings.settings_file);
    try {
        experiment.run();
    }
    catch (const std::exception& e) {
        bevarmejo::io::stream_out(std::cerr, "An error happend while building or running the experiment:\n", e.what(), "\n" );
        // experiment.finalise_with_error();
        return 1;
    }
    
    return 0;
}
