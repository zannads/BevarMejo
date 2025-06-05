#pragma once

#include <iostream>

#include "bevarmejo/utility/time.hpp"

namespace bevarmejo::sim
{

class HydSimSettings
{
/*------- Member types -------*/
public:
    using time_t = bevarmejo::time_t;

/*------- Member objects -------*/
private:
    time_t shift_start_time__s = 0l;         // Shift of the start time of the simulation
    time_t horizon__s = 0l;                 // Duration of the simulation
    time_t resolution__s = 0l;               // Minimum resolution (Time step) of the simulation (actual time steps can be smaller)
    bool f__early_termination = false;  // If true, the simulation will stop at the first warning

/*------- Member functions -------*/
// (constructor)

// (destructor)

// operator=

/*--- Element access ---*/
public:
    time_t start_time() const noexcept;

    time_t horizon() const noexcept;

    time_t resolution() const noexcept;

    bool should_terminate_early() const noexcept;

/*--- Modifiers ---*/
    auto start_time(time_t a_start_time) -> HydSimSettings&;

    auto horizon(time_t a_horizon) -> HydSimSettings&;

    auto resolution(time_t a_resolution) -> HydSimSettings&;

    auto enable_early_termination() -> HydSimSettings&;
    auto disable_early_termination() -> HydSimSettings&;

/*--- Other ---*/
    std::size_t n_steps() const noexcept
    {
        return horizon__s / resolution__s + 1; // +1 because the first report is at time 0.
    }

};

} // namespace bevarmejo::sim
