#include <iostream>
#include <vector>

#include "bevarmejo/constants.hpp"
#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements_group.hpp"
#include "bevarmejo/wds/water_distribution_system.hpp"

#include "hydraulic_functions.hpp"

namespace bevarmejo {
    wds::vars::timeseries_real resilience_index(const wds::WaterDistributionSystem& a_wds,
                                                const double req_head_dnodes) {
    // Check for the subnetworks "demand nodes", "reservoirs" and "pumps"
    // Extract head and flows from the demand nodes, and reservoirs. Power for the pumps
    // Calculate the resilience index at each time step and return it as a timeseries
    wds::vars::temporal<std::vector<double>> flows_dnodes;
    wds::vars::temporal<std::vector<double>> heads_dnodes;
    wds::vars::temporal<std::vector<double>> flows_out_reservoirs;
    wds::vars::temporal<std::vector<double>> heads_reservoirs;
    wds::vars::temporal<std::vector<double>> powers_pumps;

    std::vector<double> req_heads_dnodes(a_wds.junctions().size()+a_wds.tanks().size(), req_head_dnodes);

    /* I will implement here the logic that I will probably move later to the ElementGroup
      * class: i.e., extract from a subnetwork all the results in a single variable
      * object. E.g. for all the flows instead of having a vector of temporal of 
      * double, I will have a temporal of vector of doubles so that the results
      * are sync at the same time steps.
    */

   auto p_first_junc = *a_wds.junctions().begin();
   for (const auto& [t, flow] : p_first_junc->demand_delivered().value() ) {
        flows_dnodes.insert(std::make_pair(t, std::vector<double>() ) );
        flows_dnodes.at(t).reserve(a_wds.junctions().size()+a_wds.tanks().size());
        heads_dnodes.insert(std::make_pair(t, std::vector<double>() ) );
        heads_dnodes.at(t).reserve(a_wds.junctions().size()+a_wds.tanks().size());
   }

   for (const auto& junction : a_wds.junctions() ) {
        for (const auto& [t, flow] : junction->demand_delivered().value() ) {
            flows_dnodes.at(t).push_back(flow);
        }
        for (const auto& [t, head] : junction->head().value() ) {
            heads_dnodes.at(t).push_back(head);
        }
   }
   for (const auto& tank : a_wds.tanks() ) {
          for (const auto& [t, flow] : tank->inflow().value() ) {
                flows_dnodes.at(t).push_back(flow);
          }
          for (const auto& [t, head] : tank->head().value() ) {
                heads_dnodes.at(t).push_back(head);
          }
    }

    auto p_first_res = *a_wds.reservoirs().begin();
    for (const auto& [t, flow] : p_first_res->inflow().value() ) {
        flows_out_reservoirs.insert(std::make_pair(t, std::vector<double>() ) );
        flows_out_reservoirs.at(t).reserve(a_wds.reservoirs().size());
        heads_reservoirs.insert(std::make_pair(t, std::vector<double>() ) );
        heads_reservoirs.at(t).reserve(a_wds.reservoirs().size());
    }
    for (const auto& reservoir : a_wds.reservoirs() ) {
        for (const auto& [t, flow] : reservoir->inflow().value() ) {
            flows_out_reservoirs.at(t).push_back(flow);
        }
        for (const auto& [t, head] : reservoir->head().value() ) {
            heads_reservoirs.at(t).push_back(head);
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
                                            flows_out_reservoirs.at(t),
                                            heads_reservoirs.at(t),
                                            powers_pumps.at(t),
                                            req_heads_dnodes)));
        }
    } catch (std::runtime_error& e) {
        // No need to add any value at that time (not all time steps have all the data)
    }
    return res_index;
}

// TODO: same one but with a vector in req_head_at_dnodes

double resilience_index(const std::vector<double>& flow_dnodes, 
                        const std::vector<double>& head_dnodes,
                        const std::vector<double>& flow_reserv, 
                        const std::vector<double>& head_reserv,
                        const std::vector<double>& power_pumps,
                        const std::vector<double>& req_head_dnodes) {

    // TODO: should check dimensions here and throw an error if they are not correct

    // numerator: sum_{i=1}^{n_dnodes}(q_i*(h_i-h_i^req))
    double numerator = 0.0;
    for (std::size_t i = 0; i < flow_dnodes.size(); ++i) {
        numerator += flow_dnodes[i] * (head_dnodes[i] - req_head_dnodes[i]);
    }

    // denominator: 
    // sum_{i=1}^{n_reservoirs}(q_i*h_i)
    // +sum_{i=1}^{n_pumps}(p_i/\lambda)
    // -sum_{i=1}^{n_dnodes}(q_i*h_i^req)
    double denominator = 0.0;
    for (std::size_t i = 0; i < flow_reserv.size(); ++i) {
        denominator += flow_reserv[i] * head_reserv[i];
    }
    for (std::size_t i = 0; i < power_pumps.size(); ++i) {
        denominator += power_pumps[i]/bevarmejo::water_specific_weight_N_per_m3;
    } 
    for (std::size_t i = 0; i < flow_dnodes.size(); ++i) {
        denominator -= flow_dnodes[i] * req_head_dnodes[i];
    }

    return numerator/denominator;
}
    
} // namespace bevarmejo


