//
//  water_distribution_system.cpp
//  hanoiOptimization
//
//  Created by Dennis Zanutto on 04/07/23.
//

#include <assert.h>
#include <exception>
#include <filesystem>
#include <iostream>
#include <stdio.h>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "epanet2_2.h"

#include "bevarmejo/io.hpp"

#include "bevarmejo/wds/user_defined_elements_group.hpp"

#include "water_distribution_system.hpp"

namespace bevarmejo {
namespace wds {

WaterDistributionSystem::WaterDistributionSystem() :
    ph_(nullptr),
    _inp_file_(),
    _elements_(),
    _nodes_(),
    _links_(),
    m__aux_elements_(),
    _junctions_(),
    _tanks_(),
    _reservoirs_(),
    _pipes_(),
    _pumps_(),
    _subnetworks_(),
    _groups_(),
    m__config_options(),
    m__times(m__config_options.times.global) { }
    

    WaterDistributionSystem::WaterDistributionSystem(const WaterDistributionSystem& other) :
        ph_(other.ph_),
        _inp_file_(other._inp_file_),
        _elements_(other._elements_),
        _nodes_(other._nodes_),
        _links_(other._links_),
        m__aux_elements_(other.m__aux_elements_),
        _junctions_(other._junctions_),
        _tanks_(other._tanks_),
        _reservoirs_(other._reservoirs_),
        _pipes_(other._pipes_),
        _pumps_(other._pumps_),
        _subnetworks_(other._subnetworks_),
        _groups_(other._groups_),
        m__config_options(other.m__config_options),
        m__times(other.m__times)
        { }

    WaterDistributionSystem::WaterDistributionSystem(WaterDistributionSystem&& other) noexcept :
        ph_(other.ph_),
        _inp_file_(std::move(other._inp_file_)),
        _elements_(std::move(other._elements_)),
        _nodes_(std::move(other._nodes_)),
        _links_(std::move(other._links_)),
        m__aux_elements_(std::move(other.m__aux_elements_)),
        _junctions_(std::move(other._junctions_)),
        _tanks_(std::move(other._tanks_)),
        _reservoirs_(std::move(other._reservoirs_)),
        _pipes_(std::move(other._pipes_)),
        _pumps_(std::move(other._pumps_)),
        _subnetworks_(std::move(other._subnetworks_)),
        _groups_(std::move(other._groups_)),
        m__config_options(std::move(other.m__config_options)),
        m__times(std::move(other.m__times))
        { }

    WaterDistributionSystem& WaterDistributionSystem::operator=(const WaterDistributionSystem& rhs) {
        if (this != &rhs) {
            ph_ = rhs.ph_;
            _inp_file_ = rhs._inp_file_;
            _elements_ = rhs._elements_;
            _nodes_ = rhs._nodes_;
            _links_ = rhs._links_;
            m__aux_elements_ = rhs.m__aux_elements_;
            _junctions_ = rhs._junctions_;
            _tanks_ = rhs._tanks_;
            _reservoirs_ = rhs._reservoirs_;
            _pipes_ = rhs._pipes_;
            _pumps_ = rhs._pumps_;
            _subnetworks_ = rhs._subnetworks_;
            _groups_ = rhs._groups_;
            m__config_options = rhs.m__config_options;
            m__times = rhs.m__times;
        }
        return *this;
    }

    WaterDistributionSystem& WaterDistributionSystem::operator=(WaterDistributionSystem&& rhs) noexcept {
        if (this != &rhs) {
            ph_ = rhs.ph_;
            _inp_file_ = std::move(rhs._inp_file_);
            _elements_ = std::move(rhs._elements_);
            _nodes_ = std::move(rhs._nodes_);
            _links_ = std::move(rhs._links_);
            m__aux_elements_ = std::move(rhs.m__aux_elements_);
            _junctions_ = std::move(rhs._junctions_);
            _tanks_ = std::move(rhs._tanks_);
            _reservoirs_ = std::move(rhs._reservoirs_);
            _pipes_ = std::move(rhs._pipes_);
            _pumps_ = std::move(rhs._pumps_);
            _subnetworks_ = std::move(rhs._subnetworks_);
            _groups_ = std::move(rhs._groups_);
            m__config_options = std::move(rhs.m__config_options);
            m__times = std::move(rhs.m__times);
        }
        return *this;
    }

WaterDistributionSystem::~WaterDistributionSystem(){
    if (ph_!=nullptr){
        EN_close(ph_);
        EN_deleteproject(ph_);
        
        ph_ = nullptr;
        std::cout << "EPANET project deleted\n";
    }

    // First clear all the elements, then the time series and finally the config options
    _subnetworks_.clear();
    _groups_.clear();

    _nodes_.clear();
    _links_.clear();
    _junctions_.clear();
    _tanks_.clear();
    _reservoirs_.clear();
    _links_.clear();
    _pipes_.clear();
    _pumps_.clear();

    m__aux_elements_.patterns.clear();
    m__aux_elements_.curves.clear();

    _elements_.clear();

    m__times.ud_time_series.clear();
    // m__config_options can die in piece
}


std::unique_ptr<WaterDistributionSystem> WaterDistributionSystem::clone() const {
    std::unique_ptr<WaterDistributionSystem> wds_clone = std::make_unique<WaterDistributionSystem>();

    // Clone the elements
    // I start from curves and patterns since the other depende on them
    for (auto& old_curve : m__aux_elements_.curves) {
        std::shared_ptr<Curve> curve_clone = old_curve->clone();
        wds_clone->insert(curve_clone);
    }

    // The nodes can be complitely defined thanks to nodes and patterns, so it's
    // their moment

    // Finally once everything is in place I can copy the links and connect the
    // the network

    for (auto& old_pipe : _pipes_) {
        std::shared_ptr<Pipe> pipe_clone = old_pipe->clone();

        // Get what re the IDS of the nodes to which it was connected.
        std::string start_node_id = old_pipe->from_node()->id();
        std::string end_node_id = old_pipe->to_node()->id();

        // Find the nodes in the new network
        auto it1 = wds_clone->nodes().find(start_node_id);
        assert(it1 != wds_clone->nodes().end());
        auto it2 = wds_clone->nodes().find(end_node_id);
        assert(it2 != wds_clone->nodes().end());

        // Assign the nodes to the pipe
        pipe_clone->start_node((*it1).get());
        pipe_clone->end_node((*it2).get());

        wds_clone->insert(pipe_clone);

        // let's assume it also added the link to the nodes
    }
    return wds_clone;
}

void WaterDistributionSystem::add_subnetwork(const std::string& name, const Subnetwork& subnetwork){
    _subnetworks_.emplace(std::make_pair(name, subnetwork));
}

void WaterDistributionSystem::add_subnetwork(const std::pair<std::string, Subnetwork>& subnetwork){
    _subnetworks_.insert(subnetwork);
}

void WaterDistributionSystem::add_subnetwork(const std::filesystem::path &filename) {
    add_subnetwork( load_egroup_from_file<NetworkElement>(filename) );
}

Subnetwork& WaterDistributionSystem::subnetwork(const std::string& name) {
    auto it = _subnetworks_.find(name);
    if (it != _subnetworks_.end())
        return it->second;
    else
        throw std::runtime_error("Subnetwork with name " + name + " not found.");
}

const Subnetwork& WaterDistributionSystem::subnetwork(const std::string& name) const {
    auto it = _subnetworks_.find(name);
    if (it != _subnetworks_.end())
        return it->second;
    else
        throw std::runtime_error("Subnetwork with name " + name + " not found.");
}

void WaterDistributionSystem::remove_subnetwork(const std::string& name){
    auto it = _subnetworks_.find(name);
    if (it != _subnetworks_.end())
        _subnetworks_.erase(it);
    // else no problem, it's not there
}

void WaterDistributionSystem::clear_results() const {
    for (auto& node: nodes()) {
        node->clear_results();
    }
    for (auto& link: links()) {
        link->clear_results();
    }
}

const aux::TimeSeries& WaterDistributionSystem::time_series(const std::string& name) const {
    if (name == l__CONSTANT_TS)
        return m__times.constant;
    
    if (name == l__PATTERN_TS)
        return m__times.EN_pattern;
    
    if (name == l__RESULT_TS)
        return m__times.results;

    auto it = m__times.ud_time_series.find(name);
    if (it != m__times.ud_time_series.end())
        return it->second;
    else
        throw std::runtime_error("Time series with name " + name + " not found.");
}

void WaterDistributionSystem::run_hydraulics() const{
    this->clear_results();
    m__times.results.reset();

    assert(ph_ != nullptr);
    // I assume indices are cached already 
    int errorcode = EN_openH(ph_);
    if (errorcode >= 100)
        throw std::runtime_error("Hydraulic opening failed."); // I don't think I need to close it here

    errorcode = EN_initH(ph_, 10);
    if (errorcode >= 100) {
        int errorcode2 = EN_closeH(ph_);
        throw std::runtime_error("Hydraulic initialization failed.");
    }

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
    m__times.results.reserve(n_reports);

    bool solution_has_failed = false;
    bool scheduled; // is the current time a reporting time?
    long t{ 0 }; // current time
    long delta_t{ 0 }; // real hydraulic time step
    
    do {
        errorcode = EN_runH(ph_, &t);
        if (errorcode >= 100) {
            solution_has_failed = true;
            break;
            // I don'return because I need to close the hydraulics
        }

        // if the current time is a reporting time, I save all the results
        scheduled = (t % r_step == 0);
        if (m__config_options.save_all_hsteps || scheduled) {
            m__times.results.commit(t);
            // Use polymorphism to get the results from EPANET
            for (auto node : _nodes_) {
                node->retrieve_results(ph_, t);
            }
            for (auto link : _links_) {
                link->retrieve_results(ph_, t);
            }
        }

        errorcode = EN_nextH(ph_, &delta_t);
        assert(errorcode < 100);

    } while (delta_t > 0);

    errorcode = EN_closeH(ph_);
    assert(errorcode < 100);

    if (solution_has_failed)
        throw std::runtime_error("Hydraulic solution failed.");
}

} // namespace wds
} // namespace bevarmejo
