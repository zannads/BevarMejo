#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "bevarmejo/wds/epanet_helpers/en_time_options.hpp"

namespace bevarmejo {

using time_t = bevarmejo::epanet::time_t;

namespace wds {
namespace aux {

// Forward declaration because we will keep track of time series create from this class
class TimeSeries;

class GlobalTimeOptions  {

public:
    using container= std::unordered_map<std::string, std::unique_ptr<TimeSeries>>;

private:
    time_t m__shift_start_time__s; // Shift for the start time of the simulation in seconds, but this is only for control rules and eventually for visualization. Simulations and times always start at 0.
    time_t m__duration__s;         // Duration of the simulation in seconds. This will be used for integration purposes in the time series.

    const TimeSeries m__constant; // Special time series for constant values. Always existing and never changing.

    TimeSeries m__results; // Time series for the results of the simulations.

    container m__ud_time_series; // User defined time series.
    
public:
    // Constructors
    GlobalTimeOptions();
    GlobalTimeOptions(time_t a_duration__s);
    GlobalTimeOptions(time_t a_shift_start_time__s, time_t a_duration__s);

    GlobalTimeOptions(const GlobalTimeOptions&);
    GlobalTimeOptions(GlobalTimeOptions&&);
    GlobalTimeOptions& operator=(const GlobalTimeOptions&);
    GlobalTimeOptions& operator=(GlobalTimeOptions&&);

    // Destructor default implementation
    ~GlobalTimeOptions() = default;

// Getters
public:
    time_t shift_start_time__s() const;

    time_t duration__s() const;
    const time_t* duration__s_ptr() const;

    const TimeSeries& constant() const;

    TimeSeries& results();
    const TimeSeries& results() const;

    std::size_t n_time_series() const;

    TimeSeries& time_series(const std::string& name);
    const TimeSeries& time_series(const std::string& name) const;

// Setters
public:
    void shift_start_time__s(const time_t a_shift_start_time__s);

    void duration__s(const time_t a_duration__s);

    template <typename... Args>
    std::pair<container::iterator, bool> create_time_series(const std::string& name, Args&&... args) const {
        return m__ud_time_series.emplace(name, std::make_unique<TimeSeries>(*this, std::forward<Args>(args)...));
    }

    void discard_time_series(const std::string& name);

private:
    void notify_time_series(); // When duration changes, all time series must be notified and if they are dynamic object they are shortened.
    
}; // class GlobalTimeOptions

} // namespace aux
} // namespace wds
} // namespace bevarmejo
