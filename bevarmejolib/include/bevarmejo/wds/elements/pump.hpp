#ifndef BEVARMEJOLIB__WDS_ELEMENTS__PUMP_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__PUMP_HPP

#include <memory>
#include <string>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/variable.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/link.hpp"

#include "bevarmejo/wds/elements/pattern.hpp"

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
    vars::var_int* _init_setting_;
    vars::var_real* _power_rating_;
    vars::var_real* _energy_cost_;

    std::shared_ptr<Pattern> _speed_pattern_;
    // Curve H-Q
    // Curve E-Q
    std::shared_ptr<Pattern> _energy_cost_pattern_;
    
    /*---  Results   ---*/
    vars::var_tseries_real* _instant_energy_;
    vars::var_tseries_int* _state_;
    vars::var_tseries_real* _efficiency_;

protected:
    void _add_properties() override;
    void _add_results() override;
    void _update_pointers() override;

/*--- Constructors ---*/
public:
    Pump() = delete;

    Pump(const std::string& id);

    // Copy constructor
    Pump(const Pump& other);

    // Move constructor
    Pump(Pump&& rhs) noexcept;

    // Copy assignment operator
    Pump& operator=(const Pump& rhs);

    // Move assignment operator
    Pump& operator=(Pump&& rhs) noexcept;

    // Destructor
    virtual ~Pump();

/*--- Getters and setters ---*/
public:
    /*--- Properties ---*/
    vars::var_int& init_setting() const { return *_init_setting_; }
    void init_setting(const int a_init_setting) { _init_setting_->value(a_init_setting); }
    vars::var_real& power_rating() const { return *_power_rating_; }
    void power_rating(const double a_power_rating) { _power_rating_->value(a_power_rating); }
    vars::var_real& energy_cost() const { return *_energy_cost_; }
    void energy_cost(const double a_energy_cost) { _energy_cost_->value(a_energy_cost); }
    std::shared_ptr<Pattern> speed_pattern() const { return _speed_pattern_; }
    void speed_pattern(const std::shared_ptr<Pattern> a_speed_pattern) { _speed_pattern_ = a_speed_pattern; }
    std::shared_ptr<Pattern> energy_cost_pattern() const { return _energy_cost_pattern_; }
    void energy_cost_pattern(const std::shared_ptr<Pattern> a_energy_cost_pattern) { _energy_cost_pattern_ = a_energy_cost_pattern; }

    /*---  Results   ---*/
    const vars::var_tseries_real& instant_energy() const { return *_instant_energy_; }
    const vars::var_tseries_int& state() const { return *_state_; }
    const vars::var_tseries_real& efficiency() const { return *_efficiency_; }

/*--- Methods ---*/
public:
    void retrieve_patterns(EN_Project ph, std::vector<std::shared_ptr<Pattern>>& patterns);
    // TODO: void retrieve_curves(EN_Project ph, std::vector<std::shared_ptr<Curve>>& curves);


/*--- Pure virtual methods override---*/
public:
    const std::string& element_name() const override { return l__NAME_PUMP; }
    const unsigned int element_type() const override { return ELEMENT_PUMP; }

/*--- EPANET-dependent PVMs ---*/
public:
    /*--- Properties ---*/
    void retrieve_properties(EN_Project ph) override;

    /*--- Results ---*/
    void retrieve_results(EN_Project ph, long t) override;


}; // class Pump

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__PUMP_HPP
