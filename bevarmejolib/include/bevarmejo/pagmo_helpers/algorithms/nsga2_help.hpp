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
namespace nl = nlohmann;

#include "bevarmejo/labels.hpp"
#include "bevarmejo/io/json_serializers.hpp"

namespace bevarmejo {
namespace nsga2 {
namespace defaults {

constexpr unsigned int gen = 1u;
constexpr double cr = 0.9;
constexpr double eta_c = 15.;
constexpr double m = 1./34.;
constexpr double eta_m = 7.;
// default for seed is random_device::next()
} // namespace defaults

namespace label {
const std::string __cr = "Crossover probability";
const std::string __eta_c = "Distribution index for crossover";
const std::string __m = "Mutation probability";
const std::string __eta_m = "Distribution index for mutation";
} // namespace label
} // namespace nsga2

inline pagmo::nsga2 Nsga2(nl::json settings) {
	unsigned int gen = settings.contains("Report gen") ? settings["Report gen"].get<unsigned int>() : nsga2::defaults::gen;
	double cr = settings.contains("cr") ? settings["cr"].get<double>() : nsga2::defaults::cr;
	double eta_c = settings.contains("eta_c") ? settings["eta_c"].get<double>() : nsga2::defaults::eta_c;
	double m = settings.contains("m") ? settings["m"].get<double>() : nsga2::defaults::m;
	double eta_m = settings.contains("eta_m") ? settings["eta_m"].get<double>() : nsga2::defaults::eta_m;

	if (settings.contains("Seed")) 
		return pagmo::nsga2(gen, cr, eta_c, m, eta_m, settings["Seed"].get<unsigned int>());

	// else leave the random deault seed
	return pagmo::nsga2(gen, cr, eta_c, m, eta_m);
}

namespace io {
namespace json {
namespace detail {

// Specializations for nsga2 class
inline std::pair<nl::json,std::string> static_params(const pagmo::nsga2& algo) {
    
    nl::json j;
   
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
        if (key == nsga2::label::__cr || 
            key == nsga2::label::__eta_c || 
            key == nsga2::label::__m || 
            key == nsga2::label::__eta_m) {
                assert(tokens.size() == 2); // quite sure I have a key and a value
                j[to_kebab_case(key)] = std::stod(tokens[1]);

        } else if (key == label::__seed) {
            assert(tokens.size() == 2);
            j[to_kebab_case(key)] = std::stoull(tokens[1]);

        } else {
            for (const auto& token : tokens) {
                extra_info += token;
                extra_info += ':';
            }
            extra_info.back() = '\n'; // also remove the last ':' added in the loop
        }
    }
    return std::make_pair( j, extra_info );
}

inline nl::json dynamic_params(const pagmo::nsga2& algo) = delete;
	
} // namespace detail
} // namespace json
} // namespace io
} // namespace bevarmejo

#endif /* BEVARMEJOLIB__NSGA2_HELPER_HPP */