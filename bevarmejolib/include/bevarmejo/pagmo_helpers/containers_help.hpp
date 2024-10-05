#ifndef BEVARMEJOLIB__PAGMO_HELPERS__CONTAINERS_HELP_HPP
#define BEVARMEJOLIB__PAGMO_HELPERS__CONTAINERS_HELP_HPP

#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <pagmo/algorithm.hpp>
#include <pagmo/island.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/r_policy.hpp>
#include <pagmo/s_policy.hpp>
#include <pagmo/topology.hpp>

#include <nlohmann/json.hpp>
namespace nl = nlohmann;

#include "bevarmejo/io.hpp"
#include "bevarmejo/labels.hpp"

namespace bevarmejo {
namespace io {
namespace inp {
void temp_net_to_file(const pagmo::problem& prob, const std::vector<double>& dv, const std::string& out_file);
} // namespace inp
} // namespace io
} // namespace bevarmejo

#endif // BEVARMEJOLIB__PAGMO_HELPERS__CONTAINERS_HELP_HPP
