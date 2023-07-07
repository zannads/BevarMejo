//
//  experiment.hpp
//  bemelib_classes
//
//  Created by Dennis Zanutto on 06/07/23.
//

#ifndef BEMELIB_EXPERIMENT_HPP
#define BEMELIB_EXPERIMENT_HPP

#include <chrono>
#include <iostream>
#include <filesystem>
#include <string>

#include "pagmo/algorithm.hpp"
#include "pagmo/population.hpp"

namespace bevarmejo {

class Experiment{
protected:
    std::filesystem::path _root_experiment_folder_;
    std::filesystem::path _settings_file_;
    
    std::time_t _start_time_;
    std::time_t _end_time_;
    
    std::string _name_;
    std::string _user_custom_info_;
    
    
public:
    
    void save_final_result(pagmo::population &pop, pagmo::algorithm &algo);
    
    std::string get_name();
    std::string get_extra_info();
};
}

#endif /* BEMELIB_EXPERIMENT_HPP */
