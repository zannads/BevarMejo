
#include <filesystem>
namespace fsys = std::filesystem;
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <pagmo/problem.hpp>

#include <nlohmann/json.hpp>
using json_o = nlohmann::json;

#include "bevarmejo/utility/bemexcept.hpp"

// now include all the specific Bevarmejo problems
#include "Anytown/prob_anytown.hpp"
#include "Hanoi/problem_hanoi_biobj.hpp"

#include "factories.hpp"

namespace bevarmejo {

namespace io {
namespace log {

namespace fname {
static const std::string build_problem = "build_problem";
} // namespace fname

namespace mex {
static const std::string build_problem_error = "The creation of the requested problem failed.";

static const std::string problem_suite = "problem suite";
static const std::string problem_name = "problem name";
static const std::string problem_formulation = "problem formulation";
static const std::string unknown_problem_feature(const std::string& feature) {
    return "The " + feature + " is not recognized.";
}
static const std::string unsupported_problem_feature(const std::string& feature) {
    return "The " + feature + " is not supported anymore.";
}
static const std::string unimplemented_problem_feature(const std::string& feature) {
    return "The " + feature + " is not yet implemented.";
}

static const std::string __problem_suite = "Problem suite : ";
static const std::string __problem_name = "Problem name : ";
static const std::string __problem_formulation = "Problem formulation : ";
} // namespace mex

} // namespace log
} // namespace io

pagmo::problem build_problem(const std::string &problem_name_str, const json_o &pparams, const std::vector<fsys::path> &lookup_paths) {

    bevarmejo::io::detail::ProblemName probname = bevarmejo::io::split_problem_name(problem_name_str);

    if ( probname.suite == "bevarmejo" )
    {
        if ( probname.problem == "hanoi" )
        {
            return pagmo::problem{ bevarmejo::hanoi::fbiobj::Problem(pparams, lookup_paths) };
        }
        
        if ( probname.problem == "anytown" )
        {
            return pagmo::problem{ bevarmejo::anytown::Problem(probname.formulation, pparams, lookup_paths) };
        }

        beme_throw(std::invalid_argument,
            "Impossible to build the requested problem.",
            "The provided problem name is not recognized.",
            "Problem name : ", probname.problem);
    }
    
    if ( probname.suite == "pagmo" )
    {
        beme_throw(std::invalid_argument,
            "Impossible to build the requested problem.",
            "The provided problem suite is not supported yet.",
            "Problem suite : pagmo");
    }
    
    beme_throw(std::invalid_argument,
        "Impossible to build the requested problem.",
        "The provided problem suite is not recognized.",
        "Problem suite : ", probname.suite);
}

} // namespace bevarmejo
