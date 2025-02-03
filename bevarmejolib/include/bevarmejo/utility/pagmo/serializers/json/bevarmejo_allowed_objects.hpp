#ifndef BEVARMEJOLIB__PAGMO__SERIALIZERS__JSON__BEVARMEJO_ALLOWED_OBJECTS_HPP
#define BEVARMEJOLIB__PAGMO__SERIALIZERS__JSON__BEVARMEJO_ALLOWED_OBJECTS_HPP

#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <pagmo/algorithms/nsga2.hpp>

#include "bevarmejo/io/json.hpp"
#include "bevarmejo/io/aliased_key.hpp"
#include "bevarmejo/io/keys/bemeopt.hpp"
#include "bevarmejo/utility/exceptions.hpp"
#include "bevarmejo/utility/string.hpp"

namespace bevarmejo::io::key::detail
{
static const bevarmejo::io::AliasedKey cr{"Crossover probability", "cr"}; // "Crossover probability", "cr"
static const bevarmejo::io::AliasedKey eta_c{"Distribution index for crossover", "eta_c"}; // "Distribution index for crossover", "eta_c"
static const bevarmejo::io::AliasedKey m{"Mutation probability", "m"}; // "Mutation probability", "m"
static const bevarmejo::io::AliasedKey eta_m{"Distribution index for mutation", "eta_m"}; // "Distribution index for mutation", "eta_m"
static const bevarmejo::io::AliasedKey seed{"Seed"}; // "Seed"
static const bevarmejo::io::AliasedKey verbosity{"Verbosity"}; // "Verbosity"
} // namespace bevarmejo::io::key::detail

NLOHMANN_JSON_NAMESPACE_BEGIN
template <>
struct adl_serializer<pagmo::nsga2>
{

    static void to_json(Json &j, const pagmo::nsga2 &algo)
    {
        // Reset the json object, just in case.
        j = Json{};
   
        // Unfortuntaley, except for the verbosity, there are no methods to retrieve
        // the parameters of the algorithm. I need to parse them starting from the
        // extra info string.
        
        std::string extra_info = algo.get_extra_info();
        extra_info.erase(std::remove(extra_info.begin(), extra_info.end(), '\t'), extra_info.end());
        std::vector<std::string> key_value_strings = bevarmejo::split(extra_info, '\n');

        for (const auto& kv : key_value_strings)
        {
            // Extract the key (i.e., before the ':'), if no ':' is found, the key is the whole string
            const auto tokens = bevarmejo::split(kv, ':');
            const std::string& key = tokens[0];

            if (key == bevarmejo::io::key::detail::cr[0])
            {
                j[bevarmejo::io::key::detail::cr()] = std::stod(tokens[1]);
            }
            else if (key == bevarmejo::io::key::detail::eta_c[0])
            {
                j[bevarmejo::io::key::detail::eta_c()] = std::stod(tokens[1]);
            }
            else if (key == bevarmejo::io::key::detail::m[0])
            {
                j[bevarmejo::io::key::detail::m()] = std::stod(tokens[1]);
            }
            else if (key == bevarmejo::io::key::detail::eta_m[0])
            {
                j[bevarmejo::io::key::detail::eta_m()] = std::stod(tokens[1]);
            }
            else if (key == bevarmejo::io::key::detail::seed[0])
            {
                j[bevarmejo::io::key::detail::seed()] = std::stoull(tokens[1]); 
            }
            else if (key == bevarmejo::io::key::detail::verbosity[0])
            {
                j[bevarmejo::io::key::detail::verbosity()] = std::stoul(tokens[1]);
            }
            else if (key == bevarmejo::io::key::generations[0])
            {
                j[bevarmejo::io::key::generations()] = std::stoul(tokens[1]);
            }
            else
            {
                beme_throw(std::runtime_error,
                    "Cannot serialize the pagmo::nsga2",
                    "Unknown key is not supported.",
                    "AliasedKey : ", key);
            }
        }
    }

    static void from_json(const Json &j, pagmo::nsga2 &algo)
    {
        unsigned int gen = j.value(bevarmejo::io::key::generations.as_in(j), 1u);
        double cr = j.value(bevarmejo::io::key::detail::cr.as_in(j), 0.9);
        double eta_c = j.value(bevarmejo::io::key::detail::eta_c.as_in(j), 15.);
        double m = j.value(bevarmejo::io::key::detail::m.as_in(j), 1./34.);
        double eta_m = j.value(bevarmejo::io::key::detail::eta_m.as_in(j), 7.);
        unsigned int seed = j.value(bevarmejo::io::key::detail::seed.as_in(j), pagmo::random_device::next());
        unsigned int verb = j.value(bevarmejo::io::key::detail::verbosity.as_in(j), 0u);

        algo = pagmo::nsga2(gen, cr, eta_c, m, eta_m, seed);
        algo.set_verbosity(verb);
    }

};
NLOHMANN_JSON_NAMESPACE_END

#endif // BEVARMEJOLIB__PAGMO__SERIALIZERS__JSON__BEVARMEJO_ALLOWED_OBJECTS_HPP
