// 
// resilience_index.hpp
// bevarmejo cpp library
//
// Created by Dennis Zanutto on 12/10/23.
//

#ifndef BEVARMEJOLIB__RESILIENCE_INDEX_HPP
#define BEVARMEJOLIB__RESILIENCE_INDEX_HPP

#include <iostream>
#include <vector>

namespace bevarmejo {
// Define the struct that contains the data needed to calculate the resilience index
struct netdata_4_Ir {
    std::vector<double> flow_at_dnodes;
    std::vector<double> head_at_dnodes;

    std::vector<double> flow_out_reservoirs;
    std::vector<double> head_at_reservoirs;

    std::vector<double> power_at_pumps;
};

// Define the function that calculates the resilience index
double resilience_index(const netdata_4_Ir& network_data, double req_head_at_dnodes=20.0);
double resilience_index(const netdata_4_Ir& network_data, std::vector<double>& req_head_at_dnodes);

} // namespace bevarmejo
#endif // BEVARMEJOLIB__RESILIENCE_INDEX_HPP