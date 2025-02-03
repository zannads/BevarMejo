#include <cassert>
#include <string>
#include <vector>

#include "epanet2_2.h"

#include "bevarmejo/utility/except.hpp"

#include "bevarmejo/wds/element.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "elements/pattern.hpp"

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

auto Pattern::make_from_EN_for(const WaterDistributionSystem &wds, const EN_Name_t &name) -> std::unique_ptr<Pattern>
{
    auto p_pattern = std::make_unique<Pattern>(wds, name);
    p_pattern->retrieve_EN_index();
    p_pattern->retrieve_EN_properties();

    return std::move(p_pattern);
}

void Pattern::retrieve_EN_index()
{
    assert(m__wds.ph() != nullptr);

    m__en_index = 0;
    int index = 0;
    int errorcode = EN_getpatternindex(m__wds.ph(), m__name.c_str(), &index);
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the index of the pattern.",
        "Error originating from the EPANET API",
        "Pattern ID: ", m__name);
    
    m__en_index = index;
}

void Pattern::retrieve_EN_properties()
{
    assert(m__en_index > 0);
    assert(m__wds.ph() != nullptr);

    int len = 0;
    int errorcode = EN_getpatternlen(m__wds.ph(), m__en_index, &len);
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the properties of pattern.",
        "Error originating from the EPANET API while retrieving the length of the pattern.",
        "Pattern ID: ", m__name);
    
    m__multipliers.clear();
    m__multipliers.reserve(len);

    // Start from +1. See epanet documentation.
    double val = 0.0;
    for(int i = 1; i <= len; ++i)
    {
        errorcode = EN_getpatternvalue(m__wds.ph(), m__en_index, i, &val);
        beme_throw_if_EN_error(errorcode,
            "Impossible to retrieve the properties of pattern.",
            "Error originating from the EPANET API while retrieving the value of the pattern.",
            "Pattern ID: ", m__name,
            "\n    Position: ", i,
            " Length: ", len);
            
        m__multipliers.push_back(val);
    }
}

} // namespace bevarmejo::wds
