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

#include "bevarmejo/utility/exceptions.hpp"
#include "bevarmejo/utility/io.hpp"
namespace bemeio = bevarmejo::io;

#include "bevarmejo/wds/water_distribution_system.hpp"
#include "bevarmejo/simulation/solvers/epanet/hydraulic.hpp"
#include "bevarmejo/problem/wds_problem.hpp"

namespace bevarmejo {

namespace label {
static const std::string __temp_elems = "TEs";
} // namespace label 

// Beginning the definition of the Anytown data and problem.
// As this is a benchmark problem, i.e., fully specified, we hard-code it.
// We will allow, some parts to be modified in the parameters pack of the JSON
// at construction.
namespace anytown {

// Forward definition of the Problem class.
class Problem;

// Anytown problem parameters.
static const std::string problem_name = "anytown"; // "anytown"

// We base the problem on the Anytown '.inp' file downloaded from the Centre for Water Systems.

// Anytown system characteristics:
constexpr double treatment_plant_head__ft = 10.0;

// Anytown system load conditions.
// Daily operations:
constexpr double min_pressure__psi = 40.0;
constexpr double average_daily_flow_multiplier = 1.0;
constexpr double peak_flow_multiplier = 1.3;

// Instantaneous peak flow:
constexpr double instantaneous_peak_flow_multiplier = 1.8;

// Fire flow conditions:
constexpr double min_pressure_fireflow__psi = 20.0;
constexpr double fireflow_multiplier = peak_flow_multiplier;
constexpr double fireflow_duration__s = 7200;
// From the original work:
// The fire flow required is 500 gpm (0.0316 m3/s) at all nodes except for:
// (1) 2,500 gpm (0.158 m3/s) at node 90 (19 in the file);
// (2) 1,500 gpm (0.0946 m3/s) at nodes 75, 115, and 55 (5,6,7 in the new file);
// and (3) 1,000 gpm (0.0631 m3/s) at nodes 120 and 160 (11 and 17 in the new file).
// Define the struct for junction fire flow data
struct JunctionFireFlow {
    std::string_view junction_name;
    double flow__gpm;
    double flow__m3ps;
    double flow__lps;
};
constexpr std::array<JunctionFireFlow, 19> fireflow_test_values{{
    {"1", 500.0, 0.0316, 31.6},
    {"2", 500.0, 0.0316, 31.6},
    {"3", 500.0, 0.0316, 31.6},
    {"4", 500.0, 0.0316, 31.6},
    {"5", 1500.0, 0.0946, 94.6}, // Node 75 (1,500 gpm)
    {"6", 1500.0, 0.0946, 94.6}, // Node 115 (1,500 gpm)
    {"7", 1500.0, 0.0946, 94.6}, // Node 55 (1,500 gpm)
    {"8", 500.0, 0.0316, 31.6},
    {"9", 500.0, 0.0316, 31.6},
    {"10", 500.0, 0.0316, 31.6},
    {"11", 1000.0, 0.0631, 63.1}, // Node 120 (1,000 gpm)
    {"12", 500.0, 0.0316, 31.6},
    {"13", 500.0, 0.0316, 31.6},
    {"14", 500.0, 0.0316, 31.6},
    {"15", 500.0, 0.0316, 31.6},
    {"16", 500.0, 0.0316, 31.6},
    {"17", 1000.0, 0.0631, 63.1}, // Node 160 (1,000 gpm)
    {"18", 500.0, 0.0316, 31.6},
    {"19", 2500.0, 0.158, 158.0}  // Node 90 (2,500 gpm)
}};

// Objective functions:
// 1. Net Present cost
constexpr double energy_cost__kWh = 0.12; // dollars per kWh
constexpr double discount_rate = 0.12; // 12% per year
constexpr double amortization_years = 20.0; // 20 years
// The requirements should translate in constraints, as the objective should only be net present cost.
// However, how this is done depends on the optimization algorithm and how it handles the constraint.
// We also added a constraint that was not present in the original formulation (max velocity on the pipes)
static constexpr double max_velocity__m_per_s = 1.5;
// This enum class switches this behaviour.
enum class ReliabilityObjectiveFunctionFormulation
{
    Base,
    Hierarchical,
    HierarchicalWithMaxVelocity
}; // enum class ReliabilityObjectiveFunctionFormulation

// Base
namespace fr1 {
auto of__reliability(
    const WDS& anytown
) -> double;
}
// Hierarchical
namespace fr2 {
auto of__reliability(
    const WDS& anytown,
    const bevarmejo::sim::solvers::epanet::HydSimResults &res
) -> double;
}
// Hierarchical with MAx veloicty
namespace fr3 {
auto of__reliability(
    const WDS& anytown,
    const bevarmejo::sim::solvers::epanet::HydSimResults &res,
    double max_velocity__m_per_s
) -> double;
}


// 2. Originally not defined, we will use the Todini reliability index integrated during the day
// all parameters are already specified.

// Decision variables:
// Existing pipes:
//  - do nothing,
//  - clean, or
//  - duplicate with a given diameter.
// Different cost based on if the existing pipe is in the city centre or not.
static const std::string city_pipes__subnet_name = "city_pipes";
static const std::vector<std::string> city_pipes__el_names = {"2", "3", "4", "27", "28", "29", "30", "31", "32", "33", "34", "35", "37", "38", "41"};
static const std::string exis_pipes__subnet_name = "existing_pipes";
static const std::vector<std::string> exis_pipes__el_names = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "11", "12", "17", "18", "19", "20", "21", "22", "23", "24", "26", "27", "28", "29", "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "40", "41"};

constexpr double coeff_HW_cleaned = 125.0;
constexpr double coeff_HW_new = 130.0;

struct exi_pipe_option {
    double diameter__in;
    double cost_dup_city__per_ft;
    double cost_dup_resi__per_ft;
    double cost_clean_city__per_ft;
    double cost_clean_resi__per_ft;        
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(exi_pipe_option, diameter__in, cost_dup_city__per_ft, cost_dup_resi__per_ft, cost_clean_city__per_ft, cost_clean_resi__per_ft)

static const std::vector<bevarmejo::anytown::exi_pipe_option> exi_pipe_options {
    { 6, 26.2, 14.2, 17.0, 12.0 },
    { 8, 27.8, 19.8, 17.0, 12.0 },
    { 10, 34.1, 25.1, 17.0, 12.0 },
    { 12, 41.4, 32.4, 17.0, 13.0 },
    { 14, 50.2, 40.2, 18.2, 14.2 },
    { 16, 58.5, 48.5, 19.8, 15.5 },
    { 18, 66.2, 57.2, 21.6, 17.1 },
    { 20, 76.8, 66.8, 23.5, 20.2 },
    { 24, 109.2, 85.5, 30.1, 1000000 },
    { 30, 142.5, 116.1, 41.3, 1000000 }
};

// The way that this decision variable is coded can vary.
// The "FarmaniEtAl2005" approach is the one described in the "Farmani et al., 2005" paper.
// The "Combined" one is how I would have define it to simplify the search space.
enum class ExistingPipesFormulation {
    FarmaniEtAl2005,
    Combined
}; // enum class ExistingPipesFormulation

// ExistingPipesFormulation: "FarmaniEtAl2005"
namespace fep1 {
constexpr std::size_t dv_size = 2;
constexpr std::array<bool,dv_size> dv_continous_mask{false, false};
constexpr std::size_t n_dvs_disc = dv_size;

auto bounds__exis_pipes(
    InputOrderedRegistryView<WDS::Pipe> exis_pipes,
    const std::vector<bevarmejo::anytown::exi_pipe_option> &ep_opts
) -> std::pair<std::vector<double>, std::vector<double>>;
void apply_dv__exis_pipes(
    WDS& anytown,
    std::unordered_map<std::string, double> &old_HW_coeffs,
    const std::vector<double>& dvs,
    const std::vector<bevarmejo::anytown::exi_pipe_option> &ep_opts
);
auto cost__exis_pipes(
    const WDS& anytown,
    const std::vector<double>& dvs,
    const std::vector<bevarmejo::anytown::exi_pipe_option> &ep_opts
) -> double;
void reset_dv__exis_pipes(
    WDS& anytown,
    const std::vector<double>& dvs,
    const std::unordered_map<std::string, double> &old_HW_coeffs
);
} // fep1

// ExistingPipesFormulation: "Combined"
namespace fep2 {
constexpr std::size_t dv_size = 1;
constexpr std::array<bool,dv_size> dv_continous_mask{false};
constexpr std::size_t n_dvs_disc = dv_size;

auto bounds__exis_pipes(
    InputOrderedRegistryView<WDS::Pipe> exis_pipes,
    const std::vector<bevarmejo::anytown::exi_pipe_option> &ep_opts
) -> std::pair<std::vector<double>, std::vector<double>>;
void apply_dv__exis_pipes(
    WDS& anytown,
    std::unordered_map<std::string, double> &old_HW_coeffs,
    const std::vector<double>& dvs,
    const std::vector<bevarmejo::anytown::exi_pipe_option> &ep_opts
);
auto cost__exis_pipes(
    const WDS& anytown,
    const std::vector<double>& dvs,
    const std::vector<bevarmejo::anytown::exi_pipe_option> &ep_opts
) -> double;
void reset_dv__exis_pipes(
    WDS& anytown,
    const std::vector<double>& dvs,
    const std::unordered_map<std::string, double> &old_HW_coeffs
);
} // fep2

// New pipes:
//  - select diameter (mandatory)
static const std::string new_pipes__subnet_name = "new_pipes";
static const std::vector<std::string> new_pipes__el_names = {"110", "113", "114", "115", "116", "125"};

struct new_pipe_option {
    double diameter__in;
    double cost__per_ft;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(new_pipe_option, diameter__in, cost__per_ft)

static const std::vector<bevarmejo::anytown::new_pipe_option> new_pipe_options {
    { 6, 12.8},
    { 8, 17.8},
    { 10, 22.5},
    { 12, 29.2},
    { 14, 36.2},
    { 16, 43.6},
    { 18, 51.5},
    { 20, 60.1},
    { 24, 77.0},
    { 30, 105.5}
};

// This pipes are already installed in the network with a "non-existing pipe diameter"
constexpr double nonexisting_pipe_diam__ft = 0.0001;

// NewPipesFormulation: "Default" only existing one
namespace fnp1 {
constexpr std::size_t dv_size = 1;
constexpr std::array<bool,dv_size> dv_continous_mask{false};
constexpr std::size_t n_dvs_disc = dv_size;
auto bounds__new_pipes(
    InputOrderedRegistryView<WDS::Pipe> new_pipes,
    const std::vector<bevarmejo::anytown::new_pipe_option> &np_opts
) -> std::pair<std::vector<double>, std::vector<double>>;
void apply_dv__new_pipes(
    WDS& anytown,
    const std::vector<double>& dvs,
    const std::vector<bevarmejo::anytown::new_pipe_option> &np_opts
);
auto cost__new_pipes(
    const WDS& anytown,
    const std::vector<double>& dvs,
    const std::vector<bevarmejo::anytown::new_pipe_option> &np_opts
) -> double;
void reset_dv__new_pipes(
    WDS& anytown,
    const std::vector<double>& dvs
);
} // fnp1

// Pumps:
// My preferred default pattern for the pump group (inspired by Siew et al., 2016 and modified based on early results)
constexpr std::array<double, bevarmejo::k__hours_per_day> pump_group_operations {
    3, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2,
    2, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 2
};
// Function to convert a single vector with the number of pumps in a group running
// at a given time to a set of boolean (double actually) to specifiy if a pump is on or off.
auto decompose_pumpgroup_pattern(
    std::vector<double> pg_pattern,
    const std::size_t n_pumps
) -> std::vector<std::vector<double>>;

// Pump Group Operationd Decision Variable
namespace pgo_dv {
constexpr std::size_t size = pump_group_operations.size();
constexpr std::array<bool,size> is_continous_mask{
    false, false, false, false, false, false,
    false, false, false, false, false, false,
    false, false, false, false, false, false,
    false, false, false, false, false, false
};
constexpr std::size_t n_discrete_dv = size;
auto bounds__pumps(
    InputExcludingRegistryView<WDS::Pump> pumps
) -> std::pair<std::vector<double>, std::vector<double>>;
void apply_dv__pumps(
    WDS& anytown,
    const std::vector<double>& dvs
);
auto cost__energy_per_day(
    const WDS& anytown
) -> double;
void reset_dv__pumps(
    WDS& anytown,
    const std::vector<double>& dvs
);
} // pgo_dv

// New tanks:
// up two tanks and its complete specifications, however the riser length is fixed
// (which should mean that the elevation plus this length gives you the tank bottom,
// but it has not always been interepreted like that).
constexpr std::size_t max_n_installable_tanks = 2;
static const std::string pos_tank_loc__subnet_name = "possible_tank_locations";
static const std::vector<std::string> pos_tank_loc__el_names = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "15", "16", "18", "19"};
constexpr double riser_length__ft = 101.0;

struct tank_option {
    double volume__gal;
    double cost;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(tank_option, volume__gal, cost)

static const std::vector<bevarmejo::anytown::tank_option> tank_options {
    {50000, 115000},
    {100000, 145000},
    {250000, 325000},
    {500000, 425000},
    {1000000, 600000}
};

// the other already existing tanks are operated and have already fixed characteristics.
constexpr double min_w_level_tank__ft = 225.0;
constexpr double max_w_level_tank__ft = 250.0;
constexpr double bottom_height_tank__ft = 215.0; // emergency volume between min_w_level_tank_ft and bottom_height_tank_ft

// This decision variable can be coded in many ways...
enum class NewTanksFormulation {
    Simple,
    FarmaniEtAl2005,
    LocVolRisDiamH2DRatio // Location, Volume, Riser diameter, Height-to-Diameter Ratio
}; // enum class NewTanksFormulation

// NewTanksFormulation: "Simple"
namespace fnt1 {
constexpr std::size_t dv_size = 2;
constexpr std::array<bool,dv_size> dv_continous_mask{false, false};
constexpr std::size_t n_dvs_disc = dv_size;
auto bounds__tanks(
    InputOrderedRegistryView<WDS::Junction> tank_locs,
    const std::vector<bevarmejo::anytown::tank_option> &tank_options
) -> std::pair<std::vector<double>, std::vector<double>>;
void apply_dv__tanks(
    WDS& anytown,
    const std::vector<double>& dvs,
    const std::vector<bevarmejo::anytown::tank_option> &tank_options
);
auto cost__tanks(
    const WDS& anytown,
    const std::vector<double>& dvs,
    const std::vector<bevarmejo::anytown::tank_option> &tank_options,
    const std::vector<bevarmejo::anytown::new_pipe_option> &new_pipes_options
) -> double;
void reset_dv__tanks(
    WDS& anytown,
    const std::vector<double>& dvs
);
} // fnt1

// FarmaniEtAl2005
namespace fnt2 {
constexpr std::size_t dv_size = 6;
constexpr std::array<bool,dv_size> dv_continous_mask{false, false, true, true, true, true};
constexpr std::size_t n_dvs_disc = 2;
constexpr double tank_hmax_ub__m = 250*MperFT; // new tank maximum hydraulic head level upper boundary for optimisation
constexpr double tank_hmax_lb__m = 200*MperFT; // same but lower boundary
constexpr double tank_hmin_ub__m = 240*MperFT;
constexpr double tank_hmin_lb__m = 180*MperFT;
constexpr double tank_diam_ub__m = 100*MperFT; // new tank's diameter upper boundary for optimization
constexpr double tank_diam_lb__m = 25*MperFT; // new tank's diameter lower boundary for optimization
constexpr double tank_safetyl_ub__m = 25*MperFT; // new tank's safety level upper boundary for optimization
constexpr double tank_safetyl_lb__m = 0*MperFT; // new tank's safety level lower boundary for optimization

auto bounds__tanks(
    InputOrderedRegistryView<WDS::Junction> tank_locs,
    const std::vector<bevarmejo::anytown::tank_option> &tank_options,
    const std::vector<bevarmejo::anytown::new_pipe_option> &new_pipe_options
) -> std::pair<std::vector<double>, std::vector<double>>;
void apply_dv__tanks(
    WDS& anytown,
    const std::vector<double>& dvs,
    const std::vector<bevarmejo::anytown::new_pipe_option> &new_pipes_options
);
auto cost__tanks(
    const WDS& anytown,
    const std::vector<double>& dvs,
    const std::vector<bevarmejo::anytown::tank_option> &tank_options,
    const std::vector<bevarmejo::anytown::new_pipe_option> &new_pipes_options
) -> double;
void reset_dv__tanks(
    WDS& anytown,
    const std::vector<double>& dvs
);
} // fnt2

// LocVolRisDiamH2DRatio
namespace fnt3 {
constexpr std::size_t dv_size = 4;
constexpr std::array<bool,dv_size> dv_continous_mask{false, false, false, false};
constexpr std::size_t n_dvs_disc = dv_size;
constexpr double h2d_ratio__min = 0.9;
constexpr double h2d_ratio__max = 1.5;
constexpr double hd2_ratio__step = 0.1;
constexpr int h2d_ratio__steps = (h2d_ratio__max-h2d_ratio__min)/hd2_ratio__step;

auto bounds__tanks(
    InputOrderedRegistryView<WDS::Junction> tank_locs,
    const std::vector<bevarmejo::anytown::tank_option> &tank_options,
    const std::vector<bevarmejo::anytown::new_pipe_option> &new_pipe_options
) -> std::pair<std::vector<double>, std::vector<double>>;
void apply_dv__tanks(
    WDS& anytown,
    const std::vector<double>& dvs,
    const std::vector<bevarmejo::anytown::tank_option> &tank_options,
    const std::vector<bevarmejo::anytown::new_pipe_option> &new_pipes_options
);
auto cost__tanks(
    const WDS& anytown,
    const std::vector<double>& dvs,
    const std::vector<bevarmejo::anytown::tank_option> &tank_options,
    const std::vector<bevarmejo::anytown::new_pipe_option> &new_pipes_options
) -> double;
void reset_dv__tanks(
    WDS& anytown,
    const std::vector<double>& dvs
);
}

// Definition of all possible problem formulations, based on the individual parts formulations.
enum class Formulation {
    rehab_f1,
    mixed_f1,
    opertns_f1,
    twoph_f1,
    rehab_f2,
    mixed_f2,
    rehab_f3,
    mixed_f3,
    rehab_f4,
    mixed_f4,
    rehab_f5,
    mixed_f5,
    rehab_f6,
    mixed_f6, // same as anytown_systol25::hyd_rel (except order of dvs)
    opertns_f2
}; // enum class Formulation

// For the json serializers:
namespace io::json::detail {
std::pair<Json,std::string> static_params(const bevarmejo::anytown::Problem &prob);
Json dynamic_params(const bevarmejo::anytown::Problem &prob);
}

class Problem : public WDSProblem {
private:
    using inherithed = WDSProblem;

public:
    Problem() = default;
    Problem(std::string_view a_formulation, const Json& settings, const bemeio::Paths& lookup_paths);
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
    std::string m__anytown_filename;
    sim::solvers::epanet::HydSimSettings m__eps_settings; // Settings for the main 24-hour EPS simulation

    std::vector<bevarmejo::anytown::exi_pipe_option> m__exi_pipe_options;
    std::vector<bevarmejo::anytown::new_pipe_option> m__new_pipe_options;
    std::vector<bevarmejo::anytown::tank_option> m__tank_options;
    Formulation m__formulation; // Track the problem formulation
    ExistingPipesFormulation m__exi_pipes_formulation;
    NewTanksFormulation m__new_tanks_formulation;
    ReliabilityObjectiveFunctionFormulation m__reliability_obj_func_formulation;
    bool m__has_design;
    bool m__has_operations;

    double m__max_velocity__m_per_s; // Maximum velocity for the reliability function
    mutable std::unordered_map<std::string, double> __old_HW_coeffs; // Store the old HW coefficients for reset_dv__exis_pipes
    // internal operation optimisation problem:
    pagmo::algorithm m_algo;
    mutable pagmo::population m_pop; // I need this to be mutable, so that I can invoke non-const functions on it. In particular, change the problem pointer.

    // For constructor:
    void load_network(const Json& settings, const bemeio::Paths& lookup_paths);
    void load_other_data(const Json& settings, const bemeio::Paths& lookup_paths);

    // For fitness function:
    double cost(const WDS &anytown, const std::vector<double>& dv) const;
    
    void apply_dv(const std::vector<double>& dvs) const;
    void reset_dv(const std::vector<double>& dvs) const;

    // Helper to transform the decision variables from pagmo to beme format
    std::vector<bool> get_continuous_dvs_mask() const;
private:
    // Json serializers
    friend void to_json(Json &j, const Problem &prob);
    friend void from_json(const Json &j, Problem &prob);
    
}; // class Problem

} // namespace anytown
} // namespace bevarmejo

#endif // ANYTOWN__PROB_ANYTOWN_HPP
