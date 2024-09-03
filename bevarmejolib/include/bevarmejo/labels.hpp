#ifndef BEVARMEJOLIB__LABELS_HPP
#define BEVARMEJOLIB__LABELS_HPP

#include <string>

namespace bevarmejo {
namespace label {

// Labels for the configuration and output files. 
// These are written in normal text and I will provide some functions to convert
// them in other formats (e.g., camel case, snake case, etc.)

// Ouput main file 
//--- Part 0 filename prefix and suffix ---
// Initial part of the main filename for the experiment outcome
const std::string __beme_prefix = "bemeopt__";
// Final part of the main filename for the experiment outcome
const std::string __beme_suffix = "__exp.json";

//--- Part 1 parameters and flags at the experiment or archipelago level ---
const std::string __archi = "Archipelago";
const std::string __topology = "Topology"; // Every user defined class has the follwing labels
const std::string __name = "Name"; // Name of the class
const std::string __params = "Parameters"; // To list internal parameters of the class (e.g., mutation rate)
const std::string __extra_info = "Extra info"; // To print extra info of the class (e.g., string with the formulation of the problem)
const std::string __islands = "Islands"; // To list the filenames of the single islands
const std::string __errors = "Errors";
const std::string __system = "System";
const std::string __software = "Software";
const std::string __beme_version = "Bemelib version";
//const std::string __libraries = "Libraries"; //TODO: implement this

// Output island file 
// the island final file and the runtime are the same without, bnut the latter is without the the static part
// const std::string __beme_prefix = "bemeopt_";
// const std::string __beme_suffix = suffix is actually dynamic and can change
//--- Part 1 Static parameters of an islands (constant through the evolution) ---
const std::string __algorithm = "Algorithm"; // see pattern for topology
// const std::string __name = "Name";
// const std::string __params = "Parameters";
// const std::string __extra_info = "Extra info";
const std::string __island = "Island";
// const std::string __name = "Name";
// const std::string __params = "Parameters";
// const std::string __extra_info = "Extra info";
const std::string __problem = "Problem";
// const std::string __name = "Name";
// const std::string __params = "Parameters";
// const std::string __extra_info = "Extra info";
const std::string __r_policy = "Replacement policy";
// const std::string __name = "Name";
// const std::string __params = "Parameters";
// const std::string __extra_info = "Extra info";
const std::string __s_policy = "Selection policy";
// const std::string __name = "Name";
// const std::string __params = "Parameters";
// const std::string __extra_info = "Extra info";

//--- Part 2 Dynamic parameters of an islands (change through the evolution) ---
const std::string __generations = "Generations";
const std::string __currtime = "Current time";

const std::string __pop_seed = "Population seed";
const std::string __fevals = "Fitness evaluations";
const std::string __gevals = "Gradient evaluations";
const std::string __hevals = "Hessian evaluations";
const std::string __individuals = "Individuals";
const std::string __id = "ID";
const std::string __dv = "Decision vector";
const std::string __fv = "Fitness vector";

// Input file (commented are already defined for the output file)
//--- Part 1 parameters and flags at the experiment or archipelago level ---
const std::string __exp_name = "Experiment name";
const std::string __exp_name_sh = "Exp name";
const std::string __paths = "Lookup paths";
const std::string __paths_sh = "Paths";

/* pattern for pagmo user defined classes */
// const std::string __topology = "Topology";
const std::string __topology_sh = "UDT";
// const std::string __name = "Name";
// const std::string __params = "Parameters";
const std::string __params_sh = "Params";

//--- Part 2 typical island configuration ---
const std::string __typconfig = "Typical configuration";
const std::string __isl_name = "Island name";
const std::string __isl_name_sh = "Isl name";
// const std::string __island = "Island";
const std::string __island_sh = "UDI"; // same pattern for any udc
// const std::string __algorithm = "Algorithm";
const std::string __algorithm_sh = "UDA"; // same pattern for any udc
// const std::string __problem = "Problem";
const std::string __problem_sh = "UDP"; // same pattern for any udc
// const std::string __r_policy = "Replacement policy";
const std::string __r_policy_sh = "UDRP"; // same pattern for any udc
// const std::string __s_policy = "Selection policy";
const std::string __s_policy_sh = "UDSP"; // same pattern for any udc
const std::string __population = "Population";
const std::string __size = "Size";
// const std::string __generations = "Generations";
const std::string __seed = "Seed";
const std::string __report_gen = "Report generation";
const std::string __report_gen_sh = "Report gen";

//--- Part 3 specializations ---
const std::string __specs = "Specializations";
// then inside the same pattern as the typical configuration but one object for each specialization 

// This key is if you don't want to use the specializations in the input file. 
// The typical configuration is used for all the islands but ran "Random starts"
// times with random seeds.
const std::string __rand_starts = "Random starts"; 

} /* namespace label */

// convert any STL string to camel case
template <typename T>
T to_camel_case(const T &s) {
    return s;
}

// convert any STL string to snake case
template <typename T>
T to_snake_case(const T &s) {
    return s;
}

// convert any STL string to kebab case
template <typename T>
T to_kebab_case(const T &s) {
    // kebab case means that the string is separated by hyphens '-' and all the letters are lowercase

    T result; 
    result.reserve(s.size());

    bool prev_is_upper = false;
    for (auto c : s) {
        if (std::isupper(c)) {
            if (!prev_is_upper) {
                result.push_back('-');
            }
            result.push_back(std::tolower(c));
            prev_is_upper = true;
        } else {
            result.push_back(c);
            prev_is_upper = false;
        }
    }

    // substitute all spaces with hyphens
    std::replace(result.begin(), result.end(), ' ', '-');

    // remove the beginning hyphen if it exists
    if (!result.empty() && result.front() == '-') {
        result.erase(result.begin());
    }

    return result;
}

inline std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream token_stream(s);
    while (std::getline(token_stream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

} /* namespace bevarmejo */

#endif /* BEVARMEJOLIB__LABELS_HPP */
