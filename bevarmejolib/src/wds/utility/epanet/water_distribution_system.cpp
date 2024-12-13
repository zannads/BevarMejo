#include <cassert>
#include <filesystem>
namespace fsys = std::filesystem;
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

#include "bevarmejo/utility/bemexcept.hpp"
#include "bevarmejo/utility/library_metadata.hpp"

#include "bevarmejo/io/streams.hpp"

#include "bevarmejo/utility/registry.hpp"
#include "bevarmejo/utility/registry_view.hpp"

#include "water_distribution_system.hpp"

namespace bevarmejo
{

WaterDistributionSystem::WaterDistributionSystem(const fsys::path& inp_file, std::function<void (EN_Project)> preprocessf) :
    ph_(nullptr),
    _inp_file_(inp_file),
    _nodes_(),
    _junctions_(),
    _tanks_(),
    _reservoirs_(),
    _links_(),
    _pumps_(),
    _pipes_(),
    m__aux_elements_(),
    m__id_sequences(),
    m__times()
{
    assert(!inp_file.empty());

    int errorcode = EN_createproject(&ph_);
    assert(errorcode<100);
    
    errorcode = EN_open(ph_, inp_file.c_str(), "", ""); // with '\0' doesn't work. WHy?
    if (errorcode>100){
        EN_deleteproject(ph_);

        beme_throw(std::runtime_error,
            "Impossible to create a Water Distribution System.",
            "Error while constructing from an EPANET project.",
            "Error opening the EPANET project.",
            "Error code: ", errorcode,
            "Filename: ", inp_file);
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
    this->load_EN_time_settings();

    // 1.3 Load analysis options
    // TODO: this->load_EN_analysis_options(ph_);

    // 2.0 Load the auxiliary EPANET elements
    this->load_EN_curves();
    this->load_EN_patterns();
    
    // 3.0 Load the network
    this->load_EN_nodes();
    this->load_EN_links();

    // TODO: 4.0 Load the rest of the elements
    this->load_EN_controls();
    this->load_EN_rules();
    
    return;
}

/*------- Element access -------*/
EN_Project WaterDistributionSystem::ph() const noexcept
{
    return ph_;
}

const fsys::path& WaterDistributionSystem::inp_file() const noexcept
{
    return _inp_file_;
}

/*------- Modifiers -------*/
void WaterDistributionSystem::load_EN_time_settings()
{
    assert(ph_ != nullptr);

    time_t a_time= 0l;
    int errorcode= EN_gettimeparam(ph_, EN_DURATION, &a_time);
    assert(errorcode < 100);
    m__times.duration__s(a_time);

    errorcode= EN_gettimeparam(ph_, EN_STARTTIME, &a_time);
    assert(errorcode < 100);
    m__times.shift_start_time__s(a_time);


    // Prepare for the inputs that are patterns
    epanet::PatternTimeOptions pto;
    errorcode= EN_gettimeparam(ph_, EN_PATTERNSTEP, &a_time);
    assert(errorcode < 100);
    pto.timestep__s= a_time;

    errorcode= EN_gettimeparam(ph_, EN_PATTERNSTART, &a_time);
    assert(errorcode < 100);
    pto.shift_start_time__s= a_time;

    assert(epanet::is_valid_pto(pto));
    time_t curr_t= pto.shift_start_time__s;
    if (curr_t == 0)
        curr_t = pto.timestep__s; // Because I can't commit to a timeSeries starting at 0

    std::vector<time_t> time_steps;
    while (curr_t <= m__times.duration__s())
    {
        time_steps.push_back(curr_t);
        curr_t += pto.timestep__s;
    }

    m__times.create_time_series(label::__EN_PATTERN_TS, time_steps);

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

void WaterDistributionSystem::load_EN_curves()
{
    int n_curves= 0;
    int errorcode = EN_getcount(ph_, EN_CURVECOUNT, &n_curves);
    assert(errorcode < 100);
    m__aux_elements_.curves.reserve(n_curves);

    for (int i = 1; i <= n_curves; ++i)
    {
        char curve_id[EN_MAXID+1];
        errorcode = EN_getcurveid(ph_, i, curve_id);
        assert(errorcode < 100);

        int curve_type;
        errorcode = EN_getcurvetype(ph_, i, &curve_type);
        assert(errorcode < 100);

        switch (curve_type)
        {
            case EN_GENERIC_CURVE:
                io::stream_out(std::cout, 
                    "Curve with ID \""+std::string(curve_id)+"\" is a generic curve and will not link to anything.\n");
                m__aux_elements_.curves.insert(curve_id, wds::GenericCurve::make_from_EN_for(*this, curve_id));
                break;

            case EN_VOLUME_CURVE:
                m__aux_elements_.curves.insert(curve_id, wds::VolumeCurve::make_from_EN_for(*this, curve_id));
                break;

            case EN_PUMP_CURVE:
                m__aux_elements_.curves.insert(curve_id, wds::PumpCurve::make_from_EN_for(*this, curve_id));
                break;

            case EN_EFFIC_CURVE:
                m__aux_elements_.curves.insert(curve_id, wds::EfficiencyCurve::make_from_EN_for(*this, curve_id));
                break;

            case EN_HLOSS_CURVE:
                m__aux_elements_.curves.insert(curve_id, wds::HeadlossCurve::make_from_EN_for(*this, curve_id));
                break;

            default:
                beme_throw(std::runtime_error,
                    "Unknown curve type.",
                    "The curve type is not recognized by the system.",
                    "Curve ID: ", curve_id,
                    "Curve type: ", curve_type);
        }
    }
}

void WaterDistributionSystem::load_EN_patterns()
{
    int n_patterns= 0;
    int errorcode = EN_getcount(ph_, EN_PATCOUNT, &n_patterns);
    assert(errorcode < 100);
    m__aux_elements_.patterns.reserve(n_patterns);

    for (int i = 1; i <= n_patterns; ++i)
    {
        char pattern_id[EN_MAXID+1];
        errorcode = EN_getpatternid(ph_, i, pattern_id);
        assert(errorcode < 100);

        m__aux_elements_.patterns.insert(pattern_id, wds::Pattern::make_from_EN_for(*this, pattern_id));
    }
}

void WaterDistributionSystem::load_EN_nodes()
{
    int n_nodes = 0;
    int errorcode = EN_getcount(ph_, EN_NODECOUNT, &n_nodes);
    assert(errorcode < 100);
    _nodes_.reserve(n_nodes);
     
    for (int i = 1; i <= n_nodes; ++i)
    {
        char node_id[EN_MAXID + 1];
        errorcode = EN_getnodeid(ph_, i, node_id);
        assert(errorcode < 100);

        int node_type;
        errorcode = EN_getnodetype(ph_, i, &node_type);
        assert(errorcode < 100);

        auto insert_ele_in_cont = [this, node_id](auto& container) -> void
        {
            // Insert in nodes with the mapped type of the container (e.g. Junction)
            // then, if it has worked, insert in the specific container.
            using mapped_type = typename std::decay_t<decltype(container)>::mapped_type;
            std::shared_ptr p_node = mapped_type::make_from_EN_for(*this, node_id);

            auto irtn = _nodes_.insert(node_id, std::static_pointer_cast<Node>(p_node));
            if (!irtn.inserted)
            {
                return; // Already in, return with no problems.
            }

            auto irs = container.insert(node_id, p_node);
            if (!irs.inserted)
            {            
                // If we are here it is a weird situation, because we could insert in nodes, 
                // but not in the specific container. This could happen if there is no more memory
                // or there containers are not in sync (the element still appears in the container
                // but not in the nodes).
                beme_throw(std::logic_error,
                    "Impossible to insert the element.",
                    "The element could not be inserted in the specific container (either there is no more memory or the containers lost sync).",
                    "Element name: ", node_id);
            }

            return; // Everything went well.
        };

        switch (node_type)
        {
            case EN_JUNCTION:
                insert_ele_in_cont(_junctions_);
                break;

            case EN_RESERVOIR:
                insert_ele_in_cont(_reservoirs_);
                break;
            
            case EN_TANK:
                insert_ele_in_cont(_tanks_);
                break;

            default:
                beme_throw(std::runtime_error,
                    "Unknown node type.",
                    "The node type is not recognized by the system.",
                    "Node ID: ", node_id,
                    "Node type: ", node_type);
        }
    }
}

void WaterDistributionSystem::load_EN_links()
{
    int n_links = 0;
    int errorcode = EN_getcount(ph_, EN_LINKCOUNT, &n_links);
    assert(errorcode < 100);
    _links_.reserve(n_links);

    for (int i = 1; i <= n_links; ++i)
    {
        char link_id[EN_MAXID+1];
        errorcode = EN_getlinkid(ph_, i, link_id);
        assert(errorcode < 100);

        int link_type;
        errorcode = EN_getlinktype(ph_, i, &link_type);
        assert(errorcode < 100);

        
        auto insert_ele_in_cont = [this, link_id](auto& container) -> void
        {
            // See load_EN_nodes for the explanation of this function.
            using mapped_type = typename std::decay_t<decltype(container)>::mapped_type;
            std::shared_ptr p_link  = mapped_type::make_from_EN_for(*this, link_id);
            
            auto irtn = _links_.insert(link_id, std::static_pointer_cast<Link>(p_link));
            if (!irtn.inserted)
            {
                return;
            }
            
            auto irs = container.insert(link_id, p_link);
            if (!irs.inserted)
            {
                beme_throw(std::logic_error,
                "Impossible to insert the element.",
                "The element could not be inserted in the specific container (either there is no more memory or the containers lost sync).",
                "Element name: ", link_id);
            }

            return;
        };

        switch (link_type)
        {
            case EN_PIPE:
                insert_ele_in_cont(_pipes_);
                break;

            case EN_PUMP:
                insert_ele_in_cont(_pumps_);
                break;

            default:
                beme_throw(std::runtime_error,
                    "Unknown link type.",
                    "The link type is not recognized by the system.",
                    "Link ID: ", link_id,
                    "Link type: ", link_type);
        }
    }
}

void WaterDistributionSystem::load_EN_controls()
{
    int n_controls= 0;
    int errorcode = EN_getcount(ph_, EN_CONTROLCOUNT, &n_controls);
    assert(errorcode < 100);
    // Reserve

    for (int i = 1; i <= n_controls; ++i)
    {
        /*
        char control_id[EN_MAXID+1];
        errorcode = EN_getlinkid(ph_, i, control_id);
        assert(errorcode < 100);

        ...
        */
        io::stream_out(std::cout, "Controls not implemented yet\n");
    }
}

void WaterDistributionSystem::load_EN_rules()
{
    int n_rules= 0;
    int errorcode = EN_getcount(ph_, EN_RULECOUNT, &n_rules);
    assert(errorcode < 100);
    for (int i = 1; i <= n_rules; ++i)
    {
        /*
        char rule_id[EN_MAXID+1];
        errorcode = EN_getlinkid(ph_, i, rule_id);
        assert(errorcode < 100);

        ...
        */
        io::stream_out(std::cout, "Rules not implemented yet\n");
    }
}

void WaterDistributionSystem::cache_indices()
{
    auto cache_index = [this](auto& container)
    {
        for (auto&& [id, element] : container)
            element.retrieve_EN_index();
    };

    cache_index(m__aux_elements_.patterns);
    cache_index(m__aux_elements_.curves);
    cache_index(_nodes_);
    cache_index(_links_);
}

} // namespace bevarmejo
