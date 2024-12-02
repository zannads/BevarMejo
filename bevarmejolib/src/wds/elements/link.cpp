#include <cassert>
#include <string>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/wds/epanet_helpers/en_help.hpp"

#include "bevarmejo/bemexcept.hpp"

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "link.hpp"

namespace bevarmejo::wds
{

/*------- Member functions -------*/
// (constructor)
Link::Link(const WaterDistributionSystem& wds, const EN_Name_t& name) : 
    inherited(wds, name),
    m__from_node(nullptr),
    m__to_node(nullptr),
    m__initial_status(wds.time_series(label::__CONSTANT_TS)),
    m__flow(wds.time_series(label::__RESULTS_TS))
{ }

/*------- Operators -------*/

/*------- Element access -------*/

/*------- Capacity -------*/

/*------- Modifiers -------*/
void Link::clear_results()
{
    inherited::clear_results();

    m__flow.clear();
}

void Link::retrieve_EN_index()
{
    assert(m__wds.ph() != nullptr);

    int en_index = 0;
    int errorcode = EN_getlinkindex(m__wds.ph(), m__name.c_str(), &en_index);
    if (errorcode > 100) 
        __format_and_throw<std::runtime_error>("Link", "retrieve_EN_index", "Error retrieving index of link.",
            "Error code: ", errorcode,
            "Link ID: ", m__name);

    m__en_index = en_index;
}

void Link::retrieve_EN_properties()
{
    this->__retrieve_EN_properties();

    inherited::retrieve_EN_properties();
}

void Link::__retrieve_EN_properties()
{
    assert(m__en_index != 0);
    assert(m__wds.ph() != nullptr);

    auto ph = m__wds.ph();

    // get the initial status
    double val = 0;
    int errorcode = EN_getlinkvalue(ph, m__en_index, EN_INITSTATUS, &val);
    if (errorcode > 100) 
        __format_and_throw<std::runtime_error>("Link", "retrieve_EN_properties", "Error retrieving properties of link.",
            "Property: EN_INITSTATUS",
            "Error code: ", errorcode,
            "Link ID: ", m__name);

    m__initial_status = val;

    // Assign Nodes
    int node_from_idx= 0;
    int node_to_idx= 0;
    errorcode = EN_getlinknodes(ph, this->m__en_index, &node_from_idx, &node_to_idx);
    if (errorcode > 100) 
        __format_and_throw<std::runtime_error>("Link", "retrieve_EN_properties", "Error retrieving properties of link.",
            "Property: EN_LINKNODES",
            "Error code: ", errorcode,
            "Link ID: ", m__name);

    // Install the link between the nodes
    m__from_node = m__wds.nodes().get(epanet::get_node_id(ph, node_from_idx)).get();
    m__to_node = m__wds.nodes().get(epanet::get_node_id(ph, node_to_idx)).get();
}

void Link::retrieve_EN_results()
{
    this->__retrieve_EN_results();

    inherited::retrieve_EN_results();
}

void Link::__retrieve_EN_results()
{
    assert(m__en_index != 0);
    assert(m__wds.ph() != nullptr);

    auto ph = m__wds.ph();
    auto t = m__wds.current_result_time();

    double val = 0;
    int errorcode = EN_getlinkvalue(ph, m__en_index, EN_FLOW, &val);
    if (errorcode > 100) 
        __format_and_throw<std::runtime_error>("Link", "retrieve_results", "Error retrieving results of link.",
            "Property: EN_FLOW",
            "Error code: ", errorcode,
            "Link ID: ", m__name);
    
    if (ph->parser.Flowflag != LPS)
        val = epanet::convert_flow_to_L_per_s(ph, val);

    m__flow.commit(t, val);
}

void Link::clear_results() {
    inherited::clear_results();

    m__flow.clear();
}

} // namespace bevarmejo::wds
