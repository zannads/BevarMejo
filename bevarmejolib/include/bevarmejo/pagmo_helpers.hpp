#ifndef BEVARMEJOLIB__PAGMO_HELPERS_HPP
#define BEVARMEJOLIB__PAGMO_HELPERS_HPP

#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <pagmo/algorithm.hpp>
#include <pagmo/algorithms/nsga2.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "bevarmejo/io.hpp"

namespace bevarmejo {
namespace label {
// A set of strings to which we compare against to see if the given pagmo element
// is the default one. E.g., the default topology of an archipelago is
// "Unconnected", if it is so, we won't save it to the settings file.
namespace pagmodefault {

const std::string __problem     = "Null problem";
const std::string __algorithm   = "Null algorithm";
const std::string __island      = "Thread island";
const std::string __r_policy    = "Fair replace";
const std::string __s_policy    = "Select best";
const std::string __bfe         = "Default batch fitness evaluator";
const std::string __topology    = "Unconnected";

} // namespace pagmodefault
} // namespace label

// Given a pagmo UD class of any type, usually you call get_extra_info() to get
// the settings of the class. However, this function returns a string.
// I want to return a json object for saving, and actually, I would need one for 
// static parameters and one for dynamic parameters. For now only static.

namespace reporting {

namespace pagmodetails { 
// Return two json objects, first the static parameters and second for the extra 
// info. 
template <typename T>
inline std::pair<json,std::string> static_params_to_json(const T& udc) {
    return std::make_pair(json{}, udc.get_extra_info());
}

template <typename T>
inline json dynamic_params_to_json(const T& udc) {
    return json{};
}

// Specializations for nsga2 class
template <>
inline std::pair<json,std::string> static_params_to_json<pagmo::nsga2>(const pagmo::nsga2& algo) {
    
    json j;
   
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
        if (key == "Crossover probability" || 
            key == "Distribution index for crossover" || 
            key == "Distribution index for mutation" || 
            key == "Mutation probability") {
                assert(tokens.size() == 2); // quite sure I have a key and a value
                j[key] = std::stod(tokens[1]);

        } else if (key == "Seed") {
            assert(tokens.size() == 2);
            j[key] = std::stoi(tokens[1]);

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

template <>
inline json dynamic_params_to_json<pagmo::nsga2>(const pagmo::nsga2& algo) {
    return json{};
}
} // namespace pagmodetails

// Given any pagmo container (island, algorithm, etc) return a json object with
// name of the UD class, Parameters for input to the UD class, and extra info.
// Two formulations are available, one for static parameters and one for dynamic.
// The dynamic parameters are the ones that change at runtime, the static ones are
// set at the beginning and don't change.
template <typename T>
json static_part_to_json(const T& udc) {
    json j;
    j[label::__name] = udc.get_name();
    if (!udc.get_extra_info().empty()) {
        auto extra_info = udc.get_extra_info();
        j[label::__extra_info] = extra_info.erase(std::remove(extra_info.begin(), extra_info.end(), '\t'), extra_info.end());
    }

    return j;
}

template <typename T>
json dynamic_part_to_json(const T& udc) {
    return json{ }; // Default implementation, no dynamic parameters
}

// specializations for the specific pagmo containers

/*--- Algorithm ---*/
template <>
inline json static_part_to_json<pagmo::algorithm>(const pagmo::algorithm& algo) {
    json j;
    j[label::__algorithm] = {
        {label::__name, algo.get_name()},
    };

    // based on algorithm name, I can call the specific function to convert the extra info
    if ( algo.get_name() == pagmo::nsga2().get_name() ) {
        
        auto [jparams, jextra] = pagmodetails::static_params_to_json<pagmo::nsga2>(*algo.extract<pagmo::nsga2>());
        j[label::__algorithm][label::__params] = jparams;
        j[label::__algorithm][label::__extra_info] = jextra;
    } else {
        // Default implementation
        j[label::__algorithm][label::__extra_info] = algo.get_extra_info();
    }

    return j;
}

template <>
inline json dynamic_part_to_json<pagmo::algorithm>(const pagmo::algorithm& algo) {
    return json{}; // No algorithm has dynamic parameters for now
}


} // namespace reporting
} // namespace bevarmejo

#endif // BEVARMEJOLIB__PAGMO_HELPERS_HPP
