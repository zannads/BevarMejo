#ifndef ANYTOWN__TWOPHASES__PROB_ANYTOWN_TWO_F1_HPP
#define ANYTOWN__TWOPHASES__PROB_ANYTOWN_TWO_F1_HPP

#include <iostream>
#include <filesystem>
#include <memory>
#include <utility>
#include <vector>

#include <pagmo/island.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "Anytown/prob_anytown.hpp"

namespace fsys = std::filesystem;
namespace bevarmejo {
// Data of Anytown that can't be changed.
namespace anytown {
namespace twophases {
namespace f1 {

const std::string name = "bevarmejo::anytown::twophases::f1";
const std::string extra_info = "\tVersion 1 of Anytown Rehabilitation Formulation 1\nPipes as in Farmani, Tanks as in Vamvakeridou-Lyroudia but discrete, operations optimized internally)\n";
    
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

    /* PUBLIC functions for Pagmo Algorihtm */
    // Number of objective functions
    std::vector<double>::size_type get_nobj() const { return n_obj; }

    // Number of equality constraints
    std::vector<double>::size_type get_nec() const { return n_ec; }

    // Number of INequality constraints
    std::vector<double>::size_type get_nic() const { return n_ic; }

    // Number of integer decision variables
    std::vector<double>::size_type get_nix() const { return n_ix; }

    // Number of continous decision variables is automatically retrieved with get_bounds() and get_nix()

    // Name of the problem
    std::string get_name() const { return name; }

    // Extra information about the problem
    std::string get_extra_info() const { return extra_info; }

    // Mandatory public functions necessary for the optimization algorithm:
    // Implementation of the objective function.
    std::vector<double> fitness(const std::vector<double>& dv) const;

    // Implementation of the box bounds.
    std::pair<std::vector<double>, std::vector<double>> get_bounds() const;


private: 
    /* Anytonw specific data */
    std::shared_ptr<bevarmejo::wds::WaterDistributionSystem> _anytown_;
    std::vector<anytown::pipes_alt_costs> _pipes_alt_costs_;
    std::vector<anytown::tanks_costs> _tanks_costs_;
    // internal operation problem 
    pagmo::algorithm m_algo;
    mutable pagmo::population m_pop; // I need this to be mutable, so that I can invoke non-const functions on it. In particular, change the problem pointer.

    /* Anytown specific functions */
    double cost(const std::vector<double>& dv, const double energy_cost_per_day) const;
    
    /* Helper functions */
    std::vector<double> apply_dv(std::shared_ptr<bevarmejo::wds::WaterDistributionSystem> anytown, const std::vector<double>& dv) const;
    void reset_dv(std::shared_ptr<bevarmejo::wds::WaterDistributionSystem> anytown, const std::vector<double>& dv, const std::vector<double>& old_HW_coeffs) const;
    

};

} // namespace f1
} // namespace twophases
} // namespace anytown
} // namespace bevarmejo

#endif // ANYTOWN__TWOPHASES__PROB_ANYTOWN_TWO_F1_HPP
