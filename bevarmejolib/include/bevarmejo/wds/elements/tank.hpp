#ifndef BEVARMEJOLIB__WDS_ELEMENTS__TANK_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__TANK_HPP

#include <memory>
#include <string>

#include "epanet2_2.h"

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/source.hpp"

#include "bevarmejo/wds/auxiliary/curves.hpp"

namespace bevarmejo {
namespace wds {

/// WDS Tank
/*******************************************************************************
 * The wds::junction class represents a Tank in the network. It is a dynamic element.
 ******************************************************************************/

static const std::string l__NAME_TANK= "Tank";

static const std::string l__DIAMETER = "Diameter";
static const std::string l__INITIAL_LEVEL = "InitL";
static const std::string l__MIN_LEVEL = "MinL";
static const std::string l__LEVEL = "L";
static const std::string l__MAX_LEVEL = "MaxL";
static const std::string l__INITIAL_VOLUME = "InitV";
static const std::string l__MIN_VOLUME = "MinV";
static const std::string l__VOLUME = "V";
static const std::string l__MAX_VOLUME = "MaxV";
static const std::string l__CAN_OVERFLOW = "Overf";

class Tank : public Source {

public:
    using inherited= Source;

/*--- Attributes ---*/
protected:
    /*--- Properties ---*/
    //MIXMODEL
    aux::QuantitySeries<double> m__diameter; // Constant
    std::shared_ptr<VolumeCurve> m__volume_curve;
    aux::QuantitySeries<double> m__min_volume; // Constant
    aux::QuantitySeries<double> m__min_level; // Constant
    aux::QuantitySeries<double> m__max_level; // Constant
    //MIXFRACTION
    //TANK_KBULK
    aux::QuantitySeries<int> m__can_overflow; // Constant
    aux::QuantitySeries<double> m__initial_level; // Constant
    /*---  Read-only Props ---*/ //Will become a method one day
    aux::QuantitySeries<double> m__initial_volume; // Constant
    aux::QuantitySeries<double> m__max_volume; // Constant
    //MIXZONEVOL
    /*---  Results   ---*/
    aux::QuantitySeries<double> m__level;
    aux::QuantitySeries<double> m__volume;

/*--- Constructors ---*/
public:
    Tank() = delete;
    Tank(const std::string& id, const WaterDistributionSystem& wds);

    // Copy constructor
    Tank(const Tank& other);

    // Move constructor
    Tank(Tank&& rhs) noexcept;

    // Copy assignment operator
    Tank& operator=(const Tank& rhs);

    // Move assignment operator
    Tank& operator=(Tank&& rhs) noexcept;

    virtual ~Tank() = default;

/*--- Getters and setters ---*/
public:
    /*--- Properties ---*/
    aux::QuantitySeries<double>& diameter() { return m__diameter; }
    const aux::QuantitySeries<double>& diameter() const { return m__diameter; }
    void diameter(const double a_diameter) { m__diameter.value(a_diameter); }

    std::shared_ptr<VolumeCurve> volume_curve() { return m__volume_curve; }
    std::shared_ptr<VolumeCurve> volume_curve() const { return m__volume_curve; }
    void volume_curve(const std::shared_ptr<VolumeCurve> a_volume_curve) { m__volume_curve = a_volume_curve; }

    aux::QuantitySeries<double>& min_volume() { return m__min_volume; }
    const aux::QuantitySeries<double>& min_volume() const { return m__min_volume; }
    void min_volume(const double a_min_volume) { m__min_volume.value(a_min_volume); }

    aux::QuantitySeries<double>& min_level() { return m__min_level; }
    const aux::QuantitySeries<double>& min_level() const { return m__min_level; }
    void min_level(const double a_min_level) { m__min_level.value(a_min_level); }

    aux::QuantitySeries<double>& max_level() { return m__max_level; }
    const aux::QuantitySeries<double>& max_level() const { return m__max_level; }
    void max_level(const double a_max_level) { m__max_level.value(a_max_level); }

    aux::QuantitySeries<int>& can_overflow() { return m__can_overflow; }
    const aux::QuantitySeries<int>& can_overflow() const { return m__can_overflow; }
    void can_overflow(const int a_can_overflow) { m__can_overflow.value(a_can_overflow); }

    aux::QuantitySeries<double>& initial_level() { return m__initial_level; }
    const aux::QuantitySeries<double>& initial_level() const { return m__initial_level; }
    void initial_level(const double a_initial_level) { m__initial_level.value(a_initial_level); }

    /*---  Read-only Props ---*/ //Will become a method one day
    const aux::QuantitySeries<double>& initial_volume() const { return m__initial_volume; }
    const aux::QuantitySeries<double>& max_volume() const { return m__max_volume; }

    /*---  Results   ---*/
    const aux::QuantitySeries<double>& level() const { return m__level; }
    const aux::QuantitySeries<double>& volume() const { return m__volume; }

/*--- Pure virtual methods override---*/
public:
    /*--- Properties ---*/
    const std::string& element_name() const override { return l__NAME_TANK; }
    const unsigned int element_type() const override { return ELEMENT_TANK; }

    /*--- Results ---*/

/*--- EPANET-dependent PVMs override ---*/
public:
    /*--- Properties ---*/
private:
    void __retrieve_EN_properties(EN_Project ph) override;
public:
    /*--- Results ---*/
    void retrieve_results(EN_Project ph, long t) override;

}; // class Tank

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__TANK_HPP
