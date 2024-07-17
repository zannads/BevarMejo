#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "epanet2_2.h"

#include "bevarmejo/wds/data_structures/temporal.hpp"
#include "bevarmejo/wds/data_structures/variable.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/link.hpp"

#include "bevarmejo/wds/data_structures/pattern.hpp"

#include "pump.hpp"

namespace bevarmejo {
namespace wds {

    Pump::Pump(const std::string& id) :
        inherited(id),
        _init_setting_(nullptr),
        _power_rating_(nullptr),
        _energy_cost_(nullptr),
        _speed_pattern_(nullptr),
        _energy_cost_pattern_(nullptr),
        _pump_curve_(nullptr),
        _efficiency_curve_(nullptr),
        _instant_energy_(nullptr),
        _state_(nullptr),
        _efficiency_(nullptr)
        {
            _add_properties();
            _add_results();
            _update_pointers();
        }

    // Copy constructor
    Pump::Pump(const Pump& other) : 
        inherited(other),
        _init_setting_(nullptr),
        _power_rating_(nullptr),
        _energy_cost_(nullptr),
        _speed_pattern_(other._speed_pattern_),
        _energy_cost_pattern_(other._energy_cost_pattern_),
        _pump_curve_(other._pump_curve_),
        _efficiency_curve_(other._efficiency_curve_),
        _instant_energy_(nullptr),
        _state_(nullptr),
        _efficiency_(nullptr)
        {
            _update_pointers();
        }

    // Move constructor
    Pump::Pump(Pump&& rhs) noexcept : 
        inherited(std::move(rhs)),
        _init_setting_(nullptr),
        _power_rating_(nullptr),
        _energy_cost_(nullptr),
        _speed_pattern_(std::move(rhs._speed_pattern_)),
        _energy_cost_pattern_(std::move(rhs._energy_cost_pattern_)),
        _pump_curve_(std::move(rhs._pump_curve_)),
        _efficiency_curve_(std::move(rhs._efficiency_curve_)),
        _instant_energy_(nullptr),
        _state_(nullptr),
        _efficiency_(nullptr)
        {
            _update_pointers();
        }

    // Copy assignment operator
    Pump& Pump::operator=(const Pump& rhs) {
        if (this != &rhs) {
            inherited::operator=(rhs);
            _speed_pattern_ = rhs._speed_pattern_;
            _energy_cost_pattern_ = rhs._energy_cost_pattern_;
            _pump_curve_ = rhs._pump_curve_;
            _efficiency_curve_ = rhs._efficiency_curve_;
            
            _update_pointers();
        }
        return *this;
    }

    // Move assignment operator
    Pump& Pump::operator=(Pump&& rhs) noexcept {
        if (this != &rhs) {
            inherited::operator=(std::move(rhs));
            _speed_pattern_ = std::move(rhs._speed_pattern_);
            _energy_cost_pattern_ = std::move(rhs._energy_cost_pattern_);
            _pump_curve_ = std::move(rhs._pump_curve_);
            _efficiency_curve_ = std::move(rhs._efficiency_curve_);
            
            _update_pointers();
        }
        return *this;
    }

    Pump::~Pump() { /* Everything is deleted by the inherited destructor */ }

    void Pump::retrieve_properties(EN_Project ph) {
        inherited::retrieve_properties(ph);

        int errorcode;
        double value;

        errorcode = EN_getlinkvalue(ph, index(), EN_INITSETTING, &value);
        if (errorcode > 100) 
            throw std::runtime_error("Error retrieving initial setting for pump " + id());
        this->_init_setting_->value(value);

        errorcode = EN_getlinkvalue(ph, index(), EN_PUMP_POWER, &value);
        if (errorcode > 100) 
            throw std::runtime_error("Error retrieving power rating for pump " + id());
        this->_power_rating_->value(value);

        errorcode = EN_getlinkvalue(ph, index(), EN_PUMP_ECOST, &value);
        if (errorcode > 100) 
            throw std::runtime_error("Error retrieving energy cost for pump " + id());
        this->_energy_cost_->value(value);
    }

    void Pump::retrieve_results(EN_Project ph, long t) {
        inherited::retrieve_results(ph, t);

        int errorcode;
        double value;

        errorcode = EN_getlinkvalue(ph, index(), EN_ENERGY, &value);
        if (errorcode > 100) 
            throw std::runtime_error("Error retrieving energy for pump " + id());
        this->_instant_energy_->value().emplace(std::make_pair(t, value));

        errorcode = EN_getlinkvalue(ph, index(), EN_STATUS, &value);
        if (errorcode > 100) 
            throw std::runtime_error("Error retrieving status for pump " + id());
        this->_state_->value().emplace(std::make_pair(t, static_cast<int>(value)));

        errorcode = EN_getlinkvalue(ph, index(), EN_PUMP_EFFIC, &value);
        if (errorcode > 100) 
            throw std::runtime_error("Error retrieving efficiency for pump " + id());
        this->_efficiency_->value().emplace(std::make_pair(t, value));

        // Note on conversions:
        // // Instantenous Energy is always in kW (even if they say kWh)        
        // // Status is dimensionless
        // // Efficiency is dimensionless
}

void Pump::_add_properties() {
    inherited::_add_properties();

    properties().emplace(l__INIT_SETTINGS, vars::var_int(vars::l__DIMLESS, 0));
    properties().emplace(l__POWER_RATING, vars::var_real(vars::l__W, 0));
    properties().emplace(l__ENERGY_COST, vars::var_real(vars::l__Euro, 0));

}

void Pump::_add_results() {
    inherited::_add_results();

    results().emplace(l__INSTANT_ENERGY, vars::var_tseries_real(vars::l__W));
    results().emplace(l__STATE, vars::var_tseries_int(vars::l__DIMLESS));
    results().emplace(l__EFFICIENCY, vars::var_tseries_real(vars::l__DIMLESS));
}

void Pump::_update_pointers() {
    inherited::_update_pointers();

    _init_setting_ = &std::get<vars::var_int>(properties().at(l__INIT_SETTINGS));
    _power_rating_ = &std::get<vars::var_real>(properties().at(l__POWER_RATING));
    _energy_cost_ = &std::get<vars::var_real>(properties().at(l__ENERGY_COST));

    _instant_energy_ = &std::get<vars::var_tseries_real>(results().at(l__INSTANT_ENERGY));
    _state_ = &std::get<vars::var_tseries_int>(results().at(l__STATE));
    _efficiency_ = &std::get<vars::var_tseries_real>(results().at(l__EFFICIENCY));
}


} // namespace wds
} // namespace bevarmejo
