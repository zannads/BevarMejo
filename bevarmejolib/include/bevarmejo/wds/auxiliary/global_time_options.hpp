#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "bevarmejo/wds/epanet_helpers/en_time_options.hpp"

namespace bevarmejo {

using time_t = bevarmejo::epanet::time_t;

namespace wds {
namespace aux {

// Forward declaration because we will keep track of time series create from this class
class TimeSeries;

class GlobalTimeOptions  {
private:
    time_t m__shift_start_time__s; // Shift for the start time of the simulation in seconds, but this is only for control rules and eventually for visualization. Simulations and times always start at 0.
    time_t m__duration__s;         // Duration of the simulation in seconds. This will be used for integration purposes in the time series.

    const std::unique_ptr< const TimeSeries> m__constant; // Special time series for constant values. Always existing and never changing.

    mutable std::vector<TimeSeries*> m__time_series; // Externally managed time series that depend on this global time options

public:
    // Constructors
    GlobalTimeOptions();
    GlobalTimeOptions(time_t a_duration__s);
    GlobalTimeOptions(time_t a_shift_start_time__s, time_t a_duration__s);

    // Copy and move constructors delete to maintain the uniqueness of the time series
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
    const TimeSeries& constant() const;
    std::size_t n_time_series() const;

    void shift_start_time__s(const time_t a_shift_start_time__s);
    void duration__s(const time_t a_duration__s);

    template <typename... Args>
    std::unique_ptr<TimeSeries> create_time_series(Args&&... args) const {
        auto p_time_series= std::make_unique<TimeSeries>(*this, std::forward<Args>(args)...);
        m__time_series.push_back(p_time_series.get());
        return p_time_series;
    }

private:
    void notify_time_series(); // When duration changes, all time series must be notified and if they are dynamic object they are shorten (hence the m__time_series attribute).

    friend class TimeSeries; // So that time series can notify the global time options when they are destroyed

    // registration of time series is not necessary, the time series will be registered at creation
    
    void remove_time_series(const TimeSeries* const ap_time_series) const;
    
}; // class GlobalTimeOptions

} // namespace aux
} // namespace wds
} // namespace bevarmejo
