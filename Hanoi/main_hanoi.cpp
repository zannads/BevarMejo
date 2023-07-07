/*----------------------
Author: DennisZ
descr: Main file for the optimization of the Hanoi problem.
----------------------*/

#include "main_hanoi.h"

#include <iostream>
#include <utility>

#include "pagmo/io.hpp"

#include "bevarmejo/experiment.hpp"
#include "bevarmejo/io.hpp"

using namespace pagmo;

struct nsga2p{
    unsigned int seed = 3u;
    unsigned int nfe = 100u;
    unsigned int report_nfe = 100u;
    pop_size_t pop_size = 100u;
    double cr{0.9}, eta_c{15.}, m{1./34.}, eta_m{7.};
    
    std::string rootDataFolder;
};

nsga2p quickUploadSettings(const char* settingsFile){
    
    // Load the file and check
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(settingsFile);
    
    nsga2p settingsNsga;

    settingsNsga.seed = doc.child("optProblem").child("optAlgorithm").attribute("seed").as_uint();
    
    settingsNsga.nfe = doc.child("optProblem").child("optAlgorithm").attribute("nfe").as_uint();
    
    settingsNsga.report_nfe = doc.child("optProblem").child("optAlgorithm").attribute("report_nfe").as_uint();
    
    // check I don't do mistakes
    if (settingsNsga.report_nfe == 0){
        settingsNsga.report_nfe = settingsNsga.nfe;
    }
    
    settingsNsga.rootDataFolder = doc.child("rootDataFolder").child_value();
    
    return settingsNsga;
}

int main(int argc, char* argv[])
{
    // check the number of inputs, the first one after -p should be the path to folder, the name for now I leave standard value
    std::filesystem::path experiment_folder(argv[2]);
    bevarmejo::Experiment experiment(experiment_folder);
    
    //Construct a pagmo::problem for Hanoi model
    problem p{ bevarmejo::ModelHanoi(experiment.settings_file()) };
    
    nsga2p settingsNsga = quickUploadSettings(experiment.settings_file().c_str());
    
    // Instantiate Optimization Algorithm
    algorithm algo{ nsga2(settingsNsga.report_nfe, settingsNsga.cr, settingsNsga.eta_c, settingsNsga.m, settingsNsga.eta_m, settingsNsga.seed) };

    // Instantiate population
    population pop{ std::move(p), settingsNsga.pop_size, settingsNsga.seed };
    
    // Evolve
    for(unsigned int i = 0; i*settingsNsga.report_nfe<settingsNsga.nfe; ++i){
        // Pop
        pop = algo.evolve(pop);
        // Save pop
    }
    
    experiment.save_final_result(pop, algo);
    
    return 0;
}
