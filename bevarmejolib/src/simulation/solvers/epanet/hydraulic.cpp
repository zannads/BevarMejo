#include "bevarmejo/simulation/hyd_sim_settings.hpp"
#include "bevarmejo/simulation/solvers/epanet/hydraulic.hpp"

namespace bevarmejo::sim::solvers::epanet
{

time_t HydSimSettings::report_resolution() const noexcept
{
    return report_resolution__s;
}

bool HydSimSettings::save_all_hsteps() const noexcept
{
    return f__save_all_hsteps;
}

void HydSimSettings::report_resolution(time_t a_resolution)
{
    beme_throw_if(a_resolution <= 0, std::invalid_argument,
        "Impossible to set the resolution of the reporting.",
        "The resolution must be a positive number.",
        "Resolution: ", a_resolution);

    report_resolution__s = a_resolution;
}

void HydSimSettings::save_all_hsteps(bool a_save_all)
{
    beme_throw_if(a_save_all == false /* && version is greater than ...*/, std::invalid_argument,
        "Impossible to set the required behaviour for the reporting.",
        "From version ... the default EPANET behaviour is not allowed.",
        "Save all hydraulic steps: ", a_save_all);

    f__save_all_hsteps = a_save_all;
}






// Forward declaration of the internal functions
namespace detail
{

void prepare_internal_solver(bevarmejo::WaterDistributionSystem& a_wds) noexcept;
void release_internal_solver(bevarmejo::WaterDistributionSystem& a_wds) noexcept;

namespace EarlyTerminationMode
{
    struct Never { }; // Never terminate early, return all the error/warning codes
    struct OnErrors { }; // Terminate early only if the error are critical (ignore warnings, errorcode > 100)
    struct Always { }; // Return early if any error or warning code is returned (errorcode > 0)
};

namespace ReportingMode
{
    struct All { }; // Save all the hydraulic steps
    struct Scheduled { }; // Save only the scheduled hydraulic steps (as in EPANET)
}; // namespace ReportingModes

template <typename ETM, typename RM>
bool save_results(const int errorcode, const time_t t, const HydSimSettings& a_settings, bevarmejo::WaterDistributionSystem& a_wds, HydSimResults& res) noexcept
{
    if constexpr (std::is_same_v<RM, ReportingMode::Scheduled>)
    {
        // In scheduled mode, we save only the results at the reporting time steps
        // Therefore, if it is not a reporting time step, we return early and ignore 
        // also the errorcode.

        if (t % a_settings.report_resolution() != 0)
        {
            return false;
        }
    }
    
    // We end up here both when RM is All or it is Scheduled and a reporting time step.
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

    if constexpr (std::is_same_v<ETM, EarlyTerminationMode::Never>)
    {
        return false;
    }
    else if constexpr (std::is_same_v<ETM, EarlyTerminationMode::OnErrors>)
    {
        return errorcode > 100;
    }
    else if constexpr (std::is_same_v<ETM, EarlyTerminationMode::Always>)
    {
        return errorcode > 0;
    }
    else
    {
        static_assert(false, "Unknown EarlyTerminationMode");
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
        early_termination = detail::save_results<detail::EarlyTerminationMode::OnErrors, detail::ReportingMode::All>(
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
