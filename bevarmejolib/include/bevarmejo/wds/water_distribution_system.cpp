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

water_distribution_system::water_distribution_system() :
    ph_(nullptr),
    _inp_file_(),
    _elements_(),
    _nodes_(),
    _links_(),
    _patterns_(),
    _junctions_(),
    _tanks_(),
    _reservoirs_(),
    _pipes_(),
    _pumps_(),
    _subnetworks_(),
    _groups_() 
    { }

water_distribution_system::water_distribution_system(const std::filesystem::path& inp_file) :
    ph_(nullptr),
    _inp_file_(inp_file),
    _elements_(),
    _nodes_(),
    _links_(),
    _patterns_(),
    _junctions_(),
    _tanks_(),
    _reservoirs_(),
    _pipes_(),
    _pumps_(),
    _subnetworks_(),
    _groups_()
    {
        init();
    }

water_distribution_system::~water_distribution_system(){
    if (ph_!=nullptr){
        EN_close(ph_);
        EN_deleteproject(ph_);
        
        ph_ = nullptr;
    }
}

void water_distribution_system::add_subnetwork(const std::string& name, const Subnetwork& subnetwork){
    _subnetworks_.emplace(std::make_pair(name, subnetwork));
}

void water_distribution_system::add_subnetwork(const std::pair<std::string, Subnetwork>& subnetwork){
    _subnetworks_.insert(subnetwork);
}

void water_distribution_system::add_subnetwork(const std::filesystem::path &filename) {
    add_subnetwork( load_egroup_from_file<NetworkElement>(filename) );
}

Subnetwork& water_distribution_system::subnetwork(const std::string& name) {
    auto it = _subnetworks_.find(name);
    if (it != _subnetworks_.end())
        return it->second;
    else
        throw std::runtime_error("Subnetwork with name " + name + " not found.");
}

const Subnetwork& water_distribution_system::subnetwork(const std::string& name) const {
    auto it = _subnetworks_.find(name);
    if (it != _subnetworks_.end())
        return it->second;
    else
        throw std::runtime_error("Subnetwork with name " + name + " not found.");
}

void water_distribution_system::remove_subnetwork(const std::string& name){
    auto it = _subnetworks_.find(name);
    if (it != _subnetworks_.end())
        _subnetworks_.erase(it);
    // else no problem, it's not there
}

void water_distribution_system::init(){
    assert(!_inp_file_.empty());
    
    int errorcode = EN_createproject(&ph_);
    assert(errorcode<100);
    
    errorcode = EN_open(ph_, _inp_file_.c_str(), "", ""); // with '\0' doesn't work. WHy?
    if (errorcode>100){
        EN_deleteproject(ph_);
        std::string error_message = "Error opening the .inp file with code: ";
        error_message.append(std::to_string(errorcode));
        throw std::runtime_error(error_message);
    }
    
    errorcode = EN_setreport(ph_, "MESSAGES NO");

    // Populate the elements vector
    // [1/6] Nodes
    int n_nodes;
    errorcode = EN_getcount(ph_, EN_NODECOUNT, &n_nodes);
    assert(errorcode < 100);
    _elements_.reserve(n_nodes);
    _nodes_.reserve(n_nodes);
     
    for (int i = 1; i <= n_nodes; ++i) {
        char* node_id = new char[EN_MAXID];
        errorcode = EN_getnodeid(ph_, i, node_id);
        assert(errorcode < 100);

        int node_type;
        errorcode = EN_getnodetype(ph_, i, &node_type);
        assert(errorcode < 100);

        if (node_type == EN_JUNCTION){
            _elements_.push_back(std::make_shared<Junction>(node_id));

            // Save it in _junctions_ too
            _junctions_.insert(std::dynamic_pointer_cast<Junction>(_elements_.back()));
        }
        else if (node_type == EN_RESERVOIR){
            _elements_.push_back(std::make_shared<Reservoir>(node_id));
            
            // Save it in _reservoirs_ too
            _reservoirs_.insert(std::dynamic_pointer_cast<Reservoir>(_elements_.back()));
        }
        else if (node_type == EN_TANK){
            _elements_.push_back(std::make_shared<Tank>(node_id));

            // Save it in _tanks_ too
            _tanks_.insert(std::dynamic_pointer_cast<Tank>(_elements_.back()));
        }
        else {
            throw std::runtime_error("Unknown node type\n");
        }
        delete[] node_id;

        // Save it in _nodes_ too
        _nodes_.insert(std::dynamic_pointer_cast<Node>(_elements_.back()));
    }

    // [2/6] Links
    int n_links;
    errorcode = EN_getcount(ph_, EN_LINKCOUNT, &n_links);
    assert(errorcode < 100);
    _elements_.reserve(n_links);
    _links_.reserve(n_links);

    for (int i = 1; i <= n_links; ++i) {
        char* link_id = new char[EN_MAXID];
        errorcode = EN_getlinkid(ph_, i, link_id);
        assert(errorcode < 100);

        int link_type;
        errorcode = EN_getlinktype(ph_, i, &link_type);
        assert(errorcode < 100);

        if (link_type == EN_PIPE) {
            _elements_.push_back(std::make_shared<Pipe>(link_id));

            // Save it in _pipes_ too
            _pipes_.insert(std::dynamic_pointer_cast<Pipe>(_elements_.back()));
        }
        else if (link_type == EN_PUMP) {
            _elements_.push_back(std::make_shared<Pump>(link_id));

            // Save it in _pumps_ too
            _pumps_.insert(std::dynamic_pointer_cast<Pump>(_elements_.back()));
        }
        else { //TODO: all sorts of valves if (link_type == EN_VALVE) {
            //_elements_.push_back(std::make_shared<valve>(link_id));
            stream_out(std::cout, "Valves not implemented yet or unknown link type\n");
        }
        delete[] link_id;

        // Save it in _links_ too
        _links_.insert(std::dynamic_pointer_cast<Link>(_elements_.back()));
    }

    // [3/6] Patterns
    int n_patterns;
    errorcode = EN_getcount(ph_, EN_PATCOUNT, &n_patterns);
    assert(errorcode < 100);
    _elements_.reserve(n_patterns);
    _patterns_.reserve(n_patterns);

    for (int i = 1; i <= n_patterns; ++i) {
        char* pattern_id = new char[EN_MAXID];
        errorcode = EN_getpatternid(ph_, i, pattern_id);
        assert(errorcode < 100);

        _elements_.push_back(std::make_shared<Pattern>(pattern_id));
        delete[] pattern_id;

        // Save it in _patterns_ too
        _patterns_.insert(std::dynamic_pointer_cast<Pattern>(_elements_.back()));
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
    assign_demands_EN();
    assign_patterns_EN();
    // assign_curves_EN();

    return;
}

void water_distribution_system::cache_indices() const {
    for (auto& element : _elements_) {
        element->retrieve_index(ph_);
    }
}

void water_distribution_system::assign_patterns_EN() {
    // Pumps have patterns inside them, so I need to assign them first
    for (auto& pump : _pumps_) {
        int errorcode = 0;
        double val = 0.0;
        errorcode = EN_getlinkvalue(ph_, pump->index(), EN_LINKPATTERN, &val);
        assert(errorcode < 100);

        char* __pattern_id = new char[EN_MAXID];
        errorcode = EN_getpatternid(ph_, static_cast<int>(val), __pattern_id);
        std::string pattern_id(__pattern_id);
        delete[] __pattern_id;

        auto it = _patterns_.find(pattern_id);
        assert(it != _patterns_.end());
        
        pump->speed_pattern((*it));
    }
}

void water_distribution_system::assign_demands_EN() {
    // each junction has a list of demands that add up
    for (auto& junction : _junctions_ ) {
        int errorcode = 0;
        int n_demands = 0;
        errorcode = EN_getnumdemands(ph_, junction->index(), &n_demands);
        assert(errorcode < 100);

        for (int i = 1; i <= n_demands; ++i) {
            double d_base_demand;
            errorcode = EN_getbasedemand(ph_, junction->index(), i, &d_base_demand);
            assert(errorcode < 100);

            int pattern_index;
            errorcode = EN_getdemandpattern(ph_, junction->index(), i, &pattern_index);
            assert(errorcode < 100);
            
            char* __pattern_id = new char[EN_MAXID];
            errorcode = EN_getpatternid(ph_, pattern_index, __pattern_id);
            std::string pattern_id(__pattern_id);
            delete[] __pattern_id;

            char* d_category_ = new char[EN_MAXID];
            errorcode = EN_getdemandname(ph_, junction->index(), i, d_category_);
            assert(errorcode < 100);
            std::string d_category(d_category_);
            delete[] d_category_;

            auto it = _patterns_.find(pattern_id);
            assert(it != _patterns_.end());

            junction->add_demand(d_category, d_base_demand, *it);
        }
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
