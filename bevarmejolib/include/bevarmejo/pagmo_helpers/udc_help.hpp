#ifndef BEVARMEJOLIB__PAGMO_HELPERS__UDC_HELP_HPP
#define BEVARMEJOLIB__PAGMO_HELPERS__UDC_HELP_HPP

#include <sstream>
#include <string>
#include <utility>
#include <vector>

// When I will use this objects, they will have their own file. For now I need 
// them only to print it out so they can stay here together.
#include <pagmo/islands/thread_island.hpp>

#include <pagmo/r_policies/fair_replace.hpp>

#include <pagmo/s_policies/select_best.hpp>

#include <pagmo/topologies/unconnected.hpp>


#include <nlohmann/json.hpp>
namespace nl = nlohmann;

#include "bevarmejo/io.hpp"
#include "bevarmejo/labels.hpp"

namespace bevarmejo {
namespace io {
namespace json { 
namespace detail {
/*----------------------- Thread island ------------------ */
template <>
inline std::pair<nl::json,std::string> static_params_to_json<pagmo::thread_island>(const pagmo::thread_island& isl) {
    static const std::string threadisl__label__pool_flag = "Using pool";
    std::string extra_info = isl.get_extra_info(); // I know it returns "\tUsing pool: yes" or no
    extra_info.erase(std::remove(extra_info.begin(), extra_info.end(), '\t'), extra_info.end());
    auto tokens = bevarmejo::split(extra_info, ':');
    assert(tokens.size() == 2);

    nl::json j;
    j[to_kebab_case(threadisl__label__pool_flag)] = tokens[1] == " yes" ? true : false; // space is there too

    return std::make_pair(j, std::string{});
}

// Thread island can not have dynamic parameters, so it is a compile error to call this function
template <>
inline nl::json dynamic_params_to_json<pagmo::thread_island>(const pagmo::thread_island& isl) = delete;

/*----------------------- Fair replace ------------------ */
template <>
inline std::pair<nl::json,std::string> static_params_to_json<pagmo::fair_replace>(const pagmo::fair_replace& rp) {
    // I know the extra info returns "\tAbsolute migration rate: 1" and the 
    // number is an integer or a "\tFractional migration rate: 0.1" and the
    // number is a double.
    std::string extra_info = rp.get_extra_info(); 
    extra_info.erase(std::remove(extra_info.begin(), extra_info.end(), '\t'), extra_info.end());
    auto tokens = bevarmejo::split(extra_info, ':');
    assert(tokens.size() == 2);

    nl::json j;
    if (tokens[0] == "Absolute migration rate") {
        j[to_kebab_case(tokens[0])] = std::stoi(tokens[1]);
    } else /* if (tokens[0] == "Fractional migration rate") */ {
        j[to_kebab_case(tokens[0])] = std::stod(tokens[1]);
    }

    return std::make_pair(j, std::string{});
}

// Fair replace can not have dynamic parameters, so it is a compile error to call this function
template <>
inline nl::json dynamic_params_to_json<pagmo::fair_replace>(const pagmo::fair_replace& rp) = delete;

/*----------------------- Select best ------------------ */
// exactly like fair replace
template <>
inline std::pair<nl::json,std::string> static_params_to_json<pagmo::select_best>(const pagmo::select_best& sp) {
    std::string extra_info = sp.get_extra_info(); 
    extra_info.erase(std::remove(extra_info.begin(), extra_info.end(), '\t'), extra_info.end());
    auto tokens = bevarmejo::split(extra_info, ':');
    assert(tokens.size() == 2);

    nl::json j;
    if (tokens[0] == "Absolute migration rate") {
        j[to_kebab_case(tokens[0])] = std::stoi(tokens[1]);
    } else /* if (tokens[0] == "Fractional migration rate") */ {
        j[to_kebab_case(tokens[0])] = std::stod(tokens[1]);
    }

    return std::make_pair(j, std::string{});
}

// Select best can not have dynamic parameters, so it is a compile error to call this function
template <>
inline nl::json dynamic_params_to_json<pagmo::select_best>(const pagmo::select_best& sp) = delete;

/*----------------------- Unconnected topology ------------------ */
// Unconnected topology don't have any type of parameters (static nor dynamic)
template <>
inline std::pair<nl::json,std::string> static_params_to_json<pagmo::unconnected>(const pagmo::unconnected& tp) = delete;

template <>
inline nl::json dynamic_params_to_json<pagmo::unconnected>(const pagmo::unconnected& tp) = delete;

} // namespace detail
} // namespace json
} // namespace io
} // namespace bevarmejo

#endif // BEVARMEJOLIB__PAGMO_HELPERS__UDC_HELP_HPP
