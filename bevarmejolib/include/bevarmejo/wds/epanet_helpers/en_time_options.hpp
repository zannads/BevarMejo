#pragma once

#include <cstddef>
#include <memory>
#include <vector>

namespace bevarmejo {

// Forward declaration
namespace wds {
namespace aux {
class TimeSeries;
} // namespace aux
} // namespace wds

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
    time_t m__shift_start_time__s; // Shift for the start time of the simulation in seconds, but this is only for control rules and eventually for visualization. Simulations and times always start at 0.
    time_t m__duration__s;         // Duration of the simulation in seconds

    const std::unique_ptr< const wds::aux::TimeSeries> m__constant; // Constant time series

    mutable std::vector<wds::aux::TimeSeries*> m__time_series; // Externally managed time series that depend on this global time options

public:
    // Constructors
    GlobalTimeOptions();
    GlobalTimeOptions(time_t a_duration__s);
    GlobalTimeOptions(time_t a_shift_start_time__s, time_t a_duration__s);

    // Copy and move constructors delete
    GlobalTimeOptions(const GlobalTimeOptions&) = delete;
    GlobalTimeOptions(GlobalTimeOptions&&) = delete;
    GlobalTimeOptions& operator=(const GlobalTimeOptions&) = delete;
    GlobalTimeOptions& operator=(GlobalTimeOptions&&) = delete;

    // Destructor default implementation
    ~GlobalTimeOptions() = default;

    // Clone method instead of copy constructor
    std::unique_ptr<GlobalTimeOptions> clone() const;

    // Setters and getters
public:
    time_t shift_start_time__s() const;
    time_t duration__s() const;
    const time_t* duration__s_ptr() const;
    const wds::aux::TimeSeries& constant() const;
    std::size_t n_time_series() const;

    void shift_start_time__s(const time_t a_shift_start_time__s);
    void duration__s(const time_t a_duration__s);
    
    // registration of time series is not necessary, the time series will be registered at creation
    void remove_time_series(const wds::aux::TimeSeries* const ap_time_series) const;

    template <typename... Args>
    std::unique_ptr<wds::aux::TimeSeries> create_time_series(Args&&... args) const {
        auto p_time_series= std::make_unique<wds::aux::TimeSeries>(*this, std::forward<Args>(args)...);
        m__time_series.push_back(p_time_series.get());
        return p_time_series;
    }

private:
    void notify_time_series();
    
}; // class GlobalTimeOptions

} // namespace wds
} // namespace bevarmejo
