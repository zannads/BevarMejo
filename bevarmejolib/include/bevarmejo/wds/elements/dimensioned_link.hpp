#ifndef BEVARMELIB__WDS_ELEMENTS__DIMENSIONED_LINK_HPP
#define BEVARMELIB__WDS_ELEMENTS__DIMENSIONED_LINK_HPP

#include <string>
#include <variant>

#include "bevarmejo/wds/elements/variable.hpp"
#include "bevarmejo/wds/elements/link.hpp"
#include "bevarmejo/wds/elements/element.hpp"

namespace bevarmejo {
namespace wds {

/// WDS dimensioned link
/*******************************************************************************
 * The wds::dimensioned_link class represents a dimensioned link in the network.
 ******************************************************************************/

static const std::string L_DIAMETER= "Diameter";
constexpr double kmin_diameter= 0.001;
static const std::string L_ROUGHNESS= "Roughness";
static const std::string L_MINOR_LOSS= "Minor Loss";
static const std::string L_BULK_COEFF= "Bulk Coefficient";
static const std::string L_WALL_COEFF= "Wall Coefficient";
static const std::string L_VELOCITY= "Velocity";

class dimensioned_link : public link {

public:
    using inherited= link;

protected:
    // pointer to variables 
    vars::var_real* _diameter_;
    vars::var_real* _roughness_;
    vars::var_real* _minor_loss_;
    vars::var_real* _bulk_coeff_;
    vars::var_real* _wall_coeff_;

    // pointer to results
    vars::var_tseries_real* _velocity_;

    void _add_properties() override;
    void _add_results() override;
    void _update_pointers() override;

public:
    // Constructors
    dimensioned_link() = delete;
    dimensioned_link(const std::string& id);

    // Copy constructor
    dimensioned_link(const dimensioned_link& other);

    // Move constructor
    dimensioned_link(dimensioned_link&& rhs) noexcept;

    // Copy assignment operator
    dimensioned_link& operator=(const dimensioned_link& rhs);

    // Move assignment operator
    dimensioned_link& operator=(dimensioned_link&& rhs) noexcept;

    // Destructor
    virtual ~dimensioned_link();

    vars::var_real& diameter() const { return *_diameter_; }
    vars::var_real& roughness() const { return *_roughness_; }
    vars::var_real& minor_loss() const { return *_minor_loss_; }
    vars::var_real& bulk_coeff() const { return *_bulk_coeff_; }
    vars::var_real& wall_coeff() const { return *_wall_coeff_; }

    vars::var_tseries_real& velocity() const { return *_velocity_; }

};

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMELIB__WDS_ELEMENTS__DIMENSIONED_LINK_HPP
