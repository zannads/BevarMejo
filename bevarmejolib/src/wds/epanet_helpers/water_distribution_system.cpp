#include <cassert>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/io.hpp"

#include "water_distribution_system.hpp"

namespace bevarmejo {
namespace wds {

WaterDistributionSystem::WaterDistributionSystem(const std::filesystem::path& inp_file, std::function<void (EN_Project)> preprocessf) :
    ph_(nullptr),
    _inp_file_(inp_file),
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
    m__time_series_map()
    {
        assert(!inp_file.empty());

        int errorcode = EN_createproject(&ph_);
        assert(errorcode<100);
        
        errorcode = EN_open(ph_, inp_file.c_str(), "", ""); // with '\0' doesn't work. WHy?
        if (errorcode>100){
            EN_deleteproject(ph_);

            std::ostringstream error_message;
            io::stream_out(error_message,
                "Error opening the EPANET \".inp\" file\n",
                "\tFilename: ", inp_file, 
                "\n\tError code: ", errorcode
            );
            throw std::runtime_error(error_message.str());
        }

        errorcode = EN_setreport(ph_, "SUMMARY NO");
        errorcode = EN_setreport(ph_, "STATUS NO");
        errorcode = EN_setreport(ph_, "MESSAGES NO");

        // Do everything you need to do on your Project before loading the network
        if (preprocessf)
            preprocessf(ph_);

        // There is a specific order in which to load stuff from the EPANET project, because we need to
        // create the elements in the right order as some of them depend on others (e.g. Links depend on Nodes).

        // 1.0 Load fundamental time information and options
        // 1.1 Load time information
        this->load_EN_time_settings(ph_);

        // 1.2 Allocate space for the TimeSeries objects
        // Since I loaded the time options for simulation and the pattern, I can
        // already create the necessary TimeSeries objects.
        m__time_series_map.emplace("Constant", aux::TimeSeries(m__config_options.times.global));

        m__time_series_map.emplace("Pattern", aux::TimeSeries(m__config_options.times.global));
        aux::time_t currt= m__config_options.times.pattern.shift_start_time__s;
        while (currt < m__config_options.times.global.duration__s()) {
            m__time_series_map.at("Pattern").commit(currt);
            currt += m__config_options.times.pattern.timestep__s;
        }

        m__time_series_map.emplace("Results", aux::TimeSeries(m__config_options.times.global));
        // It will be the hydraulic solver that allocates the memory for the results.

        // 1.3 Load analysis options
        // TODO: this->load_EN_analysis_options(ph_);


        // 2.0 Load the auxiliary EPANET elements
        this->load_EN_curves(ph_);
        this->load_EN_patterns(ph_);
        
        // 3.0 Load the network
        this->load_EN_nodes(ph_);
        this->load_EN_links(ph_);

        // TODO: 4.0 Load the rest of the elements
        this->load_EN_controls(ph_);
        this->load_EN_rules(ph_);

        // Nodes have pointers to demands and to links. Links have pointers to nodes. 
        // I couldn't create this relationships inside the element as it has no idea of the other elements.
        // So I create them here.
        assign_demands_EN();
        // assign_patterns_EN();
        assign_curves_EN();
        connect_network_EN();
        
        return;
    }

void WaterDistributionSystem::load_EN_time_settings(EN_Project ph) {

    aux::time_t a_time= 0l;
    int errorcode= EN_gettimeparam(ph, EN_DURATION, &a_time);
    assert(errorcode < 100);
    m__config_options.times.global.duration__s(a_time);

    errorcode= EN_gettimeparam(ph, EN_STARTTIME, &a_time);
    assert(errorcode < 100);
    // TODO: m__config_options.times.global.start_time__s(a_time);

    errorcode= EN_gettimeparam(ph, EN_PATTERNSTEP, &a_time);
    assert(errorcode < 100);
    m__config_options.times.pattern.timestep__s= a_time;
    assert(a_time > 0);

    errorcode= EN_gettimeparam(ph, EN_PATTERNSTART, &a_time);
    assert(errorcode < 100);
    m__config_options.times.pattern.shift_start_time__s= a_time;

    // What we don't care for now is:
    // EN_HYDSTEP (1)
    // EN_QUALSTEP (1)
    // EN_REPORTSTEP (2)
    // EN_REPORTSTART (2)
    // EN_RULESTEP (3)
    // EN_STATISTIC (2)
    // The read only ones:
    // EN_PERIODS, EN_HTIME, EN_QTIME, EN_HALTFLAG, EN_NEXTEVENT, EN_NEXTEVENTTANK
}

void WaterDistributionSystem::load_EN_curves(EN_Project ph) {
    int n_curves= 0;
    int errorcode = EN_getcount(ph_, EN_CURVECOUNT, &n_curves);
    assert(errorcode < 100);
    _elements_.reserve(n_curves);
    m__aux_elements_.curves.reserve(n_curves);

    for (int i= 1; i <= n_curves; ++i) {
        char curve_id[EN_MAXID+1];
        errorcode = EN_getcurveid(ph_, i, curve_id);
        assert(errorcode < 100);

        int curve_type;
        errorcode = EN_getcurvetype(ph_, i, &curve_type);
        assert(errorcode < 100);

        std::shared_ptr<Curve> p_curve;

        switch (curve_type) {
            case EN_GENERIC_CURVE:
                p_curve= std::make_shared<GenericCurve>(curve_id);
                // TODO: warning std::cout << "Curve with ID \""+curve_id+"\" is a generic curve and will not link to anything.\n";
                break;

            case EN_VOLUME_CURVE:
                p_curve= std::make_shared<VolumeCurve>(curve_id);
                break;

            case EN_PUMP_CURVE:
                p_curve= std::make_shared<PumpCurve>(curve_id);
                break;

            case EN_EFFIC_CURVE:
                p_curve= std::make_shared<EfficiencyCurve>(curve_id);
                break;

            case EN_HLOSS_CURVE:
                p_curve= std::make_shared<HeadlossCurve>(curve_id);
                break;

            default:
                throw std::runtime_error("Unknown curve type\n");
        }
        // Add the data of the curve and save it in curves too
        p_curve->retrieve_index(ph_);
        p_curve->retrieve_EN_properties(ph_);

        m__aux_elements_.curves.insert(p_curve);
        _elements_.push_back(std::static_pointer_cast<Element>(p_curve));
    }
}

void WaterDistributionSystem::load_EN_patterns(EN_Project ph) {
    int n_patterns= 0;
    int errorcode = EN_getcount(ph_, EN_PATCOUNT, &n_patterns);
    assert(errorcode < 100);
    _elements_.reserve(n_patterns);
    m__aux_elements_.patterns.reserve(n_patterns);

    for (int i = 1; i <= n_patterns; ++i) {
        char pattern_id[EN_MAXID+1];
        errorcode = EN_getpatternid(ph_, i, pattern_id);
        assert(errorcode < 100);

        auto p_pattern= std::make_shared<Pattern>(pattern_id);

        // Actually load the pattern data
        p_pattern->retrieve_index(ph_);
        p_pattern->retrieve_EN_properties(ph_);

        m__aux_elements_.patterns.insert(p_pattern);    
        _elements_.push_back(std::static_pointer_cast<Element>(p_pattern));
    }
}

void WaterDistributionSystem::load_EN_nodes(EN_Project ph) {
    int n_nodes= 0;
    int errorcode = EN_getcount(ph_, EN_NODECOUNT, &n_nodes);
    assert(errorcode < 100);
    _elements_.reserve(n_nodes);
    _nodes_.reserve(n_nodes);
     
    for (int i = 1; i <= n_nodes; ++i) {
        char node_id[EN_MAXID + 1];
        errorcode = EN_getnodeid(ph_, i, node_id);
        assert(errorcode < 100);

        int node_type;
        errorcode = EN_getnodetype(ph_, i, &node_type);
        assert(errorcode < 100);

        std::shared_ptr<Node> p_node;

        switch (node_type) {
            case EN_JUNCTION: {
                auto junction = std::make_shared<Junction>(node_id);
                _junctions_.insert(junction);
                p_node= junction;
                break;
            }

            case EN_RESERVOIR: {
                auto reservoir = std::make_shared<Reservoir>(node_id);
                _reservoirs_.insert(reservoir);
                p_node= reservoir;
                break;
            }

            case EN_TANK: {
                auto tank = std::make_shared<Tank>(node_id);
                _tanks_.insert(tank);
                p_node= tank;
                break;
            }

            default:
                throw std::runtime_error("Unknown node type\n");
        }

        p_node->retrieve_index(ph_);
        p_node->retrieve_EN_properties(ph_);
        
        _nodes_.insert(p_node);
        _elements_.push_back(p_node);
    }
}

void WaterDistributionSystem::load_EN_links(EN_Project ph) {
    int n_links= 0;
    int errorcode = EN_getcount(ph_, EN_LINKCOUNT, &n_links);
    assert(errorcode < 100);
    _elements_.reserve(n_links);
    _links_.reserve(n_links);

    for (int i = 1; i <= n_links; ++i) {
        char link_id[EN_MAXID+1];
        errorcode = EN_getlinkid(ph_, i, link_id);
        assert(errorcode < 100);

        int link_type;
        errorcode = EN_getlinktype(ph_, i, &link_type);
        assert(errorcode < 100);

        std::shared_ptr<Link> link;

        switch (link_type) {
            case EN_PIPE:{
                auto pipe= std::make_shared<Pipe>(link_id);

                // Actually load the pipe data
                pipe->retrieve_index(ph_);
                pipe->retrieve_EN_properties(ph_);

                _pipes_.insert(pipe);
                link= pipe;
                break;
            }

            case EN_PUMP: {
                auto pump= std::make_shared<Pump>(link_id);

                // Actually load the pump data
                pump->retrieve_index(ph_);
                pump->retrieve_EN_properties(ph_, m__aux_elements_.patterns, m__aux_elements_.curves);

                _pumps_.insert(pump);
                link= pump;
                break;
            }

            default:
                throw std::runtime_error("Unknown link type\n");
        }
        
        _links_.insert(link);
        _elements_.push_back(link);
    }
}

void WaterDistributionSystem::load_EN_controls(EN_Project ph) {
    int n_controls= 0;
    int errorcode = EN_getcount(ph_, EN_CONTROLCOUNT, &n_controls);
    assert(errorcode < 100);
    for (int i = 1; i <= n_controls; ++i) {
        /*char* control_id = new char[EN_MAXID];
        errorcode = EN_getcontrol(ph_, i, control_id);
        assert(errorcode < 100);

        _elements_.push_back(std::make_shared<control>(control_id));
        delete[] control_id;
        */
        io::stream_out(std::cout, "Controls not implemented yet\n");
    }
}

void WaterDistributionSystem::load_EN_rules(EN_Project ph) {
    int n_rules= 0;
    int errorcode = EN_getcount(ph_, EN_RULECOUNT, &n_rules);
    assert(errorcode < 100);
    for (int i = 1; i <= n_rules; ++i) {
        /*char* rule_id = new char[EN_MAXID];
        errorcode = EN_getrule(ph_, i, rule_id);
        assert(errorcode < 100);

        _elements_.push_back(std::make_shared<rule>(rule_id));
        delete[] rule_id;
        */
        io::stream_out(std::cout, "Rules not implemented yet\n");
    }
}


void WaterDistributionSystem::cache_indices() const {
    for (auto& element : _elements_) {
        element->retrieve_index(ph_);
    }
}

void WaterDistributionSystem::assign_patterns_EN() {
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

        auto it = m__aux_elements_.patterns.find(pattern_id);
        assert(it != m__aux_elements_.patterns.end());
        
        pump->speed_pattern((*it));
    }
}

void WaterDistributionSystem::assign_demands_EN() {
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

            // Pattern id can be "" if the demand is constant
            if (pattern_id.empty()) {
                junction->add_demand(d_category, d_base_demand, nullptr);
            }
            else {
                // I need to find the pattern in the patterns map (it should be there
                auto it = m__aux_elements_.patterns.find(pattern_id);
                assert(it != m__aux_elements_.patterns.end());

                junction->add_demand(d_category, d_base_demand, *it);
            }
        }
    }
}

void WaterDistributionSystem::assign_curves_EN() {
    // Tanks, pumps and valves have curves inside them, so I need to assign them

    // Pumps have two types of curves: pump curve and efficiency curve
    for (auto& pump : _pumps_) {
        // First the pump curve
        assert(pump->index() > 0);
        double val = 0.0;
        int errco = EN_getlinkvalue(ph_, pump->index(), EN_PUMP_HCURVE, &val);
        assert(errco <= 100);

        if (val != 0.0) {
            char* __curve_id = new char[EN_MAXID];
            errco = EN_getcurveid(ph_, static_cast<int>(val), __curve_id);
            assert(errco <= 100);
            std::string curve_id(__curve_id);
            delete[] __curve_id;

            auto it = m__aux_elements_.curves.find(curve_id);
            assert(it != m__aux_elements_.curves.end());

            pump->pump_curve(std::dynamic_pointer_cast<PumpCurve>(*it) );
        }
        else pump->pump_curve(nullptr);

        // Finally the efficiency curve 
        errco = EN_getlinkvalue(ph_, pump->index(), EN_PUMP_ECURVE, &val);
        assert(errco <= 100);
        
        if (val != 0.0) {
            char* __curve_id = new char[EN_MAXID];
            errco = EN_getcurveid(ph_, static_cast<int>(val), __curve_id);
            assert(errco <= 100);
            std::string curve_id(__curve_id);
            delete[] __curve_id;

            auto it = m__aux_elements_.curves.find(curve_id);
            assert(it != m__aux_elements_.curves.end());

            pump->efficiency_curve(std::dynamic_pointer_cast<EfficiencyCurve>(*it) );
        }
        else pump->efficiency_curve(nullptr);
    }

    for (auto& tank : _tanks_){
        assert(tank->index() > 0);

        double val = 0.0;
        int errco = EN_getnodevalue(ph_, tank->index(), EN_VOLCURVE, &val);
        assert(errco <= 100);

        tank->volume_curve(nullptr);
        if (val == 0.0)
            continue;
        
        char* __curve_id = new char[EN_MAXID+1];
        errco = EN_getcurveid(ph_, static_cast<int>(val), __curve_id);
        assert(errco <= 100);
        std::string curve_id(__curve_id);
        delete[] __curve_id;

        auto it = m__aux_elements_.curves.find(curve_id);
        assert(it != m__aux_elements_.curves.end());

        tank->volume_curve(std::dynamic_pointer_cast<VolumeCurve>(*it) );
    }

    // TODO: valves

}

void WaterDistributionSystem::connect_network_EN() {
    for (auto& link : _links_) {
        // retrieve the old property of the already existing pipe
		assert(link->index() != 0);
        int errorcode = 0;
        int out_node1_idx = 0;
		int out_node2_idx = 0;
		errorcode = EN_getlinknodes(ph_, link->index(), &out_node1_idx, &out_node2_idx);
		assert(errorcode <= 100);

        std::string out_node1_id = epanet::get_node_id(ph_, out_node1_idx);
        std::string out_node2_id = epanet::get_node_id(ph_, out_node2_idx);
        
        // find the nodes in the network
        auto it1 = _nodes_.find(out_node1_id);
        assert(it1 != _nodes_.end());
        auto it2 = _nodes_.find(out_node2_id);
        assert(it2 != _nodes_.end());

        // assign the nodes to the link
        link->start_node((*it1).get());
        link->end_node((*it2).get());

        // assign the link to the nodes
        (*it1)->add_link(link.get());
        (*it2)->add_link(link.get());
    }
}


} // namespace wds
} // namespace bevarmejo