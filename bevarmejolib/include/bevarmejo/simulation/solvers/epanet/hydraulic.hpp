#pragma once

#include "bevarmejo/wds/water_distribution_system.hpp"
#include "bevarmejo/simulation/hyd_sim_settings.hpp"

#include "bevarmejo/simulation/solvers/epanet/water_demand_modelling.hpp"

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
    time_t m__report_resolution__s = 0l;  // Step of the reporting in EPANET

    double m__demand_multiplier = 1.; // Global water demand multiplier (applied when preparing the matrices to solve).

/*------- Member functions -------*/
// (constructor)
public:
    HydSimSettings() = default;
    HydSimSettings(const HydSimSettings&) = default;
    HydSimSettings(HydSimSettings&&) noexcept = default;

// (destructor)
public:
    ~HydSimSettings() = default;

// operator=
public:
    HydSimSettings& operator=(const HydSimSettings&) = default;
    HydSimSettings& operator=(HydSimSettings&&) noexcept = default;

/*--- Element access ---*/
public:
    auto report_resolution() const noexcept -> time_t;

    auto demand_multiplier() const noexcept -> double;

/*--- Modifiers ---*/
public:
    auto report_resolution(time_t a_resolution) -> HydSimSettings&;

    auto demand_multiplier(double a_multiplier) -> HydSimSettings&;

}; // class HydSimSettings

using HydSimResults = bevarmejo::wds::aux::QuantitySeries<int>;
HydSimResults solve_hydraulics(bevarmejo::WaterDistributionSystem& a_wds, const bevarmejo::sim::solvers::epanet::HydSimSettings& a_settings);

bool is_successful(const HydSimResults& a_results) noexcept;
bool is_successful_with_warnings(const HydSimResults& a_results) noexcept;

} // namespace bevarmejo::sim::solvers::epanet
