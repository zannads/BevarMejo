//
// node.cpp
//
// Created by Dennis Zanutto on 20/10/23.
#include <cassert>
#include <string>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/link.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "node.hpp"

namespace bevarmejo::wds
{

/*------- Member functions -------*/
// (constructor)
Node::Node(const WaterDistributionSystem& wds, const EN_Name_t& name) :
    inherited(wds, name),
    m__x_coord(0.0),
    m__y_coord(0.0),
    m__elevation(0.0),
    m__links(),
    m__head(wds.time_series(label::__RESULTS_TS)),
    m__pressure(wds.time_series(label::__RESULTS_TS))
{ }

/*------- Operators -------*/

/*------- Element access -------*/
// === Read/Write properties ===
double& Node::x_coord()
{
    return m__x_coord;
}
double Node::x_coord() const
{
    return m__x_coord;
}

double& Node::y_coord()
{
    return m__y_coord;
}
double Node::y_coord() const
{
    return m__y_coord;
}

double& Node::elevation()
{
    return m__elevation;
}
double Node::elevation() const
{
    return m__elevation;
}

double& Node::z_coord()
{
    return m__elevation;
}
double Node::z_coord() const
{
    return m__elevation;
}

// === Read-only properties ===
auto Node::connected_links() const -> const ConnectedLinks&
{
    return m__links;
}

// === Results ===
auto Node::head() const -> const HeadSeries&
{
    return m__head;
}

auto Node::pressure() const -> const PressureSeries&
{
    return m__pressure;
}

/*------- Capacity -------*/

/*------- Modifiers -------*/
void Node::clear_results()
{
    inherited::clear_results();

    m__head.clear();
    m__pressure.clear();
}

void Node::retrieve_EN_index()
{
    assert(m__wds.ph() != nullptr);

    m__en_index = 0;
    int index = 0;
    int errorcode = EN_getnodeindex(m__wds.ph(), m__name.c_str(), &index);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the index of the node.",
        "Error code: ", errorcode,
        "Node ID: ", m__name);

    m__en_index = index;
}

void Node::retrieve_EN_properties()
{
    this->__retrieve_EN_properties();

    inherited::retrieve_EN_properties();
}

void Node::__retrieve_EN_properties()
{
    assert(m__en_index > 0);
    assert(m__wds.ph() != nullptr);
    
    auto ph = m__wds.ph();
    
    double x=0,
           y=0;
    int errorcode = EN_getcoord(ph, m__en_index, &x, &y);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the properties of the node.",
        "Error while retrieving coordinates.",
        "Error code: ", errorcode,
        "Node ID: ", m__name);

    if (ph->parser.Unitsflag == US) {
        x *= MperFT;
        y *= MperFT;
    }
    m__x_coord = x;
    m__y_coord = y;

    double z = 0.0;
    errorcode = EN_getnodevalue(ph, m__en_index, EN_ELEVATION, &z);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the properties of the node.",
        "Error while retrieving value: EN_ELEVATION",
        "Error code: ", errorcode,
        "Node ID: ", m__name);
    
    if (ph->parser.Unitsflag == US)
        z *= MperFT;
    m__elevation = z;
}

void Node::retrieve_EN_results()
{
    this->__retrieve_EN_results();

    inherited::retrieve_EN_results();
}

void Node::__retrieve_EN_results()
{
    assert(m__en_index > 0);
    assert(m__wds.ph() != nullptr);

    auto ph = m__wds.ph();
    auto t = m__wds.current_result_time();

    double val = 0;
    int errorcode = EN_getnodevalue(ph, m__en_index, EN_HEAD, &val);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the results of the node.",
        "Error while retrieving value: EN_HEAD",
        "Error code: ", errorcode,
        "Node ID: ", m__name);
        
    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    m__head.commit(t, val);

    errorcode = EN_getnodevalue(ph, m__en_index, EN_PRESSURE, &val);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the results of the node.",
        "Error while retrieving value: EN_PRESSURE",
        "Error code: ", errorcode,
        "Node ID: ", m__name);

    if (ph->parser.Unitsflag == US)
        val *= MperFT/PSIperFT;
    m__pressure.commit(t, val);
}

void Node::x_coord(double a_x_coord)
{
    m__x_coord = a_x_coord;
}

void Node::y_coord(double a_y_coord)
{
    m__y_coord = a_y_coord;
}

void Node::elevation(double a_elevation)
{
    m__elevation = a_elevation;
}

void Node::z_coord(double a_z_coord)
{
    m__elevation = a_z_coord;
}

void Node::connect_link(Link_ptr a_link)
{
    if (a_link != nullptr)
        m__links.insert(a_link);
}

void Node::disconnect_link(Link_ptr a_link)
{
    if (a_link != nullptr)
        m__links.erase(a_link);
}

} // namespace bevarmejo::wds
