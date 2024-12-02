//
// element.cpp
//
// Created by Dennis Zanutto on 20/10/23.

#include <string>

#include "epanet2_enums.h"

#include "bevarmejo/bemexcept.hpp"
#include "bevarmejo/wds/auxiliary/quantity_series.hpp"
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
    if (name.size() > EN_MAXID)
        __format_and_throw<std::invalid_argument>("Element", "Element", "The element ID/name is too long.",
            "To allow for EPANET compatibility the ID/name must be shorter than 32 characters.",
            "ID/name: ", name,
            "Length: ", name.size());

    // Let's retrieve the index and the properties of the element, if possible.
    if (m__wds.ph() == nullptr)
        return;

    this->retrieve_EN_index();
    this->retrieve_EN_properties();
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

} // namespace bevarmejo::wds
