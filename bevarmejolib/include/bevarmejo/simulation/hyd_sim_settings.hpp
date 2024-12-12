#pragma once

#include <iostream>

#include "bevarmejo/utility/epanet/time.hpp"

namespace bevarmejo::sim
{

class HydSimSettings
{
/*------- Member types -------*/
public:
    using time_t = bevarmejo::epanet::time_t;

/*------- Member objects -------*/
private:
    time_t shift_start_time__s = 0l;         // Shift of the start time of the simulation
    time_t horizon__s = 0l;                 // Duration of the simulation
    time_t resolution__s = 0l;               // Minimum resolution (Time step) of the simulation (actual time steps can be smaller)

/*------- Member functions -------*/

};

} // namespace bevarmejo::sim
