#include <cassert>
#include <string>
#include <vector>

#include "epanet2_2.h"

#include "bevarmejo/bemexcept.hpp"

#include "bevarmejo/wds/elements/element.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "pattern.hpp"

namespace bevarmejo::wds
{

Pattern::Pattern(const WaterDistributionSystem& wds, const EN_Name_t& name) :
    Element(wds, name),
    m__multipliers()
{
    if (m__wds.ph() == nullptr)
        return;

    this->retrieve_EN_index();
    this->retrieve_EN_properties();
}

void Pattern::retrieve_EN_index()
{
    assert(m__wds.ph() != nullptr);

    m__en_index = 0;
    int index = 0;
    int errorcode = EN_getpatternindex(m__wds.ph(), m__name.c_str(), &index);
    if (errorcode > 100)
        __format_and_throw<std::runtime_error>("Pattern", "retrieve_EN_index", "Error retrieving index of pattern.",
            "Error code: ", errorcode,
            "Pattern ID: ", m__name);

    m__en_index = index;
}

void Pattern::retrieve_EN_properties()
{
    assert(m__en_index > 0);
    assert(m__wds.ph() != nullptr);

    int len = 0;
    int errorcode = EN_getpatternlen(m__wds.ph(), m__en_index, &len);
    if (errorcode > 100)
        __format_and_throw<std::runtime_error>("Pattern", "retrieve_EN_properties", "Error retrieving the properties of pattern.",
            "Error while retrieving the length of the pattern.",
            "Error code: ", errorcode,
            "Pattern ID: ", m__name);
    
    m__multipliers.clear();
    m__multipliers.reserve(len);

    // Start from +1. See epanet documentation.
    double val = 0.0;
    for(int i = 1; i <= len; ++i)
    {
        errorcode = EN_getpatternvalue(m__wds.ph(), m__en_index, i, &val); 
        if (errorcode > 100)
            __format_and_throw<std::runtime_error>("Pattern", "retrieve_EN_properties", "Error retrieving the properties of pattern.",
                "Error while retrieving the value of the pattern.",
                "Error code: ", errorcode,
                "Pattern ID: ", m__name,
                "Value: ", i,
                "Length: ", len);
        
        m__multipliers.push_back(val);
    }
}

} // namespace bevarmejo::wds
