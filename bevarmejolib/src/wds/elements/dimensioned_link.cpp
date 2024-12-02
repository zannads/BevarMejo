#include <cassert>
#include <string>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/wds/epanet_helpers/en_help.hpp"

#include "bevarmejo/bemexcept.hpp"

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/link.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "dimensioned_link.hpp"

namespace bevarmejo::wds
{

/*------- Member functions -------*/
// (constructor)
DimensionedLink::DimensionedLink(const WaterDistributionSystem& wds, const EN_Name_t& name) : 
    inherited(wds, name),
    m__diameter(wds.time_series(label::__CONSTANT_TS)),
    m__roughness(wds.time_series(label::__CONSTANT_TS)),
    m__minor_loss(wds.time_series(label::__CONSTANT_TS)),
    m__bulk_coeff(wds.time_series(label::__CONSTANT_TS)),
    m__wall_coeff(wds.time_series(label::__CONSTANT_TS)),
    m__velocity(wds.time_series(label::__RESULTS_TS))
{ }

/*------- Operators -------*/

/*------- Element access -------*/

/*------- Capacity -------*/

/*------- Modifiers -------*/
void DimensionedLink::clear_results()
{
    inherited::clear_results();

    m__velocity.clear();
}

void DimensionedLink::retrieve_EN_properties()
{
    this->__retrieve_EN_properties();

    inherited::retrieve_EN_properties();
}

void DimensionedLink::__retrieve_EN_properties()
{   
    assert(m__en_index > 0);
    assert(m__wds.ph() != nullptr);

    auto ph = m__wds.ph();

    double val = 0;
    int errorcode = EN_getlinkvalue(ph, m__en_index, EN_DIAMETER, &val);
    if (errorcode > 100)
        __format_and_throw<std::runtime_error>("DimensionedLink", "retrieve_EN_properties", "Error retrieving properties of link.",
            "Property: EN_DIAMETER",
            "Error code: ", errorcode,
            "Link ID: ", m__name);

    if (ph->parser.Unitsflag == US)
        val = val/12*MperFT*1000; // from inches to ft, then to m, finally to mm
    m__diameter = val;

    errorcode = EN_getlinkvalue(ph, m__en_index, EN_ROUGHNESS, &val);
    if (errorcode > 100)
        __format_and_throw<std::runtime_error>("DimensionedLink", "retrieve_EN_properties", "Error retrieving properties of link.",
            "Property: EN_ROUGHNESS",
            "Error code: ", errorcode,
            "Link ID: ", m__name);

    // for now only HW coeff is supported
    m__roughness = val;

    errorcode = EN_getlinkvalue(ph, m__en_index, EN_MINORLOSS, &val);
    if (errorcode > 100)
        __format_and_throw<std::runtime_error>("DimensionedLink", "retrieve_EN_properties", "Error retrieving properties of link.",
            "Property: EN_MINORLOSS",
            "Error code: ", errorcode,
            "Link ID: ", m__name);
    
    // DIMLESS
    m__minor_loss = val;

    errorcode = EN_getlinkvalue(ph, m__en_index, EN_KBULK, &val);
    if (errorcode > 100)
        __format_and_throw<std::runtime_error>("DimensionedLink", "retrieve_EN_properties", "Error retrieving properties of link.",
            "Property: EN_KBULK",
            "Error code: ", errorcode,
            "Link ID: ", m__name);
    
    // for now I don't care about this
    m__bulk_coeff = val;

    errorcode = EN_getlinkvalue(ph, m__en_index, EN_KWALL, &val);
    if (errorcode > 100)
        __format_and_throw<std::runtime_error>("DimensionedLink", "retrieve_EN_properties", "Error retrieving properties of link.",
            "Property: EN_KWALL",
            "Error code: ", errorcode,
            "Link ID: ", m__name);
    
    // for now I don't care about this
    m__wall_coeff= val;
}

void DimensionedLink::retrieve_EN_results()
{
    this->__retrieve_EN_results();

    inherited::retrieve_EN_results();
}

void DimensionedLink::__retrieve_EN_results()
{
    assert(m__en_index > 0);
    assert(m__wds.ph() != nullptr);

    auto ph = m__wds.ph();
    auto t = m__wds.current_result_time();

    double val = 0;
    int errorcode = EN_getlinkvalue(ph, m__en_index, EN_VELOCITY, &val);
    if (errorcode > 100)
        __format_and_throw<std::runtime_error>("DimensionedLink", "retrieve_EN_results", "Error retrieving results of link.",
            "Property: EN_VELOCITY",
            "Error code: ", errorcode,
            "Link ID: ", m__name);

    // Before saving I need to conver it to m/s
    if (ph->parser.Unitsflag == US)
        val = val*MperFT; // from ft/s to m/s

    m__velocity.commit(t, val);
}

} // namespace bevarmejo::wds
