#pragma once

#include <cstddef>
#include <memory>
#include <vector>

namespace bevarmejo {

namespace epanet {

using time_t = long; // seconds

struct PatternTimeOptions {
    time_t shift_start_time__s = 0l;         // Shift of the start time of the pattern
    time_t timestep__s = 3600l;             // Step of the pattern
};

inline bool is_valid_pto(const PatternTimeOptions& a_pto) {
    return a_pto.timestep__s > 0;
}

inline bool is_valid_pto(PatternTimeOptions* ap_pto) {
    return ap_pto != nullptr && is_valid_pto(*ap_pto);
}

// class TimeOptions;
struct SimulationTimeOptions {
    time_t shift_start_time__s = 0l;         // Shift of the start time of the simulation
    time_t duration__s = 0l;                 // Duration of the simulation
};

inline bool is_valid_sto(const SimulationTimeOptions& a_sto) {
    return a_sto.duration__s > 0;
}

inline bool is_valid_sto(SimulationTimeOptions* ap_sto) {
    return ap_sto != nullptr && is_valid_sto(*ap_sto);
}

} // namespace epanet
} // namespace bevarmejo
