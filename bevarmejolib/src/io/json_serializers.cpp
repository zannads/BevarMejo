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

#include "bevarmejo/pagmo_helpers/algorithms/nsga2_help.hpp"
#include "Anytown/prob_anytown.hpp"
#include "Hanoi/problem_hanoi_biobj.hpp"

#include "json_serializers.hpp"

namespace bevarmejo {
namespace io {
namespace json {
/*-------------------- Algorithm --------------------*/
nl::json static_descr(const pagmo::algorithm& algo) {
    nl::json j = {
        {to_kebab_case(label::__name), algo.get_name()},
    };

    // based on algorithm name, I can call the specific function to convert the extra info
    if ( algo.is<pagmo::nsga2>()  ) { // Equivalent to algo.get_name() == pagmo::nsga2().get_name()
        
        auto [jparams, jextra] = detail::static_params(*algo.extract<pagmo::nsga2>());
        j[to_kebab_case(label::__params)] = jparams;
        j[to_kebab_case(label::__extra_info)] = jextra;
    } else {
        // Default implementation
        j[to_kebab_case(label::__extra_info)] = algo.get_extra_info();
    }

    return nl::json{ {to_kebab_case(label::__algorithm), j} };
}

nl::json dynamic_descr(const pagmo::algorithm& algo) {
    return nl::json{}; // No algorithm has dynamic parameters for now
}

/*-------------------- Problem --------------------*/

nl::json static_descr(const pagmo::problem& prob) {
    nl::json j = {
        {to_kebab_case(label::__name), prob.get_name()},
    };

    // based on the problem, I can call its own specific implementation
    
    if ( prob.is<bevarmejo::anytown::Problem>() ) {
        auto [jparams, jextra] = bevarmejo::anytown::io::json::detail::static_params(*prob.extract<bevarmejo::anytown::Problem>());
        j[to_kebab_case(label::__params)] = jparams;
        j[to_kebab_case(label::__extra_info)] = jextra;
    } /*else if ( prob.is<bevarmejo::hanoi::fbiobj::Problem>() ) {
        auto [jparams, jextra] = bevarmejo::hanoi::fbiobj::io::json::detail::static_params(*prob.extract<bevarmejo::hanoi::fbiobj::Problem>());
        j[to_kebab_case(label::__params)] = jparams;
        j[to_kebab_case(label::__extra_info)] = jextra;
    }*/ else {
        // Default implementation
        j[to_kebab_case(label::__extra_info)] = prob.get_extra_info();
    }
    
    return nl::json{ {to_kebab_case(label::__problem), j} };
}

nl::json dynamic_descr(const pagmo::problem& prob) {
    
    // Name is NOT a dynamic parameter, so it is not saved here, same for extra info
    
    nl::json j { };
    
    // Based on the problem, I can call its own specific implementation
    if ( prob.is<bevarmejo::anytown::Problem>() ) { 
        nl::json jdp = bevarmejo::anytown::io::json::detail::dynamic_params(*prob.extract<bevarmejo::anytown::Problem>());
        
        if (!jdp.empty()) j[to_kebab_case(label::__params)] = jdp;
    }
    
    if (j.empty()) 
        return j;

    // If I reach here, I have to return the dynamic parameters
    return nl::json{ {to_kebab_case(label::__problem), j} };
}

/*-------------------- Island --------------------*/
nl::json static_descr(const pagmo::island& isl) {
    nl::json j = {
        {to_kebab_case(label::__name), isl.get_name()},
    };

    if ( isl.is<pagmo::thread_island>() ) {
        auto [jparams, extra_str] = detail::static_params(*isl.extract<pagmo::thread_island>());
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

/*-------------------- Replacement policy --------------------*/
nl::json static_descr(const pagmo::r_policy& rp) {
    nl::json j = {
        {to_kebab_case(label::__name), rp.get_name()}
    };

    if ( rp.is<pagmo::fair_replace>() ) {
        auto [jparams, extra_str] = detail::static_params(*rp.extract<pagmo::fair_replace>());
        j[to_kebab_case(label::__params)] = jparams;
        // j[to_kebab_case(label::__extra_info)] = extra_str; // I know it is empty
    } else {
        j[to_kebab_case(label::__extra_info)] = rp.get_extra_info();
    }

    return nl::json{ {to_kebab_case(label::__r_policy), j} };
}

/*-------------------- Selection policy --------------------*/
nl::json static_descr(const pagmo::s_policy& sp) {
    nl::json j = {
        {to_kebab_case(label::__name), sp.get_name()}
    };

    if ( sp.is<pagmo::select_best>() ) {
        auto [jparams, extra_str] = detail::static_params(*sp.extract<pagmo::select_best>());
        j[to_kebab_case(label::__params)] = jparams;
        // j[[to_kebab_case(label::__extra_info)] = extra_str; // I know it is empty
    } else {
        j[to_kebab_case(label::__extra_info)] = sp.get_extra_info();
    }

    return nl::json{ {to_kebab_case(label::__s_policy), j} };
}

/*-------------------- Topology --------------------*/
nl::json static_descr(const pagmo::topology& tp) {
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

namespace detail {

/*----------------------- Thread island ------------------ */
std::pair<nl::json,std::string> static_params(const pagmo::thread_island& isl) {
    static const std::string threadisl__label__pool_flag = "Using pool";
    std::string extra_info = isl.get_extra_info(); // I know it returns "\tUsing pool: yes" or no
    extra_info.erase(std::remove(extra_info.begin(), extra_info.end(), '\t'), extra_info.end());
    auto tokens = bevarmejo::split(extra_info, ':');
    assert(tokens.size() == 2);

    nl::json j;
    j[to_kebab_case(threadisl__label__pool_flag)] = tokens[1] == " yes" ? true : false; // space is there too

    return std::make_pair(j, std::string{});
}

/*----------------------- Fair replace ------------------ */
std::pair<nl::json,std::string> static_params(const pagmo::fair_replace& rp) {
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

/*----------------------- Select best ------------------ */
// exactly like fair replace
std::pair<nl::json,std::string> static_params(const pagmo::select_best& sp) {
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

} // namespace detail

} // namespace json
} // namespace io
} // namespace bevarmejo
