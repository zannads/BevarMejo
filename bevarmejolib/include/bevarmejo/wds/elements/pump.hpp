#ifndef BEVARMEJOLIB__WDS_ELEMENTS__PUMP_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__PUMP_HPP

#include <memory>
#include <string>

#include "epanet2_2.h"

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/link.hpp"

#include "bevarmejo/wds/auxiliary/pattern.hpp"
#include "bevarmejo/wds/auxiliary/curve.hpp"
#include "bevarmejo/wds/auxiliary/curves.hpp"

namespace bevarmejo {
namespace wds {

/// WDS Pump
/*******************************************************************************
 * The wds::Pump class represents a pump in the network.
 ******************************************************************************/

static const std::string l__NAME_PUMP= "Pump";
static const std::string l__INIT_SETTINGS = "InitSettings";
static const std::string l__INSTANT_ENERGY = "InstantEnergy"; // Also known as power -.-'
static const std::string l__STATE = "State";
static const std::string l__EFFICIENCY = "Efficiency";
static const std::string l__POWER_RATING = "PowerRating";
static const std::string l__ENERGY_COST = "EnergyCost";

class Pump : public Link {

public:
    using inherited= Link;

/*--- Attributes ---*/
protected:
    /*--- Properties ---*/
    aux::QuantitySeries<int> m__init_setting; // Constant
    aux::QuantitySeries<double> m__power_rating; // Constant
    aux::QuantitySeries<double> m__energy_cost; // Constant because it uses the pattern together

    std::shared_ptr<const Pattern> _speed_pattern_;
    std::shared_ptr<const Pattern> _energy_cost_pattern_;
    std::shared_ptr<const PumpCurve> _pump_curve_;
    std::shared_ptr<const EfficiencyCurve> _efficiency_curve_;
    
    
    /*---  Results   ---*/
    aux::QuantitySeries<double> m__instant_energy;
    aux::QuantitySeries<int> m__state;
    aux::QuantitySeries<double> m__efficiency;

/*--- Constructors ---*/
public:
    Pump() = delete;

    Pump(const std::string& id, const WaterDistributionSystem& wds);

    // Copy constructor
    Pump(const Pump& other);

    // Move constructor
    Pump(Pump&& rhs) noexcept;

    // Copy assignment operator
    Pump& operator=(const Pump& rhs);

    // Move assignment operator
    Pump& operator=(Pump&& rhs) noexcept;

    // Destructor
    virtual ~Pump() = default;

/*--- Getters and setters ---*/
public:
    /*--- Properties ---*/
    aux::QuantitySeries<int>& init_setting() { return m__init_setting; }
    const aux::QuantitySeries<int>& init_setting() const { return m__init_setting; }
    void init_setting(const int a_init_setting) { m__init_setting.value(a_init_setting); }

    aux::QuantitySeries<double>& power_rating() { return m__power_rating; }
    const aux::QuantitySeries<double>& power_rating() const { return m__power_rating; }
    void power_rating(const double a_power_rating) { m__power_rating.value(a_power_rating); }

    aux::QuantitySeries<double>& energy_cost() { return m__energy_cost; }
    const aux::QuantitySeries<double>& energy_cost() const { return m__energy_cost; }
    void energy_cost(const double a_energy_cost) { m__energy_cost.value(a_energy_cost); }

    std::shared_ptr<const Pattern> speed_pattern() const { return _speed_pattern_; }
    void speed_pattern(std::shared_ptr<const Pattern> a_speed_pattern) { _speed_pattern_ = std::move(a_speed_pattern); }

    std::shared_ptr<const Pattern> energy_cost_pattern() const { return _energy_cost_pattern_; }
    void energy_cost_pattern(std::shared_ptr<const Pattern> a_energy_cost_pattern) { _energy_cost_pattern_ = std::move(a_energy_cost_pattern); }

    std::shared_ptr<const PumpCurve> pump_curve() const { return _pump_curve_; }
    void pump_curve(std::shared_ptr<const PumpCurve> a_pump_curve) { _pump_curve_ = std::move(a_pump_curve); }
    
    std::shared_ptr<const EfficiencyCurve> efficiency_curve() const { return _efficiency_curve_; }
    void efficiency_curve(std::shared_ptr<const EfficiencyCurve> a_efficiency_curve) { _efficiency_curve_ = std::move(a_efficiency_curve); }

    /*---  Results   ---*/
    const aux::QuantitySeries<double>& instant_energy() const { return m__instant_energy; }
    const aux::QuantitySeries<int>& state() const { return m__state; }
    const aux::QuantitySeries<double>& efficiency() const { return m__efficiency; }

/*--- Methods ---*/
public:

/*--- Pure virtual methods override---*/
public:
    const std::string& element_name() const override { return l__NAME_PUMP; }
    const unsigned int element_type() const override { return ELEMENT_PUMP; }
    virtual void clear_results() override;

/*--- EPANET-dependent PVMs ---*/
public:
    /*--- Properties ---*/
protected:
    void __retrieve_EN_properties(EN_Project ph) override;
public:
    /*--- Results ---*/
    void retrieve_results(EN_Project ph, long t) override;


}; // class Pump

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__PUMP_HPP
