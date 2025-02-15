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

/*--- Modifiers ---*/
void HydSimSettings::start_time(time_t a_start_time)
{
    beme_throw_if(a_start_time < 0 || a_start_time > 24*60*60, std::invalid_argument,
        "Impossible to set the start time of the simulation.",
        "The start time must be between 0 and 24 hours and it is expressd in seconds.",
        "Start time: ", a_start_time);

    shift_start_time__s = a_start_time;
}

void HydSimSettings::horizon(time_t a_horizon)
{
    beme_throw_if(a_horizon < 0, std::invalid_argument,
        "Impossible to set the duration of the simulation.",
        "The duration must be a positive number and it is expressed in seconds.",
        "Duration: ", a_horizon);

    horizon__s = a_horizon;
}

void HydSimSettings::resolution(time_t a_resolution)
{
    beme_throw_if(a_resolution <= 0, std::invalid_argument,
        "Impossible to set the resolution of the simulation.",
        "The resolution must be a positive number and it is expressed in seconds.",
        "Resolution: ", a_resolution);

    resolution__s = a_resolution;
}

} // namespace bevarmejo::sim
