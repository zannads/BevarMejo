#include <iostream>
#include <vector>

#include "bevarmejo/constants.hpp"
#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements_group.hpp"
#include "bevarmejo/wds/water_distribution_system.hpp"

#include "hydraulic_functions.hpp"

namespace bevarmejo {

bool is_head_deficient(const wds::Junction& a_junction, const double min_head) { 
    for (const auto& [t, head] : a_junction.head().value() ) {
        if (head < min_head) {
            return true;
        }
    }
    return false;
}

bool is_pressure_deficient(const wds::Junction &a_junction, const double min_pressure) {
    for (const auto& [t, pressure] : a_junction.pressure().value() ) {
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

    for (const auto& [t, head] : a_junction.head().value() ) {

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

    for (const auto& [t, pressure] : a_junction.pressure().value() ) {

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
    for (const auto& [t, head] : first_junc->head().value() ) {
        deficiency.insert(std::make_pair(t, 0.0));
    }

    // cumulative deficiency of each junction
    for (const auto& junction : a_wds.junctions() ) {
        for (const auto& [t, head] : junction->head().value() ) {
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
    for (const auto& [t, pressure] : first_junc->pressure().value() ) {
        deficiency.insert(std::make_pair(t, 0.0));
    }

    // cumulative deficiency of each junction
    for (const auto& junction : a_wds.junctions() ) {
        for (const auto& [t, pressure] : junction->pressure().value() ) {
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
        for (const auto& [t, level] : tank->level().value() ) {
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
    wds::vars::temporal<std::vector<double>> flows_dnodes;
    wds::vars::temporal<std::vector<double>> heads_dnodes;
    wds::vars::temporal<std::vector<double>> flows_sources;
    wds::vars::temporal<std::vector<double>> heads_sources;
    wds::vars::temporal<std::vector<double>> powers_pumps;

    std::vector<double> req_heads_dnodes_m(a_wds.junctions().size(), min_press_dnodes_m);

    /* I will implement here the logic that I will probably move later to the ElementGroup
      * class: i.e., extract from a subnetwork all the results in a single variable
      * object. E.g. for all the flows instead of having a vector of temporal of 
      * double, I will have a temporal of vector of doubles so that the results
      * are sync at the same time steps.
    */

    auto itj = a_wds.junctions().begin();
    auto ith = req_heads_dnodes_m.begin();
    while (itj != a_wds.junctions().end()) {
        // add the elevation to the min pressure to get the required head.
        *ith += (*itj)->elevation();

        if ( itj == a_wds.junctions().begin() ) {
            for (const auto& [t, flow] : (*itj)->demand_delivered().value() ) {
                flows_dnodes.insert(std::make_pair(t, std::vector<double>() ) );
                flows_dnodes.at(t).reserve(a_wds.junctions().size());
                heads_dnodes.insert(std::make_pair(t, std::vector<double>() ) );
                heads_dnodes.at(t).reserve(a_wds.junctions().size());
            }
        }

        for (const auto& [t, flow] : (*itj)->demand_delivered().value() ) {
            flows_dnodes.at(t).push_back(flow);
        }
        for (const auto& [t, head] : (*itj)->head().value() ) {
            heads_dnodes.at(t).push_back(head);
        }

        ++itj;
        ++ith;
    }

    auto p_first_res = *a_wds.reservoirs().begin();
    for (const auto& [t, flow] : p_first_res->inflow().value() ) {
        flows_sources.insert(std::make_pair(t, std::vector<double>() ) );
        flows_sources.at(t).reserve(a_wds.reservoirs().size() + a_wds.tanks().size() );
        heads_sources.insert(std::make_pair(t, std::vector<double>() ) );
        heads_sources.at(t).reserve(a_wds.reservoirs().size() + a_wds.tanks().size() );
    }
    for (const auto& reservoir : a_wds.reservoirs() ) {
        for (const auto& [t, flow] : reservoir->inflow().value() ) {
            flows_sources.at(t).push_back(flow);
        }
        for (const auto& [t, head] : reservoir->head().value() ) {
            heads_sources.at(t).push_back(head);
        }
    }
    for (const auto& tank : a_wds.tanks() ) {
          for (const auto& [t, flow] : tank->inflow().value() ) {
                flows_sources.at(t).push_back(flow);
          }
          for (const auto& [t, head] : tank->head().value() ) {
                heads_sources.at(t).push_back(head);
          }
    }

    auto p_first_pump = *a_wds.pumps().begin();
    for (const auto& [t, power] : p_first_pump->instant_energy().value() ) {
        powers_pumps.insert(std::make_pair(t, std::vector<double>() ) );
        powers_pumps.at(t).reserve(a_wds.pumps().size());
    }
    for (const auto& pump : a_wds.pumps() ) {
        for (const auto& [t, power] : pump->instant_energy().value() ) {
            powers_pumps.at(t).push_back(power);
        }
    }

    wds::vars::timeseries_real res_index;
    // It may be that they all have different lengths... we try at each time step 
    try {
        for (const auto& [t, flow_dnodes] : flows_dnodes ) {
            res_index.insert(std::make_pair(t, 
                        resilience_index(   flow_dnodes, 
                                            heads_dnodes.at(t),
                                            flows_sources.at(t),
                                            heads_sources.at(t),
                                            powers_pumps.at(t),
                                            req_heads_dnodes_m)));
        }
    } catch (std::runtime_error& e) {
        // No need to add any value at that time (not all time steps have all the data)
    } catch (std::out_of_range& e) { // this is more likely to happen
        // No need to add any value at that time (not all time steps have all the data)
    }
    return res_index;
}

double resilience_index(const std::vector<double>& flow_dnodes, 
                        const std::vector<double>& head_dnodes,
                        const std::vector<double>& flow_reserv, 
                        const std::vector<double>& head_reserv,
                        const std::vector<double>& power_pumps,
                        const std::vector<double>& req_head_dnodes) {

    assert(flow_dnodes.size() == head_dnodes.size());
    assert(flow_reserv.size() == head_reserv.size());
    assert(flow_dnodes.size() == req_head_dnodes.size());


    // numerator: sum_{i=1}^{n_dnodes}(q_i*(h_i-h_i^req))
    double numerator = 0.0;
    for (std::size_t i = 0; i < flow_dnodes.size(); ++i) {
        // if the head is lower than the required head I don't want to count it, if you need to pass this info use minimum pressure function
        double head_diff = head_dnodes[i] - req_head_dnodes[i];
        if (head_diff < 0)
            head_diff = 0.0; 

        numerator += flow_dnodes[i] * head_diff / 1000; // from L/s to M^3/s
    }

    // denominator: 
    // sum_{i=1}^{n_reservoirs}(q_i*h_i)
    // +sum_{i=1}^{n_pumps}(p_i/\lambda)
    // -sum_{i=1}^{n_dnodes}(q_i*h_i^req)
    double denominator = 0.0;
    for (std::size_t i = 0; i < flow_reserv.size(); ++i) {
        denominator += -flow_reserv[i] * head_reserv[i] / 1000; // Power entering the system, water leaving the reservoir so the flow is negative
    }
    for (std::size_t i = 0; i < power_pumps.size(); ++i) {
        denominator += power_pumps[i]/bevarmejo::water_specific_weight_N_per_m3*1000; // Power is already positive as it represents the energy consumed
    } 
    for (std::size_t i = 0; i < flow_dnodes.size(); ++i) {
        denominator -= flow_dnodes[i] * req_head_dnodes[i] / 1000;
    }

    // return the Ir if the denominator is not 0 otherwise return - infinity (min of double)
    return denominator > 0 ? numerator/denominator : std::numeric_limits<double>::min();
}
    
} // namespace bevarmejo


