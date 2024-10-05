#pragma once

#include <string>
#include <algorithm>
#include <utility>

#include <nlohmann/json.hpp>
namespace nl = nlohmann;

#include <pagmo/algorithm.hpp>
#include <pagmo/island.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/r_policy.hpp>
#include <pagmo/s_policy.hpp>
#include <pagmo/topology.hpp>

#include <pagmo/algorithms/nsga2.hpp>
#include <pagmo/islands/thread_island.hpp>
#include <pagmo/r_policies/fair_replace.hpp>
#include <pagmo/s_policies/select_best.hpp>
#include <pagmo/topologies/unconnected.hpp>

// Given a pagmo UD class of any type, usually you call get_extra_info() to get
// the settings of the class. However, this function returns a string.
// I want to return a json object for saving, and actually, I would need one for 
// static parameters and one for dynamic parameters. For now only static.

// Given any pagmo container (island, algorithm, etc) return a json object with
// name of the UD class, Parameters for input to the UD class, and extra info.
// Two formulations are available, one for static parameters and one for dynamic.
// The dynamic parameters are the ones that change at runtime, the static ones are
// set at the beginning of the optimisation and don't change.
// For each user defined class in an experiment (udc), this could have a set of 
// static parameters and a set of dynamic parameters (more like states of the class).
// The static parameters remain the same during en evolution, while the dynamic
// parameters change. Together with the static parameters, I allow a string with
// extra information that can be used to store any other information (e.g., an
// explanation of the problem, the topology, etc.)

namespace bevarmejo {
namespace io {
namespace json {

/*-------------------- Algorithm --------------------*/
nl::json static_descr(const pagmo::algorithm& algo);
nl::json dynamic_descr(const pagmo::algorithm& algo);

/*-------------------- Problem --------------------*/
nl::json static_descr(const pagmo::problem& prob);
nl::json dynamic_descr(const pagmo::problem& prob);

/*-------------------- Island --------------------*/
nl::json static_descr(const pagmo::island& isl);
// Islands can not have dynamic parameters, so it is a compile error to call this function
nl::json dynamic_descr(const pagmo::island& isl) = delete;

/*-------------------- Replacement policy --------------------*/
nl::json static_descr(const pagmo::r_policy& rp);
nl::json dynamic_descr(const pagmo::r_policy& rp) = delete;

/*-------------------- Selection policy --------------------*/
nl::json static_descr(const pagmo::s_policy& sp);
nl::json dynamic_descr(const pagmo::s_policy& sp) = delete;

/*-------------------- Topology --------------------*/
nl::json static_descr(const pagmo::topology& tp);
nl::json dynamic_descr(const pagmo::topology& tp) = delete;

namespace detail {

/*----------------------- Thread island ------------------ */
std::pair<nl::json,std::string> static_params(const pagmo::thread_island& isl);
// Thread island can not have dynamic parameters, so it is a compile error to call this function
nl::json dynamic_params(const pagmo::thread_island& isl) = delete;

/*----------------------- Fair replace ------------------ */
std::pair<nl::json,std::string> static_params(const pagmo::fair_replace& rp);
// Fair replace can not have dynamic parameters, so it is a compile error to call this function
nl::json dynamic_params(const pagmo::fair_replace& rp) = delete;

/*----------------------- Select best ------------------ */
// exactly like fair replace
std::pair<nl::json,std::string> static_params(const pagmo::select_best& sp);
// Select best can not have dynamic parameters, so it is a compile error to call this function
nl::json dynamic_params(const pagmo::select_best& sp) = delete;

/*----------------------- Unconnected topology ------------------ */
// Unconnected topology don't have any type of parameters (static nor dynamic)
std::pair<nl::json,std::string> static_params(const pagmo::unconnected& tp) = delete;
nl::json dynamic_params(const pagmo::unconnected& tp) = delete;

} // namespace detail

} // namespace json
} // namespace io
} // namespace bevarmejo
