//
// node.cpp
//
// Created by Dennis Zanutto on 20/10/23.
#include <cassert>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/wds/data_structures/temporal.hpp"
#include "bevarmejo/wds/data_structures/variable.hpp"

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
    _links_(),
    _elevation_(0.0),
    _head_(nullptr),
    _pressure_(nullptr)
    {
        _add_properties();
        _add_results();
        _update_pointers();
    }

// Copy constructor
Node::Node(const Node& other) : 
    inherited(other),
    _x_coord_(other._x_coord_),
    _y_coord_(other._y_coord_),
    _links_(other._links_), 
    _elevation_(other._elevation_),
    _head_(nullptr),
    _pressure_(nullptr)
    {
        _update_pointers();
    }

// Move constructor
Node::Node(Node&& rhs) noexcept : 
    inherited(std::move(rhs)),
    _x_coord_(rhs._x_coord_),
    _y_coord_(rhs._y_coord_),
    _links_(std::move(rhs._links_)),
    _elevation_(rhs._elevation_),
    _head_(nullptr),
    _pressure_(nullptr)
    {
        _update_pointers();
    }

// Copy assignment operator
Node& Node::operator=(const Node& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        _x_coord_ = rhs._x_coord_;
        _y_coord_ = rhs._y_coord_;
        _links_ = rhs._links_;
        _elevation_ = rhs._elevation_;
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
Node& Node::operator=(Node&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        _x_coord_ = rhs._x_coord_;
        _y_coord_ = rhs._y_coord_;
        _links_ = std::move(rhs._links_);
        _elevation_ = rhs._elevation_;
        _update_pointers();
    }
    return *this;
}

Node::~Node() {
    // Both properties and results are cleared when the inherited destructor 
    // is called.
}

void Node::add_link(Link *a_link) {
    if (a_link != nullptr)
        _links_.insert(a_link);
}

void Node::remove_link(Link *a_link) {
    if (a_link != nullptr)
        _links_.erase(a_link);
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
    this->_head_->value().insert(std::make_pair(t, val));

    errorcode = EN_getnodevalue(ph, index(), EN_PRESSURE, &val);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving pressure of node " + id() + " from EPANET project.");

    if (ph->parser.Unitsflag == US)
        val *= MperFT/PSIperFT;
    this->_pressure_->value().insert(std::make_pair(t, val));
}

void Node::_add_properties() {
    inherited::_add_properties();
    // no properties to add
}

void Node::_add_results() {
    inherited::_add_results();

    results().emplace(LABEL_PRESSURE, vars::var_tseries_real(LABEL_PRESSURE_UNITS));
    results().emplace(LABEL_HEAD, vars::var_tseries_real(LABEL_PRESSURE_UNITS));
}

void Node::_update_pointers() {
    inherited::_update_pointers();

    _head_ = &std::get<vars::var_tseries_real>(results().at(LABEL_HEAD));
    _pressure_ = &std::get<vars::var_tseries_real>(results().at(LABEL_PRESSURE));
}

} // namespace wds
} // namespace bevarmejo
