#ifndef BEVARMEJOLIB__PAGMO__SERIALIZERS__JSON__CONTAINERS_HPP
#define BEVARMEJOLIB__PAGMO__SERIALIZERS__JSON__CONTAINERS_HPP

#include <iostream>

#include <pagmo/algorithm.hpp>
#include <pagmo/island.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/r_policy.hpp>
#include <pagmo/s_policy.hpp>
#include <pagmo/topology.hpp>

#include <nlohmann/json.hpp>
using json_o = nlohmann::json;

#include "bevarmejo/utility/bemexcept.hpp"

#include "bevarmejo/utility/pagmo/serializers/json/bevarmejo_allowed_objects.hpp"
#include "bevarmejo/utility/pagmo/serializers/json/default_objects.hpp"

// Temp, then I will move the keys here!

#include "bevarmejo/io/keys/beme.hpp"

NLOHMANN_JSON_NAMESPACE_BEGIN
template <>
struct adl_serializer<pagmo::algorithm>
{

    static void to_json(json_o &j, const pagmo::algorithm &algo)
    {
        // Reset, just in case.
        j = json_o{};
    
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

    static void from_json(const json_o &j, pagmo::algorithm &algo)
    {
        // The key "type" is mandatory, describes the algorithm class to be used.
        // The key "params" is optional, contains the parameters for the algorithm.
        // If not present, an empty object is used so that the default parameters are used.
        beme_throw_if(!bevarmejo::io::key::type.exists_in(j),
            std::runtime_error,
            "Cannot build the pagmo::algorithm",
            "The mandatory key 'type' is missing.");
        
        auto algo_type = bevarmejo::io::json::extract(bevarmejo::io::key::type).from(j).get<std::string>();

        json_o algo_params{};
        if (bevarmejo::io::key::params.exists_in(j))
        {
            algo_params = bevarmejo::io::json::extract(bevarmejo::io::key::params).from(j);
        }

        // Based on the algo_type, I have to build the algorithm
        if (algo_type == "nsga2") // TODO: transform into a key
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

NLOHMANN_JSON_NAMESPACE_END

#endif // BEVARMEJOLIB__PAGMO__SERIALIZERS__JSON__CONTAINERS_HPP
