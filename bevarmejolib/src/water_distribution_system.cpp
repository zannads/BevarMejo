//
//  water_distribution_system.cpp
//  hanoiOptimization
//
//  Created by Dennis Zanutto on 04/07/23.
//

#include <assert.h>
#include <filesystem>
#include <iostream>
#include <stdio.h>
#include <stdexcept>
#include <string>
#include <utility>

#include "epanet2_2.h"

#include "water_distribution_system.hpp"

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

    _subnetworks_ = src._subnetworks_;
}

WaterDistributionSystem::WaterDistributionSystem(WaterDistributionSystem &&src) noexcept{
    _inp_filename_ = std::move(src._inp_filename_);

    ph_ = src.ph_;
    src.ph_ = nullptr;

    _subnetworks_ = std::move(src._subnetworks_);
}

WaterDistributionSystem& WaterDistributionSystem::operator=(const WaterDistributionSystem& rhs) {
    if (this != &rhs){
        WaterDistributionSystem temp(rhs);
        ph_ = temp.ph_;
        temp.ph_ = nullptr;
        std::swap(_inp_filename_, temp._inp_filename_);
        std::swap(_subnetworks_, temp._subnetworks_);
    }
    return *this;
}

WaterDistributionSystem& WaterDistributionSystem::operator=(WaterDistributionSystem &&rhs) noexcept {
    ph_ = rhs.ph_;
    rhs.ph_ = nullptr;
    
    _inp_filename_ = std::move(rhs._inp_filename_);
    _subnetworks_ = std::move(rhs._subnetworks_);
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

std::vector<std::vector<std::vector<double>>> WaterDistributionSystem::run_hydraulics() const
{
    // Empty 3d vector of results
    std::vector<std::vector<std::vector<double>>> results;

    int errorcode = EN_openH(ph_);
    if (errorcode >= 100)
        return results;

    errorcode = EN_initH(ph_, 10);
    if (errorcode >= 100)
        return results;

    // if the inp file is correct these errors should be always 0
    long h_step;
    errorcode = EN_gettimeparam(ph_, EN_HYDSTEP, &h_step);
    assert(errorcode < 100);
    long r_step;
    errorcode = EN_gettimeparam(ph_, EN_REPORTSTEP, &r_step);
    assert(errorcode < 100);
    long horizon;
    errorcode = EN_gettimeparam(ph_, EN_DURATION, &horizon);
    assert(errorcode < 100);

    long n_reports = horizon / r_step + 1; // +1 because the first report is at time 0

    // here I should build the the 3 objects a priori 
    // in the future with subnetworks I could remove this complex data retrival
    int n_nodes;
    errorcode = EN_getcount(ph_, EN_NODECOUNT, &n_nodes);
    assert(errorcode < 100);

    int n_links;
    errorcode = EN_getcount(ph_, EN_LINKCOUNT, &n_links);
    assert(errorcode < 100);

    int n_pumps{ 0 };
    for (int i = 1; i <= n_links; ++i) {
        int link_type;
        errorcode = EN_getlinktype(ph_, i, &link_type);
        assert(errorcode < 100);
        if (link_type == EN_PUMP)
            n_pumps++;
    }

    std::vector<std::vector<double>> pressures(n_reports, std::vector<double>(n_nodes));
    std::vector<std::vector<double>> flows    (n_reports, std::vector<double>(n_links));
    std::vector<std::vector<double>> energies (n_reports, std::vector<double>(n_pumps, 0.));


    bool solution_has_failed = false;
    bool scheduled; // is the current time a reporting time?
    long t{ 0 }; // current time
    long delta_t{ 0 }; // real hydraulic time step
    unsigned int r_iter{ 0 }; // index of the current report

    do {
        errorcode = EN_runH(ph_, &t);
        if (errorcode >= 100) {
            solution_has_failed = true;
            break;
            // I don'return because I need to close the hydraulics
        }

        errorcode = EN_nextH(ph_, &delta_t);
        assert(errorcode < 100);

        // if the current time is a reporting time, I save all the results
        scheduled = (t % r_step == 0);
        if (scheduled) {
            // save pressures and flows
            for (int j = 1; j <= n_nodes; ++j) {
                errorcode = EN_getnodevalue(ph_, j, EN_PRESSURE, &pressures[r_iter][j - 1]);
                assert(errorcode < 100);
            }
            for (int j = 1; j <= n_links; ++j) {
                errorcode = EN_getlinkvalue(ph_, j, EN_FLOW, &flows[r_iter][j - 1]);
                assert(errorcode < 100);
            }
            // at scheduled time step, i.e., when I save the report, I should save the energy 
            // of this instant in the next reporting time step. So I increment r_iter before 
            // saving the energy 
            ++r_iter;
        }

        // always add energy (at the next one) but be careful of the last step
        if (n_pumps > 0 && r_iter < n_reports) {
            int pump_iter = 0;
            for (int j = 1; j <= n_links; ++j) {

                int link_type;
                errorcode = EN_getlinktype(ph_, j, &link_type);
                assert(errorcode < 100);
                if (link_type == EN_PUMP) {
                    double instant_energy;
                    errorcode = EN_getlinkvalue(ph_, j, EN_ENERGY, &instant_energy);
                    assert(errorcode < 100);

                    energies[r_iter][pump_iter] += instant_energy * delta_t;
                    ++pump_iter;
                }
            }
        }

        // get ready for the next step
        t += delta_t;
    } while (delta_t > 0);

    errorcode = EN_closeH(ph_);
    assert(errorcode < 100);

    if (solution_has_failed)
        return results;

    // if the solution is correct, I return the whole results
    results.push_back(pressures);
    results.push_back(flows);
    results.push_back(energies);

    return results;
}

void WaterDistributionSystem::add_subnetwork(const std::filesystem::path& subnetwork_filename) {
    // simply a wrapper as all chekc operations are done inside the class
    _subnetworks_.push_back(Subnetwork(subnetwork_filename));
}

} /* namespace bevarmejo */
