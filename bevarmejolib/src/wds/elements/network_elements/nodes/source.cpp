#include <string>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/utility/except.hpp"

#include "bevarmejo/wds/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/network_elements/node.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "bevarmejo/wds/elements/network_elements/nodes/source.hpp"

namespace bevarmejo::wds
{

/*------- Member functions -------*/
// (constructor)
Source::Source(const WaterDistributionSystem& wds, const EN_Name_t& name) :
    inherited(wds, name),
    m__inflow(wds.time_series(label::__RESULTS_TS)),
    m__source_elevation(wds.time_series(label::__RESULTS_TS))
{ }

/*------- Element access -------*/
// === Read/Write properties ===

// === Read-only properties ===
bool Source::has_demand() const
{
    return false;
}

// === Results ===
auto Source::inflow() const -> const FlowSeries&
{
    return m__inflow;
}

/*------- Capacity -------*/

/*------- Modifiers -------*/

void Source::clear_results()
{
    inherited::clear_results();

    m__inflow.clear();
    m__source_elevation.clear();
}

void Source::retrieve_EN_results()
{
    this->__retrieve_EN_results();

    inherited::retrieve_EN_results();
}

void Source::__retrieve_EN_results()
{
    assert( m__en_index > 0 );
    assert( m__wds.ph() != nullptr );

    auto ph = m__wds.ph();
    auto t = m__wds.current_result_time();

    double val = 0.0;
    int errorcode = EN_getnodevalue(ph, m__en_index, EN_DEMAND, &val);
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the results of the source.",
        "Error originating from the EPANET API while retrieving value: EN_DEMAND",
        "Source ID: ", m__name);
    
    if (ph->parser.Flowflag != LPS)
        val = epanet::convert_flow_to_L_per_s(ph, val);
    m__inflow.commit(t, val);

    errorcode = EN_getnodevalue(ph, m__en_index, EN_HEAD, &val);
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the results of the source.",
        "Error originating from the EPANET API while retrieving value: EN_HEAD",
        "Source ID: ", m__name);

    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    m__source_elevation.commit(t, val);
}


} // namespace bevarmejo::wds
