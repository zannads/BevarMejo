#include <string>
#include <memory>

#include "wds_problem.hpp"

namespace bevarmejo {

WDSProblem::WDSProblem() : 
    m__name("EmptyWDSProblem"),
    m__extra_info(""),
    m__dv_adapter(),
    m__inp_base_filename(),
    m__metrics_filename()
    { }

WDSProblem::WDSProblem(const std::string& name, const std::string& extra_info) : 
    m__name(name), 
    m__extra_info(extra_info),
    m__dv_adapter(),
    m__inp_base_filename(),
    m__metrics_filename()
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

auto WDSProblem::enable_save_inp(
    std::string a_inp_base_filename
) -> WDSProblem&
{
    if (a_inp_base_filename.empty()) {
        return this->disable_save_inp();
    }
    m__inp_base_filename = std::move(a_inp_base_filename);
    return *this;
}

auto WDSProblem::disable_save_inp() noexcept -> WDSProblem&
{
    m__inp_base_filename = std::string();
    return *this;
}

auto WDSProblem::enable_save_metrics(
    std::string a_metrics_filename
) -> WDSProblem&
{
    if (a_metrics_filename.empty()) {
        return this->disable_save_metrics();
    }
    m__metrics_filename = std::move(a_metrics_filename);
    return *this;
}

auto WDSProblem::disable_save_metrics() noexcept -> WDSProblem&
{
    m__metrics_filename = std::string();
    return *this;
}

} // namespace bevarmejo
