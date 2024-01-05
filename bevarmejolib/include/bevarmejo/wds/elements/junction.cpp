#include <cassert>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/variable.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"

#include "bevarmejo/wds/elements/pattern.hpp"
#include "bevarmejo/wds/elements/demand.hpp"

#include "junction.hpp"

namespace bevarmejo {
namespace wds {

Junction::Junction(const std::string& id) : 
    inherited(id),
    _demands_(),
    _demand_constant_(nullptr),
    _demand_requested_(nullptr),
    _demand_delivered_(nullptr),
    _demand_undelivered_(nullptr)
    {
        _add_properties();
        _add_results();
        _update_pointers();
    }

// Copy constructor
Junction::Junction(const Junction& other) : 
    inherited(other),
    _demands_(other._demands_),
    _demand_constant_(nullptr),
    _demand_requested_(nullptr),
    _demand_delivered_(nullptr),
    _demand_undelivered_(nullptr)
    {
        _update_pointers();
    }

// Move constructor
Junction::Junction(Junction&& rhs) noexcept : 
    inherited(std::move(rhs)),
    _demands_(std::move(rhs._demands_)),
    _demand_constant_(nullptr),
    _demand_requested_(nullptr),
    _demand_delivered_(nullptr),
    _demand_undelivered_(nullptr)
    {
        _update_pointers();
    }

// Copy assignment operator
Junction& Junction::operator=(const Junction& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        _demands_ = rhs._demands_;
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
Junction& Junction::operator=(Junction&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        _demands_ = std::move(rhs._demands_);
        _update_pointers();
    }
    return *this;
}

Junction::~Junction() { /* Everything is deleted by the inherited destructor */ }

Demand& Junction::demand(const std::string &a_category) {
    for (auto& d : _demands_) {
        if (d.category() == a_category) {
            return d;
        }
    }
    throw std::out_of_range("Demand category " + a_category + " not found in junction " + id());
}

void Junction::add_demand(const std::string &a_category, const double a_base_dem, const std::shared_ptr<Pattern> a_pattern) {
    _demands_.emplace_back(Demand(a_category, a_base_dem, a_pattern));
}

auto Junction::_find_demand(const std::string &a_category) const {
    for (auto it = _demands_.begin(); it != _demands_.end(); ++it) {
        if (it->category() == a_category) {
            return it;
        }
    }
    throw std::out_of_range("Demand category " + a_category + " not found in junction " + id());
}

void Junction::remove_demand(const std::string &a_category) {
    auto d_p_demand = _find_demand(a_category);
    if (d_p_demand != _demands_.end()) {
        _demands_.erase(d_p_demand);
    }
}

const bool Junction::has_demand() const {
    return _demand_constant_->value() > 0 || !_demands_.empty();
}

void Junction::retrieve_properties(EN_Project ph)
{
    inherited::retrieve_properties(ph);

    //TODO: get the demands
}

void Junction::retrieve_results(EN_Project ph, long t=0) {
    inherited::retrieve_results(ph, t);

    int errorcode = 0;
    double val = 0;
    errorcode = EN_getnodevalue(ph, index(), EN_DEMAND, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving demand for node " + id()+"\n");

    if (ph->parser.Unitsflag != LPS)
        val = epanet::convert_flow_to_L_per_s(ph, val);
    this->_demand_requested_->value().insert(std::make_pair(t, val));

    errorcode = EN_getnodevalue(ph, index(), EN_DEMANDDEFICIT, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving demand deficit for node " + id()+"\n");

    if (ph->parser.Unitsflag != LPS)
        val = epanet::convert_flow_to_L_per_s(ph, val);
    this->_demand_undelivered_->value().insert(std::make_pair(t, val));

    this->_demand_delivered_->value().insert(std::make_pair(t, 
        _demand_requested_->value().at(t) - _demand_undelivered_->value().at(t)));
}

void Junction::_add_properties() {
    inherited::_add_properties();

    properties().emplace(LDEMAND_CONSTANT, vars::var_real(vars::l__L_per_s,0));
}

void Junction::_add_results() {
    inherited::_add_results();

    results().emplace(LDEMAND_REQUESTED, vars::var_tseries_real(vars::l__L_per_s));
    results().emplace(LDEMAND_DELIVERED, vars::var_tseries_real(vars::l__L_per_s));
    results().emplace(LDEMAND_UNDELIVERED, vars::var_tseries_real(vars::l__L_per_s));
}

void Junction::_update_pointers() {
    inherited::_update_pointers();

    _demand_constant_ = &std::get<vars::var_real>(properties().at(LDEMAND_CONSTANT));

    _demand_requested_ = &std::get<vars::var_tseries_real>(results().at(LDEMAND_REQUESTED));
    _demand_delivered_ = &std::get<vars::var_tseries_real>(results().at(LDEMAND_DELIVERED));
    _demand_undelivered_ = &std::get<vars::var_tseries_real>(results().at(LDEMAND_UNDELIVERED));
}

} // namespace wds
} // namespace bevarmejo
