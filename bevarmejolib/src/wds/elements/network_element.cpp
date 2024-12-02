#include <string>

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"
#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/water_distribution_system.hpp"

#include "network_element.hpp"

namespace bevarmejo::wds
{

NetworkElement::NetworkElement(const WaterDistributionSystem& wds, const EN_Name_t& name) :
    inherited(wds, name),
    m__ud_results()
{ }

void NetworkElement::clear_results()
{
    for (auto& [name, result] : m__ud_results)
        result->clear();
}

void NetworkElement::retrieve_EN_results()
{
    // For each user-defined result, retrieve the value using the lambda inside the map.
    // and knowing that all other properties are already retrieved.  
}

} // namespace bevarmejo::wds
