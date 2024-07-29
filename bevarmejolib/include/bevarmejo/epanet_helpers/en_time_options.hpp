#ifndef BEVARMEJOLIB__WDS_DATA_STRUCTURES__TIME_OPTIONS_HPP
#define BEVARMEJOLIB__WDS_DATA_STRUCTURES__TIME_OPTIONS_HPP

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

class GlobalTimeOptions  {
private:
    time_t m__shift_start_time__s = 0l; // Shift for the start time of the simulation in seconds, but this is only for control rules and eventually for visualization. Simulations and times always start at 0.
    time_t m__duration__s = 0l;         // Duration of the simulation in seconds

public:
    // Constructors
    GlobalTimeOptions() = default;
    GlobalTimeOptions(time_t a_duration__s) :
        m__shift_start_time__s(0) {
        if (a_duration__s >= 0) 
            m__duration__s = a_duration__s;
        else
            throw std::invalid_argument("GlobalTimeOptions::GlobalTimeOptions: Duration must be greater than or equal to 0.");
    }
    GlobalTimeOptions(time_t a_shift_start_time__s, time_t a_duration__s) {
        if (a_duration__s >= 0) 
            m__duration__s = a_duration__s;
        else
            throw std::invalid_argument("GlobalTimeOptions::GlobalTimeOptions: Duration must be greater than or equal to 0.");
        m__shift_start_time__s = a_shift_start_time__s;
    }

    // Copy and move constructors default
    GlobalTimeOptions(const GlobalTimeOptions&) = default;
    GlobalTimeOptions(GlobalTimeOptions&&) = default;
    GlobalTimeOptions& operator=(const GlobalTimeOptions&) = default;
    GlobalTimeOptions& operator=(GlobalTimeOptions&&) = default;
    ~GlobalTimeOptions() = default;

    // Setters and getters
public:
    time_t shift_start_time__s() const { return m__shift_start_time__s; }
    time_t duration__s() const { return m__duration__s; }

    void shift_start_time__s(time_t a_shift_start_time__s) { m__shift_start_time__s = a_shift_start_time__s; }

    void duration__s(time_t a_duration__s) {
        if (a_duration__s >= 0) 
            m__duration__s = a_duration__s;
        else
            throw std::invalid_argument("GlobalTimeOptions::duration__s: Duration must be greater than or equal to 0.");
    }
    
}; // class GlobalTimeOptions

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_DATA_STRUCTURES__TIME_OPTIONS_HPP
