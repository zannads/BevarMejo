#ifndef BEVARMEJOLIB__PAGMO_HELPERS__CONTAINERS_HELP_HPP
#define BEVARMEJOLIB__PAGMO_HELPERS__CONTAINERS_HELP_HPP

#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <pagmo/algorithm.hpp>
#include <pagmo/algorithms/nsga2.hpp>

#include <pagmo/island.hpp>
#include <pagmo/islands/thread_island.hpp>

#include <pagmo/problem.hpp>

#include <pagmo/r_policy.hpp>
#include <pagmo/r_policies/fair_replace.hpp>

#include <pagmo/s_policy.hpp>
#include <pagmo/s_policies/select_best.hpp>

#include <pagmo/topology.hpp>
#include <pagmo/topologies/unconnected.hpp>


#include <nlohmann/json.hpp>
namespace nl = nlohmann;

#include "bevarmejo/io.hpp"
#include "bevarmejo/labels.hpp"

#include "bevarmejo/pagmo_helpers/algorithms/nsga2_help.hpp"

namespace bevarmejo {
namespace label {
// A set of strings to which we compare against to see if the given pagmo element
// is the default one. E.g., the default topology of an archipelago is
// "Unconnected", if it is so, we won't save it to the settings file.
namespace pagmodefault {

const std::string __problem     = "Null problem";
const std::string __algorithm   = "Null algorithm";
const std::string __island      = "Thread island";
const std::string __r_policy    = "Fair replace";
const std::string __s_policy    = "Select best";
const std::string __bfe         = "Default batch fitness evaluator";
const std::string __topology    = "Unconnected";

} // namespace pagmodefault
} // namespace label

// Given a pagmo UD class of any type, usually you call get_extra_info() to get
// the settings of the class. However, this function returns a string.
// I want to return a json object for saving, and actually, I would need one for 
// static parameters and one for dynamic parameters. For now only static.

namespace io {
namespace json { 
// ---------------------------------------------------------------------
// Specializations for the specific pagmo containers
// ---------------------------------------------------------------------

/*-------------------- Algorithm --------------------*/
template <>
inline nl::json static_part_to_json<pagmo::algorithm>(const pagmo::algorithm& algo) {
    nl::json j = {
        {to_kebab_case(label::__name), algo.get_name()},
    };

    // based on algorithm name, I can call the specific function to convert the extra info
    if ( algo.is<pagmo::nsga2>()  ) { // Equivalent to algo.get_name() == pagmo::nsga2().get_name()
        
        auto [jparams, jextra] = detail::static_params_to_json<pagmo::nsga2>(*algo.extract<pagmo::nsga2>());
        j[to_kebab_case(label::__params)] = jparams;
        j[to_kebab_case(label::__extra_info)] = jextra;
    } else {
        // Default implementation
        j[to_kebab_case(label::__extra_info)] = algo.get_extra_info();
    }

    return nl::json{ {to_kebab_case(label::__algorithm), j} };
}

template <>
inline nl::json dynamic_part_to_json<pagmo::algorithm>(const pagmo::algorithm& algo) {
    return nl::json{}; // No algorithm has dynamic parameters for now
}

/*-------------------- Problem --------------------*/
// TODO: add the problem and see how to deal with my user defined problems

/*-------------------- Island --------------------*/
template <>
inline nl::json static_part_to_json<pagmo::island>(const pagmo::island& isl) {
    nl::json j = {
        {to_kebab_case(label::__name), isl.get_name()},
    };

    if ( isl.is<pagmo::thread_island>() ) {
        auto [jparams, extra_str] = detail::static_params_to_json<pagmo::thread_island>(*isl.extract<pagmo::thread_island>());
        j[to_kebab_case(label::__params)] = jparams;
        // j[to_kebab_case(label::__extra_info)] = extra_str; // I know it is empty
    } else {
        j[to_kebab_case(label::__extra_info)] = isl.get_extra_info();
    }

    // Islands have a special key, i.e., the population seed that is instantied
    // in the island. To not have a key for the population, and the seed is  
    // specific for an islands, make sense to add it here.
    j[to_kebab_case(label::__pop_seed)] = isl.get_population().get_seed();

    return nl::json{ {to_kebab_case(label::__island), j} };
}

// Islands can not have dynamic parameters, so it is a compile error to call this function
template <>
inline nl::json dynamic_part_to_json<pagmo::island>(const pagmo::island& isl) = delete;

/*-------------------- Replacement policy --------------------*/
template <>
inline nl::json static_part_to_json<pagmo::r_policy>(const pagmo::r_policy& rp) {
    nl::json j = {
        {to_kebab_case(label::__name), rp.get_name()}
    };

    if ( rp.is<pagmo::fair_replace>() ) {
        auto [jparams, extra_str] = detail::static_params_to_json<pagmo::fair_replace>(*rp.extract<pagmo::fair_replace>());
        j[to_kebab_case(label::__params)] = jparams;
        // j[to_kebab_case(label::__extra_info)] = extra_str; // I know it is empty
    } else {
        j[to_kebab_case(label::__extra_info)] = rp.get_extra_info();
    }

    return nl::json{ {to_kebab_case(label::__r_policy), j} };
}

template <>
inline nl::json dynamic_part_to_json<pagmo::r_policy>(const pagmo::r_policy& rp) = delete;

/*-------------------- Selection policy --------------------*/
template <>
inline nl::json static_part_to_json<pagmo::s_policy>(const pagmo::s_policy& sp) {
    nl::json j = {
        {to_kebab_case(label::__name), sp.get_name()}
    };

    if ( sp.is<pagmo::select_best>() ) {
        auto [jparams, extra_str] = detail::static_params_to_json<pagmo::select_best>(*sp.extract<pagmo::select_best>());
        j[to_kebab_case(label::__params)] = jparams;
        // j[[to_kebab_case(label::__extra_info)] = extra_str; // I know it is empty
    } else {
        j[to_kebab_case(label::__extra_info)] = sp.get_extra_info();
    }

    return nl::json{ {to_kebab_case(label::__s_policy), j} };
}

template <>
inline nl::json dynamic_part_to_json<pagmo::s_policy>(const pagmo::s_policy& sp) = delete;

/*-------------------- Topology --------------------*/
template <>
inline nl::json static_part_to_json<pagmo::topology>(const pagmo::topology& tp) {
    nl::json j = {
        {to_kebab_case(label::__name), tp.get_name()}
    };

    if ( tp.is<pagmo::unconnected>() ) {
        // I know it is empty, calling here would result in an error at compile time
    } else {
        j[to_kebab_case(label::__extra_info)] = tp.get_extra_info();
    }

    return nl::json{ {to_kebab_case(label::__topology), j} };
}

template <>
inline nl::json dynamic_part_to_json<pagmo::topology>(const pagmo::topology& tp) = delete;

} // namespace json
} // namespace io
} // namespace bevarmejo

#endif // BEVARMEJOLIB__PAGMO_HELPERS__CONTAINERS_HELP_HPP
