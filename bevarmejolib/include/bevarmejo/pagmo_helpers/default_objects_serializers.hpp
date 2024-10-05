#pragma once

#include <algorithm>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>
using json_o = nlohmann::json;

#include <pagmo/algorithms/null_algorithm.hpp>
#include <pagmo/problems/null_problem.hpp>
#include <pagmo/islands/thread_island.hpp>
#include <pagmo/r_policies/fair_replace.hpp>
#include <pagmo/s_policies/select_best.hpp>
#include <pagmo/topologies/unconnected.hpp>

// When serializing a pagmo object, based on the type of object in it, the 
// function will switch the detail::static_params or detail::dynamic_params for 
// that object type. 
// Here, I declare the functions for the default pagmo objects.
// They don't have dynamic parameters, so I will only implement the static ones.

namespace bevarmejo::io::json::detail {

/*----------------------- Thread island ------------------ */
std::pair<json_o,std::string> static_params(const pagmo::thread_island& isl);
json_o dynamic_params(const pagmo::thread_island& isl) = delete;

/*----------------------- Fair replace ------------------ */
std::pair<json_o,std::string> static_params(const pagmo::fair_replace& rp);
json_o dynamic_params(const pagmo::fair_replace& rp) = delete;

/*----------------------- Select best ------------------ */
std::pair<json_o,std::string> static_params(const pagmo::select_best& sp);
json_o dynamic_params(const pagmo::select_best& sp) = delete;

/*----------------------- Unconnected topology ------------------ */
// Unconnected topology don't have any type of parameters (static nor dynamic)
std::pair<json_o,std::string> static_params(const pagmo::unconnected& tp) = delete;
json_o dynamic_params(const pagmo::unconnected& tp) = delete;

} //  namespace bevarmejo::io::json::detail
