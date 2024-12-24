//
// element.cpp
//
// Created by Dennis Zanutto on 20/10/23.

#include <string>

#include "epanet2_enums.h"

#include "bevarmejo/utility/bemexcept.hpp"
#include "bevarmejo/wds/utility/quantity_series.hpp"
#include "bevarmejo/wds/water_distribution_system.hpp"

#include "element.hpp"

namespace bevarmejo::wds
{

Element::Element(const WaterDistributionSystem& wds, const EN_Name_t& name) :
    m__wds(wds),
    m__name(name),
    m__en_index(0),
    m__ud_properties()
{
    // For EPANET compatibility the string must be shorter then EN_MAXID
    beme_throw_if(name.size() > EN_MAXID, std::invalid_argument,
        "Impossible to create the element.",
        "The element ID/name is too long (must be shorter than 32 characters for EPANET compatibility).",
        "ID/name: ", name,
        "Length: ", name.size());
}

auto Element::name() const -> const EN_Name_t&
{
    return m__name;
}

auto Element::EN_id() const -> const EN_Name_t&
{
    return name();
}

auto Element::EN_index() const -> EN_Index_t
{
    return m__en_index;
}

/*------- Modifiers -------*/
void Element::retrieve_EN_properties()
{
    // As for the NetworkElement and the ud results. 
    // TODO
}

} // namespace bevarmejo::wds
