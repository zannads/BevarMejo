//
//  water_distribution_system.cpp
//  hanoiOptimization
//
//  Created by Dennis Zanutto on 04/07/23.
//

#include "water_distribution_system.hpp"

#include <assert.h>
#include <iostream>
#include <stdio.h>
#include <stdexcept>
#include <string>
#include <utility>

#include "epanet2_2.h"

namespace bevarmejo {
WaterDistributionSystem::WaterDistributionSystem(){
    ph_ = nullptr;
}

//WaterDistributionSystem::WaterDistributionSystem(const std::filesystem::path& inp_path){};

WaterDistributionSystem::WaterDistributionSystem(std::string inp_filename){
    this->set_inpfile(inp_filename);
    
    this->init();
}

// Copy constructor
// this is not actually a copy constructor but rather a reinitialization one because I'm starting drom the inp file every time.
WaterDistributionSystem::WaterDistributionSystem(const WaterDistributionSystem &src){
    _inp_filename_ = src._inp_filename_;
    
    init();
}

WaterDistributionSystem::WaterDistributionSystem(WaterDistributionSystem &&src) noexcept{
    _inp_filename_ = std::move(src._inp_filename_);

    ph_ = src.ph_;
    src.ph_ = nullptr;
}

WaterDistributionSystem& WaterDistributionSystem::operator=(const WaterDistributionSystem& rhs) {
    if (this != &rhs){
        WaterDistributionSystem temp(rhs);
        ph_ = temp.ph_;
        temp.ph_ = nullptr;
        std::swap(_inp_filename_, temp._inp_filename_);
    }
    return *this;
}

WaterDistributionSystem& WaterDistributionSystem::operator=(WaterDistributionSystem &&rhs) noexcept {
    ph_ = rhs.ph_;
    rhs.ph_ = nullptr;
    
    _inp_filename_ = std::move(rhs._inp_filename_);
    return *this;
}

WaterDistributionSystem::~WaterDistributionSystem(){
    if (ph_!=nullptr){
        EN_close(ph_);
        EN_deleteproject(ph_);
        
        ph_ = nullptr;
    }
}

void WaterDistributionSystem::init(){
    assert(!_inp_filename_.empty());
    
    int errorcode = EN_createproject(&ph_);
    assert(errorcode<100);
    
    errorcode = EN_open(ph_, _inp_filename_.c_str(), "", ""); // with '\0' doesn't work. WHy?
    if (errorcode>100){
        EN_deleteproject(ph_);
        std::string error_message = "Error opening the .inp file with code: ";
        error_message.append(std::to_string(errorcode));
        throw std::runtime_error(error_message);
    }
    
    EN_setreport(ph_, "MESSAGES NO");
}

void WaterDistributionSystem::set_inpfile(const std::string inp_filename){
    _inp_filename_ = inp_filename;
}

std::string WaterDistributionSystem::get_inpfile() const{
    return _inp_filename_;
}

} /* namespace bevarmejo */
