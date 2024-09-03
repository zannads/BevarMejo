//
// node.cpp
//
// Created by Dennis Zanutto on 20/10/23.
#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/link.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "node.hpp"

namespace bevarmejo {
namespace wds {

Node::Node(const std::string& id, const WaterDistributionSystem& wds) : 
    inherited(id, wds),
    _x_coord_(0.0),
    _y_coord_(0.0),
    m__links(),
    _elevation_(0.0),
    m__head(wds.time_series(l__RESULT_TS)),
    m__pressure(wds.time_series(l__RESULT_TS)) { }

// Copy constructor
Node::Node(const Node& other) : 
    inherited(other),
    _x_coord_(other._x_coord_),
    _y_coord_(other._y_coord_),
    m__links(other.m__links), 
    _elevation_(other._elevation_),
    m__head(other.m__head),
    m__pressure(other.m__pressure) { }

// Move constructor
Node::Node(Node&& rhs) noexcept : 
    inherited(std::move(rhs)),
    _x_coord_(rhs._x_coord_),
    _y_coord_(rhs._y_coord_),
    m__links(std::move(rhs.m__links)),
    _elevation_(rhs._elevation_),
    m__head(std::move(rhs.m__head)),
    m__pressure(std::move(rhs.m__pressure)) { }

// Copy assignment operator
Node& Node::operator=(const Node& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        _x_coord_ = rhs._x_coord_;
        _y_coord_ = rhs._y_coord_;
        m__links = rhs.m__links;
        _elevation_ = rhs._elevation_;
        m__head = rhs.m__head;
        m__pressure = rhs.m__pressure;
    }
    return *this;
}

// Move assignment operator
Node& Node::operator=(Node&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        _x_coord_ = rhs._x_coord_;
        _y_coord_ = rhs._y_coord_;
        m__links = std::move(rhs.m__links);
        _elevation_ = rhs._elevation_;
        m__head = std::move(rhs.m__head);
        m__pressure = std::move(rhs.m__pressure); { }
    }
    return *this;
}

void Node::add_link(Link *a_link) {
    if (a_link != nullptr)
        m__links.insert(a_link);
}

void Node::remove_link(Link *a_link) {
    if (a_link != nullptr)
        m__links.erase(a_link);
}

void Node::retrieve_index(EN_Project ph) {
    int en_index;
    int errorcode = 0;
    errorcode = EN_getnodeindex(ph, id().c_str(), &en_index);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving index of node " + id() + " from EPANET project.");

    this->index(en_index);
}

void Node::__retrieve_EN_properties(EN_Project ph) {
    assert(index() != 0);
    int errorcode = 0;    
    double x=0,
           y=0,
           z=0;

    errorcode = EN_getcoord(ph, index(), &x, &y);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving coordinates of node " + id() + " from EPANET project.");

    if (ph->parser.Unitsflag == US) {
        x *= MperFT;
        y *= MperFT;
    }
    this->x_coord(x);
    this->y_coord(y);

    errorcode = EN_getnodevalue(ph, index(), EN_ELEVATION, &z);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving elevation of node " + id() + " from EPANET project.");
    
    if (ph->parser.Unitsflag == US)
        z *= MperFT;
    this->elevation(z);
}

void Node::retrieve_results(EN_Project ph, long t=0) {
    assert(index() != 0);
    int errorcode = 0;
    double val = 0;

    errorcode = EN_getnodevalue(ph, index(), EN_HEAD, &val);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving head of node " + id() + " from EPANET project.");
    
    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    m__head.commit(t, val);

    errorcode = EN_getnodevalue(ph, index(), EN_PRESSURE, &val);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving pressure of node " + id() + " from EPANET project.");

    if (ph->parser.Unitsflag == US)
        val *= MperFT/PSIperFT;
    m__pressure.commit(t, val);
}

void Node::clear_results() {
    inherited::clear_results();

    m__head.clear();
    m__pressure.clear();
}

} // namespace wds
} // namespace bevarmejo
