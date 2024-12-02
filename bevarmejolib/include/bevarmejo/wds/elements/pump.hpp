#ifndef BEVARMEJOLIB__WDS_ELEMENTS__PUMP_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__PUMP_HPP

#include <memory>
#include <string>

#include "bevarmejo/wds/auxiliary/curve.hpp"
#include "bevarmejo/wds/auxiliary/curves.hpp"
#include "bevarmejo/wds/auxiliary/pattern.hpp"
#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/link.hpp"

namespace bevarmejo::wds
{

class Pump;
template <>
struct TypeTraits<Pump>
{
    static constexpr const char* name = "Pump";
    static constexpr unsigned int code = 2211;
    static constexpr bool is_EN_complete = true;
};

class Pump final : public Link
{
/// WDS Pump
/*******************************************************************************
 * The wds::Pump class represents a pump in the network.
 ******************************************************************************/

/*------- Member types -------*/
public:
    using self_type = Pump;
    using self_traits = TypeTraits<self_type>;
    using inherited = Link;
    using PowerSeries = aux::QuantitySeries<double>;
    using StatusSeries = aux::QuantitySeries<int>;
    using EfficiencySeries = aux::QuantitySeries<double>;
private:
    friend class WaterDistributionSystem;

/*------- Member objects -------*/
protected:
    // === Properties ===
    aux::QuantitySeries<int> m__init_setting; // Constant
    aux::QuantitySeries<double> m__power_rating; // Constant
    aux::QuantitySeries<double> m__energy_cost; // Constant because it uses the pattern together

    std::shared_ptr<const Pattern> _speed_pattern_;
    std::shared_ptr<const Pattern> _energy_cost_pattern_;
    std::shared_ptr<const PumpCurve> _pump_curve_;
    std::shared_ptr<const EfficiencyCurve> _efficiency_curve_;
    
    // === Results ===
    aux::QuantitySeries<double> m__instant_energy; // Electrical power consumed at each time step
    StatusSeries m__state;
    EfficiencySeries m__efficiency;

/*------- Member functions -------*/
// (constructor)
protected:
    Pump() = delete;
    Pump(const WaterDistributionSystem& wds, const EN_Name_t& name); // Constructor

// (destructor)
public:
    virtual ~Pump() = default;

// clone()
public:

/*------- Element access -------*/
public:
    // === Read/Write properties ===
    const char* type_name() const override;

    unsigned int type_code() const override;

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

    // === Results ===
    const aux::QuantitySeries<double>& instant_energy() const { return m__instant_energy; }
    const aux::QuantitySeries<int>& state() const { return m__state; }
    const aux::QuantitySeries<double>& efficiency() const { return m__efficiency; }

/*------- Capacity -------*/
public:

/*------- Modifiers -------*/
public:
    void clear_results() override;

    void retrieve_EN_properties() override;

    void retrieve_EN_results() override;

}; // class Pump

} // namespace bevarmejo::wds

#endif // BEVARMEJOLIB__WDS_ELEMENTS__PUMP_HPP
