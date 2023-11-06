//
// node.cpp
//
// Created by Dennis Zanutto on 20/10/23.
#include <cassert>

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/results.hpp"
#include "bevarmejo/wds/elements/variables.hpp"
#include "bevarmejo/wds/elements/variable.hpp"
#include "bevarmejo/wds/elements/temporal.hpp"

#include "node.hpp"

namespace bevarmejo {
namespace wds {

node::node() : inherited(),
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

node::node(const std::string& id) : inherited(id),
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
node::node(const node& other) : inherited(other),
                                _x_coord_(other._x_coord_),
                                _y_coord_(other._y_coord_),
                                _elevation_(other._elevation_),
                                _head_(nullptr),
                                _pressure_(nullptr)
                                {
                                    _update_pointers();
                                }

// Move constructor
node::node(node&& rhs) noexcept : inherited(std::move(rhs)),
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
    // results are cleared when the inherited destructor is called
}

void node::add_link(link *l)
{
    if (l != nullptr) {
        _links_.insert(l);
    }
}

void node::remove_link(link *l)
{
    if (l != nullptr) {
        _links_.erase(l);
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

void node::_add_properties()
{
    inherited::_add_properties();
    // no properties to add
}

void node::_add_results()
{
    inherited::_add_results();
    results().temporal_reals().emplace(LABEL_PRESSURE, LABEL_PRESSURE_UNITS);
    results().temporal_reals().emplace(LABEL_HEAD, LABEL_PRESSURE_UNITS);
}

void node::_update_pointers() {
    inherited::_update_pointers();
    _head_ = &(results().temporal_reals().at(LABEL_HEAD));
    _pressure_ = &(results().temporal_reals().at(LABEL_PRESSURE));
}

} // namespace wds
} // namespace bevarmejo
