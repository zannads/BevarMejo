#ifndef BEVARMEJOLIB__WDS_DATA_STRUCTURES__TIME_OPTIONS_HPP
#define BEVARMEJOLIB__WDS_DATA_STRUCTURES__TIME_OPTIONS_HPP

#include <vector>

namespace bevarmejo {
namespace wds {

using time_t = long;

struct GlobalTimeOptions {
    time_t shift_start_time__s = 0;         // Shift for the start time of the simulation in seconds, but this is only for control rules and eventually for visualization. Simulations and times always start at 0.
    time_t duration__s = 3600;              // Duration of the simulation in seconds
};
struct PatternTimeOptions {
    time_t shift_start_time__s = 0;         // Shift of the start time of the pattern
    time_t timestep__s = 3600;             // Step of the pattern
};

using TimeSteps = std::vector<time_t>;

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_DATA_STRUCTURES__TIME_OPTIONS_HPP
