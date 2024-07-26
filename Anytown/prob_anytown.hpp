// 
//  model_anytown.hpp
//	anytown original test
//
//  Created by Dennis Zanutto on 11/07/23.

#ifndef ANYTOWN__PROBANYTOWN_HPP
#define ANYTOWN__PROBANYTOWN_HPP
//#define DEBUGSIM

#include <iostream>
#include <string>
#include <vector>

namespace bevarmejo {

namespace label {
static const std::string __temp_elems = "TEs";
} // namespace label 


// Data of Anytown: hard coded as they don't change for now.
// If you need to change them, but still harcoded, "override" them in a new namespace for that type of problem.
namespace anytown {

constexpr double treatment_plant_head_ft = 10.0;
constexpr double min_w_level_tank_ft = 225.0;
constexpr double max_w_level_tank_ft = 250.0;
constexpr double bottom_height_tank_ft = 215.0; // emergency volume between min_w_level_tank_ft and bottom_height_tank_ft
constexpr double min_pressure_psi = 40.0;
constexpr double average_daily_flow_multiplier = 1.0;
constexpr double peak_flow_multiplier = 1.3;
constexpr double instantaneous_peak_flow_multiplier = 1.8;    
// Constants for fire flow conditions.
constexpr double min_pressure_fireflow_psi = 20.0;
constexpr double fireflow_multiplier = peak_flow_multiplier;
constexpr double fireflow_duration_hours = 2.0;
// Actual fire flow conditions will be loaded from file.

// Parameters for the fitness function.
constexpr double coeff_HW_cleaned = 125.0;
constexpr double coeff_HW_new = 130.0;

constexpr double energy_cost_kWh = 0.12; // dollars per kWh
constexpr double discount_rate = 0.12; // 12% per year
constexpr double amortization_years = 20.0; // 20 years

constexpr double riser_length_ft = 101.0;

// "Constants" representing the actions we can do on the network.
constexpr std::size_t max_n_installable_tanks = 2;

constexpr double _nonexisting_pipe_diam_ft = 0.0001;

// Structs for reading data from file.
struct pipes_alt_costs {
    double diameter_in;
    double new_cost;
    double dup_city;
    double dup_residential;
    double clean_city;
    double clean_residential;        
}; 
std::istream& operator >> (std::istream& is, pipes_alt_costs& pac);

struct tanks_costs {
    double volume_gal;
    double cost;
};
std::istream& operator >> (std::istream& is, tanks_costs& tc);

std::vector<std::vector<double>> decompose_pumpgroup_pattern(std::vector<double> pg_pattern, const std::size_t n_pumps);

} // namespace anytown
 
} // namespace bevarmejo

#endif // ANYTOWN__PROBANYTOWN_HPP
