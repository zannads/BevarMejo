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
#include "bevarmejo/library_metadata.hpp"

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
    m__times(*m__config_options.times.global)
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
        time_t curr_t= m__config_options.times.pattern.shift_start_time__s;
        aux::TimeSteps time_steps;
        while (curr_t < m__config_options.times.global->duration__s()) {
            time_steps.push_back(curr_t);
            curr_t += m__config_options.times.pattern.timestep__s;
        }
        m__times.EN_pattern= m__config_options.times.global->create_time_series(time_steps);

        // The result object has already been created in the constructor (see m__times)
        // BUT, it will be the hydraulic solver that allocates the memory for the results.
        // This is why it is marked mutable.

        // 1.3 Load analysis options
        // TODO: this->load_EN_analysis_options(ph_);
        if ( VersionManager::user().version() < VersionManager::v(2024,4,0) ) 
            m__config_options.save_all_hsteps = false;
        // else m__config_options.save_all_hsteps = true;

        // 2.0 Load the auxiliary EPANET elements
        this->load_EN_curves(ph_);
        this->load_EN_patterns(ph_);
        
        // 3.0 Load the network
        this->load_EN_nodes(ph_);
        this->load_EN_links(ph_);

        // TODO: 4.0 Load the rest of the elements
        this->load_EN_controls(ph_);
        this->load_EN_rules(ph_);
        
        return;
    }

void WaterDistributionSystem::load_EN_time_settings(EN_Project ph) {

    time_t a_time= 0l;
    int errorcode= EN_gettimeparam(ph, EN_DURATION, &a_time);
    assert(errorcode < 100);
    m__config_options.times.global->duration__s(a_time);

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
    // 1: Will be handled by the solvers and not in the WDS class
    // 2: Completely ignored and discarded, we save everything
    // 3: Not implemented yet
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
                auto junction = std::make_shared<Junction>(node_id, *this);

                _junctions_.insert(junction);
                p_node= junction;
                break;
            }

            case EN_RESERVOIR: {
                auto reservoir = std::make_shared<Reservoir>(node_id, *this);

                _reservoirs_.insert(reservoir);
                p_node= reservoir;
                break;
            }

            case EN_TANK: {
                auto tank = std::make_shared<Tank>(node_id, *this);

                _tanks_.insert(tank);
                p_node= tank;
                break;
            }

            default:
                throw std::runtime_error("Unknown node type\n");
        }

        // Actually load the node data
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
                auto pipe= std::make_shared<Pipe>(link_id, *this);

                _pipes_.insert(pipe);
                link= pipe;
                break;
            }

            case EN_PUMP: {
                auto pump= std::make_shared<Pump>(link_id, *this);

                _pumps_.insert(pump);
                link= pump;
                break;
            }

            default:
                throw std::runtime_error("Unknown link type\n");
        }
        
        // Actually load the link data
        link->retrieve_index(ph_);
        link->retrieve_EN_properties(ph_);

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

} // namespace wds
} // namespace bevarmejo