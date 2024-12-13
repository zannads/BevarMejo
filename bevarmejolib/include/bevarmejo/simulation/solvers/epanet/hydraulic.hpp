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
    bool f__save_all_hsteps = true;       // Bool to turn on/off the report behaviour like in EPANET

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
    time_t report_resolution() const noexcept;
    bool save_all_hsteps() const noexcept;

/*--- Modifiers ---*/
public:
    void report_resolution(time_t a_resolution);
    void save_all_hsteps(bool a_save_all);

}; // class HydSimSettings

using HydSimResults = bevarmejo::wds::aux::QuantitySeries<int>;
HydSimResults solve_hydraulics(bevarmejo::WaterDistributionSystem& a_wds, const bevarmejo::sim::solvers::epanet::HydSimSettings& a_settings);

bool is_successful(const HydSimResults& a_results) noexcept;

} // namespace bevarmejo::sim::solvers::epanet
