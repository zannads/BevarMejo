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
    time_t m__report_resolution__s;  // Step of the reporting in EPANET

    std::unique_ptr<WaterDemandModel> m__wdm; // Water Demand Model object (DDA or PDA).

    double m__demand_multiplier = 1.; // Global water demand multiplier (applied when preparing the matrices to solve).

/*------- Member functions -------*/
// (constructor)
public:
    HydSimSettings();
    HydSimSettings(const HydSimSettings&);
    HydSimSettings(HydSimSettings&&) noexcept = default;

// (destructor)
public:
    ~HydSimSettings() = default;

// operator=
public:
    HydSimSettings& operator=(const HydSimSettings&);
    HydSimSettings& operator=(HydSimSettings&&) noexcept = default;

/*--- Element access ---*/
public:
    auto report_resolution() const noexcept -> time_t;

    auto demand_multiplier() const noexcept -> double;

/*--- Modifiers ---*/
public:
    auto report_resolution(time_t a_resolution) -> HydSimSettings&;

    auto use_demand_driven_analysis() -> HydSimSettings&;
    auto use_pressure_driven_analysis(
        const double a_minimum_pressure__m,
        const double a_required_pressure__m,
        const double a_pressure_exponent
    ) -> HydSimSettings&;

    auto demand_multiplier(double a_multiplier) -> HydSimSettings&;

}; // class HydSimSettings

using HydSimResults = bevarmejo::wds::aux::QuantitySeries<int>;
HydSimResults solve_hydraulics(bevarmejo::WaterDistributionSystem& a_wds, const bevarmejo::sim::solvers::epanet::HydSimSettings& a_settings);

bool is_successful(const HydSimResults& a_results) noexcept;
bool is_successful_with_warnings(const HydSimResults& a_results) noexcept;

} // namespace bevarmejo::sim::solvers::epanet
