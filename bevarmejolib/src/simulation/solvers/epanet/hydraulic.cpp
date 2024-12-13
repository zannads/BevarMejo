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
void release_internal_solver(bevarmejo::WaterDistributionSystem& a_wds) noexcept;

// Early termination mode, flag that tells if the simulation should stop on warnings. (Useful to reduce computation time in optimisations).
template <bool ETM>
bool save_results(const int errorcode, const time_t t, const HydSimSettings& a_settings, bevarmejo::WaterDistributionSystem& a_wds, HydSimResults& res) noexcept
{

#if LIBRARY_VERSION <= 240400
// If you save only the reporting time steps (as EPANET and wntr do) instead of all hydraulic steps,
// you may get an incorrect estimate of the energy leaving the system. This happens because EPANET
// can insert extra time steps in the simulation. When energy is calculated as Flow x Head x TimeStep,
// the TimeStep used is the reporting one, while Flow and Head are values at the beginning of the reported period.
// The correct calculation should sum (Flow x Head x HydraulicTimeStep) for all intermediate hydraulic time steps.
// Until v24.04.00, the default behaviour was to save only the reporting time steps.
    if (t % a_settings.report_resolution() != 0)
    {
        return false;
    }
#endif
    
    // First we need to add the time to the time series of results.
    // Then I can add the value to all the QuantitySeries linked to that result.
    a_wds.result_time_series().commit(t);

    if (errorcode <= 100)
    {
        res.commit(t, 0);
    }
    else
    {
        res.commit(t, errorcode);
    }

    for (auto&& [id, node] : a_wds.nodes())
    {
        node.retrieve_EN_results();
    }

    for (auto&& [id, link] : a_wds.links())
    {
        link.retrieve_EN_results();
    }

    // We are always quitting when there is a memory error.
    if (errorcode > 100)
    {
        return true;
    }

    if constexpr (ETM)
    {
        // In case of warnings (simulation failed), we return early
        return errorcode > 0;
    }
    else // No early termination
    {
        return false;
    }
}

} // namespace detail

auto solve_hydraulics(bevarmejo::WaterDistributionSystem& a_wds, const HydSimSettings& a_settings) -> HydSimResults
{
    // Reset previous results and allocate memory for the new ones
    a_wds.clear_results();
    a_wds.result_time_series().reset();

    /*-------------------------*/
    // This part will be removed once I have the settings working.
    auto ph = a_wds.ph();
    long h_step = 0;
    int errorcode = EN_gettimeparam(ph, EN_HYDSTEP, &h_step);
    assert(errorcode < 100);
    long r_step = 0;
    errorcode = EN_gettimeparam(ph, EN_REPORTSTEP, &r_step);
    assert(errorcode < 100);
    long horizon = 0;
    errorcode = EN_gettimeparam(ph, EN_DURATION, &horizon);
    assert(errorcode < 100);

    /*-------------------------*/
    // This will look like slightly different, but the idea is the same.
    long n_reports = horizon / r_step + 1; // +1 because the first report is at time 0
    a_wds.result_time_series().reserve(n_reports);

    auto res = HydSimResults(a_wds.result_time_series());

    detail::prepare_internal_solver(a_wds);

    // Run the simulation
    time_t t = 0; // current time
    time_t delta_t = 0; // real hydraulic time step

    do
    {
        int errorcode = EN_runH(ph, &t);

        bool early_termination = false;
        // Based onthe settings and the version call the right save_results
        early_termination = detail::save_results<false>(
            errorcode, t, a_settings, a_wds, res);
        
        if (early_termination)
        {
            break;
        }

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

auto detail::prepare_internal_solver(bevarmejo::WaterDistributionSystem& a_wds) noexcept -> void
{
    auto ph = a_wds.ph();
    assert(ph != nullptr);
    
    int errorcode = EN_openH(ph);
    assert(errorcode <= 100);
    
    errorcode = EN_initH(ph, 10);
    assert(errorcode <= 100);
}

auto detail::release_internal_solver(bevarmejo::WaterDistributionSystem& a_wds) noexcept -> void
{
    auto ph = a_wds.ph();
    int errorcode = EN_closeH(ph);
    assert(errorcode < 100);
}

} // namespace bevarmejo::sim::solvers::epanet
