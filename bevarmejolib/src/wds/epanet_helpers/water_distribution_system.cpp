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

#include "bevarmejo/bemexcept.hpp"
#include "bevarmejo/library_metadata.hpp"

#include "bevarmejo/io/streams.hpp"

#include "bevarmejo/utility/registry.hpp"
#include "bevarmejo/utility/registry_view.hpp"

#include "water_distribution_system.hpp"

namespace bevarmejo::wds
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
    m__times(),
    m__config_options()
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

void WaterDistributionSystem::load_EN_time_settings(EN_Project ph)
{
    time_t a_time= 0l;
    int errorcode= EN_gettimeparam(ph, EN_DURATION, &a_time);
    assert(errorcode < 100);
    m__times.duration__s(a_time);

    errorcode= EN_gettimeparam(ph, EN_STARTTIME, &a_time);
    assert(errorcode < 100);
    m__times.shift_start_time__s(a_time);


    // Prepare for the inputs that are patterns
    epanet::PatternTimeOptions pto;
    errorcode= EN_gettimeparam(ph, EN_PATTERNSTEP, &a_time);
    assert(errorcode < 100);
    pto.timestep__s= a_time;

    errorcode= EN_gettimeparam(ph, EN_PATTERNSTART, &a_time);
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

void WaterDistributionSystem::load_EN_curves(EN_Project ph)
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

        auto get_curve_ptr = [this, curve_id](auto irs) -> std::shared_ptr<Curve>
        {
            if (irs.inserted)
                return irs.iterator.operator->();

            if (irs.iterator != m__aux_elements_.curves.end())
            { // Not inserted because already existing. 
                io::stream_out(std::cout, 
                    "Curve with ID \""+std::string(curve_id)+"\" already exists in the network.\n");

                return nullptr;
            }

            // Insertion failed for other reasons an withouth throwing...
            return nullptr;
        };

        std::shared_ptr<Curve> p_curve;
        switch (curve_type)
        {
            case EN_GENERIC_CURVE:
                io::stream_out(std::cout, 
                    "Curve with ID \""+std::string(curve_id)+"\" is a generic curve and will not link to anything.\n");
                p_curve = get_curve_ptr(m__aux_elements_.curves.emplace<GenericCurve>(curve_id, curve_id));
                break;

            case EN_VOLUME_CURVE:
                p_curve = get_curve_ptr(m__aux_elements_.curves.emplace<VolumeCurve>(curve_id, curve_id));
                break;

            case EN_PUMP_CURVE:
                p_curve = get_curve_ptr(m__aux_elements_.curves.emplace<PumpCurve>(curve_id, curve_id));
                break;

            case EN_EFFIC_CURVE:
                p_curve = get_curve_ptr(m__aux_elements_.curves.emplace<EfficiencyCurve>(curve_id, curve_id));
                break;

            case EN_HLOSS_CURVE:
                p_curve = get_curve_ptr(m__aux_elements_.curves.emplace<HeadlossCurve>(curve_id, curve_id));
                break;

            default:
                throw std::runtime_error("Unknown curve type\n");
        }
        if (valid(p_curve))
        {
            // Actually load the curve data
            p_curve->retrieve_index(ph_);
            p_curve->retrieve_EN_properties(ph_);
        }
        else // it was already in (and I printed the message in the lambda)
            continue;
    }
}

void WaterDistributionSystem::load_EN_patterns(EN_Project ph)
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

        auto irs = m__aux_elements_.patterns.emplace(pattern_id, pattern_id);
        if (irs.inserted)
        {
            // Actually load the pattern data
            irs.iterator->retrieve_index(ph_);
            irs.iterator->retrieve_EN_properties(ph_);
        }
        else // it was already in (I need to print the message this time)
            io::stream_out(std::cout, 
                "Pattern with ID \""+std::string(pattern_id)+"\" already exists in the network.\n");
    }
}

void WaterDistributionSystem::load_EN_nodes(EN_Project ph)
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

        auto insert_ele_in_cont = [this, node_id](auto& container) -> bool
        {
            // Insert in nodes with the mapped type of the container (e.g. Junction)
            // then if it worked insert in the specific container.
            // double node_id because my SystemElements still need the id...
            using mapped_type = typename std::decay_t<decltype(container)>::mapped_type;
            auto irtn = _nodes_.emplace<mapped_type>(node_id, node_id, *this);
            if (!irtn.inserted)
                return false;

            auto irs = container.insert(node_id, std::static_pointer_cast<mapped_type>(irtn.iterator.operator->()));
            if (irs.inserted)
                return true;
            
            // If we are here it is a weird situation, because we could insert in nodes, 
            // but not in the specific container. This could happen if there is no more memory
            // or there containers are not in sync (the element still appears in the container
            // but not in the nodes).
            __format_and_throw<std::logic_error>("WaterDistributionSystem", "load_EN_nodes()", "Impossible to insert the element.",
                "The element with ID \""+std::string(node_id)+"\" could not be inserted in the specific container.",
                "Either therere is no more memory or the containers lost sync.");
        };

        bool f__inserted = false;
        switch (node_type)
        {
            case EN_JUNCTION:
                f__inserted = insert_ele_in_cont(_junctions_);
                break;

            case EN_RESERVOIR:
                f__inserted = insert_ele_in_cont(_reservoirs_);
                break;
            
            case EN_TANK:
                f__inserted = insert_ele_in_cont(_tanks_);
                break;

            default:
                throw std::runtime_error("Unknown node type\n");
        }
        if (f__inserted)
        {
            // Actually load the node data
            // I could use the iterator to get the pointer to the element (as done for patterns), but it complicates
            // the code because of the return type of the insert method and the fact that it 
            // returns an iterator which I have not made default constructible.
            auto p_node = _nodes_.get(node_id);
            p_node->retrieve_index(ph_);
            p_node->retrieve_EN_properties(ph_);
        }
        else // it was already in
            io::stream_out(std::cout, 
                "WaterDistributionSystem::load_EN_nodes() : Impossible to insert the element with ID: \"",
                 node_id, "\". The element is already in the nodes container.\n");
    }
}

void WaterDistributionSystem::load_EN_links(EN_Project ph)
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

        
        auto insert_ele_in_cont = [this, link_id](auto& container) -> bool
        {
            // See load_EN_nodes for the explanation of this function.
            using mapped_type = typename std::decay_t<decltype(container)>::mapped_type;
            auto irtn = _links_.emplace<mapped_type>(link_id, link_id, *this);
            if (!irtn.inserted)
                return false;

            auto irs = container.insert(link_id, std::static_pointer_cast<mapped_type>(irtn.iterator.operator->()));
            if (irs.inserted)
                return true;

            __format_and_throw<std::logic_error>("WaterDistributionSystem", "load_EN_links()", "Impossible to insert the element.",
                "The element with ID \""+std::string(link_id)+"\" could not be inserted in the specific container.",
                "Either therere is no more memory or the containers lost sync.");
        };

        bool f__inserted = false;
        switch (link_type)
        {
            case EN_PIPE:
                f__inserted = insert_ele_in_cont(_pipes_);
                break;

            case EN_PUMP:
                f__inserted = insert_ele_in_cont(_pumps_);
                break;

            default:
                throw std::runtime_error("Unknown link type\n");
        }
        if (f__inserted)
        {
            // Actually load the link data
            auto p_link = _links_.get(link_id);
            p_link->retrieve_index(ph_);
            p_link->retrieve_EN_properties(ph_);
        }
        else // it was already in
            io::stream_out(std::cout, 
                "WaterDistributionSystem::load_EN_links() : Impossible to insert the element with ID: \"",
                 link_id, "\". The element is already in the links container.\n");
    }
}

void WaterDistributionSystem::load_EN_controls(EN_Project ph)
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

void WaterDistributionSystem::load_EN_rules(EN_Project ph)
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

void WaterDistributionSystem::cache_indices() const
{
    auto cache_index = [this](auto& container)
    {
        for (auto& [id, element] : container)
            element.retrieve_index(ph_);
    };

    cache_index(m__aux_elements_.patterns);
    cache_index(m__aux_elements_.curves);
    cache_index(_nodes_);
    cache_index(_links_);
}

} // namespace bevarmejo::wds
