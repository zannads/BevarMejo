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
#include "bevarmejo/pagmo_helpers/udc_help.hpp"

#include "Anytown/mixed/prob_at_mix_f1.hpp"
#include "Anytown/rehab/prob_at_reh_f1.hpp"
#include "Anytown/operations/prob_at_ope_f1.hpp"
#include "Anytown/twophases/prob_at_2ph_f1.hpp"

#include "Hanoi/problem_hanoi_biobj.hpp"

#include "containers_help.hpp"

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
// TODO: add the problem and see how to deal with my user defined problems

nl::json static_descr(const pagmo::problem& prob) {
    nl::json j = {
        {to_kebab_case(label::__name), prob.get_name()},
    };

    // based on the problem, I can call its own specific implementation
    if ( prob.is<bevarmejo::anytown::mixed::f1::Problem>() ) {
        auto [jparams, jextra] = detail::static_params<bevarmejo::anytown::mixed::f1::Problem>(*prob.extract<bevarmejo::anytown::mixed::f1::Problem>());
        j[to_kebab_case(label::__params)] = jparams;
        j[to_kebab_case(label::__extra_info)] = jextra;
    } else if ( prob.is<bevarmejo::anytown::rehab::f1::Problem>() ) {
        auto [jparams, jextra] = detail::static_params<bevarmejo::anytown::rehab::f1::Problem>(*prob.extract<bevarmejo::anytown::rehab::f1::Problem>());
        j[to_kebab_case(label::__params)] = jparams;
        j[to_kebab_case(label::__extra_info)] = jextra;
    } else if ( prob.is<bevarmejo::anytown::operations::f1::Problem>() ) {
        auto [jparams, jextra] = detail::static_params<bevarmejo::anytown::operations::f1::Problem>(*prob.extract<bevarmejo::anytown::operations::f1::Problem>());
        j[to_kebab_case(label::__params)] = jparams;
        j[to_kebab_case(label::__extra_info)] = jextra;
    } else if ( prob.is<bevarmejo::anytown::twophases::f1::Problem>() ) {
        auto [jparams, jextra] = detail::static_params<bevarmejo::anytown::twophases::f1::Problem>(*prob.extract<bevarmejo::anytown::twophases::f1::Problem>());
        j[to_kebab_case(label::__params)] = jparams;
        j[to_kebab_case(label::__extra_info)] = jextra;
    } else if ( prob.is<bevarmejo::hanoi::fbiobj::Problem>() ) {
        auto [jparams, jextra] = detail::static_params<bevarmejo::hanoi::fbiobj::Problem>(*prob.extract<bevarmejo::hanoi::fbiobj::Problem>());
        j[to_kebab_case(label::__params)] = jparams;
        j[to_kebab_case(label::__extra_info)] = jextra;
    } else {
        // Default implementation
        j[to_kebab_case(label::__extra_info)] = prob.get_extra_info();
    }

    return nl::json{ {to_kebab_case(label::__problem), j} };
}

nl::json dynamic_descr(const pagmo::problem& prob) {
    
    // Name is NOT a dynamic parameter, so it is not saved here, same for extra info

    nl::json j { };
    // Based on the problem, I can call its own specific implementation
    if ( prob.is<bevarmejo::anytown::twophases::f1::Problem>() ) { 
        j[to_kebab_case(label::__params)] = detail::dynamic_params<bevarmejo::anytown::twophases::f1::Problem>(*prob.extract<bevarmejo::anytown::twophases::f1::Problem>());
    }
    else {
        // Default implementation is empty
        return nl::json{};
    }

    // If I reach here, I have to return the dynamic parameters
    return nl::json{ {to_kebab_case(label::__problem), j} };
}

/*-------------------- Island --------------------*/
nl::json static_descr(const pagmo::island& isl) {
    nl::json j = {
        {to_kebab_case(label::__name), isl.get_name()},
    };

    if ( isl.is<pagmo::thread_island>() ) {
        auto [jparams, extra_str] = detail::static_params<pagmo::thread_island>(*isl.extract<pagmo::thread_island>());
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
        auto [jparams, extra_str] = detail::static_params<pagmo::fair_replace>(*rp.extract<pagmo::fair_replace>());
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
        auto [jparams, extra_str] = detail::static_params<pagmo::select_best>(*sp.extract<pagmo::select_best>());
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

} // namespace json

void inp::temp_net_to_file(const pagmo::problem& prob, const std::vector<double>& dv, const std::string& out_file) {
    if ( prob.is<bevarmejo::anytown::Problem>() ) {
        inp::detail::temp_net_to_file<bevarmejo::anytown::Problem>(*prob.extract<bevarmejo::anytown::Problem>(), dv, out_file);
    } else if ( prob.is<bevarmejo::hanoi::fbiobj::Problem>() ) {
        //inp::detail::temp_net_to_file<bevarmejo::hanoi::fbiobj::Problem>(*prob.extract<bevarmejo::hanoi::fbiobj::Problem>(), dv, out_file);
    } else {
        throw std::runtime_error("Problem type not supported for inp file generation");
    }
}

} // namespace io
} // namespace bevarmejo
