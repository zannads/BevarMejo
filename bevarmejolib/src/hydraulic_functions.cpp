#include <iostream>
#include <vector>

#include "bevarmejo/constants.hpp"
#include "bevarmejo/wds/data_structures/temporal.hpp"
#include "bevarmejo/wds/elements_group.hpp"
#include "bevarmejo/wds/water_distribution_system.hpp"

#include "hydraulic_functions.hpp"

namespace bevarmejo {

bool is_head_deficient(const wds::Junction& a_junction, const double min_head) { 
    for (const auto& [t, head] : a_junction.head() ) {
        if (head < min_head) {
            return true;
        }
    }
    return false;
}

bool is_pressure_deficient(const wds::Junction &a_junction, const double min_pressure) {
    for (const auto& [t, pressure] : a_junction.pressure() ) {
        if (pressure < min_pressure) {
            return true;
        }
    }
    return false;
    
}

wds::vars::timeseries_real head_deficiency(const wds::Junction &a_junction, const double min_head, const bool relative) {
    wds::vars::timeseries_real deficiency;

    assert(min_head > 0.0);
    double weight = 1.0;
    if (relative) weight = 1/min_head;

    for (const auto& [t, head] : a_junction.head() ) {

        double deficit = head < min_head ? (min_head - head)*weight : 0.0;

        deficiency.insert(std::make_pair(t, deficit));
    }

    return deficiency;
}

wds::vars::timeseries_real pressure_deficiency(const wds::Junction &a_junction, const double min_pressure, const bool relative) {
    wds::vars::timeseries_real deficiency;

    assert(min_pressure > 0.0);
    double weight = 1.0;
    if (relative) weight = 1/min_pressure;

    for (const auto& [t, pressure] : a_junction.pressure() ) {

        double deficit = pressure < min_pressure ? (min_pressure - pressure)*weight : 0.0;

        deficiency.insert(std::make_pair(t, deficit));
    }

    return deficiency;
    
}

wds::vars::timeseries_real head_deficiency(const wds::WaterDistributionSystem &a_wds, const double min_head, const bool relative) {
    wds::vars::timeseries_real deficiency;

    assert(min_head > 0.0);
    double weight = 1.0;
    if (relative) weight = 1/ min_head/ a_wds.junctions().size();

    // just allocate the memory for the timeseries
    auto first_junc = *a_wds.junctions().begin();
    for (const auto& [t, head] : first_junc->head() ) {
        deficiency.insert(std::make_pair(t, 0.0));
    }

    // cumulative deficiency of each junction
    for (const auto& junction : a_wds.junctions() ) {
        for (const auto& [t, head] : junction->head() ) {
            if (head < min_head) 
                deficiency.at(t) += (min_head - head)*weight;
        }
    }
    
    return deficiency;
}

wds::vars::timeseries_real pressure_deficiency(const wds::WaterDistributionSystem &a_wds, const double min_pressure, const bool relative) {
    wds::vars::timeseries_real deficiency;

    assert(min_pressure > 0.0);
    double weight = 1.0;
    if (relative) weight = 1/ min_pressure/ a_wds.junctions().size();

    // just allocate the memory for the timeseries
    auto first_junc = *a_wds.junctions().begin();
    for (const auto& [t, pressure] : first_junc->pressure() ) {
        deficiency.insert(std::make_pair(t, 0.0));
    }

    // cumulative deficiency of each junction
    for (const auto& junction : a_wds.junctions() ) {
        for (const auto& [t, pressure] : junction->pressure() ) {
            if (pressure < min_pressure) 
                deficiency.at(t) += (min_pressure - pressure)*weight;
        }
    }
    
    return deficiency;
    
}

double tanks_operational_levels_use(const wds::Tanks &a_tanks) {
    // This function checks if the tanks have a complete cycle of emptying and filling in one simulation.
    // So the minimum level has to be minimized and the max level maximized, in other words, I need to minimize the 
    // difference between the simulation min level and the tank minimum level and between the simulation max level 
    // and the tank maximum level. They are both to minimize and they have equal importance so I sum them.
    // Since they are limited between min and max levels of the tank I can normalize them to get a unique metric for the whole network.
    // I will return the sum of the normalized levels. (0 perfect use of the tank 1 no use of the tank at all constant value for the whole simulation)

    double sum_levels = 0.0;
    for (const auto& tank : a_tanks) {
        double max_tank_lev = tank->max_level().value();
        double min_tank_lev = tank->min_level().value();

        double min_sim_lev = max_tank_lev;
        double max_sim_lev = min_tank_lev;
        for (const auto& [t, level] : tank->level() ) {
            if (level < min_sim_lev) min_sim_lev = level;
            if (level > max_sim_lev) max_sim_lev = level;
        }

        // sum_levels += ( (min_sim_lev-min_tank_lev) + (max_tank_lev-max_sim_lev) )/( max_tank_lev - min_tank_lev ) 
        // simplified with common part
        sum_levels += 1 - ( min_sim_lev - max_sim_lev )/( max_tank_lev - min_tank_lev );
    }
    sum_levels /= a_tanks.size(); // average over the number of tanks

    return sum_levels;
}

    wds::vars::timeseries_real resilience_index_from_min_pressure(const wds::WaterDistributionSystem& a_wds,
                                                const double min_press_dnodes_m) {
    // Check for the subnetworks "demand nodes", "reservoirs" and "pumps"
    // Extract head and flows from the demand nodes, and reservoirs. Power for the pumps
    // Calculate the resilience index at each time step and return it as a timeseries

    std::vector<double> req_heads_dnodes_m(a_wds.junctions().size(), min_press_dnodes_m);

    // add the elevation to the min pressure to get the required head.
    auto itj = a_wds.junctions().begin();
    auto ith = req_heads_dnodes_m.begin();
    while (itj != a_wds.junctions().end()) {
        
        *ith += (*itj)->elevation();

        ++itj;
        ++ith;
    }

    wds::vars::timeseries_real res_index;
    for (const auto& [t, temp] : (*a_wds.junctions().begin())->demand_requested() ) {
 
        std::vector<double> req_flows_dnodes;   req_flows_dnodes.reserve(a_wds.junctions().size());
        std::vector<double> heads_dnodes;       heads_dnodes.reserve(a_wds.junctions().size());
        std::vector<double> flows_sources;      flows_sources.reserve(a_wds.reservoirs().size() + a_wds.tanks().size());
        std::vector<double> heads_sources;      heads_sources.reserve(a_wds.reservoirs().size() + a_wds.tanks().size());
        std::vector<double> powers_pumps;       powers_pumps.reserve(a_wds.pumps().size());

        for (const auto& junction : a_wds.junctions() ) {
            req_flows_dnodes.push_back(junction->demand_requested().when_t(t));
            heads_dnodes.push_back(junction->head().when_t(t));
        }
        for (const auto& reservoir : a_wds.reservoirs() ) {
            flows_sources.push_back(reservoir->inflow().when_t(t));
            heads_sources.push_back(reservoir->head().when_t(t));
        }
        for (const auto& tank : a_wds.tanks() ) {
            flows_sources.push_back(tank->inflow().when_t(t));
            heads_sources.push_back(tank->head().when_t(t));
        }
        for (const auto& pump : a_wds.pumps() ) {
            powers_pumps.push_back(pump->instant_energy().value().at(t));
        }


        res_index.insert(std::make_pair(t, 
                    resilience_index(   req_flows_dnodes, 
                                        heads_dnodes,
                                        req_heads_dnodes_m,
                                        flows_sources,
                                        heads_sources,
                                        powers_pumps
                                    )));
        
    }

    return res_index;
}

// follows the same orderd you find in the formula in Todini (2000)
double resilience_index(const std::vector<double>& req_flows_dnodes_lps, 
                        const std::vector<double>& head_dnodes_m,
                        const std::vector<double>& req_head_dnodes_m,
                        const std::vector<double>& flow_reserv_lps, 
                        const std::vector<double>& head_reserv_m,
                        const std::vector<double>& power_pumps_kw) {

    assert(req_flows_dnodes_lps.size() == head_dnodes_m.size());
    assert(head_dnodes_m.size() == req_head_dnodes_m.size());
    assert(flow_reserv_lps.size() == head_reserv_m.size());


    // numerator: sum_{i=1}^{n_dnodes}(q_i*(h_i-h_i^req))
    bool dda_invalidate = false;
    double numerator = 0.0;
    for (std::size_t i = 0; ( !dda_invalidate ) && ( i < req_flows_dnodes_lps.size() ); ++i) {
        // if the head is lower than zero, and the demand is not zero, then the head is not enough, the results are 
        // coming from a Demand Driven Analysis. By the definition, you are not 
        // satisfying the demand, so the Ir is 0. We just need one case.
        if (head_dnodes_m[i] < 0 && req_flows_dnodes_lps[i] > 0)
            dda_invalidate = true;
        else
            numerator += req_flows_dnodes_lps[i] * (head_dnodes_m[i] - req_head_dnodes_m[i]) / 1000; // from L/s to M^3/s
    }
    if (dda_invalidate) return 0.0;

    // denominator: 
    // sum_{i=1}^{n_reservoirs}(q_i*h_i)
    // +sum_{i=1}^{n_pumps}(p_i/\lambda)
    // -sum_{i=1}^{n_dnodes}(q_i*h_i^req)
    double denominator = 0.0;
    for (std::size_t i = 0; i < flow_reserv_lps.size(); ++i) {
        denominator += -flow_reserv_lps[i] * head_reserv_m[i] / 1000; // Power entering the system, water leaving the reservoir so the flow is negative
    }
    for (std::size_t i = 0; i < power_pumps_kw.size(); ++i) {
        denominator += power_pumps_kw[i]/bevarmejo::water_specific_weight_N_per_m3*1000; // Power is already positive as it represents the energy consumed
    } 
    for (std::size_t i = 0; i < req_flows_dnodes_lps.size(); ++i) {
        denominator -= req_flows_dnodes_lps[i] * req_head_dnodes_m[i] / 1000;
    }

    // return the Ir if the denominator is not 0 otherwise return - infinity (min of double)
    return denominator > 0 ? numerator/denominator : std::numeric_limits<double>::min();
}
    
} // namespace bevarmejo


