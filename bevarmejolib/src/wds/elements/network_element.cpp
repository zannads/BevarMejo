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

auto NetworkElement::results() const -> const ResultsMap&
{
    return m__ud_results;
}

void NetworkElement::clear_results()
{
    m__ud_results.clear();
}

} // namespace bevarmejo::wds