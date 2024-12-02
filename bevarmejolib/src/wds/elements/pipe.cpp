#include <cassert>
#include <memory>
#include <string>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/wds/epanet_helpers/en_help.hpp"

#include "bevarmejo/bemexcept.hpp"

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/link.hpp"
#include "bevarmejo/wds/elements/dimensioned_link.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "pipe.hpp"

namespace bevarmejo::wds
{

/*------- Member functions -------*/
// (constructor)

Pipe::Pipe(const WaterDistributionSystem& wds, const EN_Name_t& name) : 
    inherited(wds, name),
    m__length(wds.time_series(label::__CONSTANT_TS))
{ }

Pipe::Pipe(const WaterDistributionSystem& wds, const EN_Name_t& name, const Pipe& other) :
    inherited(wds, name), // This should be inherited(wds, name, other) // TODO: It should call the copy properties constructor.
    m__length(wds.time_series(label::__CONSTANT_TS), other.m__length)
{ }

/*------- Operators -------*/

/*------- Element access -------*/
const char* Pipe::type_name() const
{
    return self_traits::name;
}

unsigned int Pipe::type_code() const
{
    return self_traits::code;
}

/*------- Capacity -------*/

/*------- Modifiers -------*/
void Pipe::retrieve_EN_properties()
{
    this->__retrieve_EN_properties();

    inherited::retrieve_EN_properties();
}

void Pipe::__retrieve_EN_properties()
{   
    assert(m__en_index > 0);
    assert(m__wds.ph() != nullptr);

    auto ph = m__wds.ph();

    double val = 0.0;
    int errorcode = EN_getlinkvalue(ph, m__en_index, EN_LENGTH, &val);
    if (errorcode > 100)
        __format_and_throw<std::runtime_error>("Pipe", "retrieve_EN_properties", "Error retrieving properties of pipe.",
            "Property: EN_LENGTH",
            "Error code: ", errorcode,
            "Pipe ID: ", m__name);

    if(ph->parser.Unitsflag == US)
        val *= MperFT; // from ft to m
    m__length = val;
}

} // namespace bevarmejo::wds