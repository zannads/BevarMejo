#include <string>

#include "epanet2_2.h"

#include "bevarmejo/bemexcept.hpp"

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"
#include "bevarmejo/wds/elements/element.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "curve.hpp"

namespace bevarmejo::wds
{

Curve::Curve(const WaterDistributionSystem& wds, const EN_Name_t& name) :
    Element(wds, name)
{ }

void Curve::retrieve_EN_index()
{
    assert(m__wds.ph() != nullptr);

    m__en_index = 0;
    int index = 0;
    int errorcode = EN_getcurveindex(m__wds.ph(), m__name.c_str(), &index);
    if (errorcode > 100)
        __format_and_throw<std::runtime_error>("Curve", "retrieve_EN_index", "Error retrieving index of curve.",
            "Error code: ", errorcode,
            "Curve ID: ", m__name);

    m__en_index = index;
}

} // namespace bevarmejo::wds
