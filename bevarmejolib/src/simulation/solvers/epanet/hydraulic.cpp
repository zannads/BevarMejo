#include <memory>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/utility/exceptions.hpp"

#include "bevarmejo/simulation/hyd_sim_settings.hpp"
#include "bevarmejo/simulation/solvers/epanet/hydraulic.hpp"

namespace bevarmejo::sim::solvers::epanet
{

/*------- Member functions -------*/
// (constructor)
HydSimSettings::HydSimSettings() :
    inherited(),
    m__report_resolution__s(resolution() > horizon() ? resolution() : horizon()),
    m__wdm(std::make_unique<DemandDrivenAnalysis>()),
    m__demand_multiplier(1.0)
{ }

HydSimSettings::HydSimSettings(const HydSimSettings& other) :
    inherited(other),
    m__report_resolution__s(other.m__report_resolution__s),
    m__wdm(other.m__wdm ? other.m__wdm->clone() : nullptr),
    m__demand_multiplier(other.m__demand_multiplier)
{ }

HydSimSettings& HydSimSettings::operator=(const HydSimSettings& other)
{
    if (this != &other) {
        inherited::operator=(other);
        m__report_resolution__s = other.m__report_resolution__s;
        m__wdm = other.m__wdm ? other.m__wdm->clone() : nullptr;
        m__demand_multiplier = other.m__demand_multiplier;
    }
    return *this;
}

auto HydSimSettings::report_resolution() const noexcept -> time_t
{
    return m__report_resolution__s;
}

auto HydSimSettings::demand_multiplier() const noexcept -> double
{
    return m__demand_multiplier;
}

auto HydSimSettings::report_resolution(time_t a_resolution) -> HydSimSettings&
{
    beme_throw_if(a_resolution <= 0, std::invalid_argument,
        "Impossible to set the resolution of the reporting.",
        "The resolution must be a positive number.",
        "Resolution: ", a_resolution);

    m__report_resolution__s = a_resolution;

    return *this;
}

auto HydSimSettings::demand_driven_analysis() -> HydSimSettings&
{
    m__wdm = std::make_unique<DemandDrivenAnalysis>();

    return *this;
}

auto HydSimSettings::pressure_driven_analysis(
    const double a_minimum_pressure__m,
    const double a_required_pressure__m,
    const double a_pressure_exponent
) -> HydSimSettings&
{
    m__wdm = std::make_unique<PressureDrivenAnalysis>(
        a_minimum_pressure__m,
        a_required_pressure__m,
        a_pressure_exponent
    );

    return *this;
}

auto HydSimSettings::demand_multiplier(double a_multiplier) -> HydSimSettings&
{
    beme_throw_if(a_multiplier < 0.0, std::invalid_argument,
        "Impossible to set the global water demand multiplier.",
        "The multiplier must be a non-negative number.",
        "Multiplier: ", a_multiplier);

    m__demand_multiplier = a_multiplier;

    return *this;
}

auto HydSimSettings::apply_water_demand_model(EN_Project a_ph) const -> void
{
    // If uses DDA
    if (dynamic_cast<const DemandDrivenAnalysis*>(m__wdm.get()) != nullptr)
    {
        int errorcode = EN_setdemandmodel(a_ph, EN_DDA, 0.0, 0.0, 0.0);
        assert(errorcode <= 100);
        return;
    }
    
    // otherwise we are using PDA
    assert(dynamic_cast<const PressureDrivenAnalysis*>(m__wdm.get()) != nullptr);
    auto pda = dynamic_cast<const PressureDrivenAnalysis*>(m__wdm.get());

    // If the units are US, pressures must be in PSI
    if (a_ph->parser.Unitsflag == US)
    {
        int errorcode = EN_setdemandmodel(
            a_ph, EN_DDA,
            pda->minimum_pressure__m()/MperFT*PSIperFT,
            pda->required_pressure__m()/MperFT*PSIperFT,
            pda->pressure_exponent()
        );
        assert(errorcode <= 100);
        return;
    }
    
    // otherwise it's in SI
    int errorcode = EN_setdemandmodel(
        a_ph, EN_DDA,
        pda->minimum_pressure__m(),
        pda->required_pressure__m(),
        pda->pressure_exponent()
    );
    assert(errorcode <= 100);
    return;

}

// Forward declaration of the internal functions
namespace detail
{

void prepare_internal_solver(bevarmejo::WaterDistributionSystem& a_wds, const HydSimSettings& a_settings) noexcept;
void retrieve_results(const int errorcode, const time_t t, bevarmejo::WaterDistributionSystem& a_wds, HydSimResults& res);
void release_internal_solver(bevarmejo::WaterDistributionSystem& a_wds) noexcept;

} // namespace detail

auto solve_hydraulics(bevarmejo::WaterDistributionSystem& a_wds, const HydSimSettings& a_settings) -> HydSimResults
{
    // Reset previous results and allocate memory for the new ones
    a_wds.clear_results();
    a_wds.result_time_series().reset();

    auto ph = a_wds.ph();
    
    a_wds.result_time_series().reserve(a_settings.n_steps());

    auto res = HydSimResults(a_wds.result_time_series());

    detail::prepare_internal_solver(a_wds, a_settings);

    // Run the simulation
    time_t t = 0; // current time
    time_t delta_t = 0; // real hydraulic time step

    do
    {
        int errorcode = EN_runH(ph, &t);

#if BEME_VERSION < 240401
        // If you save only the reporting time steps (as EPANET and wntr do) instead of all hydraulic steps,
        // you may get an incorrect estimate of the energy leaving the system. This happens because EPANET
        // can insert extra time steps in the simulation. When energy is calculated as Flow x Head x TimeStep,
        // the TimeStep used is the reporting one, while Flow and Head are values at the beginning of the reported period.
        // The correct calculation should sum (Flow x Head x HydraulicTimeStep) for all intermediate hydraulic time steps.
        // This is correctly done for the ENERGY property in EPANET, but if you compute any metric integrating through time 
        // a variable of a network element (e.g., the volume of undelivered demand of a node), you will get a wrong estimate.
        // Until v24.04.00, the default behaviour was to save only the reporting time steps.
        if (t % a_settings.report_resolution() == 0)
        {
#endif
        // Retrieve_results guarantees that all results were written or none.
        detail::retrieve_results(errorcode, t, a_wds, res);

        // In early termination mode, a warning is sufficient to stop the simulation.
        // (This may be useful to save some runtime for very bad solutions.)
        // Otherwise, the simulation will stop only if a memory error occurs.
        if (errorcode > (a_settings.should_terminate_early() ? 0 : 100))
        {
            break;
        }

#if BEME_VERSION < 240401
        }
#endif
      
        errorcode = EN_nextH(ph, &delta_t);
        assert(errorcode < 100);
    }
    while (delta_t > 0);

    detail::release_internal_solver(a_wds);

    return res;
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

auto is_successful_with_warnings(const HydSimResults& a_result) noexcept -> bool
{
    for (const auto& [t, v] : a_result)
    {
        if (v > 100)
        {
            return false;
        }
    }

    return true;
}

auto detail::prepare_internal_solver(bevarmejo::WaterDistributionSystem& a_wds, const HydSimSettings& a_settings) noexcept -> void
{
    auto ph = a_wds.ph();
    assert(ph != nullptr);

    // Let's set the options...
    int errorcode = EN_settimeparam(ph, EN_DURATION, a_settings.horizon());
    assert(errorcode <= 100);

    errorcode = EN_settimeparam(ph, EN_HYDSTEP, a_settings.resolution());
    assert(errorcode <= 100);

    errorcode = EN_settimeparam(ph, EN_STARTTIME, a_settings.start_time());
    assert(errorcode <= 100);

    errorcode = EN_settimeparam(ph, EN_REPORTSTEP, a_settings.report_resolution());
    assert(errorcode <= 100);

    a_settings.apply_water_demand_model(ph);

    errorcode = EN_setoption(ph, EN_DEMANDMULT, a_settings.demand_multiplier());
    assert(errorcode <= 100);
    
    errorcode = EN_openH(ph);
    assert(errorcode <= 100);
    
    errorcode = EN_initH(ph, 10);
    assert(errorcode <= 100);
}

void detail::retrieve_results(const int errorcode, const time_t t, bevarmejo::WaterDistributionSystem& a_wds, HydSimResults& res)
{
    // First we need to add the time to the time series of results.
    // Then I can add the value to all the QuantitySeries linked to that result.
    a_wds.result_time_series().commit(t);

    res.commit(t, errorcode);

    for (auto&& [id, node] : a_wds.nodes())
    {
        node.retrieve_EN_results();
    }

    for (auto&& [id, link] : a_wds.links())
    {
        link.retrieve_EN_results();
    }

    // TODO: check that all results were actually retrieve, some try catch blocks and noexcept this function.

    return;
}

auto detail::release_internal_solver(bevarmejo::WaterDistributionSystem& a_wds) noexcept -> void
{
    auto ph = a_wds.ph();
    int errorcode = EN_closeH(ph);
    assert(errorcode < 100);
}

} // namespace bevarmejo::sim::solvers::epanet
