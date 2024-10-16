#include <cassert>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/wds/epanet_helpers/en_help.hpp"

#include "bevarmejo/wds/epanet_helpers/en_time_options.hpp"
#include "bevarmejo/wds/auxiliary/time_series.hpp"
#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"

#include "bevarmejo/wds/auxiliary/pattern.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "junction.hpp"

namespace bevarmejo {
namespace wds {

Junction::Junction(const std::string& id, const WaterDistributionSystem& wds) : 
    inherited(id, wds),
    m__demands(),
    m__demand(wds.time_series(label::__RESULTS_TS)),
    m__consumption(wds.time_series(label::__RESULTS_TS)),
    m__undelivered_demand(wds.time_series(label::__RESULTS_TS)) { }

// Copy constructor
Junction::Junction(const Junction& other) : 
    inherited(other),
    m__demands(other.m__demands),
    m__demand(other.m__demand),
    m__consumption(other.m__consumption),
    m__undelivered_demand(other.m__undelivered_demand) { }

// Move constructor
Junction::Junction(Junction&& rhs) noexcept : 
    inherited(std::move(rhs)),
    m__demands(std::move(rhs.m__demands)),
    m__demand(std::move(rhs.m__demand)),
    m__consumption(std::move(rhs.m__consumption)),
    m__undelivered_demand(std::move(rhs.m__undelivered_demand)) { }

// Copy assignment operator
Junction& Junction::operator=(const Junction& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);

        m__demands = rhs.m__demands;
        m__demand = rhs.m__demand;
        m__consumption = rhs.m__consumption;
        m__undelivered_demand = rhs.m__undelivered_demand;
    }
    return *this;
}

// Move assignment operator
Junction& Junction::operator=(Junction&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));

        m__demands = std::move(rhs.m__demands);
        m__demand = std::move(rhs.m__demand);
        m__consumption = std::move(rhs.m__consumption);
        m__undelivered_demand = std::move(rhs.m__undelivered_demand);
    }
    return *this;
}

Junction::FlowSeries& Junction::demand(const std::string &a_category) {
    return m__demands.at(a_category);
}

const Junction::FlowSeries& Junction::demand(const std::string &a_category) const {
    return m__demands.at(a_category);
}

const bool Junction::has_demand() const {
    return !m__demands.empty();
}

void Junction::__retrieve_EN_properties(EN_Project ph)  {
    inherited::__retrieve_EN_properties(ph);
    auto patterns= m__wds.patterns();

    int n_demands= 0;
    int errorcode= EN_getnumdemands(ph, this->index(), &n_demands);
    assert(errorcode < 100);

    for (std::size_t i= 1; i <= n_demands; ++i) {
        double base_demand= 0.0;
        errorcode= EN_getbasedemand(ph, this->index(), i, &base_demand);
        assert(errorcode < 100);

        int pattern_index= 0;
        errorcode= EN_getdemandpattern(ph, this->index(), i, &pattern_index);
        assert(errorcode < 100);

        char __pattern_id[EN_MAXID+1];
        errorcode= EN_getpatternid(ph, pattern_index, __pattern_id);
        assert(errorcode < 100);
        std::string pattern_id(__pattern_id);

        char __demand_category[EN_MAXID+1];
        errorcode= EN_getdemandname(ph, this->index(), i, __demand_category);
        assert(errorcode < 100);
        std::string demand_category(__demand_category);

        std::shared_ptr<Pattern> pattern= nullptr;
        // Pattern id can be "" if the demand is constant
        if (pattern_id.empty()) {
            // Means it's a constant demand
            aux::QuantitySeries<double> cdemand(m__wds.time_series(label::__CONSTANT_TS));
            cdemand.commit(0l, base_demand);

            m__demands.insert(std::make_pair(demand_category, cdemand));
        }
        else { // It's a pattern demand
            aux::QuantitySeries<double> pdemand(m__wds.time_series(label::__EN_PATTERN_TS));

            auto it= patterns.find(pattern_id);
            assert(it != patterns.end());
            pattern= *it;

            // TODO: this is very much wrong because it doesn't consider the shift time step 
            // and that patterns may have a different length and I may need to wrap around.
            auto ilen= m__wds.time_series(label::__EN_PATTERN_TS).size();
            for (auto i= 0l; i < ilen; ++i) {
                auto __time= m__wds.time_series(label::__EN_PATTERN_TS).at(i);
                pdemand.commit(__time, base_demand * pattern->at(i % pattern->size()));
            }

            m__demands.insert(std::make_pair(demand_category, pdemand));
        }
    }   
}

void Junction::retrieve_results(EN_Project ph, long t=0) {
    inherited::retrieve_results(ph, t);

    int errorcode = 0;
    double d_demand = 0;
    double d_dem_deficit = 0;
    errorcode = EN_getnodevalue(ph, index(), EN_DEMAND, &d_demand);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving demand for node " + id()+"\n");

    if (ph->parser.Unitsflag != LPS)
        d_demand = epanet::convert_flow_to_L_per_s(ph, d_demand);
    m__demand.commit(t, d_demand);

    errorcode = EN_getnodevalue(ph, index(), EN_DEMANDDEFICIT, &d_dem_deficit);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving demand deficit for node " + id()+"\n");

    if (ph->parser.Unitsflag != LPS)
        d_dem_deficit = epanet::convert_flow_to_L_per_s(ph, d_dem_deficit);
    // IF DDA is on and head is negative, the demand was not satisfied and it should go as a demand undelivered
    if (/*Assume we know it is DDA*/ m__head.when_t(t) < 0)
        d_dem_deficit = d_demand;
    m__undelivered_demand.commit(t, d_dem_deficit);

    m__consumption.commit(t, d_demand-d_dem_deficit);
}

void Junction::clear_results() {
    inherited::clear_results();

    m__demand.clear();
    m__consumption.clear();
    m__undelivered_demand.clear();
}

} // namespace wds
} // namespace bevarmejo
