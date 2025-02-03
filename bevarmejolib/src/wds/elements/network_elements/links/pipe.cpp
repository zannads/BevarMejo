#include <cassert>
#include <memory>
#include <string>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/wds/utility/epanet/en_help.hpp"

#include "bevarmejo/utility/exceptions.hpp"

#include "bevarmejo/wds/utility/quantity_series.hpp"

#include "bevarmejo/wds/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/network_elements/link.hpp"
#include "bevarmejo/wds/elements/network_elements/links/dimensioned_link.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "bevarmejo/wds/elements/network_elements/links/pipe.hpp"

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

// (EPANET constructor)
auto Pipe::make_from_EN_for(const WaterDistributionSystem& wds, const EN_Name_t& name) -> std::unique_ptr<Pipe>
{
    auto p_pipe = std::make_unique<Pipe>(wds, name);
    p_pipe->retrieve_EN_index();
    p_pipe->retrieve_EN_properties();

    return std::move(p_pipe);
}

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
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the properties of the pipe.",
        "Error originating from the EPANET API while retrieving value: EN_LENGTH",
        "Pipe ID: ", m__name);

    if(ph->parser.Unitsflag == US)
        val *= MperFT; // from ft to m
    m__length = val;
}

} // namespace bevarmejo::wds