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

#include "bevarmejo/utility/string_manip.hpp"

#include "bevarmejo/utility/pagmo/default_objects_serializers.hpp"

namespace bevarmejo::io::json::detail {

/*----------------------- Thread island ------------------ */
std::pair<json_o,std::string> static_params(const pagmo::thread_island& isl) {
    static const std::string threadisl__label__pool_flag = "Using pool";
    std::string extra_info = isl.get_extra_info(); // I know it returns "\tUsing pool: yes" or no
    extra_info.erase(std::remove(extra_info.begin(), extra_info.end(), '\t'), extra_info.end());
    auto tokens = bevarmejo::split(extra_info, ':');
    assert(tokens.size() == 2);

    json_o j;
    j[to_kebab_case(threadisl__label__pool_flag)] = tokens[1] == " yes" ? true : false; // space is there too

    return std::make_pair(j, std::string{});
}

/*----------------------- Fair replace ------------------ */
std::pair<json_o,std::string> static_params(const pagmo::fair_replace& rp) {
    // I know the extra info returns "\tAbsolute migration rate: 1" and the 
    // number is an integer or a "\tFractional migration rate: 0.1" and the
    // number is a double.
    std::string extra_info = rp.get_extra_info(); 
    extra_info.erase(std::remove(extra_info.begin(), extra_info.end(), '\t'), extra_info.end());
    auto tokens = bevarmejo::split(extra_info, ':');
    assert(tokens.size() == 2);

    json_o j;
    if (tokens[0] == "Absolute migration rate") {
        j[to_kebab_case(tokens[0])] = std::stoi(tokens[1]);
    } else /* if (tokens[0] == "Fractional migration rate") */ {
        j[to_kebab_case(tokens[0])] = std::stod(tokens[1]);
    }

    return std::make_pair(j, std::string{});
}

/*----------------------- Select best ------------------ */
// exactly like fair replace
std::pair<json_o,std::string> static_params(const pagmo::select_best& sp) {
    std::string extra_info = sp.get_extra_info(); 
    extra_info.erase(std::remove(extra_info.begin(), extra_info.end(), '\t'), extra_info.end());
    auto tokens = bevarmejo::split(extra_info, ':');
    assert(tokens.size() == 2);

    json_o j;
    if (tokens[0] == "Absolute migration rate") {
        j[to_kebab_case(tokens[0])] = std::stoi(tokens[1]);
    } else /* if (tokens[0] == "Fractional migration rate") */ {
        j[to_kebab_case(tokens[0])] = std::stod(tokens[1]);
    }

    return std::make_pair(j, std::string{});
}

} //  namespace bevarmejo::io::json::detail