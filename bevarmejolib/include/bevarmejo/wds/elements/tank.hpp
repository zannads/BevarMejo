#ifndef BEVARMEJOLIB__WDS_ELEMENTS__TANK_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__TANK_HPP

#include <memory>
#include <string>

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"
#include "bevarmejo/wds/auxiliary/curves.hpp"

#include "bevarmejo/wds/elements/source.hpp"

namespace bevarmejo::wds
{

class Tank;
template <>
struct TypeTraits<Tank>
{
    static constexpr const char* name = "Tank";
    static constexpr unsigned int code = 22111;
    static constexpr bool is_EN_complete = true;
};

class Tank final : public Source
{
    /// WDS Tank
    /*******************************************************************************
     * The wds::junction class represents a Tank in the network. It is a dynamic element.
     ******************************************************************************/

/*------- Member types -------*/
public:
    using self_type = Tank;
    using self_traits = TypeTraits<self_type>;
    using inherited = Source;
    using LevelSeries = aux::QuantitySeries<double>;
    using VolumeSeries = aux::QuantitySeries<double>;
private:
    friend class WaterDistributionSystem;

/*------- Member objects -------*/
protected:
    // === Properties ===
    //MIXMODEL
    aux::QuantitySeries<double> m__diameter; // Constant
    std::shared_ptr<const VolumeCurve> m__volume_curve;
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

    // === Results ===
    LevelSeries m__level;
    VolumeSeries m__volume;

/*------- Member functions -------*/
// (constructor)
protected:
    Tank() = delete;
    Tank(const WaterDistributionSystem& wds, const EN_Name_t& name); // Constructor

// (destructor)
public:
    virtual ~Tank() = default;

// clone()
public:

/*------- Operators -------*/
// operator=
public:

/*------- Element access -------*/
public:
    // === Read/Write properties ===
    const char* type_name() const override;

    unsigned int type_code() const override;

    aux::QuantitySeries<double>& diameter() { return m__diameter; }
    const aux::QuantitySeries<double>& diameter() const { return m__diameter; }
    void diameter(const double a_diameter) { m__diameter.value(a_diameter); }

    std::shared_ptr<const VolumeCurve> volume_curve() { return m__volume_curve; }
    std::shared_ptr<const VolumeCurve> volume_curve() const { return m__volume_curve; }
    void volume_curve(std::shared_ptr<const VolumeCurve> a_volume_curve) { m__volume_curve = std::move(a_volume_curve); }

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

    // === Read-Only Properties === //Will become a method one day
    const aux::QuantitySeries<double>& initial_volume() const { return m__initial_volume; }
    const aux::QuantitySeries<double>& max_volume() const { return m__max_volume; }

    // === Results ===
    const aux::QuantitySeries<double>& level() const { return m__level; }
    const aux::QuantitySeries<double>& volume() const { return m__volume; }

/*------- Capacity -------*/
public:

/*------- Modifiers -------*/
public:
    void clear_results() override;

    void retrieve_EN_properties() override;

    void retrieve_EN_results() override;
}; // class Tank

} // namespace bevarmejo::wds

#endif // BEVARMEJOLIB__WDS_ELEMENTS__TANK_HPP
