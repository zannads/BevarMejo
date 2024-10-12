#ifndef BEVARMEJOLIB__PAGMO_HELPERS__CONTAINERS_HELP_HPP
#define BEVARMEJOLIB__PAGMO_HELPERS__CONTAINERS_HELP_HPP

#include <string>
#include <vector>

#include <pagmo/problem.hpp>

namespace bevarmejo {
namespace io {
namespace inp {
void temp_net_to_file(const pagmo::problem& prob, const std::vector<double>& dv, const std::string& out_file);
} // namespace inp
} // namespace io
} // namespace bevarmejo

#endif // BEVARMEJOLIB__PAGMO_HELPERS__CONTAINERS_HELP_HPP
