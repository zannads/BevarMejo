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

#include "io.hpp"

#include "water_distribution_system.hpp"

namespace bevarmejo {
namespace wds {

water_distribution_system::water_distribution_system(){
    ph_ = nullptr;
}

//water_distribution_system::water_distribution_system(const std::filesystem::path& inp_path){};

water_distribution_system::water_distribution_system(const std::string& inp_filename){
    this->set_inpfile(inp_filename);
    
    this->init();
}

// Copy constructor
// this is not actually a copy constructor but rather a reinitialization one because I'm starting drom the inp file every time.
water_distribution_system::water_distribution_system(const water_distribution_system &src){
    _inp_filename_ = src._inp_filename_;
    
    init();

    _subnetworks_ = src._subnetworks_;
}

water_distribution_system::water_distribution_system(water_distribution_system &&src) noexcept{
    _inp_filename_ = std::move(src._inp_filename_);

    ph_ = src.ph_;
    src.ph_ = nullptr;

    _elements_ = std::move(src._elements_);
    _subnetworks_ = std::move(src._subnetworks_);
}

water_distribution_system& water_distribution_system::operator=(const water_distribution_system& rhs) {
    if (this != &rhs){
        water_distribution_system temp(rhs);
        ph_ = temp.ph_;
        temp.ph_ = nullptr;
        std::swap(_inp_filename_, temp._inp_filename_);
        std::swap(_elements_, temp._elements_);
        std::swap(_subnetworks_, temp._subnetworks_);
    }
    return *this;
}

water_distribution_system& water_distribution_system::operator=(water_distribution_system &&rhs) noexcept {
    ph_ = rhs.ph_;
    rhs.ph_ = nullptr;
    
    _inp_filename_ = std::move(rhs._inp_filename_);
    _elements_ = std::move(rhs._elements_);
    _subnetworks_ = std::move(rhs._subnetworks_);
    return *this;
}

water_distribution_system::~water_distribution_system(){
    if (ph_!=nullptr){
        EN_close(ph_);
        EN_deleteproject(ph_);
        
        ph_ = nullptr;
    }
}

void water_distribution_system::init(){
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

    // Populate the elements vector
    int n_nodes;
    errorcode = EN_getcount(ph_, EN_NODECOUNT, &n_nodes);
    assert(errorcode < 100);
    int n_links;
    errorcode = EN_getcount(ph_, EN_LINKCOUNT, &n_links);
    assert(errorcode < 100);
    _elements_.reserve(n_nodes + n_links);
    
    // [1/6] Nodes 
    for (int i = 1; i <= n_nodes; ++i) {
        char* node_id = new char[EN_MAXID];
        errorcode = EN_getnodeid(ph_, i, node_id);
        assert(errorcode < 100);

        int node_type;
        errorcode = EN_getnodetype(ph_, i, &node_type);
        assert(errorcode < 100);

        if (node_type == EN_JUNCTION){
            _elements_.push_back(std::make_shared<junction>(node_id));
        }
        else if (node_type == EN_RESERVOIR){
            // TODO: _elements_.push_back(std::make_shared<reservoir>(node_id));
            stream_out(std::cout, "Reservoirs not implemented yet\n");
        }
        else if (node_type == EN_TANK){
            _elements_.push_back(std::make_shared<tank>(node_id));
        }
        else {
            throw std::runtime_error("Unknown node type\n");
        }
        delete[] node_id;

        // Save it in _nodes_ too
        _nodes_.push_back(std::dynamic_pointer_cast<node>(_elements_.back()));
    }

    // [2/6] Links
    for (int i = 1; i <= n_links; ++i) {
        char* link_id = new char[EN_MAXID];
        errorcode = EN_getlinkid(ph_, i, link_id);
        assert(errorcode < 100);

        int link_type;
        errorcode = EN_getlinktype(ph_, i, &link_type);
        assert(errorcode < 100);

        if (link_type == EN_PIPE) {
            _elements_.push_back(std::make_shared<pipe>(link_id));
        }
        else if (link_type == EN_PUMP) {
            // TODO: _elements_.push_back(std::make_shared<pump>(link_id));
            stream_out(std::cout, "Pumps not implemented yet\n");
        }
        else { //TODO: all sorts of valves if (link_type == EN_VALVE) {
            //_elements_.push_back(std::make_shared<valve>(link_id));
            stream_out(std::cout, "Valves not implemented yet or unknown link type\n");
        }
        delete[] link_id;

        // Save it in _links_ too
        _links_.push_back(std::dynamic_pointer_cast<link>(_elements_.back()));
    }

    // [3/6] Patterns
    int n_patterns;
    errorcode = EN_getcount(ph_, EN_PATCOUNT, &n_patterns);
    assert(errorcode < 100);
    for (int i = 1; i <= n_patterns; ++i) {
        char* pattern_id = new char[EN_MAXID];
        errorcode = EN_getpatternid(ph_, i, pattern_id);
        assert(errorcode < 100);

        _elements_.push_back(std::make_shared<pattern>(pattern_id));
        delete[] pattern_id;
    }

    // [4/6] Curves
    int n_curves;
    errorcode = EN_getcount(ph_, EN_CURVECOUNT, &n_curves);
    assert(errorcode < 100);
    for (int i = 1; i <= n_curves; ++i) {
        /*char* curve_id = new char[EN_MAXID];
        errorcode = EN_getcurveid(ph_, i, curve_id);
        assert(errorcode < 100);

        _elements_.push_back(std::make_shared<curve>(curve_id));
        delete[] curve_id;
        */
        stream_out(std::cout, "Curves not implemented yet\n");
    }

    // [5/6] Controls (simple control)
    int n_controls;
    errorcode = EN_getcount(ph_, EN_CONTROLCOUNT, &n_controls);
    assert(errorcode < 100);
    for (int i = 1; i <= n_controls; ++i) {
        /*char* control_id = new char[EN_MAXID];
        errorcode = EN_getcontrol(ph_, i, control_id);
        assert(errorcode < 100);

        _elements_.push_back(std::make_shared<control>(control_id));
        delete[] control_id;
        */
        stream_out(std::cout, "Controls not implemented yet\n");
    }

    // [6/6] Rules (control rules)
    int n_rules;
    errorcode = EN_getcount(ph_, EN_RULECOUNT, &n_rules);
    assert(errorcode < 100);
    for (int i = 1; i <= n_rules; ++i) {
        /*char* rule_id = new char[EN_MAXID];
        errorcode = EN_getrule(ph_, i, rule_id);
        assert(errorcode < 100);

        _elements_.push_back(std::make_shared<rule>(rule_id));
        delete[] rule_id;
        */
        stream_out(std::cout, "Rules not implemented yet\n");
    }

    // Fill the data from EPANET. Use polymorphism in the right way :)
    for (auto& element : _elements_) {
        element->retrieve_index(ph_);
        element->retrieve_properties(ph_);
    }

    // Nodes have pointers to demands and to links. Links have pointers to nodes. 
    // I couldn't create this relationships inside the element as it has no idea of the other elements.
    // So I create them here.
    // connect_network(ph_);
    // assign_patterns(ph_);

}

void water_distribution_system::set_inpfile(const std::string inp_filename){
    _inp_filename_ = inp_filename;
}

std::string water_distribution_system::get_inpfile() const{
    return _inp_filename_;
}

void water_distribution_system::cache_indices() const {
    for (auto& element : _elements_) {
        element->retrieve_index(ph_);
    }
}

void water_distribution_system::run_hydraulics() const{
    // I assume indices are cached already 
    int errorcode = EN_openH(ph_);
    if (errorcode >= 100)
        return; // I don't think I need to close it here

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
        if (scheduled) {
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

void water_distribution_system::add_subnetwork(const std::filesystem::path& subnetwork_filename) {
    // simply a wrapper as all chekc operations are done inside the class
    _subnetworks_.insert(subnetwork(subnetwork_filename));
}

bevarmejo::wds::subnetwork water_distribution_system::get_subnetwork(const std::string &name) const
{
    auto subnet_it = _subnetworks_.find(bevarmejo::wds::subnetwork(name));
    if (subnet_it == _subnetworks_.end())
        throw std::runtime_error("Subnetwork not found");
    return *subnet_it;
}

bool water_distribution_system::is_in_subnetork(const std::string &name, const std::string &id) const {
    bool found = false;
    
    auto subnet_it = _subnetworks_.find(bevarmejo::wds::subnetwork(name));
    if (subnet_it == _subnetworks_.end())
        throw std::runtime_error("Subnetwork not found");
    for (std::size_t i = 0; i < subnet_it->size(); ++i){
        if (subnet_it->at(i) == id){
            found = true;
            break;
        }
    }

    return found;
}

std::string water_distribution_system::get_node_id(int index) const {
    char* node_id = new char[EN_MAXID];
    int errorcode = EN_getnodeid(ph_, index, node_id);
    assert(errorcode <= 100);
    
    std::string node_id_str(node_id);
    delete[] node_id;

    return node_id_str;
}

} // namespace wds
} // namespace bevarmejo 