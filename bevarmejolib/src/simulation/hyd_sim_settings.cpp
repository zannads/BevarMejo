#include "bevarmejo/utility/exceptions.hpp"
#include "bevarmejo/utility/time.hpp"

#include "bevarmejo/simulation/hyd_sim_settings.hpp"

namespace bevarmejo::sim
{

/*--- Element access ---*/
time_t HydSimSettings::start_time() const noexcept
{
    return shift_start_time__s;
}

time_t HydSimSettings::horizon() const noexcept
{
    return horizon__s;
}

time_t HydSimSettings::resolution() const noexcept
{
    return resolution__s;
}

bool HydSimSettings::should_terminate_early() const noexcept
{
    return f__early_termination;
}

/*--- Modifiers ---*/
auto HydSimSettings::start_time(time_t a_start_time) -> HydSimSettings&
{
    beme_throw_if(a_start_time < 0 || a_start_time > 24*60*60, std::invalid_argument,
        "Impossible to set the start time of the simulation.",
        "The start time must be between 0 and 24 hours and it is expressd in seconds.",
        "Start time: ", a_start_time);

    shift_start_time__s = a_start_time;

    return *this;
}

auto HydSimSettings::horizon(time_t a_horizon) -> HydSimSettings&
{
    beme_throw_if(a_horizon < 0, std::invalid_argument,
        "Impossible to set the duration of the simulation.",
        "The duration must be a positive number and it is expressed in seconds.",
        "Duration: ", a_horizon);

    horizon__s = a_horizon;

    return *this;
}

auto HydSimSettings::resolution(time_t a_resolution) -> HydSimSettings&
{
    beme_throw_if(a_resolution <= 0, std::invalid_argument,
        "Impossible to set the resolution of the simulation.",
        "The resolution must be a positive number and it is expressed in seconds.",
        "Resolution: ", a_resolution);

    resolution__s = a_resolution;

    return *this;
}

auto HydSimSettings::enable_early_termination() -> HydSimSettings&
{
    f__early_termination = true;

    return *this;
}

auto HydSimSettings::disable_early_termination() -> HydSimSettings&
{
    f__early_termination = false;

    return *this;
}

} // namespace bevarmejo::sim
