#include "bevarmejo/simulation/hyd_sim_settings.hpp"
#include "bevarmejo/simulation/solvers/epanet/hydraulic.hpp"

namespace bevarmejo::sim::solvers::epanet
{

time_t HydSimSettings::report_resolution() const noexcept
{
    return report_resolution__s;
}

void HydSimSettings::report_resolution(time_t a_resolution)
{
    beme_throw_if(a_resolution <= 0, std::invalid_argument,
        "Impossible to set the resolution of the reporting.",
        "The resolution must be a positive number.",
        "Resolution: ", a_resolution);

    report_resolution__s = a_resolution;
}

// Forward declaration of the internal functions
namespace detail
{

void prepare_internal_solver(bevarmejo::WaterDistributionSystem& a_wds) noexcept;
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

    detail::prepare_internal_solver(a_wds);

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
        if (t % a_settings.report_resolution() != 0)
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

auto detail::prepare_internal_solver(bevarmejo::WaterDistributionSystem& a_wds) noexcept -> void
{
    auto ph = a_wds.ph();
    assert(ph != nullptr);
    
    int errorcode = EN_openH(ph);
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
