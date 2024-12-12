#pragma once

#include "bevarmejo/wds/water_distribution_system.hpp"
#include "bevarmejo/simulation/hyd_sim_settings.hpp"

namespace bevarmejo::sim::solvers::epanet
{

class HydSimSettings final : public bevarmejo::sim::HydSimSettings
{
/*------- Member types -------*/
private:
    using inherited = bevarmejo::sim::HydSimSettings;
public:
    using time_t = inherited::time_t;

/*------- Member objects -------*/
private:
    time_t report_resolution__s = 0l;  // Step of the reporting in EPANET
    bool save_all_hsteps = true;       // Bool to turn on/off the report behaviour like in EPANET

/*------- Member functions -------*/

}; // class HydSimSettings

using HydSimResults = bevarmejo::wds::aux::QuantitySeries<int>;
HydSimResults solve_hydraulics(bevarmejo::WaterDistributionSystem& a_wds, const bevarmejo::sim::solvers::epanet::HydSimSettings& a_settings);

bool is_successful(const HydSimResults& a_results) noexcept;

namespace detail
{

void prepare_internal_solver(bevarmejo::WaterDistributionSystem& a_wds) noexcept;

void clean_internal_solver(bevarmejo::WaterDistributionSystem& a_wds) noexcept;

} // namespace detail

} // namespace bevarmejo::sim::solvers::epanet
