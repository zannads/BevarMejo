#include <string>

#include "epanet2_2.h"

#include "bevarmejo/bemexcept.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/source.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "reservoir.hpp"

namespace bevarmejo::wds
{

Reservoir::Reservoir(const WaterDistributionSystem& wds, const EN_Name_t& name) :
    inherited(wds, name)
{ }

const char* Reservoir::type_name() const
{
    return self_traits::name;
}

unsigned int Reservoir::type_code() const
{
    return self_traits::code;
}

} // namespace bevarmejo::wds
