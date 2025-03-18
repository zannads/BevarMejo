#ifndef BEVARMELIB__WDS_ELEMENTS__DIMENSIONED_LINK_HPP
#define BEVARMELIB__WDS_ELEMENTS__DIMENSIONED_LINK_HPP

#include <string>

#include "bevarmejo/wds/utility/quantity_series.hpp"

#include "bevarmejo/wds/elements/network_elements/link.hpp"

namespace bevarmejo::wds
{

class DimensionedLink;
template <>
struct TypeTraits<DimensionedLink>
{
    static constexpr const char* name = "DimensionedLink";
    static constexpr unsigned int code = 1211;
    static constexpr bool is_EN_complete = false;
};

class DimensionedLink : public Link
{
/// WDS dimensioned Link
/*******************************************************************************
 * The wds::DimensionedLink class represents a dimensioned link in the network.
 ******************************************************************************/

/*------- Member types -------*/
public:
    using self_type = DimensionedLink;
    using self_traits = TypeTraits<self_type>;
    using inherited = Link;
    using VelocitySeries = aux::QuantitySeries<double>;
private:
    friend class WaterDistributionSystem;

/*------- Member objects -------*/
protected:
    // === Properties ===
    aux::QuantitySeries<double> m__diameter;
    aux::QuantitySeries<double> m__roughness;
    aux::QuantitySeries<double> m__minor_loss;
    aux::QuantitySeries<double> m__bulk_coeff;
    aux::QuantitySeries<double> m__wall_coeff;

    // === Results ===
    VelocitySeries m__velocity;

/*------- Member functions -------*/
// (constructor)
public:
    DimensionedLink() = delete;
    DimensionedLink(const WaterDistributionSystem& wds, const EN_Name_t& name); // Constructor
    
// (destructor)
public:
    virtual ~DimensionedLink() = default;

// clone()
public:

/*------- Operators -------*/
// operator=
public:

/*------- Element access -------*/
public:
    // === Read/Write properties ===
    aux::QuantitySeries<double>& diameter() { return m__diameter; }
    const aux::QuantitySeries<double>& diameter() const { return m__diameter; }
    void diameter(const double a_diameter) { m__diameter.value(a_diameter); }

    aux::QuantitySeries<double>& roughness() { return m__roughness; }
    const aux::QuantitySeries<double>& roughness() const { return m__roughness; }
    void roughness(const double a_roughness) { m__roughness.value(a_roughness); }

    aux::QuantitySeries<double>& minor_loss() { return m__minor_loss; }
    const aux::QuantitySeries<double>& minor_loss() const { return m__minor_loss; }
    void minor_loss(const double a_minor_loss) { m__minor_loss.value(a_minor_loss); }

    aux::QuantitySeries<double>& bulk_coeff() { return m__bulk_coeff; }
    const aux::QuantitySeries<double>& bulk_coeff() const { return m__bulk_coeff; }
    void bulk_coeff(const double a_bulk_coeff) { m__bulk_coeff.value(a_bulk_coeff); }

    aux::QuantitySeries<double>& wall_coeff() { return m__wall_coeff; }
    const aux::QuantitySeries<double>& wall_coeff() const { return m__wall_coeff; }
    void wall_coeff(const double a_wall_coeff) { m__wall_coeff.value(a_wall_coeff); }

    // === Results ===
    const VelocitySeries& velocity() const { return m__velocity; }

/*------- Capacity -------*/
public:

/*------- Modifiers -------*/
public:
    virtual void clear_results() override;

    virtual void retrieve_EN_properties() override;
    virtual void retrieve_EN_results() override;
private:
    void __retrieve_EN_properties();
    void __retrieve_EN_results();

}; // class DimensionedLink

} // namespace bevarmejo::wds

#endif // BEVARMELIB__WDS_ELEMENTS__DIMENSIONED_LINK_HPP
