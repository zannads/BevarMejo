/*----------------------
Author: DennisZ
descr: Main file for the optimization of the Hanoi problem.
----------------------*/

#include "main_hanoi.h"

#include <iostream>
#include <string>
#include <filesystem>
#include <utility>

#include "pagmo/io.hpp"

#include "bevarmejo/experiment.hpp"
#include "bevarmejo/io.hpp"
#include "bevarmejo/nsga2_helper.hpp"

int main(int argc, char* argv[])
{
    // Parse the inputs, ideally I could change anything and should perform checks. 
    // For now the structure will be: -p path/to/experiment/folder
    //                                -s seed
    std::filesystem::path experiment_folder(argv[2]);
    unsigned int seed = std::stoi(argv[4]);

    // Create an experiment object to handle results and settings
    bevarmejo::Experiment experiment(experiment_folder, seed); 
    experiment.set_name("hanoi_nsga2"); 
    
    // Construct a pagmo::problem for Hanoi model
    pagmo::problem p{ bevarmejo::ModelHanoi(experiment.input_dir(), experiment.model_settings()) };
    
    // Construct a pagmo::algorithm for NSGA2
    bevarmejo::nsga2p settingsNsga = bevarmejo::quick_settings_upload(experiment.algorithm_settings());
    settingsNsga.seed = seed;
    pagmo::algorithm algo{ pagmo::nsga2(settingsNsga.report_nfe, settingsNsga.cr, settingsNsga.eta_c, settingsNsga.m, settingsNsga.eta_m, settingsNsga.seed) };

    // Instantiate population
    pagmo::population pop{ std::move(p), settingsNsga.pop_size, settingsNsga.seed };
    
    // Evolve
    for(unsigned int i = 0; i*settingsNsga.report_nfe<settingsNsga.nfe; ++i){
        // Pop
        pop = algo.evolve(pop);
        // Save pop (maybe in the future)
    }
    // Save final result and end time
	experiment.finished();
    experiment.save_final_result(pop, algo);
    
    return 0;
}
