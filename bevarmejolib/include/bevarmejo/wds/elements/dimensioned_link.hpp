#ifndef BEVARMELIB__WDS_ELEMENTS__DIMENSIONED_LINK_HPP
#define BEVARMELIB__WDS_ELEMENTS__DIMENSIONED_LINK_HPP

#include <string>

#include "epanet2_2.h"

#include "bevarmejo/wds/data_structures/temporal.hpp"
#include "bevarmejo/wds/data_structures/variable.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
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
    vars::var_real* _diameter_;
    vars::var_real* _roughness_;
    vars::var_real* _minor_loss_;
    vars::var_real* _bulk_coeff_;
    vars::var_real* _wall_coeff_;

    /*---  Results   ---*/
    vars::var_tseries_real* _velocity_;

protected:
    void _add_properties() override;
    void _add_results() override;
    void _update_pointers() override;

/*--- Constructors ---*/
public:
    DimensionedLink() = delete;
    DimensionedLink(const std::string& id);

    // Copy constructor
    DimensionedLink(const DimensionedLink& other);

    // Move constructor
    DimensionedLink(DimensionedLink&& rhs) noexcept;

    // Copy assignment operator
    DimensionedLink& operator=(const DimensionedLink& rhs);

    // Move assignment operator
    DimensionedLink& operator=(DimensionedLink&& rhs) noexcept;

    // Destructor
    virtual ~DimensionedLink();

/*--- Getters and setters ---*/
public:
    /*--- Properties ---*/
    vars::var_real& diameter() const { return *_diameter_; }
    void diameter(const double a_diameter) { _diameter_->value(a_diameter); }
    vars::var_real& roughness() const { return *_roughness_; }
    void roughness(const double a_roughness) { _roughness_->value(a_roughness); }
    vars::var_real& minor_loss() const { return *_minor_loss_; }
    vars::var_real& bulk_coeff() const { return *_bulk_coeff_; }
    vars::var_real& wall_coeff() const { return *_wall_coeff_; }

    /*---  Results   ---*/
    vars::var_tseries_real& velocity() const { return *_velocity_; }

/*--- Pure virtual methods override---*/

/*--- EPANET-dependent PVMs override ---*/
public:
    void retrieve_properties(EN_Project ph) override;
    void retrieve_results(EN_Project ph, long t) override;

};

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMELIB__WDS_ELEMENTS__DIMENSIONED_LINK_HPP
