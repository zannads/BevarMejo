#ifndef BEVARMEJOLIB__PAGMO_HELPERS__CONTAINERS_HELP_HPP
#define BEVARMEJOLIB__PAGMO_HELPERS__CONTAINERS_HELP_HPP

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include <pagmo/algorithm.hpp>
#include <pagmo/island.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/r_policy.hpp>
#include <pagmo/s_policy.hpp>
#include <pagmo/topology.hpp>

#include <nlohmann/json.hpp>
using json_o = nlohmann::json;

namespace bevarmejo::io::inp
{
void temp_net_to_file(const pagmo::problem& prob, const std::vector<double>& dv, const std::string& out_file);
} // namespace bevarmejo::io::inp

// Serialize any pagmo container into a dynamic object (e.g., json). The 
// containers are: algorithm, problem, island, r_policy, s_policy, topology.
// Each of these containers can have static and dynamic parameters. The static
// parameters are set at the beginning of the optimisation and don't change.
// The dynamic parameters are the ones that change at runtime. Since I would 
// like to track the evolution of the dynamic parameters, I need two functions
// to return the specific type of information that I need.

namespace bevarmejo::io::json {

/*-------------------- Algorithm --------------------*/
json_o static_descr(const pagmo::algorithm& algo);
json_o dynamic_descr(const pagmo::algorithm& algo);

/*-------------------- Problem --------------------*/
json_o static_descr(const pagmo::problem& prob);
json_o dynamic_descr(const pagmo::problem& prob);

/*-------------------- Island --------------------*/
json_o static_descr(const pagmo::island& isl);
// Islands can not have dynamic parameters, so it is a compile error to call this function
json_o dynamic_descr(const pagmo::island& isl) = delete;

/*-------------------- Replacement policy --------------------*/
json_o static_descr(const pagmo::r_policy& rp);
json_o dynamic_descr(const pagmo::r_policy& rp) = delete;

/*-------------------- Selection policy --------------------*/
json_o static_descr(const pagmo::s_policy& sp);
json_o dynamic_descr(const pagmo::s_policy& sp) = delete;

/*-------------------- Topology --------------------*/
json_o static_descr(const pagmo::topology& tp);
json_o dynamic_descr(const pagmo::topology& tp) = delete;

}   // namespace bevarmejo::io::json


#endif // BEVARMEJOLIB__PAGMO_HELPERS__CONTAINERS_HELP_HPP
