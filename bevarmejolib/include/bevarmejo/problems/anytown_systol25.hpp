#ifndef PROBLEMS__ANYTOWN_SYSTOL25_HPP
#define PROBLEMS__ANYTOWN_SYSTOL25_HPP

#include <utility>
#include <vector>

#include "bevarmejo/utility/exceptions.hpp"
#include "bevarmejo/utility/io.hpp"
namespace bemeio = bevarmejo::io;

#include "bevarmejo/wds/water_distribution_system.hpp"
#include "bevarmejo/simulation/solvers/epanet/hydraulic.hpp"
#include "bevarmejo/problem/wds_problem.hpp"

#include "bevarmejo/problems/anytown.hpp"

namespace bevarmejo
{
namespace anytown_systol25
{
static const std::string problem_name = "anytown_systol25";

// Forward definition of the Problem class.
class Problem;

// Definition of the possible formulations:
enum class Formulation
{
    hr, // Hydraulic reliability
    mr, // Mechanical reliability
    fr // Firefighting reliability
}; // enum class Formulation

class Problem : public WDSProblem
{
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
private:
    void load_networks(const Json& settings, const bemeio::Paths& lookup_paths);
    void load_other_data(const Json& settings, const bemeio::Paths& lookup_paths);

public:
    // Number of objective functions
    std::vector<double>::size_type get_nobj() const;

    // Number of equality constraints
    std::vector<double>::size_type get_nec() const;

    // Number of INequality constraints
    std::vector<double>::size_type get_nic() const;

    // Mandatory public functions necessary for the optimization algorithm:
    // Implementation of the objective function.
    std::vector<double> fitness(const std::vector<double>& dvs) const;

    // Implementation of the box bounds.
    std::pair<std::vector<double>, std::vector<double>> get_bounds() const;

    void save_solution(const std::vector<double>& dvs, const fsys::path& out_file) const;

protected:
    Formulation m__formulation; // Track the problem formulation

    // All problems formulation:
    std::shared_ptr<bevarmejo::WaterDistributionSystem> m__anytown;
    std::string m__anytown_filename;
        
    mutable std::unordered_map<std::string, double> __old_HW_coeffs; // Store the old HW coefficients for reset_dv__exis_pipes

    sim::solvers::epanet::HydSimSettings m__eps_settings; // Settings for the main 24-hour EPS simulation
    
    // Mechanical reliability formulation
    sim::solvers::epanet::HydSimSettings m__mrsim__settings; // Settings for the mechanical reliability simulation

    // Firefighting reliability formulation
    std::shared_ptr<bevarmejo::WaterDistributionSystem> m__ff_anytown; // Anytown network to simulate the fire flows...
    std::string m__ff_anytown_filename;

    sim::solvers::epanet::HydSimSettings m__ffsim_settings; // Settings to simulate the fireflows

protected:
    // Methods 
    // For fitness function:
    auto cost(const std::vector<double>& dv) const -> double;

    auto hydraulic_reliability_perspective() const -> double;

    auto mechanical_reliability_perspective() const -> double;

    auto firefighting_reliability_perspective() const -> double;
    
    void apply_dv(const std::vector<double>& dvs) const;
    void reset_dv(const std::vector<double>& dvs) const;

    // Helper to transform the decision variables from pagmo to beme format
    std::vector<bool> get_continuous_dvs_mask() const override;
private:
    // Json serializers
    friend void to_json(Json &j, const Problem &prob);
    friend void from_json(const Json &j, Problem &prob);
}; // class Problem
    
} // namespace anytown_systol25

} // namespace bevarmejo

#endif // PROBLEMS__ANYTOWN_SYSTOL25_HPP
