#include <cassert>
#include <string>
#include <unordered_map>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/wds/epanet_helpers/en_help.hpp"

#include "bevarmejo/wds/auxiliary/pattern.hpp"
#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "junction.hpp"

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

    for (std::size_t i= 1; i <= n_demands; ++i) {
        double base_demand= 0.0;
        errorcode= EN_getbasedemand(ph, m__en_index, i, &base_demand);
        assert(errorcode < 100);

        int pattern_index= 0;
        errorcode= EN_getdemandpattern(ph, m__en_index, i, &pattern_index);
        assert(errorcode < 100);

        char __pattern_id[EN_MAXID+1];
        errorcode= EN_getpatternid(ph, pattern_index, __pattern_id);
        assert(errorcode < 100);
        std::string pattern_id(__pattern_id);

        char __demand_category[EN_MAXID+1];
        errorcode= EN_getdemandname(ph, m__en_index, i, __demand_category);
        assert(errorcode < 100);
        std::string demand_category(__demand_category);

        // Pattern id can be "" if the demand is constant
        if (pattern_id.empty())
        {   
            aux::QuantitySeries<double> cdemand(m__wds.time_series(label::__CONSTANT_TS));
            cdemand.commit(0l, base_demand);

            m__demands.insert(std::make_pair(demand_category, cdemand));
        }
        else // It's a pattern demand
        {   
            aux::QuantitySeries<double> pdemand(m__wds.time_series(label::__EN_PATTERN_TS));

            const auto& pattern = m__wds.patterns().at(pattern_id);

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
    this->__retrieve_EN_results();

    inherited::retrieve_EN_results();
}

void Junction::__retrieve_EN_results()
{
    assert(m__en_index > 0);
    assert(m__wds.ph() != nullptr);

    auto ph = m__wds.ph();
    auto t = m__wds.current_result_time();

    // get_nodevalue(EN_DEMAND) returns the water flowing in/out of the junction.
    // This is the sum of the demands of the junction and the emitter flow.
    // The pure node demand (the sum of the demands of the junction) is stored 
    // in DemandFlow. (See epanet.c get_nodvalue(EN_DEMAND) for more details).
    double consumed = 0.0;
    int errorcode = EN_getnodevalue(ph, m__en_index, EN_DEMAND, &consumed);
    if (errorcode > 100)
        __format_and_throw<std::runtime_error>("Junction", "retrieve_EN_results", "Error retrieving results of junction.",
            "Property: EN_DEMAND",
            "Error code: ", errorcode,
            "Junction ID: ", m__name);
    
    double undeliv = 0.0;
    errorcode = EN_getnodevalue(ph, m__en_index, EN_DEMANDDEFICIT, &undeliv);
    if (errorcode > 100)
        __format_and_throw<std::runtime_error>("Junction", "retrieve_EN_results", "Error retrieving results of junction.",
            "Property: EN_DEMANDDEFICIT",
            "Error code: ", errorcode,
            "Junction ID: ", m__name);

    double emitter_flow = 0.0;
    errorcode = EN_getnodevalue(ph, m__en_index, EN_EMITTERFLOW, &emitter_flow);
    if (errorcode > 100)
        __format_and_throw<std::runtime_error>("Junction", "retrieve_EN_results", "Error retrieving results of junction.",
            "Property: EN_EMITTERFLOW",
            "Error code: ", errorcode,
            "Junction ID: ", m__name);
    
    // If a Junction with a demand is experiencing a negative head with a DDA,
    // the demand was not satisfied and it should go as a demand undelivered.
    // I have not uploaded the head of the junction yet because it is a node property.
    // Therefore I simply check if the head at that index is negative.

    if (consumed > 0 && ph->hydraul.DemandModel == DDA && ph->hydraul.NodeHead[m__en_index] < 0)
    {
        m__demand.commit(t, epanet::convert_flow_to_L_per_s(ph, consumed));
        m__consumption.commit(t, epanet::convert_flow_to_L_per_s(ph, 0));
        m__undelivered_demand.commit(t, epanet::convert_flow_to_L_per_s(ph, consumed));       
    }
    else
    {
        m__demand.commit(t, epanet::convert_flow_to_L_per_s(ph, (consumed - emitter_flow)));
        m__consumption.commit(t, epanet::convert_flow_to_L_per_s(ph, consumed));
        m__undelivered_demand.commit(t, epanet::convert_flow_to_L_per_s(ph, undeliv));    
    }
    
    
}



} // namespace bevarmejo::wds
