#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <pagmo/problem.hpp>

#include "Anytown/prob_anytown.hpp"

#include "Hanoi/problem_hanoi_biobj.hpp"

#include "containers_help.hpp"

namespace bevarmejo {
namespace io {
void inp::temp_net_to_file(const pagmo::problem& prob, const std::vector<double>& dv, const std::string& out_file) {
    if ( prob.is<bevarmejo::anytown::Problem>() ) {
        inp::detail::temp_net_to_file<bevarmejo::anytown::Problem>(*prob.extract<bevarmejo::anytown::Problem>(), dv, out_file);
    } else if ( prob.is<bevarmejo::hanoi::fbiobj::Problem>() ) {
        //inp::detail::temp_net_to_file<bevarmejo::hanoi::fbiobj::Problem>(*prob.extract<bevarmejo::hanoi::fbiobj::Problem>(), dv, out_file);
    } else {
        throw std::runtime_error("Problem type not supported for inp file generation");
    }
}

} // namespace io
} // namespace bevarmejo
