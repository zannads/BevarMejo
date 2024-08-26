#ifndef BEVARMELIB__WDS_ELEMENTS__DIMENSIONED_LINK_HPP
#define BEVARMELIB__WDS_ELEMENTS__DIMENSIONED_LINK_HPP

#include <string>

#include "epanet2_2.h"

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/link.hpp"

namespace bevarmejo {
namespace wds {

/// WDS dimensioned Link
/*******************************************************************************
 * The wds::DimensionedLink class represents a dimensioned link in the network.
 ******************************************************************************/

static const std::string L_DIAMETER= "Diameter";
constexpr double kmin_diameter= 0.001;
static const std::string L_ROUGHNESS= "Roughness";
static const std::string L_MINOR_LOSS= "Minor Loss";
static const std::string L_BULK_COEFF= "Bulk Coefficient";
static const std::string L_WALL_COEFF= "Wall Coefficient";
static const std::string L_VELOCITY= "Velocity";

class DimensionedLink : public Link {

public:
    using inherited= Link;

/*--- Attributes ---*/
protected:
    /*--- Properties ---*/
    aux::QuantitySeries<double> m__diameter;
    aux::QuantitySeries<double> m__roughness;
    aux::QuantitySeries<double> m__minor_loss;
    aux::QuantitySeries<double> m__bulk_coeff;
    aux::QuantitySeries<double> m__wall_coeff;

    /*---  Results   ---*/
    aux::QuantitySeries<double> m__velocity;

/*--- Constructors ---*/
public:
    DimensionedLink() = delete;
    DimensionedLink(const std::string& id, const WaterDistributionSystem& wds);

    // Copy constructor
    DimensionedLink(const DimensionedLink& other);

    // Move constructor
    DimensionedLink(DimensionedLink&& rhs) noexcept;

    // Copy assignment operator
    DimensionedLink& operator=(const DimensionedLink& rhs);

    // Move assignment operator
    DimensionedLink& operator=(DimensionedLink&& rhs) noexcept;

    // Destructor
    virtual ~DimensionedLink() = default;

/*--- Getters and setters ---*/
public:
    /*--- Properties ---*/
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

    /*---  Results   ---*/
    const aux::QuantitySeries<double>& velocity() const { return m__velocity; }

/*--- Pure virtual methods override---*/

/*--- EPANET-dependent PVMs override ---*/
public:
    void retrieve_results(EN_Project ph, long t) override;
protected:
    void __retrieve_EN_properties(EN_Project ph) override;

};

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMELIB__WDS_ELEMENTS__DIMENSIONED_LINK_HPP
