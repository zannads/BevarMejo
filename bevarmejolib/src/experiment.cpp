//
//  experiment.cpp
//  bemelib_classes
//
//  Created by Dennis Zanutto on 06/07/23.
//

#include <chrono>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "pagmo/algorithm.hpp"
#include "pagmo/population.hpp"

#include "io.hpp"

#include "experiment.hpp"

namespace bevarmejo {

std::string Experiment::get_name(){
    std::ostringstream oss;
    oss << _name_;
    return oss.str();
}

std::string Experiment::get_extra_info(){
    // TODO extend
    std::ostringstream oss;
    oss << _user_custom_info_;
    return oss.str();
}

void Experiment::save_final_result(pagmo::population &pop, pagmo::algorithm &algo){
    
    //TODO create your own name
    
    // open and create the file
    std::ofstream ofs("test.out");
    if (!ofs.is_open())
        return;
    
    // 0. write the header (system and experiment info)
    stream_param(ofs, get_name(), "");
    stream_param(ofs, get_extra_info(), "");
    std::string sJunk{};
    sJunk ="Current time: ";
    const auto now = std::chrono::system_clock::now();
    const time_t t_c = std::chrono::system_clock::to_time_t(now);
    sJunk += ctime(&t_c);
    ofs <<sJunk;
    ofs <<std::endl;
    
    // 1. Algorithm
    stream_param(ofs, "Algorithm: ", algo.get_name());
    stream_param(ofs, "", algo.get_extra_info());
    ofs <<std::endl;
    
    // 2. Model Problem
    stream_param(ofs, "Problem name: ", pop.get_problem().get_name());
    stream_param(ofs, "", pop.get_problem().get_extra_info());
    
    ofs <<std::endl;
    
    // 3. Population
    stream_param(ofs, "Population:", "");
    stream_param(ofs, "\tFitness evaluations: ", pop.get_problem().get_fevals());
    auto last_individual = pop.size();
    stream_param(ofs, "\tPopulation size: ", last_individual);
    
    auto population_ids = pop.get_ID();
    auto pop_dvs        = pop.get_x();
    auto pop_fitnesses  = pop.get_f();
    
    //stream_param(ofs, "List of individuals: ", "");
    for (auto individual = 0u; individual<last_individual; ++individual){
        stream_param(ofs, "#", individual);
        stream_param(ofs, "\tID: ", population_ids[individual]);
        stream_param(ofs, "\tDecision vector: ", pop_dvs[individual]);
        stream_param(ofs, "\tFitness vector: ", pop_fitnesses[individual]);
    }
}
}
