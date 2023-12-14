//
// bevarmejo cpp library
//
// Created by Dennis Zanutto on 13/10/23.
//

#ifndef BEVARMEJOLIB__HYDRAULIC_FUNCTIONS_HPP
#define BEVARMEJOLIB__HYDRAULIC_FUNCTIONS_HPP

#include <iostream>
#include <vector>

#include "bevarmejo/wds/water_distribution_system.hpp"
#include "bevarmejo/wds/elements/temporal.hpp"

namespace bevarmejo {
    
    template <typename T>
    bool minimum_pressure_satisfied(const std::vector<T>& head_at_dnodes, T min_head_at_dnodes=20.0){
        for (const auto& head : head_at_dnodes){
            if (head < min_head_at_dnodes){
                return false;
            }
        }
        return true;
    }

    template <typename T>
    bool minimum_pressure_satisfied(const std::vector<T>& head_at_dnodes, const std::vector<T>& min_head_at_dnodes) {
        // check the 2 vectors have the same size
        if (head_at_dnodes.size() != min_head_at_dnodes.size()) {
            throw std::runtime_error("The problem is not well-defined: the size of demand nodes and they minimum pressure don't match.");
        }
        
        for (std::size_t i = 0; i < head_at_dnodes.size(); ++i) {
            if (head_at_dnodes[i] < min_head_at_dnodes[i]) {
                return false;
            }
        }
        return true;
    }

    // TODO: functions not returning a bool but a value propotional to the mismatch between the minimum pressure and the actual pressure

    // Resilience index as defined in Todini (2000)
    // Both for single value of minimum pressure and not
    wds::vars::timeseries_real resilience_index(const wds::water_distribution_system& a_wds,
                                                const double req_head_dnodes=20.0);
    /*wds::vars::timeseries_real resilience_index(wds::water_distribution_system* a_wds,
                                                const std::vector<double>& req_head_dnodes);
    double resilience_index(const std::vector<double>& flow_dnodes, 
                            const std::vector<double>& head_dnodes,
                            const std::vector<double>& flow_reserv, 
                            const std::vector<double>& head_reserv,
                            const std::vector<double>& power_pumps,
                            const double req_head_dnodes=20.0);*/
    double resilience_index(const std::vector<double>& flow_dnodes, 
                            const std::vector<double>& head_dnodes,
                            const std::vector<double>& flow_reserv, 
                            const std::vector<double>& head_reserv,
                            const std::vector<double>& power_pumps,
                            const std::vector<double>& req_head_dnodes);
 
} // namespace bevarmejo

#endif // BEVARMEJOLIB__HYDRAULIC_FUNCTIONS_HPP