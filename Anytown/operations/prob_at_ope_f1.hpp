#ifndef ANYTOWN__OPERATIONS__PROB_ANYTOWN_OPE_F1_HPP
#define ANYTOWN__OPERATIONS__PROB_ANYTOWN_OPE_F1_HPP

#include <iostream>
#include <filesystem>
#include <memory>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "Anytown/prob_anytown.hpp"

namespace fsys = std::filesystem;
namespace bevarmejo {
namespace anytown{
namespace operations {
namespace f1 {

constexpr std::size_t n_obj = 2u;
constexpr std::size_t n_ec = 0u;
constexpr std::size_t n_ic = 0u;
constexpr std::size_t n_fit = n_obj + n_ec + n_ic;
constexpr std::size_t n_dv = 24u;
constexpr std::size_t n_ix = 24u; 
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
    std::string get_name() const { return io::value::opertns_f1; }

    // Extra information about the problem
    std::string get_extra_info() const { return io::other::opertns_f1_exinfo; }

    // Mandatory public functions necessary for the optimization algorithm:
    // Implementation of the objective function.
    std::vector<double> fitness(const std::vector<double>& dv) const;

    // Implementation of the box bounds.
    std::pair<std::vector<double>, std::vector<double>> get_bounds() const;


private: 
    /* Anytonw specific data */
    std::shared_ptr<bevarmejo::wds::WaterDistributionSystem> m_anytown;

    /* Anytown specific functions */
    double cost(const bevarmejo::wds::WaterDistributionSystem& a_wds) const;
    
    /* Helper functions */
    void apply_dv(bevarmejo::wds::WaterDistributionSystem& anytown, const std::vector<double>& dvs) const;
    void reset_dv(bevarmejo::wds::WaterDistributionSystem& anytown, const std::vector<double>& dvs) const { return; } // Nothing to reset as I always overwrite the values
    
public:
    void anytown(std::shared_ptr<bevarmejo::wds::WaterDistributionSystem> anytown) { m_anytown = anytown; }


}; // class Problem

} // namespace f1
} // namespace operations
} // namespace anytown
} // namespace bevarmejo

#endif // ANYTOWN__OPERATIONS__PROB_ANYTOWN_OPE_F1_HPP
