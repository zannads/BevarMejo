#ifndef BEVARMEJOLIB__PAGMO__SERIALIZERS__JSON__CONTAINERS_HPP
#define BEVARMEJOLIB__PAGMO__SERIALIZERS__JSON__CONTAINERS_HPP

#include <iostream>

#include <pagmo/algorithm.hpp>
#include <pagmo/island.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/r_policy.hpp>
#include <pagmo/s_policy.hpp>
#include <pagmo/topology.hpp>

#include "bevarmejo/io/json.hpp"
#include "bevarmejo/io/keys/beme.hpp"
#include "bevarmejo/utility/exceptions.hpp"

// Pagmo objects that can be serialized
#include "bevarmejo/utility/pagmo/serializers/json/bevarmejo_allowed_objects.hpp"
#include "bevarmejo/utility/pagmo/serializers/json/default_objects.hpp"

// Bevarmejo objects that can be serialized
#include "bevarmejo/problems/anytown.hpp"
#include "bevarmejo/problems/hanoi.hpp"

NLOHMANN_JSON_NAMESPACE_BEGIN

/*-------------------------------- Algorithm ---------------------------------*/
template <>
struct adl_serializer<pagmo::algorithm>
{
    static void to_json(Json &j, const pagmo::algorithm &algo)
    {
        // Reset, just in case.
        j = Json{};
    
        // based on algorithm name, I can call the specific function to convert the extra info
        if ( algo.is<pagmo::nsga2>()  )
        {
            j[bevarmejo::io::key::type()] = "pagmo::nsga2";
            j[bevarmejo::io::key::params()] = *algo.extract<pagmo::nsga2>();
        }
        else
        {
            beme_throw(std::runtime_error,
                "Cannot serialize the pagmo::algorithm",
                "The algorithm is not supported.",
                "Algorithm name : ", algo.get_name());
        }
    }

    static void from_json(const Json &j, pagmo::algorithm &algo)
    {
        // The key "type" is mandatory, describes the algorithm class to be used.
        // The key "params" is optional, contains the parameters for the algorithm.
        // If not present, an empty object is used so that the default parameters are used.
        beme_throw_if(!bevarmejo::io::key::type.exists_in(j),
            std::runtime_error,
            "Cannot build the pagmo::algorithm",
            "The mandatory key 'type' is missing.");
        
        auto algo_type = j.at(bevarmejo::io::key::type.as_in(j)).get<std::string>();

        auto algo_params = j.value(bevarmejo::io::key::params.as_in(j), Json{});

        // Based on the algo_type, I have to build the algorithm
#if BEME_VERSION < 240601
        if (algo_type == "nsga2") // TODO: transform into a key
#else
        if (algo_type == "pagmo::nsga2")
#endif
        {
            algo = algo_params.get<pagmo::nsga2>();
        }
        else
        {
            beme_throw(std::runtime_error,
                "Cannot build the pagmo::algorithm",
                "The algorithm type is not supported.",
                "Algorithm type : ", algo_type);
        }
    }
};

/*-------------------------------- Island ------------------------------------*/
template <>
struct adl_serializer<pagmo::island>
{
    static void to_json(Json &j, const pagmo::island &isl)
    {
        // Reset, just in case.
        j = Json{};

        // based on the island name, call its serializer
        if ( isl.is<pagmo::thread_island>() )
        {
            j[bevarmejo::io::key::type()] = "pagmo::thread_island";
            j[bevarmejo::io::key::params()] = *isl.extract<pagmo::thread_island>();
        }
        else
        {
            beme_throw(std::runtime_error,
                "Cannot serialize the pagmo::island",
                "The island is not supported.",
                "Island name : ", isl.get_name());
        }

        // Islands have a special key, i.e., the population seed that is instantied
        // in the island. To not have a key for the population, and the seed is  
        // specific for an islands, make sense to add it here.
        j[bevarmejo::io::key::seed()] = isl.get_population().get_seed();
    }

    static void from_json(const Json &j, pagmo::island &isl)
    {
        beme_throw(std::runtime_error,
            "Cannot build the pagmo::island",
            "The island is not supported.");   
    }
};

/*-------------------------------- Problem -----------------------------------*/
template <>
struct adl_serializer<pagmo::problem>
{
    static void to_json(Json &j, const pagmo::problem &prob)
    {
        // Reset, just in case.
        j = Json{};

        // based on the problem name, call its serializer
        if ( prob.is<bevarmejo::hanoi::fbiobj::Problem>() )
        {
            std::cout << "Problem formulation : hanoi::fbiobj not yet implemented" << std::endl;
        }
        else if ( prob.is<bevarmejo::anytown::Problem>() )
        {
            j[bevarmejo::io::key::type()] = prob.get_name();
            j[bevarmejo::io::key::params()] = *prob.extract<bevarmejo::anytown::Problem>();
        }
        else
        {
            beme_throw(std::runtime_error,
                "Cannot serialize the pagmo::problem",
                "The problem is not supported.",
                "Problem name : ", prob.get_name());
        }
    }

    static void from_json(const Json &j, pagmo::problem &prob)
    {
        // The key "type" is mandatory, describes the problem class to be used.
        // The key "params" is optional, contains the parameters for the problem.
        // If not present, an empty object is used so that the default parameters are used.
        // The key "lookup_paths" is optional, contains the paths to lookup for the files used inside the bevarmejo problems. it will be removed later.

        beme_throw_if(!bevarmejo::io::key::type.exists_in(j),
            std::runtime_error,
            "Cannot build the pagmo::problem",
            "The mandatory key 'type' is missing.");

        auto prob_type = j.at(bevarmejo::io::key::type.as_in(j)).get<std::string>();
        
        auto prob_params = j.value(bevarmejo::io::key::params.as_in(j), Json{});

        auto lookup_paths = j.value(bevarmejo::io::key::lookup_paths.as_in(j), std::vector<std::filesystem::path>{});

        // Based on the prob_type, I have to build the problem
        if (prob_type == "bevarmejo::hanoi::fbiobj")
        {
            prob = pagmo::problem{ bevarmejo::hanoi::fbiobj::Problem(prob_params, lookup_paths) };
        }
        /* if starts with bevarmejo::anytown, pass also a string view starting from after the second::*/ 
        else if (prob_type.find("bevarmejo::anytown") == 0)
        {
            // bevarmejo::anytown:: has 20 characters
            prob = pagmo::problem{ bevarmejo::anytown::Problem(std::string_view(prob_type).substr(20), prob_params, lookup_paths) };
        }
        else
        {
            beme_throw(std::runtime_error,
                "Cannot build the pagmo::problem",
                "The problem type is not supported.",
                "Problem type : ", prob_type);
        }
    }
};

/*---------------------------- Replacement policy ----------------------------*/
template <>
struct adl_serializer<pagmo::r_policy>
{
    static void to_json(Json &j, const pagmo::r_policy &rp)
    {
       // Reset, just in case.
       j = Json{};

        // based on the r_policy name, call its serializer.
        if ( rp.is<pagmo::fair_replace>() )
        {
            j[bevarmejo::io::key::type()] = "pagmo::fair_replace";
            j[bevarmejo::io::key::params()] = *rp.extract<pagmo::fair_replace>();
        }
        else
        {
            beme_throw(std::runtime_error,
                "Cannot serialize the pagmo::r_policy",
                "The r_policy is not supported.",
                "r_policy name : ", rp.get_name());
        }
    }

    static void from_json(const Json &j, pagmo::r_policy &rp)
    {
        beme_throw(std::runtime_error,
            "Cannot build the pagmo::r_policy",
            "The r_policy is not supported.");
    }
};

/*---------------------------- Selection policy -----------------------------*/
template <>
struct adl_serializer<pagmo::s_policy>
{
    static void to_json(Json &j, const pagmo::s_policy &sp)
    {
        // Reset, just in case.
        j = Json{};

        // based on the s_policy name, call its serializer.
        if ( sp.is<pagmo::select_best>() )
        {
            j[bevarmejo::io::key::type()] = "pagmo::select_best";
            j[bevarmejo::io::key::params()] = *sp.extract<pagmo::select_best>();
        }
        else
        {
            beme_throw(std::runtime_error,
                "Cannot serialize the pagmo::s_policy",
                "The s_policy is not supported.",
                "s_policy name : ", sp.get_name());
        }
    }

    static void from_json(const Json &j, pagmo::s_policy &sp)
    {
        beme_throw(std::runtime_error,
            "Cannot build the pagmo::s_policy",
            "The s_policy is not supported.");
    }
};

/*-------------------------------- Topology ----------------------------------*/
template <>
struct adl_serializer<pagmo::topology>
{
    static void to_json(Json &j, const pagmo::topology &tp)
    {
        // Reset, just in case.
        j = Json{};

        // based on the topology name, call its serializer.
        if ( tp.is<pagmo::unconnected>() )
        {
            j[bevarmejo::io::key::type()] = "pagmo::unconnected";
        }
        else
        {
            beme_throw(std::runtime_error,
                "Cannot serialize the pagmo::topology",
                "The topology is not supported.",
                "Topology name : ", tp.get_name());
        }
    
    }

    static void from_json(const Json &j, pagmo::topology &tp)
    {
        beme_throw(std::runtime_error,
            "Cannot build the pagmo::topology",
            "The topology is not supported.");
    }
};


NLOHMANN_JSON_NAMESPACE_END

#endif // BEVARMEJOLIB__PAGMO__SERIALIZERS__JSON__CONTAINERS_HPP
