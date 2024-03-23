#ifndef ANYTOWN__TWOPHASES__PROB_ANYTOWN_TWO_F1_HPP
#define ANYTOWN__TWOPHASES__PROB_ANYTOWN_TWO_F1_HPP

#include <iostream>
#include <filesystem>
#include <memory>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "prob_anytown.hpp"

namespace fsys = std::filesystem;
namespace bevarmejo {
// Data of Anytown that can't be changed.
namespace anytown {
namespace twophases {
namespace f1 {

const std::string name = "bevarmejo::anytown::twophases::f1";
const std::string extra_info = "\tVersion 1 of Anytown Rehabilitation Formulation 1\nPipes as in Farmani, Tanks as in Vamvakeridou-Lyroudia but discrete, operations optimized internally)\n";
    
// Dimensions of the problem.
constexpr std::size_t n_obj = 2u;
constexpr std::size_t n_ec = 0u;
constexpr std::size_t n_ic = 0u;
constexpr std::size_t n_fit = n_obj + n_ec + n_ic;
constexpr std::size_t n_dv = 80u;
constexpr std::size_t n_ix = 80u; // Will transform the tank volume to a continuous variable in the future.
constexpr std::size_t n_cx = n_dv-n_ix;
    
class Problem {

};

} // namespace f1
} // namespace twophases
} // namespace anytown
} // namespace bevarmejo

#endif // ANYTOWN__TWOPHASES__PROB_ANYTOWN_TWO_F1_HPP
