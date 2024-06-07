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
namespace nl = nlohmann;

#include "bevarmejo/io.hpp"
#include "bevarmejo/labels.hpp"
#include "bevarmejo/pagmo_helpers/containers_help.hpp"

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

    void save_solution(const std::vector<double>& dv, const fsys::path& out_file) const {
        auto pass_on_info = apply_dv(this->_anytown_, dv);

	    int errco = EN_saveinpfile(this->_anytown_->ph_, out_file.c_str());
	    assert(errco <= 100);

	    reset_dv(this->_anytown_, dv, pass_on_info);
    }

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
    
public:
    friend std::pair<nl::json, std::string> bevarmejo::io::json::detail::static_params<>(const Problem& prob);
    friend nl::json bevarmejo::io::json::detail::dynamic_params<>(const Problem& prob); 
};

} // namespace f1
} // namespace twophases
} // namespace anytown

namespace io {

// Specializations on how to write it to a json file
template <>
inline std::pair<nl::json, std::string> json::detail::static_params<bevarmejo::anytown::twophases::f1::Problem>(const bevarmejo::anytown::twophases::f1::Problem& prob) {
    
    nl::json jparams { };
    jparams[to_kebab_case(std::string("inp"))] = prob._anytown_->inp_file();
    // Skip pipes alt costs and tanks costs even if they should be there 
    jparams[to_kebab_case(std::string("m_algo"))] = json::static_descr(prob.m_algo)[to_kebab_case(label::__algorithm)];
    
    return std::make_pair(jparams, prob.get_extra_info());
} // static_params

template <>
inline nl::json json::detail::dynamic_params<bevarmejo::anytown::twophases::f1::Problem>(const bevarmejo::anytown::twophases::f1::Problem& prob) {
    
    // Info about the population here are a little bit easier as I know the type of the problem, the number of fitness evaluation and I don't need tome
    auto population_ids = prob.m_pop.get_ID();
    auto pop_dvs = prob.m_pop.get_x();
    auto pop_fvs = prob.m_pop.get_f();
    nl::json jpop { };
    for (auto individual = 0u; individual < prob.m_pop.size(); ++individual ) {
        jpop.push_back( { {to_kebab_case(label::__id), population_ids[individual]}, 
                          {to_kebab_case(label::__dv), pop_dvs[individual]},
                          {to_kebab_case(label::__fv), pop_fvs[individual]} } );
    }
    
    return nl::json{ {to_kebab_case(std::string("m_pop")), { {to_kebab_case(label::__individuals), jpop } } } };
    
} // dynamic_params

} // namespace io 

} // namespace bevarmejo

#endif // ANYTOWN__TWOPHASES__PROB_ANYTOWN_TWO_F1_HPP
