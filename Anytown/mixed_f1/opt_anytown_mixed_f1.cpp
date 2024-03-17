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
#include "bevarmejo/nsga2_helper.hpp"

#include "model_anytown.hpp"

#include "bevarmejo/io.hpp"

int main(int argc, char* argv[]) {   
    
    // Parse the inputs, ideally I could change anything and should perform checks. 
    // For now the structure will be: -p path/to/experiment/folder
    //                                -s seed
    std::filesystem::path experiment_folder(argv[2]);
    unsigned int seed = std::stoi(argv[4]);
    
    // Create an experiment object to handle results and settings
    bevarmejo::Experiment experiment("anytown_nsga2", experiment_folder, seed);

    // Construct a pagmo::problem for ANYTOWN model
    pagmo::problem p{ bevarmejo::ModelAnytown(experiment.input_dir(), experiment.model_settings()) };

    // Construct a pagmo::algorithm for NSGA2
    bevarmejo::nsga2p settings_nsgaII = bevarmejo::quick_settings_upload(experiment.algorithm_settings());
    settings_nsgaII.seed = seed;
    pagmo::algorithm algo{ pagmo::nsga2(settings_nsgaII.report_nfe, settings_nsgaII.cr, settings_nsgaII.eta_c, settings_nsgaII.m, settings_nsgaII.eta_m, settings_nsgaII.seed) };

    // Instantiate population
    pagmo::population pop{ std::move(p), settings_nsgaII.pop_size, settings_nsgaII.seed };

    try {
        experiment.build(algo, pop);

        experiment.run(ceil(settings_nsgaII.nfe/settings_nsgaII.report_nfe));
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        experiment.save_outcome();
        return 1;
    }

    experiment.save_outcome();
    
    return 0;
}
