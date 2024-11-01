/*--------------------------------
Author: DennisZ
descr: A quick definition of functions to upload the data in the right way.
--------------------------------*/

#ifndef BEVARMEJOLIB__NSGA2_HELPER_HPP
#define BEVARMEJOLIB__NSGA2_HELPER_HPP

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
#include "bevarmejo/utility/string_manip.hpp"

namespace bevarmejo::nsga2::io::key {
static const bevarmejo::io::key::Key cr{"Crossover probability", "cr"}; // "Crossover probability", "cr"
static const bevarmejo::io::key::Key eta_c{"Distribution index for crossover", "eta_c"}; // "Distribution index for crossover", "eta_c"
static const bevarmejo::io::key::Key m{"Mutation probability", "m"}; // "Mutation probability", "m"
static const bevarmejo::io::key::Key eta_m{"Distribution index for mutation", "eta_m"}; // "Distribution index for mutation", "eta_m"
static const bevarmejo::io::key::Key seed{"Seed"}; // "Seed"
} // namespace bevarmejo::nsga2::io::key

namespace bevarmejo {

inline pagmo::nsga2 Nsga2(const json_o &settings) {

    // Default values
    unsigned int gen = 1u;
    double cr = 0.9;
    double eta_c = 15.;
    double m = 1./34.;
    double eta_m = 7.;
    unsigned int seed = pagmo::random_device::next();

    if (io::key::repgen.exists_in(settings))
        gen = io::json::extract(io::key::repgen).from(settings).get<unsigned int>();

    if (nsga2::io::key::cr.exists_in(settings))
        cr = io::json::extract(nsga2::io::key::cr).from(settings).get<double>();

    if (nsga2::io::key::eta_c.exists_in(settings))
        eta_c = io::json::extract(nsga2::io::key::eta_c).from(settings).get<double>();

    if (nsga2::io::key::m.exists_in(settings))
        m = io::json::extract(nsga2::io::key::m).from(settings).get<double>();

    if (nsga2::io::key::eta_m.exists_in(settings))
        eta_m = io::json::extract(nsga2::io::key::eta_m).from(settings).get<double>();

    if (nsga2::io::key::seed.exists_in(settings))
        seed = io::json::extract(nsga2::io::key::seed).from(settings).get<unsigned int>();

	return pagmo::nsga2(gen, cr, eta_c, m, eta_m, seed);
}

namespace io::json::detail {

// Specializations for nsga2 class
inline std::pair<json_o,std::string> static_params(const pagmo::nsga2& algo) {
    
    json_o j;
   
    // I known it returns some extra info and I know the type of these info,
    // this ways the specialization.
    std::string extra_info = algo.get_extra_info();
    extra_info.erase(std::remove(extra_info.begin(), extra_info.end(), '\t'), extra_info.end());
    std::vector<std::string> key_value_strings = bevarmejo::split(extra_info, '\n');
    extra_info.clear();

    for (const auto& kv : key_value_strings) {
        // If it contains one of the parameters I know, I add it to the json object, 
        // otherwise I add it to the extra info string.

        // Extract the key (i.e., before the ':'), if no ':' is found, the key is the whole string
        const auto tokens = split(kv, ':');
        const std::string& key = tokens[0];

        // If the key is in my list of convertible parameters I add it to the json object
        // otherwise repush all the tokens to the extra info string stream
        if (key == nsga2::io::key::cr[0]) 
            j[nsga2::io::key::cr()] = std::stod(tokens[1]); // quite sure I have a key and a value

        else if (key == nsga2::io::key::eta_c[0]) 
            j[nsga2::io::key::eta_c()] = std::stod(tokens[1]);

        else if (key == nsga2::io::key::m[0])
            j[nsga2::io::key::m()] = std::stod(tokens[1]);

        else if (key == nsga2::io::key::eta_m[0])
            j[nsga2::io::key::eta_m()] = std::stod(tokens[1]);

        else if (key == nsga2::io::key::seed[0])
            j[nsga2::io::key::seed()] = std::stoull(tokens[1]); 

        else {
            for (const auto& token : tokens) {
                extra_info += token;
                extra_info += ':';
            }
            extra_info.back() = '\n'; // also remove the last ':' added in the loop
        }
    }
    return std::make_pair( j, extra_info );
}

inline json_o dynamic_params(const pagmo::nsga2& algo) = delete;
	
} // namespace io::json::detail

} // namespace bevarmejo

#endif /* BEVARMEJOLIB__NSGA2_HELPER_HPP */
