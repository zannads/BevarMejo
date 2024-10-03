
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <pagmo/problem.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "bevarmejo/bemexcept.hpp"
#include "bevarmejo/labels.hpp"

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

pagmo::problem build_problem(json jinput, std::vector<std::filesystem::path> lookup_paths) {

    auto probname_s = jinput[label::__name].get<std::string>();
    bevarmejo::io::detail::ProblemName probname = bevarmejo::io::split_problem_name(probname_s);

    auto pparams = jinput[label::__params];

    if ( probname.suite == "bevarmejo" ) {

        if ( probname.problem == "hanoi" ) {
            return pagmo::problem{ bevarmejo::hanoi::fbiobj::Problem(pparams, lookup_paths) };
        }
        
        if ( probname.problem == "anytown" ) {

            if (probname.formulation == bevarmejo::anytown::io::value::rehab_f1)
                return pagmo::problem{ bevarmejo::anytown::Problem(bevarmejo::anytown::Formulation::rehab_f1, pparams, lookup_paths) };

            if (probname.formulation == bevarmejo::anytown::io::value::mixed_f1)
                return pagmo::problem{ bevarmejo::anytown::Problem(bevarmejo::anytown::Formulation::mixed_f1, pparams, lookup_paths) };

            if (probname.formulation == bevarmejo::anytown::io::value::opertns_f1)
                return pagmo::problem{ bevarmejo::anytown::Problem(bevarmejo::anytown::Formulation::opertns_f1, pparams, lookup_paths) };

            if (probname.formulation == bevarmejo::anytown::io::value::twoph_f1)
                // return pagmo::problem{ bevarmejo::anytown::twophases::f1::Problem(pparams, lookup_paths) };
                __format_and_throw<std::invalid_argument, bevarmejo::FunctionError>(io::log::fname::build_problem, 
                    io::log::mex::build_problem_error, 
                    io::log::mex::unsupported_problem_feature(io::log::mex::problem_formulation),
                    io::log::mex::__problem_formulation, probname.formulation
                );

            if (probname.formulation == bevarmejo::anytown::io::value::rehab_f2)
                return pagmo::problem{ bevarmejo::anytown::Problem(bevarmejo::anytown::Formulation::rehab_f2, pparams, lookup_paths) };

            if (probname.formulation == bevarmejo::anytown::io::value::mixed_f2)
                return pagmo::problem{ bevarmejo::anytown::Problem(bevarmejo::anytown::Formulation::mixed_f2, pparams, lookup_paths) };

            // Get to this point it's unrecongnized
            __format_and_throw<std::invalid_argument, bevarmejo::FunctionError>(io::log::fname::build_problem, 
                io::log::mex::build_problem_error, 
                io::log::mex::unknown_problem_feature(io::log::mex::problem_formulation),
                io::log::mex::__problem_formulation, probname.formulation
            );
        }

        __format_and_throw<std::invalid_argument, bevarmejo::FunctionError>(io::log::fname::build_problem, 
            io::log::mex::build_problem_error, 
            io::log::mex::unknown_problem_feature(io::log::mex::problem_name),
            io::log::mex::__problem_name, probname.problem
        );
    }
    
    
    if ( probname.suite == "pagmo" ) {
        __format_and_throw<std::invalid_argument, bevarmejo::FunctionError>(io::log::fname::build_problem, 
            io::log::mex::build_problem_error, 
            io::log::mex::unimplemented_problem_feature(io::log::mex::problem_suite),
            io::log::mex::__problem_suite, probname.suite
        ); 
    }
    
    __format_and_throw<std::invalid_argument, bevarmejo::FunctionError>(io::log::fname::build_problem, 
        io::log::mex::build_problem_error, 
        io::log::mex::unknown_problem_feature(io::log::mex::problem_suite),
        io::log::mex::__problem_suite, probname.suite
    );

}

} // namespace bevarmejo
