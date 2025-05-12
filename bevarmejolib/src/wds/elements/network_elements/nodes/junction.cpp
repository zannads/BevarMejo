#include <cassert>
#include <string>
#include <unordered_map>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/wds/utility/epanet/en_help.hpp"

#include "bevarmejo/wds/elements/pattern.hpp"
#include "bevarmejo/wds/utility/quantity_series.hpp"

#include "bevarmejo/wds/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/network_elements/node.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "bevarmejo/wds/elements/network_elements/nodes/junction.hpp"

namespace bevarmejo::wds
{

/*------- Member functions -------*/
// (constructor)
Junction::Junction(const WaterDistributionSystem& wds, const EN_Name_t& name) :
    inherited(wds, name),
    m__demands(),
    m__demand(wds.time_series(label::__RESULTS_TS)),
    m__consumption(wds.time_series(label::__RESULTS_TS)),
    m__undelivered_demand(wds.time_series(label::__RESULTS_TS))
{ }

// (EPANET constructor)
auto Junction::make_from_EN_for(const WaterDistributionSystem& wds, const EN_Name_t& name) -> std::unique_ptr<Junction>
{
    auto p_junction = std::make_unique<Junction>(wds, name);
    p_junction->retrieve_EN_index();
    p_junction->retrieve_EN_properties();

    return std::move(p_junction);
}

/*------- Operators -------*/

/*------- Element access -------*/
// === Read/Write properties ===
const char* Junction::type_name() const
{
    return self_traits::name;
}

unsigned int Junction::type_code() const
{
    return self_traits::code;
}

// === Read-only properties ===
bool Junction::has_demand() const
{
    return !m__demands.empty();
}

auto Junction::demand() const -> const FlowSeries&
{
    return m__demand;
}
auto Junction::demand_requested() const -> const FlowSeries&
{
    return demand();
}

// === Results ===
auto Junction::demand_delivered() const -> const FlowSeries&
{
    return consumption();
}

auto Junction::consumption() const -> const FlowSeries&
{
    return m__consumption;
}

auto Junction::demand_undelivered() const -> const FlowSeries&
{
    return m__undelivered_demand;
}

/*------- Capacity -------*/

/*------- Modifiers -------*/
void Junction::clear_results()
{
    inherited::clear_results();

    m__demand.clear();
    m__consumption.clear();
    m__undelivered_demand.clear();
}

void Junction::retrieve_EN_properties()
{
    this->__retrieve_EN_properties();

    inherited::retrieve_EN_properties();
}

void Junction::__retrieve_EN_properties()
{
    assert(m__en_index > 0);
    assert(m__wds.ph() != nullptr);

    auto ph = m__wds.ph();

    int n_demands= 0;
    int errorcode= EN_getnumdemands(ph, m__en_index, &n_demands);
    assert(errorcode < 100);

    for (int i= 1; i <= n_demands; ++i) {
        double base_demand= 0.0;
        errorcode= EN_getbasedemand(ph, m__en_index, i, &base_demand);
        assert(errorcode < 100);

        char __demand_category[EN_MAXID+1];
        errorcode= EN_getdemandname(ph, m__en_index, i, __demand_category);
        assert(errorcode < 100);
        std::string demand_category(__demand_category);

        int pattern_index= 0;
        errorcode= EN_getdemandpattern(ph, m__en_index, i, &pattern_index);
        assert(errorcode < 100);

        // EPANET uses index == 0 to signal no pattern, i.e., constant demand
        if (pattern_index == 0)
        {   
            aux::QuantitySeries<double> cdemand(m__wds.time_series(label::__CONSTANT_TS));
            cdemand.commit(0l, base_demand);

            m__demands.insert(std::make_pair(demand_category, cdemand));
        }
        else // It's a pattern demand
        {   
            char __pattern_id[EN_MAXID+1];
            errorcode= EN_getpatternid(ph, pattern_index, __pattern_id);
            assert(errorcode < 100);
            std::string pattern_id(__pattern_id);

            aux::QuantitySeries<double> pdemand(m__wds.time_series(label::__EN_PATTERN_TS));

            const auto& pattern = m__wds.pattern(pattern_id);

            // TODO: this is very much wrong because it doesn't consider the shift time step 
            // and that patterns may have a different length and I may need to wrap around.
            auto ilen= m__wds.time_series(label::__EN_PATTERN_TS).size();
            for (auto i= 0l; i < ilen; ++i)
            {
                auto __time= m__wds.time_series(label::__EN_PATTERN_TS).at(i);
                pdemand.commit(__time, base_demand * pattern.at(i % pattern.size()));
            }

            m__demands.insert(std::make_pair(demand_category, pdemand));
        }
    }   
}

void Junction::retrieve_EN_results()
{
    inherited::retrieve_EN_results();

    this->__retrieve_EN_results();
}

void Junction::__retrieve_EN_results()
{
    assert(m__en_index > 0);
    assert(m__wds.ph() != nullptr);

    auto ph = m__wds.ph();
    auto t = m__wds.current_result_time();

    double undeliv = 0.0;
    int errorcode = EN_getnodevalue(ph, m__en_index, EN_DEMANDDEFICIT, &undeliv);
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the properties of the junction.",
        "Error originating from the EPANET API while retrieving value: EN_DEMANDDEFICIT",
        "Junction ID: ", m__name);
    undeliv = epanet::convert_flow_to_L_per_s(ph, undeliv);

    double emitter_flow = 0.0;
    errorcode = EN_getnodevalue(ph, m__en_index, EN_EMITTERFLOW, &emitter_flow);
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the properties of the junction.",
        "Error originating from the EPANET API while retrieving value: EN_EMITTERFLOW",
        "Junction ID: ", m__name);
    emitter_flow = epanet::convert_flow_to_L_per_s(ph, emitter_flow);

    // TODO: get also the leakage flow if v 240712
    
    // If a Junction with a demand is experiencing a negative pressure with a DDA,
    // the demand was not satisfied and it should go as a demand undelivered.
    // This is equivalent to check if the warning flag of EPANET is set to 6.
    double outflow = m__outflow.when_t(t);
#if BEME_VERSION <241100
    // HOTFIX, a junction must not experience negative pressure, head could be
    // slighlty above zero, but below the elevation and water would not flow out.
    // A simple oversight in the code, that may have caused wrong calculations in the past.
    if (ph->hydraul.DemandModel == DDA && outflow > 0 &&  m__head.when_t(t) < 0)
#else
    if (ph->hydraul.DemandModel == DDA && outflow > 0 &&  m__head.when_t(t) < m__elevation)
#endif
    {
        m__demand.commit(t, outflow);
        m__consumption.commit(t, 0);
        m__undelivered_demand.commit(t, outflow);
    }
    else
    {
        m__demand.commit(t, outflow - emitter_flow);
        m__consumption.commit(t, outflow);
        m__undelivered_demand.commit(t, undeliv);
    }
}

} // namespace bevarmejo::wds
