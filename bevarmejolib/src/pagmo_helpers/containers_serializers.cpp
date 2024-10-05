// Include the pagmo containers
#include <pagmo/algorithm.hpp>
#include <pagmo/island.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/r_policy.hpp>
#include <pagmo/s_policy.hpp>
#include <pagmo/topology.hpp>

// Include the stream out library
#include <nlohmann/json.hpp>
using json_o = nlohmann::json;

// Include the detailed implementation of each object 
#include "bevarmejo/pagmo_helpers/default_objects_serializers.hpp"
#include "bevarmejo/pagmo_helpers/algorithms/nsga2_help.hpp"
#include "Anytown/prob_anytown.hpp"
#include "Hanoi/problem_hanoi_biobj.hpp"

#include "containers_serializers.hpp"

namespace bevarmejo::io::json {

/*-------------------- Algorithm --------------------*/
json_o static_descr(const pagmo::algorithm& algo) {
    json_o j = {
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

    return json_o{ {to_kebab_case(label::__algorithm), j} };
}

json_o dynamic_descr(const pagmo::algorithm& algo) {
    return json_o{}; // No algorithm has dynamic parameters for now
}

/*-------------------- Problem --------------------*/

json_o static_descr(const pagmo::problem& prob) {
    json_o j = {
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
    
    return json_o{ {to_kebab_case(label::__problem), j} };
}

json_o dynamic_descr(const pagmo::problem& prob) {
    
    // Name is NOT a dynamic parameter, so it is not saved here, same for extra info
    
    json_o j { };
    
    // Based on the problem, I can call its own specific implementation
    if ( prob.is<bevarmejo::anytown::Problem>() ) { 
        json_o jdp = bevarmejo::anytown::io::json::detail::dynamic_params(*prob.extract<bevarmejo::anytown::Problem>());
        
        if (!jdp.empty()) j[to_kebab_case(label::__params)] = jdp;
    }
    
    if (j.empty()) 
        return j;

    // If I reach here, I have to return the dynamic parameters
    return json_o{ {to_kebab_case(label::__problem), j} };
}

/*-------------------- Island --------------------*/
json_o static_descr(const pagmo::island& isl) {
    json_o j = {
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

    return json_o{ {to_kebab_case(label::__island), j} };
}

/*-------------------- Replacement policy --------------------*/
json_o static_descr(const pagmo::r_policy& rp) {
    json_o j = {
        {to_kebab_case(label::__name), rp.get_name()}
    };

    if ( rp.is<pagmo::fair_replace>() ) {
        auto [jparams, extra_str] = detail::static_params(*rp.extract<pagmo::fair_replace>());
        j[to_kebab_case(label::__params)] = jparams;
        // j[to_kebab_case(label::__extra_info)] = extra_str; // I know it is empty
    } else {
        j[to_kebab_case(label::__extra_info)] = rp.get_extra_info();
    }

    return json_o{ {to_kebab_case(label::__r_policy), j} };
}

/*-------------------- Selection policy --------------------*/
json_o static_descr(const pagmo::s_policy& sp) {
    json_o j = {
        {to_kebab_case(label::__name), sp.get_name()}
    };

    if ( sp.is<pagmo::select_best>() ) {
        auto [jparams, extra_str] = detail::static_params(*sp.extract<pagmo::select_best>());
        j[to_kebab_case(label::__params)] = jparams;
        // j[[to_kebab_case(label::__extra_info)] = extra_str; // I know it is empty
    } else {
        j[to_kebab_case(label::__extra_info)] = sp.get_extra_info();
    }

    return json_o{ {to_kebab_case(label::__s_policy), j} };
}

/*-------------------- Topology --------------------*/
json_o static_descr(const pagmo::topology& tp) {
    json_o j = {
        {to_kebab_case(label::__name), tp.get_name()}
    };

    if ( tp.is<pagmo::unconnected>() ) {
        // I know it is empty, calling here would result in an error at compile time
    } else {
        j[to_kebab_case(label::__extra_info)] = tp.get_extra_info();
    }

    return json_o{ {to_kebab_case(label::__topology), j} };
}

}   // namespace bevarmejo::io::json