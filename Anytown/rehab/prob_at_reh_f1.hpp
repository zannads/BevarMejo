#ifndef ANYTOWN__REHAB__PROB_ANYTOWN_REHAB_F1_HPP
#define ANYTOWN__REHAB__PROB_ANYTOWN_REHAB_F1_HPP
//#define DEBUGSIM

#include <iostream>
#include <filesystem>
#include <memory>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "prob_anytown.hpp"

namespace fsys = std::filesystem;
namespace bevarmejo {
// Data of Anytown: hard coded as they don't change for now.
namespace anytown {
namespace rehab {
namespace f1 {
    
// Dimensions of the problem.
constexpr std::size_t n_obj = 2u;
constexpr std::size_t n_ec = 0u;
constexpr std::size_t n_ic = 0u;
constexpr std::size_t n_fit = n_obj + n_ec + n_ic;
constexpr std::size_t n_dv = 80u;
constexpr std::size_t n_ix = 80u; // Will transform the tank volume to a continuous variable in the future.
constexpr std::size_t n_cx = n_dv-n_ix;
    
class Problem {
public: 
    Problem() = default;

    Problem(json settings, std::vector<std::filesystem::path> lookup_paths);

    // Copy constructor
    Problem(const Problem& other) = default;

    // Move constructor
    Problem(Problem&& other) noexcept = default;

    // Copy assignment operator
    Problem& operator=(const Problem& rhs) = default;

    // Move assignment operator
    Problem& operator=(Problem&& rhs) noexcept = default;

    // Destructor
    ~Problem() = default;

    // Try to have copy and move constructor automatically created

    /* PUBLIC functions for Pagmo Algorihtm */
    // Number of objective functions
    std::vector<double>::size_type get_nobj() const;

    // Number of equality constraints
    std::vector<double>::size_type get_nec() const;

    // Number of INequality constraints
    std::vector<double>::size_type get_nic() const;

    // Number of integer decision variables
    std::vector<double>::size_type get_nix() const;

    // Number of continous decision variables is automatically retrieved with get_bounds() and get_nix()

    //
    std::string get_extra_info() const;

    // Mandatory public functions necessary for the optimization algorithm:
    // Implementation of the objective function.
    std::vector<double> fitness(const std::vector<double>& dv) const;

    // Implementation of the box bounds.
    std::pair<std::vector<double>, std::vector<double>> get_bounds() const;


private: 
    /* Anytonw specific data */
    mutable std::shared_ptr<bevarmejo::wds::WaterDistributionSystem> _anytown_;
    std::vector<anytown::pipes_alt_costs> _pipes_alt_costs_;
    std::vector<anytown::tanks_costs> _tanks_costs_;

    /* Anytown specific functions */
    double cost(const std::vector<double>& dv, const double energy_cost_per_day) const;
    
    /* Helper functions */
    std::vector<double> apply_dv(std::shared_ptr<bevarmejo::wds::WaterDistributionSystem> anytown, const std::vector<double>& dv) const;
    void reset_dv(std::shared_ptr<bevarmejo::wds::WaterDistributionSystem> anytown, const std::vector<double>& dv, const std::vector<double>& old_HW_coeffs) const;
    
}; // class Problem
 
} // namespace f1
} // namespace rehab
} // namespace anytown
} // namespace bevarmejo

#endif // ANYTOWN__REHAB__PROB_ANYTOWN_REHAB_F1_HPP
