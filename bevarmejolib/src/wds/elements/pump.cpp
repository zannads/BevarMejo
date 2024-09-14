#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/link.hpp"

#include "bevarmejo/wds/auxiliary/pattern.hpp"
#include "bevarmejo/wds/auxiliary/curve.hpp"
#include "bevarmejo/wds/auxiliary/curves.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "pump.hpp"

namespace bevarmejo {
namespace wds {

Pump::Pump(const std::string& id, const WaterDistributionSystem& wds) :
    inherited(id, wds),
    _speed_pattern_(nullptr),
    _energy_cost_pattern_(nullptr),
    _pump_curve_(nullptr),
    _efficiency_curve_(nullptr),
    m__init_setting(wds.time_series(label::__CONSTANT_TS)),
    m__power_rating(wds.time_series(label::__CONSTANT_TS)),
    m__energy_cost(wds.time_series(label::__CONSTANT_TS)),
    m__instant_energy(wds.time_series(label::__RESULTS_TS)),
    m__state(wds.time_series(label::__RESULTS_TS)),
    m__efficiency(wds.time_series(label::__RESULTS_TS)) { }

// Copy constructor
Pump::Pump(const Pump& other) : 
    inherited(other),
    _speed_pattern_(other._speed_pattern_),
    _energy_cost_pattern_(other._energy_cost_pattern_),
    _pump_curve_(other._pump_curve_),
    _efficiency_curve_(other._efficiency_curve_),
    m__init_setting(other.m__init_setting),
    m__power_rating(other.m__power_rating),
    m__energy_cost(other.m__energy_cost),
    m__instant_energy(other.m__instant_energy),
    m__state(other.m__state),
    m__efficiency(other.m__efficiency) { }

// Move constructor
Pump::Pump(Pump&& rhs) noexcept : 
    inherited(std::move(rhs)),
    _speed_pattern_(std::move(rhs._speed_pattern_)),
    _energy_cost_pattern_(std::move(rhs._energy_cost_pattern_)),
    _pump_curve_(std::move(rhs._pump_curve_)),
    _efficiency_curve_(std::move(rhs._efficiency_curve_)),
    m__init_setting(std::move(rhs.m__init_setting)),
    m__power_rating(std::move(rhs.m__power_rating)),
    m__energy_cost(std::move(rhs.m__energy_cost)),
    m__instant_energy(std::move(rhs.m__instant_energy)),
    m__state(std::move(rhs.m__state)),
    m__efficiency(std::move(rhs.m__efficiency)) { }

// Copy assignment operator
Pump& Pump::operator=(const Pump& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        _speed_pattern_ = rhs._speed_pattern_;
        _energy_cost_pattern_ = rhs._energy_cost_pattern_;
        _pump_curve_ = rhs._pump_curve_;
        _efficiency_curve_ = rhs._efficiency_curve_;
        m__init_setting = rhs.m__init_setting;
        m__power_rating = rhs.m__power_rating;
        m__energy_cost = rhs.m__energy_cost;
        m__instant_energy = rhs.m__instant_energy;
        m__state = rhs.m__state;
        m__efficiency = rhs.m__efficiency;
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
        m__init_setting = std::move(rhs.m__init_setting);
        m__power_rating = std::move(rhs.m__power_rating);
        m__energy_cost = std::move(rhs.m__energy_cost);
        m__instant_energy = std::move(rhs.m__instant_energy);
        m__state = std::move(rhs.m__state);
        m__efficiency = std::move(rhs.m__efficiency);
    }
    return *this;
}

void Pump::__retrieve_EN_properties(EN_Project ph) {
    inherited::__retrieve_EN_properties(ph);
    auto patterns= m__wds.patterns();
    auto curves= m__wds.curves();

    double value= 0.0;
    int errorcode= EN_getlinkvalue(ph, index(), EN_INITSETTING, &value);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving initial setting for pump " + id());
    m__init_setting= value;

    errorcode= EN_getlinkvalue(ph, index(), EN_PUMP_POWER, &value);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving power rating for pump " + id());
    m__power_rating= value;

    errorcode= EN_getlinkvalue(ph, index(), EN_PUMP_ECOST, &value);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving energy cost for pump " + id());
    m__energy_cost= value;

    { // Assign EN Pattern
        errorcode= EN_getlinkvalue(ph, this->index(), EN_LINKPATTERN, &value);
        assert(errorcode < 100);
        char pattern_id[EN_MAXID+1];
        errorcode= EN_getpatternid(ph, static_cast<int>(value), pattern_id);
        assert(errorcode < 100);
        auto it= patterns.find(pattern_id);
        assert(it != patterns.end());

        speed_pattern(*it);
    }
    { // Assign EN Curves (Pump curve and Efficiency curve)
        errorcode= EN_getlinkvalue(ph, this->index(), EN_PUMP_HCURVE, &value);
        assert(errorcode < 100);
        
        if (value != 0.0) {
            char curve_id[EN_MAXID+1];
            errorcode= EN_getcurveid(ph, static_cast<int>(value), curve_id);
            assert(errorcode < 100);
            auto it= curves.find(curve_id);
            assert(it != curves.end());

            std::shared_ptr<PumpCurve> pump_curve= std::dynamic_pointer_cast<PumpCurve>(*it);
            assert(pump_curve != nullptr); 
            // EPANET says that this curve with this ID should be a pump curve.
            // If this assertion fails it means there are some inconsistencies in the upload from EPANET.
            this->pump_curve(pump_curve);
        }
        else this->pump_curve(nullptr);

        errorcode= EN_getlinkvalue(ph, this->index(), EN_PUMP_ECURVE, &value);
        assert(errorcode < 100);

        if (value != 0.0) {
            char curve_id[EN_MAXID+1];
            errorcode= EN_getcurveid(ph, static_cast<int>(value), curve_id);
            assert(errorcode < 100);
            auto it= curves.find(curve_id);
            assert(it != curves.end());

            std::shared_ptr<EfficiencyCurve> efficiency_curve= std::dynamic_pointer_cast<EfficiencyCurve>(*it);
            assert(efficiency_curve != nullptr); 
            // EPANET says that this curve with this ID should be an efficiency curve.
            // If this assertion fails it means there are some inconsistencies in the upload from EPANET.
            this->efficiency_curve(efficiency_curve);
        }
        else this->efficiency_curve(nullptr);
    }
}

void Pump::retrieve_results(EN_Project ph, long t) {
    inherited::retrieve_results(ph, t);

    int errorcode;
    double value;

    errorcode = EN_getlinkvalue(ph, index(), EN_ENERGY, &value);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving energy for pump " + id());
    m__instant_energy.commit(t, value);

    errorcode = EN_getlinkvalue(ph, index(), EN_STATUS, &value);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving status for pump " + id());
    m__state.commit(t, static_cast<int>(value));

    errorcode = EN_getlinkvalue(ph, index(), EN_PUMP_EFFIC, &value);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving efficiency for pump " + id());
    m__efficiency.commit(t, value);

    // Note on conversions:
    // // Instantenous Energy is always in kW (even if they say kWh)        
    // // Status is dimensionless
    // // Efficiency is dimensionless
}

void Pump::clear_results() {
    inherited::clear_results();

    m__instant_energy.clear();
    m__state.clear();
    m__efficiency.clear();
}

} // namespace wds
} // namespace bevarmejo
