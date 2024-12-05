//
// bevarmejo cpp library
//
// Created by Dennis Zanutto on 13/10/23.
//

#ifndef BEVARMEJOLIB__HYDRAULIC_FUNCTIONS_HPP
#define BEVARMEJOLIB__HYDRAULIC_FUNCTIONS_HPP

#include <vector>

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

namespace bevarmejo {

    // Function that checks if a junction satisfies the minimum pressure
    // min_head=14.0 m circa 20 psi 
    bool is_head_deficient(const wds::Junction& a_junction, const double min_head=14.0);
    bool is_pressure_deficient(const wds::Junction& a_junction, const double min_pressure=14.0);
    wds::aux::QuantitySeries<double> head_deficiency(const wds::Junction& a_junction, const double min_head=14.0, const bool relative=false);
    wds::aux::QuantitySeries<double> pressure_deficiency(const wds::Junction& a_junction, const double min_pressure=14.0, const bool relative=false);
    wds::aux::QuantitySeries<double> head_deficiency(const WaterDistributionSystem& a_wds, const double min_head=14.0, const bool relative=false);
    wds::aux::QuantitySeries<double> pressure_deficiency(const WaterDistributionSystem& a_wds, const double min_pressure=14.0, const bool relative=false);
    // TODO: correct definition of head_deficiency would be for a (ordered) vector of junctions and a (ordered) vector of min_head
    // aux::QuantitySeries<double> head_deficiency(const wds::UserDefinedElementsGroup<wds::Junction>& a_junctions, const double min_head); // calling the vectorized version
    // aux::QuantitySeries<double> head_deficiency(const wds::UserDefinedElementsGroup<wds::Junction>& a_junctions, const std::vector<double>& min_head);
    
    double tanks_operational_levels_use(InputIncludingRegistryView<WDS::Tank> a_tanks);

    // TODO: functions not returning a bool but a value propotional to the mismatch between the minimum pressure and the actual pressure

    // Resilience index as defined in Todini (2000)
    // Both for single value of minimum pressure and not
    wds::aux::QuantitySeries<double> resilience_index_from_min_pressure(const WaterDistributionSystem& a_wds,
                                                const double min_press_dnodes_m=14.0);
    

// Resilience index as defined in Todini (2000).
// This function calculates the resilience index for a single time step simulation.
// This function assumes that the simulation was a Demand Driven Analysis (DDA)
// and that the simulation did not fail.
// There is only a simple safety check for negative heads in the nodes, but this
// is not enough to ensure that the simulation was valid (head can be positive 
// but with high elevation, pressures can be negative).

double resilience_index(const std::vector<double>& req_flows_dnodes_lps, 
                        const std::vector<double>& head_dnodes_m,
                        const std::vector<double>& req_head_dnodes_m,
                        const std::vector<double>& flow_reserv_lps, 
                        const std::vector<double>& head_reserv_m,
                        const std::vector<double>& power_pumps_kw);

} // namespace bevarmejo

#endif // BEVARMEJOLIB__HYDRAULIC_FUNCTIONS_HPP