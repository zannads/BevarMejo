#include <cassert>
#include <string>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/wds/utility/epanet/en_help.hpp"

#include "bevarmejo/utility/except.hpp"

#include "bevarmejo/wds/utility/quantity_series.hpp"

#include "bevarmejo/wds/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/network_elements/node.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "bevarmejo/wds/elements/network_elements/link.hpp"

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
auto Link::from_node() const -> Node_ptr
{
    return m__from_node;
}

auto Link::to_node() const -> Node_ptr
{
    return m__to_node;
}

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
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the index of the link.",
        "Error originating from the EPANET API.",
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
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the properties of the link.",
        "Error originating from the EPANET API while retrieving value: EN_INITSTATUS",
        "Link ID: ", m__name);

    m__initial_status = val;

    // Assign Nodes
    int node_from_idx= 0;
    int node_to_idx= 0;
    errorcode = EN_getlinknodes(ph, this->m__en_index, &node_from_idx, &node_to_idx);
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the properties of the link.",
        "Error originating from the EPANET API while retrieving value: EN_INITSTATUS",
        "Link ID: ", m__name);

    // Install the link between the nodes
    m__from_node = m__wds.get_node(epanet::get_node_id(ph, node_from_idx)).get();
    m__to_node = m__wds.get_node(epanet::get_node_id(ph, node_to_idx)).get();
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
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the results of the link.",
        "Error originating from the EPANET API while retrieving value: EN_FLOW",
        "Link ID: ", m__name);
    
    if (ph->parser.Flowflag != LPS)
        val = epanet::convert_flow_to_L_per_s(ph, val);

    m__flow.commit(t, val);
}

void Link::from_node(Node_ptr a_node)
{
    m__from_node = a_node;
}

void Link::to_node(Node_ptr a_node)
{
    m__to_node = a_node;
}

} // namespace bevarmejo::wds
