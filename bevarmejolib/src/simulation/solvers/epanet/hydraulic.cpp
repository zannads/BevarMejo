#include "bevarmejo/simulation/hyd_sim_settings.hpp"
#include "bevarmejo/simulation/solvers/epanet/hydraulic.hpp"

namespace bevarmejo::sim::solvers::epanet
{

auto solve_hydraulics(bevarmejo::WaterDistributionSystem& a_wds, const HydSimSettings& a_settings) -> HydSimResults
{
    detail::prepare_internal_solver(a_wds);

    // Run the hydraulics
    a_wds.run_hydraulics();

    detail::clean_internal_solver(a_wds);

    return HydSimResults(a_wds.result_time_series());
}

auto is_successful(const HydSimResults& a_result) noexcept -> bool
{
    for (const auto& [t, v] : a_result)
    {
        if (v != 0)
        {
            return false;
        }
    }

    return true;
}

auto detail::prepare_internal_solver(bevarmejo::WaterDistributionSystem& a_wds) noexcept -> void
{
    // I don't need to do anything here.
}

auto detail::clean_internal_solver(bevarmejo::WaterDistributionSystem& a_wds) noexcept -> void
{
    // I don't need to do anything here.
}

} // namespace bevarmejo::sim::solvers::epanet
