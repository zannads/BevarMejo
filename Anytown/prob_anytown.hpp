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
#include <string_view>
#include <utility>
#include <vector>

#include <pagmo/problem.hpp>
#include <pagmo/algorithms/nsga2.hpp>
#include <pagmo/algorithm.hpp>
#include <pagmo/population.hpp>
#include <pagmo/island.hpp>

#include <nlohmann/json.hpp>
using json_o = nlohmann::json;

#include "bevarmejo/wds/water_distribution_system.hpp"
#include "bevarmejo/problem/wds_problem.hpp"

namespace bevarmejo {

namespace label {
static const std::string __temp_elems = "TEs";
} // namespace label 


// Data of Anytown: hard coded as they don't change for now.
// If you need to change them, but still harcoded, "override" them in a new namespace for that type of problem.
namespace anytown {

static const std::string nname = "anytown::"; // "anytown::"

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

// Definition of all possible formulations:
enum class Formulation {
    rehab_f1,
    mixed_f1,
    opertns_f1,
    twoph_f1,
    rehab_f2,
    mixed_f2
}; // enum class Formulation

// For the json serializer
namespace io::json::detail {
std::pair<json_o,std::string> static_params(const bevarmejo::anytown::Problem &prob);
json_o dynamic_params(const bevarmejo::anytown::Problem &prob);
}

// For the bounds
namespace fep1 {
std::pair<std::vector<double>, std::vector<double>> bounds__exis_pipes(InputOrderedRegistryView<WDS::Pipe> exis_pipes, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs);
}
namespace fep2 {
std::pair<std::vector<double>, std::vector<double>> bounds__exis_pipes(InputOrderedRegistryView<WDS::Pipe> exis_pipes, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs);
}  
std::pair<std::vector<double>, std::vector<double>> bounds__new_pipes(InputOrderedRegistryView<WDS::Pipe> new_pipes, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs);
std::pair<std::vector<double>, std::vector<double>> bounds__pumps(const WDS::Pumps &pumps);
namespace fnt1 {
std::pair<std::vector<double>, std::vector<double>> bounds__tanks(InputOrderedRegistryView<WDS::Junction> tank_locs, const std::vector<bevarmejo::anytown::tanks_costs> &tanks_costs);
}

// For fitness function:
//     For apply dv:
namespace fep1 {
void apply_dv__exis_pipes(WDS& anytown, std::unordered_map<std::string, double> &old_HW_coeffs, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs);
}
namespace fep2 {
void apply_dv__exis_pipes(WDS& anytown, std::unordered_map<std::string, double> &old_HW_coeffs, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs);
}
void apply_dv__new_pipes(WDS& anytown, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs);
void apply_dv__pumps(WDS& anytown, const std::vector<double>& dvs);
namespace fnt1 {
void apply_dv__tanks(WDS& anytown, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::tanks_costs> &tanks_costs);
}

//      For cost function:
namespace fep1 {
double cost__exis_pipes(const WDS& anytown, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs);
}
namespace fep2 {
double cost__exis_pipes(const WDS& anytown, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs);
}
double cost__new_pipes(const WDS& anytown, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs);
double cost__energy_per_day(const WDS& anytown);
namespace fnt1 {
double cost__tanks(const WDS& anytown, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::tanks_costs> &tanks_costs, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs);
}

//      For reliability (modified) function:
double of__reliability(const WDS& anytown);

//     For reset dv:
namespace fep1 {
void reset_dv__exis_pipes(WDS& anytown, const std::vector<double>& dvs, const std::unordered_map<std::string, double> &old_HW_coeffs);
}
namespace fep2 {
void reset_dv__exis_pipes(WDS& anytown, const std::vector<double>& dvs, const std::unordered_map<std::string, double> &old_HW_coeffs);
}
void reset_dv__new_pipes(WDS& anytown, const std::vector<double>& dvs);
void reset_dv__pumps(WDS& anytown, const std::vector<double>& dvs);
namespace fnt1 {
void reset_dv__tanks(WDS& anytown, const std::vector<double>& dvs);
}

class Problem : public WDSProblem {
private:
    using inherithed = WDSProblem;

public:
    Problem() = default;
    Problem(std::string_view a_formulation, const json_o& settings, const std::vector<fsys::path>& lookup_paths);
    Problem(const Problem& other) = default;
    Problem(Problem&& other) noexcept = default;
    Problem& operator=(const Problem& rhs) = default;
    Problem& operator=(Problem&& rhs) noexcept = default;
    ~Problem() = default;

/*----------------------*/
// PUBLIC functions for Pagmo Algorihtm 
/*----------------------*/

public:
    // Number of objective functions
    std::vector<double>::size_type get_nobj() const;

    // Number of equality constraints
    std::vector<double>::size_type get_nec() const;

    // Number of INequality constraints
    std::vector<double>::size_type get_nic() const;

    // Number of integer decision variables
    std::vector<double>::size_type get_nix() const;

    // Number of continous decision variables is automatically retrieved with get_bounds() and get_nix()

    // Mandatory public functions necessary for the optimization algorithm:
    // Implementation of the objective function.
    std::vector<double> fitness(const std::vector<double>& dvs) const;

    // Implementation of the box bounds.
    std::pair<std::vector<double>, std::vector<double>> get_bounds() const;

    void save_solution(const std::vector<double>& dvs, const fsys::path& out_file) const;

protected:
    // Anytown specific data
    mutable std::shared_ptr<bevarmejo::WaterDistributionSystem> m__anytown;
    std::vector<bevarmejo::anytown::pipes_alt_costs> m__pipes_alt_costs;
    std::vector<bevarmejo::anytown::tanks_costs> m__tanks_costs;
    Formulation m__formulation; // Track the problem formulation (affect the dvs for now)
    mutable std::unordered_map<std::string, double> __old_HW_coeffs; // Store the old HW coefficients for reset_dv__exis_pipes
    // internal operation optimisation problem:
    pagmo::algorithm m_algo;
    mutable pagmo::population m_pop; // I need this to be mutable, so that I can invoke non-const functions on it. In particular, change the problem pointer.

    // For constructor:
    void load_network(json_o settings, std::vector<fsys::path> lookup_paths, std::function<void (EN_Project)> preprocessf = [](EN_Project ph){ return;});
    void load_subnets(json_o settings, std::vector<fsys::path> lookup_paths);
    void load_other_data(json_o settings, std::vector<fsys::path> lookup_paths);

    // For fitness function:
    double cost(const WDS &anytown, const std::vector<double>& dv) const;
    
    void apply_dv(std::shared_ptr<bevarmejo::WaterDistributionSystem> anytown, const std::vector<double>& dvs) const;
    void reset_dv(std::shared_ptr<bevarmejo::WaterDistributionSystem> anytown, const std::vector<double>& dvs) const;

private:
    // make the serializer a friend
    friend std::pair<json_o,std::string> io::json::detail::static_params(const Problem &prob);
    friend json_o io::json::detail::dynamic_params(const Problem &prob);
    
}; // class Problem

} // namespace anytown
} // namespace bevarmejo

#endif // ANYTOWN__PROB_ANYTOWN_HPP
