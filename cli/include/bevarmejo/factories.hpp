#ifndef FACTORIES_HPP
#define FACTORIES_HPP

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <pagmo/problem.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "bevarmejo/labels.hpp"

// now include all the specific Bevarmejo problems
#include "Anytown/prob_anytown.hpp"
#include "Anytown/rehab/prob_at_reh_f1.hpp"
#include "Anytown/mixed/prob_at_mix_f1.hpp"
#include "Anytown/operations/prob_at_ope_f1.hpp"

#include "Hanoi/problem_hanoi_biobj.hpp"

namespace bevarmejo {

inline pagmo::problem build_problem(json jinput, std::vector<std::filesystem::path> lookup_paths) {
    pagmo::problem p{};

    auto probname_s = jinput[label::__name].get<std::string>();
    bevarmejo::io::detail::ProblemName probname = bevarmejo::io::split_problem_name(probname_s);

    auto pparams = jinput[label::__params];

    if ( probname.suite == "bevarmejo" ) {

        if ( probname.problem == "hanoi" ) {
            p = bevarmejo::hanoi::fbiobj::Problem(pparams, lookup_paths);
        }
        else if ( probname.problem == "anytown" ) {

            if (probname.formulation == bevarmejo::anytown::io::value::rehab_f1)
                p = bevarmejo::anytown::rehab::f1::Problem(pparams, lookup_paths);

            else if (probname.formulation == bevarmejo::anytown::io::value::mixed_f1)
                p = bevarmejo::anytown::mixed::f1::Problem(pparams, lookup_paths);

            else if (probname.formulation == bevarmejo::anytown::io::value::opertns_f1)
                p = bevarmejo::anytown::operations::f1::Problem(pparams, lookup_paths);

            else if (probname.formulation == bevarmejo::anytown::io::value::twoph_f1)
                // p = bevarmejo::anytown::twophases::f1::Problem(pparams, lookup_paths);
                throw std::runtime_error("The twophases formulation is not yet implemented.");

            else {
                throw std::runtime_error("The problem formulation is not recognized.");
            }
            
        }
        else {
            throw std::runtime_error("The problem name is not recognized.");
        }

    }
    else if ( probname.suite == "pagmo" ) {
        throw std::runtime_error("The pagmo problems are not yet implemented.");
    }
    else {
        throw std::runtime_error("The problem suite is not recognized.");
    }

    return p;
}

} // namespace bevarmejo

#endif // FACTORIES_HPP
