#include <string>

#include "wds_problem.hpp"

namespace bevarmejo {

WDSProblem::WDSProblem() : 
    m__name("EmptyWDSProblem"),
    m__extra_info(""),
    m__dv_adapter()
    { }

WDSProblem::WDSProblem(const std::string& name, const std::string& extra_info) : 
    m__name(name), 
    m__extra_info(extra_info),
    m__dv_adapter()
    { }

std::string WDSProblem::get_name() const { return m__name; }

std::string WDSProblem::get_extra_info() const { return m__extra_info; }

} // namespace bevarmejo
