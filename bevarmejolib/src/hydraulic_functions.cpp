#include <iostream>
#include <vector>

#include "bevarmejo/constants.hpp"
#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements_group.hpp"
#include "bevarmejo/wds/water_distribution_system.hpp"

#include "hydraulic_functions.hpp"

namespace bevarmejo {
    wds::vars::timeseries_real resilience_index(const wds::water_distribution_system& a_wds,
                                                const double req_head_dnodes) {
    // Check for the subnetworks "demand nodes", "reservoirs" and "pumps"
    // Extract head and flows from the demand nodes, and reservoirs. Power for the pumps
    // Calculate the resilience index at each time step and return it as a timeseries
    wds::vars::temporal<std::vector<double>> flows_dnodes(a_wds.junctions().size()+a_wds.tanks().size());
    wds::vars::temporal<std::vector<double>> heads_dnodes(a_wds.junctions().size()+a_wds.tanks().size());
    wds::vars::temporal<std::vector<double>> flows_out_reservoirs(a_wds.reservoirs().size());
    wds::vars::temporal<std::vector<double>> heads_reservoirs(a_wds.reservoirs().size());
    wds::vars::temporal<std::vector<double>> powers_pumps(a_wds.pumps().size());

    std::vector<double> req_heads_dnodes(a_wds.junctions().size()+a_wds.tanks().size(), req_head_dnodes);

    for (const auto& junction : a_wds.junctions()) {
        if (junction->has_demand()){
            continue;
        }
    }
    

    wds::vars::timeseries_real res_index;
    // It may be that they all have different lengths... we try at each time step 
    try {
        for (const auto& [time, flow_dnodes] : flows_dnodes) {
            res_index.insert(std::make_pair(time, 
                            resilience_index(   flow_dnodes, 
                                                heads_dnodes.at(time),
                                                flows_out_reservoirs.at(time),
                                                heads_reservoirs.at(time),
                                                powers_pumps.at(time),
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


