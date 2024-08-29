#ifndef ANYTOWN__MIXED__PROB_ANYTOWN_MIXED_F1_HPP
#define ANYTOWN__MIXED__PROB_ANYTOWN_MIXED_F1_HPP
//#define DEBUGSIM

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
// Data of Anytown that can't be changed.
namespace anytown {
namespace mixed {
namespace f1 {

const std::string name = "bevarmejo::anytown::mixed::f1";
const std::string extra_info = "\tVersion 1 of Anytown Mixed Formulation 1\nOperations as dv, pipes as in Farmani, Tanks as in Vamvakeridou-Lyroudia but discrete)\n";
    
// Dimensions of the problem.
constexpr std::size_t n_obj = 2u;
constexpr std::size_t n_ec = 0u;
constexpr std::size_t n_ic = 0u;
constexpr std::size_t n_fit = n_obj + n_ec + n_ic;
constexpr std::size_t n_dv = 104u;
constexpr std::size_t n_ix = 104u; // Will transform the tank volume to a continuous variable in the future.
constexpr std::size_t n_cx = n_dv-n_ix;


// Here the problem is actually construted.
class Problem : public anytown::Problem {
public:
    using inherited= anytown::Problem;

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

    void save_solution(const std::vector<double>& dv, const fsys::path& out_file) const {
        auto pass_on_info = apply_dv(this->m__anytown, dv);

	    int errco = EN_saveinpfile(this->m__anytown->ph_, out_file.c_str());
	    assert(errco <= 100);

	    reset_dv(this->m__anytown, dv, pass_on_info);
    }

private: 
    /* Anytown specific functions */
    double cost(const WDS &anytown, const std::vector<double>& dv) const;
    
    /* Helper functions */
    std::vector<double> apply_dv(std::shared_ptr<bevarmejo::wds::WaterDistributionSystem> anytown, const std::vector<double>& dv) const;
    void reset_dv(std::shared_ptr<bevarmejo::wds::WaterDistributionSystem> anytown, const std::vector<double>& dv, const std::vector<double>& old_HW_coeffs) const;
    
}; // class Problem

} // namespace f1
} // namespace mixed
} // namespace anytown
} // namespace bevarmejo

#endif // ANYTOWN__MIXED__PROB_ANYTOWN_MIXED_F1_HPP
