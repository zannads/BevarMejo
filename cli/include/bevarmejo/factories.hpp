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

    auto probname = jinput[label::__name].get<std::string>();
    auto pparams = jinput[label::__params];

    if ( probname == bevarmejo::anytown::rehab::f1::name) {
        p = bevarmejo::anytown::rehab::f1::Problem(pparams, lookup_paths);
    }
    else if ( probname == bevarmejo::anytown::mixed::f1::name) {
        p = bevarmejo::anytown::mixed::f1::Problem(pparams, lookup_paths);
    }
    else if ( probname == bevarmejo::anytown::operations::f1::name) {
        p = bevarmejo::anytown::operations::f1::Problem(pparams, lookup_paths);
    }
    else if ( probname == bevarmejo::hanoi::fbiobj::name) {
        p = bevarmejo::hanoi::fbiobj::Problem(pparams, lookup_paths);
    }
    else {
        throw std::runtime_error("The problem name is not recognized.");
    }

    return p;
}

} // namespace bevarmejo

#endif // FACTORIES_HPP
