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

#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/variable.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/link.hpp"

#include "node.hpp"

namespace bevarmejo {
namespace wds {

node::node(const std::string& id) : 
    inherited(id),
    _x_coord_(0.0),
    _y_coord_(0.0),
    _elevation_(0.0),
    _head_(nullptr),
    _pressure_(nullptr)
    {
        _add_properties();
        _add_results();
        _update_pointers();
    }

// Copy constructor
node::node(const node& other) : 
    inherited(other),
    _x_coord_(other._x_coord_),
    _y_coord_(other._y_coord_),
    _elevation_(other._elevation_),
    _head_(nullptr),
    _pressure_(nullptr)
    {
        _update_pointers();
    }

// Move constructor
node::node(node&& rhs) noexcept : 
    inherited(std::move(rhs)),
    _x_coord_(std::move(rhs._x_coord_)),
    _y_coord_(std::move(rhs._y_coord_)),
    _elevation_(std::move(rhs._elevation_)),
    _head_(nullptr),
    _pressure_(nullptr)
    {
        _update_pointers();
    }

// Copy assignment operator
node& node::operator=(const node& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        _x_coord_ = rhs._x_coord_;
        _y_coord_ = rhs._y_coord_;
        _elevation_ = rhs._elevation_;
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
node& node::operator=(node&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        _x_coord_ = std::move(rhs._x_coord_);
        _y_coord_ = std::move(rhs._y_coord_);
        _elevation_ = std::move(rhs._elevation_);
        _update_pointers();
    }
    return *this;
}

node::~node() {
    // Both properties and results are cleared when the inherited destructor 
    // is called.
}

void node::add_link(link *a_link)
{
    if (a_link != nullptr) {
        _links_.insert(a_link);
    }
}

void node::remove_link(link *a_link)
{
    if (a_link != nullptr) {
        _links_.erase(a_link);
    }
}

void node::retrieve_index(EN_Project ph) {
    int en_index;
    int errorcode = 0;
    errorcode = EN_getnodeindex(ph, id().c_str(), &en_index);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving index of node " + id() + " from EPANET project.");

    this->index(en_index);
}

void node::retrieve_properties(EN_Project ph) {
    assert(index() != 0);
    int errorcode = 0;    
    double x, y, z;
    errorcode = EN_getcoord(ph, index(), &x, &y);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving coordinates of node " + id() + " from EPANET project.");
    this->x_coord(x);
    this->y_coord(y);
    errorcode = EN_getnodevalue(ph, index(), EN_ELEVATION, &z);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving elevation of node " + id() + " from EPANET project.");
    this->elevation(z);
}

void node::retrieve_results(EN_Project ph, long t=0) {
    assert(index() != 0);
    int errorcode = 0;
    double d_head, d_pressure;
    errorcode = EN_getnodevalue(ph, index(), EN_HEAD, &d_head);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving head of node " + id() + " from EPANET project.");
    this->_head_->value().insert(std::make_pair(t, d_head));
    errorcode = EN_getnodevalue(ph, index(), EN_PRESSURE, &d_pressure);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving pressure of node " + id() + " from EPANET project.");
    this->_pressure_->value().insert(std::make_pair(t, d_pressure));
}

void node::_add_properties() {
    inherited::_add_properties();
    // no properties to add
}

void node::_add_results() {
    inherited::_add_results();

    results().emplace(LABEL_PRESSURE, vars::var_tseries_real(LABEL_PRESSURE_UNITS));
    results().emplace(LABEL_HEAD, vars::var_tseries_real(LABEL_PRESSURE_UNITS));
}

void node::_update_pointers() {
    inherited::_update_pointers();

    _head_ = &std::get<vars::var_tseries_real>(results().at(LABEL_HEAD));
    _pressure_ = &std::get<vars::var_tseries_real>(results().at(LABEL_PRESSURE));
}

} // namespace wds
} // namespace bevarmejo
