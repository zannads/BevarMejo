#include <string>

#include "epanet2_2.h"

#include "bevarmejo/utility/bemexcept.hpp"

#include "bevarmejo/wds/utility/quantity_series.hpp"
#include "bevarmejo/wds/element.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "curve.hpp"

namespace bevarmejo::wds
{

/*------- Member functions -------*/
// (constructor)
Curve::Curve(const WaterDistributionSystem& wds, const EN_Name_t& name) :
    Element(wds, name)
{ }

/*------- Operators -------*/

/*------- Element access -------*/
EN_Project Curve::pass_ph() const noexcept
{
    return m__wds.ph();
}

void Curve::retrieve_EN_index()
{
    assert(m__wds.ph() != nullptr);

    m__en_index = 0;
    int index = 0;
    int errorcode = EN_getcurveindex(m__wds.ph(), m__name.c_str(), &index);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the index of the curve.",
        "Error code: ", errorcode,
        "Curve ID: ", m__name);
        
    m__en_index = index;
}

} // namespace bevarmejo::wds
