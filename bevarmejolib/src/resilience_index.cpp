//
// resilience_index.cpp
// bevarmejo cpp library
//
// Created by Dennis Zanutto on 12/10/23.
//

#include <iostream>
#include <vector>

#include "constants.hpp"
#include "resilience_index.hpp"

namespace bevarmejo {

// Define the function that calculates the resilience index
double resilience_index(const netdata_4_Ir& network_data, double req_head_at_dnodes) {
    std::vector<double> req_heads_at_dnodes(network_data.flow_at_dnodes.size(), req_head_at_dnodes);
    return resilience_index(network_data, req_heads_at_dnodes);
}

double resilience_index(const netdata_4_Ir& network_data, std::vector<double>& req_head_at_dnodes) {

// TODO: should check dimensions here and throw an error if they are not correct

// numerator: sum_{i=1}^{n_dnodes}(q_i*(h_i-h_i^req))
double numerator = 0.0;
for (std::size_t i = 0; i < network_data.flow_at_dnodes.size(); ++i) {
    numerator += network_data.flow_at_dnodes[i] * (network_data.head_at_dnodes[i] - req_head_at_dnodes[i]);
}

// denominator: 
// sum_{i=1}^{n_reservoirs}(q_i*h_i)
// +sum_{i=1}^{n_pumps}(p_i/\lambda)
// -sum_{i=1}^{n_dnodes}(q_i*h_i^req)
double denominator = 0.0;
for (std::size_t i = 0; i < network_data.flow_out_reservoirs.size(); ++i) {
    denominator += network_data.flow_out_reservoirs[i] * network_data.head_at_reservoirs[i];
}
for (std::size_t i = 0; i < network_data.power_at_pumps.size(); ++i) {
    denominator += network_data.power_at_pumps[i]/bevarmejo::water_specific_weight_N_per_m3;
} 
for (std::size_t i = 0; i < network_data.flow_at_dnodes.size(); ++i) {
    denominator -= network_data.flow_at_dnodes[i] * req_head_at_dnodes[i];
}

return numerator/denominator;
}
} // namespace bevarmejo