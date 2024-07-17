#ifndef BEVARMEJOLIB__WDS_DATA_STRUCTURES__TIME_OPTIONS_HPP
#define BEVARMEJOLIB__WDS_DATA_STRUCTURES__TIME_OPTIONS_HPP

#include <vector>

namespace bevarmejo {
namespace wds {

struct GlobalTimeOptions {
    long start_time__s = 0;               // Start time of the simulation in seconds
    long duration__s = 3600;              // Duration of the simulation in seconds
};
struct PatternTimeOptions {
    long shift_start_time__s = 0;         // Shift of the start time of the pattern
    long timestep__s = 3600;             // Step of the pattern
};

using TimeSteps = std::vector<long>;

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_DATA_STRUCTURES__TIME_OPTIONS_HPP
