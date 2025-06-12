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

auto WDSProblem::get_nx() const -> std::vector<double>::size_type {
    return this->get_bounds().first.size();
}

auto WDSProblem::get_nix() const -> std::vector<double>::size_type {
    auto mask = this->get_continuous_dvs_mask();
	return std::count(mask.begin(), mask.end(), false);
}

auto WDSProblem::get_continuous_dvs_mask() const -> std::vector<bool> {
    // By default, Pagmo assumes every dv as continuous.
    // Therefore, we will do the same.
    return std::vector<bool>(this->get_nx(), true);
}

auto WDSProblem::get_ncx() const -> std::vector<double>::size_type {
    return this->get_nx() - this->get_nix();
}

} // namespace bevarmejo
