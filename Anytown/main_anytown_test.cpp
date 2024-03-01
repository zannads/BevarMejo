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

#include "src/model_anytown.hpp"

#include "bevarmejo/io.hpp"

int main(int argc, char* argv[]) {   
    
    // Parse the inputs, ideally I could change anything and should perform checks. 
    // For now the structure will be: -p path/to/experiment/folder
    //                                -s seed
    //std::filesystem::path experiment_folder(argv[2]);
    //unsigned int seed = std::stoi(argv[4]);
    
    std::filesystem::path experiment_folder{"/Users/denniszanutto/data/BevarMejoData/_anytown_test_"};
    unsigned int seed = 7;
    
    // Create an experiment object to handle results and settings
    bevarmejo::Experiment experiment(experiment_folder, seed);
    experiment.set_name("anytown_nsga2");

    std::vector<double> dvs{1, 5, 0, 2, 1, 8, 0, 5, 1, 4, 0, 2, 1, 0, 0, 5, 1, 7, 0, 3, 0, 2, 0, 2, 2, 4, 2, 7, 1, 0, 0, 9, 1, 3, 2, 4, 0, 3, 0, 3, 0, 0, 1, 1, 1, 3, 2, 9, 0, 5, 0, 3, 1, 7, 0, 5, 0, 3, 0, 3, 0, 9, 2, 9, 0, 4, 0, 6, 1, 4, 3, 2, 0, 1, 4, 3, 0, 3, 0, 3, 3, 1, 2, 3, 3, 2, 3, 0, 0, 1, 0, 0, 3, 0, 0, 2, 0, 3, 3, 2, 0, 1, 0, 0};
    std::vector<double> obj{2, 8, 1, 5, 2, 1, 1, 0, 0, 5, 2, 0, 0, 5, 2, 1, 1, 5, 0, 4, 1, 6, 0, 5, 0, 9, 0, 3, 2, 0, 1, 6, 0, 3, 2, 4, 2, 3, 0, 3, 1, 3, 0, 5, 2, 4, 1, 6, 2, 6, 0, 9, 0, 7, 2, 6, 2, 9, 0, 4, 1, 2, 2, 9, 1, 5, 1, 5, 0, 4, 3, 8, 0, 1, 4, 3, 0, 2, 0, 3, 3, 3, 2, 0, 2, 2, 1, 3, 0, 0, 3, 2, 3, 0, 0, 0, 2, 3, 3, 2, 7, 1, 16, 0};
    // Construct a pagmo::problem for ANYTOWN model
    pagmo::problem p{ bevarmejo::ModelAnytown(experiment.input_dir(), experiment.model_settings()) };

   

    // Construct a pagmo::algorithm for NSGA2
    bevarmejo::nsga2p settings_nsgaII = bevarmejo::quick_settings_upload(experiment.algorithm_settings());
    settings_nsgaII.seed = seed;
    pagmo::algorithm algo{ pagmo::nsga2(settings_nsgaII.report_nfe, settings_nsgaII.cr, settings_nsgaII.eta_c, settings_nsgaII.m, settings_nsgaII.eta_m, settings_nsgaII.seed) };

    // Instantiate population
    pagmo::population pop{ std::move(p), settings_nsgaII.pop_size, settings_nsgaII.seed };

    // Evolve
    for (unsigned int i = 0; i * settings_nsgaII.report_nfe < settings_nsgaII.nfe; ++i) {
        std::cout << "Starting generation " << i+1 <<"/" << floor(settings_nsgaII.nfe/settings_nsgaII.report_nfe) << std::endl;
        // Pop
        pop = algo.evolve(pop);
        // Save pop (maybe in the future)
        std::cout << "Ending generation " << i+1 <<"/" << floor(settings_nsgaII.nfe/settings_nsgaII.report_nfe) << std::endl;
    }
    // Save final result and end time
    experiment.finished();
    experiment.save_final_result(pop, algo);
    
    return 0;
}
