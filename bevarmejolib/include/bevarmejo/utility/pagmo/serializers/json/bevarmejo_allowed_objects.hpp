#ifndef BEVARMEJOLIB__PAGMO__SERIALIZERS__JSON__BEVARMEJO_ALLOWED_OBJECTS_HPP
#define BEVARMEJOLIB__PAGMO__SERIALIZERS__JSON__BEVARMEJO_ALLOWED_OBJECTS_HPP

#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <pagmo/algorithms/nsga2.hpp>

#include <nlohmann/json.hpp>
using json_o = nlohmann::json;

#include "bevarmejo/io/key.hpp"
#include "bevarmejo/io/keys/bemeopt.hpp"
#include "bevarmejo/utility/bemexcept.hpp"
#include "bevarmejo/utility/string_manip.hpp"

namespace bevarmejo::io::key::detail
{
static const bevarmejo::io::key::Key cr{"Crossover probability", "cr"}; // "Crossover probability", "cr"
static const bevarmejo::io::key::Key eta_c{"Distribution index for crossover", "eta_c"}; // "Distribution index for crossover", "eta_c"
static const bevarmejo::io::key::Key m{"Mutation probability", "m"}; // "Mutation probability", "m"
static const bevarmejo::io::key::Key eta_m{"Distribution index for mutation", "eta_m"}; // "Distribution index for mutation", "eta_m"
static const bevarmejo::io::key::Key seed{"Seed"}; // "Seed"
static const bevarmejo::io::key::Key verbosity{"Verbosity"}; // "Verbosity"
} // namespace bevarmejo::io::key::detail

NLOHMANN_JSON_NAMESPACE_BEGIN
template <>
struct adl_serializer<pagmo::nsga2>
{

    static void to_json(json_o &j, const pagmo::nsga2 &algo)
    {
        // Reset the json object, just in case.
        j = json_o{};
   
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
                    "Key : ", key);
            }
        }
    }

    static void from_json(const json_o &j, pagmo::nsga2 &algo)
    {
        unsigned int gen = 1u;
        double cr = 0.9;
        double eta_c = 15.;
        double m = 1./34.;
        double eta_m = 7.;
        unsigned int seed = pagmo::random_device::next();
        unsigned int verb = 0;

        if (bevarmejo::io::key::generations.exists_in(j))
            gen = bevarmejo::io::json::extract(bevarmejo::io::key::generations).from(j).get<unsigned int>();

        if (bevarmejo::io::key::detail::cr.exists_in(j))
            cr = bevarmejo::io::json::extract(bevarmejo::io::key::detail::cr).from(j).get<double>();

        if (bevarmejo::io::key::detail::eta_c.exists_in(j))
            eta_c = bevarmejo::io::json::extract(bevarmejo::io::key::detail::eta_c).from(j).get<double>();

        if (bevarmejo::io::key::detail::m.exists_in(j))
            m = bevarmejo::io::json::extract(bevarmejo::io::key::detail::m).from(j).get<double>();

        if (bevarmejo::io::key::detail::eta_m.exists_in(j))
            eta_m = bevarmejo::io::json::extract(bevarmejo::io::key::detail::eta_m).from(j).get<double>();

        if (bevarmejo::io::key::detail::seed.exists_in(j))
            seed = bevarmejo::io::json::extract(bevarmejo::io::key::detail::seed).from(j).get<unsigned int>();

        algo = pagmo::nsga2(gen, cr, eta_c, m, eta_m, seed);

        if (bevarmejo::io::key::detail::verbosity.exists_in(j))
            verb = bevarmejo::io::json::extract(bevarmejo::io::key::detail::verbosity).from(j).get<unsigned int>();

        algo.set_verbosity(verb);
    }

};
NLOHMANN_JSON_NAMESPACE_END

#endif // BEVARMEJOLIB__PAGMO__SERIALIZERS__JSON__BEVARMEJO_ALLOWED_OBJECTS_HPP
