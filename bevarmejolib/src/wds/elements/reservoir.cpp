#include <string>

#include "epanet2_2.h"

#include "bevarmejo/utility/bemexcept.hpp"

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

auto Reservoir::make_from_EN_for(const WaterDistributionSystem& wds, const EN_Name_t& name) -> std::unique_ptr<Reservoir>
{
    auto p_reservoir = std::make_unique<Reservoir>(wds, name);
    p_reservoir->retrieve_EN_index();
    p_reservoir->retrieve_EN_properties();

    return std::move(p_reservoir);
}

const char* Reservoir::type_name() const
{
    return self_traits::name;
}

unsigned int Reservoir::type_code() const
{
    return self_traits::code;
}

} // namespace bevarmejo::wds
