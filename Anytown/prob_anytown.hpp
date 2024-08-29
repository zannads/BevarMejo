// 
//  model_anytown.hpp
//	anytown original test
//
//  Created by Dennis Zanutto on 11/07/23.

#ifndef ANYTOWN__PROB_ANYTOWN_HPP
#define ANYTOWN__PROB_ANYTOWN_HPP
//#define DEBUGSIM

#include <iostream>
#include <filesystem>
namespace fsys = std::filesystem;
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "bevarmejo/wds/water_distribution_system.hpp"

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

// Forward definition of the Problem class, we need this first, 
class Problem;

// Anytown functions for all formulations:
// For the bounds
std::pair<std::vector<double>, std::vector<double>> bounds__new_pipes(const bevarmejo::anytown::Problem &prob);
std::pair<std::vector<double>, std::vector<double>> bounds__pumps(const bevarmejo::anytown::Problem &prob);

// For fitness function:
//     For apply dv:
void apply_dv__new_pipes(WDS& anytown, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs);
void apply_dv__pumps(WDS& anytown, const std::vector<double>& dvs);

//     For reset dv:
void reset_dv__new_pipes(WDS& anytown);
void reset_dv__pumps(WDS& anytown);

//      For cost function:
double cost__new_pipes(const WDS& anytown, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs);
double cost__energy_per_day(const WDS& anytown);

//      For reliability (modified) function:
double of__reliability(const WDS& anytown);

// Anytown functions for specific formulations
namespace f1 {
// For the bounds
std::pair<std::vector<double>, std::vector<double>> bounds__exis_pipes(const bevarmejo::anytown::Problem &prob);
std::pair<std::vector<double>, std::vector<double>> bounds__tanks(const bevarmejo::anytown::Problem &prob);

// For fitness function:
//     For apply dv:
std::vector<double> apply_dv__exis_pipes(WDS& anytown, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs);
void apply_dv__tanks(WDS& anytown, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::tanks_costs> &tanks_costs);

//     For reset dv:
void reset_dv__exis_pipes(WDS& anytown, const std::vector<double>& dvs, const std::vector<double>& old_HW_coeffs);
void reset_dv__tanks(WDS& anytown, const std::vector<double>& dvs);

//      For cost function:
double cost__exis_pipes(const WDS& anytown, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs);
double cost__tanks(const WDS& anytown, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::tanks_costs> &tanks_costs, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs);

} // namespace f1

namespace f2 {

} // namespace f2

class Problem {
public:
    Problem() = default;
    Problem(const Problem& other) = default;
    Problem(Problem&& other) noexcept = default;
    Problem& operator=(const Problem& rhs) = default;
    Problem& operator=(Problem&& rhs) noexcept = default;
    ~Problem() = default;

protected:
    // Anytown specific data
    mutable std::shared_ptr<bevarmejo::wds::WaterDistributionSystem> m__anytown;
    std::vector<bevarmejo::anytown::pipes_alt_costs> m__pipes_alt_costs;
    std::vector<bevarmejo::anytown::tanks_costs> m__tanks_costs;

    // For constructor:
    void load_network(json settings, std::vector<fsys::path> lookup_paths, std::function<void (EN_Project)> preprocessf = [](EN_Project ph){ return;});
    void load_subnets(json settings, std::vector<fsys::path> lookup_paths);
    void load_other_data(json settings, std::vector<fsys::path> lookup_paths);

    // For the bounds:
    friend std::pair<std::vector<double>, std::vector<double>> f1::bounds__exis_pipes(const bevarmejo::anytown::Problem &prob);
    friend std::pair<std::vector<double>, std::vector<double>> bounds__new_pipes(const bevarmejo::anytown::Problem &prob);
    friend std::pair<std::vector<double>, std::vector<double>> bounds__pumps(const bevarmejo::anytown::Problem &prob);
    friend std::pair<std::vector<double>, std::vector<double>> f1::bounds__tanks(const bevarmejo::anytown::Problem &prob);
}; // class Problem

} // namespace anytown
} // namespace bevarmejo

#endif // ANYTOWN__PROB_ANYTOWN_HPP
