#ifndef HANOI__PROBLEM_HANOI_F1_HPP
#define HANOI__PROBLEM_HANOI_F1_HPP

#include <cmath>
#include <iostream>
#include <filesystem>
namespace fsys = std::filesystem;
#include <memory>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>
using json_o = nlohmann::json;

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "bevarmejo/problem/wds_problem.hpp"

namespace bevarmejo {
namespace hanoi {

constexpr double head_source_m = 100.0;
constexpr double min_head_m = 30.0;

constexpr std::size_t n_available_diams = 6u;
constexpr std::array<double, n_available_diams> available_diams_in = {12, 16, 20, 24, 30, 40};

// for cost function: C of pipe i = 1.1 * D_i^1.5 * L_i 
constexpr double a = 1.1;
constexpr double b = 1.5;

namespace fbiobj {
class Problem;

// Dimensions of the problem.
constexpr std::size_t n_obj = 2u;
constexpr std::size_t n_ec = 0u;
constexpr std::size_t n_ic = 0u;
constexpr std::size_t n_fit = n_obj + n_ec + n_ic;
constexpr std::size_t n_dv = 34u;
constexpr std::size_t n_ix = 34u; 
constexpr std::array<const char*, n_dv> changeable_pipe_ids = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
                                                                "11", "12", "13", "14", "15", "16", "17", "18", "19", "20",
                                                                "21", "22", "23", "24", "25", "26", "27", "28", "29", "30",
                                                                "31", "32", "33", "34" };
namespace label {
const std::string __changeable_pipes = "cp";
} // namespace label

namespace io::json::detail {
std::pair<json_o,std::string> static_params(const bevarmejo::hanoi::fbiobj::Problem &prob);
json_o dynamic_params(const bevarmejo::hanoi::fbiobj::Problem &prob) = delete;
} // namespace io::json::detail

class Problem final : public WDSProblem
{
/*------- Member types -------*/
private:
    using inherited = WDSProblem;

/*------- Member objects -------*/
private:
    std::shared_ptr<bevarmejo::WaterDistributionSystem> m_hanoi;
    std::array<double, n_available_diams> m_diams_cost; // This is a * D_i^b so it can be computed offline once.

/*------- Member functions -------*/
// (constructor)
public:
    Problem();
    Problem(const json_o& settings, const std::vector<fsys::path>& lookup_paths);
    Problem(const Problem& other) = default;
    Problem(Problem&& other) noexcept = default;

// (destructor)
public:
    ~Problem() = default;

// operator=
public:
    Problem& operator=(const Problem& rhs) = default;
    Problem& operator=(Problem&& rhs) noexcept = default;

/*------- Pagmo-required functions -------*/
    // Number of objective functions
    std::vector<double>::size_type get_nobj() const { return n_obj; }

    // Number of equality constraints
    std::vector<double>::size_type get_nec() const { return n_ec; }

    // Number of INequality constraints
    std::vector<double>::size_type get_nic() const { return n_ic; }

    // Number of integer decision variables
    std::vector<double>::size_type get_nix() const { return n_ix; }

    // Mandatory public functions necessary for the optimization algorithm:
    // Implementation of the objective function.
    std::vector<double> fitness(const std::vector<double>& dv) const;

    // Implementation of the box bounds.
    std::pair<std::vector<double>, std::vector<double>> get_bounds() const;


    double cost(const std::vector<double>& dv) const;

    void apply_dv(WaterDistributionSystem& a_wds, const std::vector<double>& dv) const;

    // No need to use reset as at every run the same design variables are for sure overwritten.

private:
    friend std::pair<json_o,std::string> io::json::detail::static_params(const Problem &prob);
};

} // namespace fbiobj
} // namespace hanoi
} // namespace bevarmejo

#endif // HANOI__PROBLEM_HANOI_F1_HPP
