#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "bevarmejo/utility/except.hpp"

#include "bevarmejo/utility/epanet/time.hpp"

namespace bevarmejo {

namespace time {
using time_t = bevarmejo::epanet::time_t;
using Instant= time_t;
using TimeSteps= std::vector<Instant>;

struct AbsoluteTime { };
struct RelativeTime { };

} // namespace time

namespace label {

static const std::string __CONSTANT_TS = "Constant";
static const std::string __RESULTS_TS = "Results";

} // namespace label 


namespace log {
namespace cname {
static const std::string global_times= "GlobalTimes";
} // namespace cname

namespace fname {
static const std::string create_time_series= "create_time_series";
} // namespace fname
    
} // namespace log

namespace wds {
namespace aux {

// Forward declaration because we will keep track of time series create from this class
class TimeSeries;

class GlobalTimes  {

public:
    using container= std::unordered_map<std::string, std::unique_ptr<TimeSeries>>;

private:
    time::Instant m__shift_start_time__s; // Shift for the start time of the simulation in seconds, but this is only for control rules and eventually for visualization. Simulations and times always start at 0.
    time::Instant m__duration__s;         // Duration of the simulation in seconds. This will be used for integration purposes in the time series.

    const std::unique_ptr<const TimeSeries> m__constant; // Special time series for constant values. Always existing and never changing.

    std::unique_ptr<TimeSeries> m__results; // Time series for the results of the simulations. Always existing and mutable by the user.

    container m__ud_time_series; // User defined time series.
    
public:
    // Constructors
    GlobalTimes();
    GlobalTimes(time::Instant a_duration__s);
    GlobalTimes(time::Instant a_shift_start_time__s, time::Instant a_duration__s);

    GlobalTimes(const GlobalTimes&);
    GlobalTimes(GlobalTimes&&);

    GlobalTimes& operator=(const GlobalTimes&);
    GlobalTimes& operator=(GlobalTimes&&);

    // Destructor default implementation
    ~GlobalTimes() = default;

// Getters
public:
    time::Instant shift_start_time__s() const;

    time::Instant duration__s() const;

    const TimeSeries& constant() const;

    TimeSeries& results();
    const TimeSeries& results() const;

    std::size_t n_time_series() const;

    TimeSeries& time_series(const std::string& name);
    const TimeSeries& time_series(const std::string& name) const;

// Setters
public:
    void shift_start_time__s(const time::Instant a_shift_start_time__s);

    void duration__s(const time::Instant a_duration__s);

    template <typename... Args>
    std::pair<container::iterator, bool> create_time_series(const std::string& name, Args&&... args)
    {
        beme_throw_if(name == label::__CONSTANT_TS || name == label::__RESULTS_TS, std::invalid_argument,
            "Impossible to create the requested time series.",
            "Time series name is reserved and cannot be used.",
            "Name: ", name);
        
        return m__ud_time_series.emplace(name, std::make_unique<TimeSeries>(*this, std::forward<Args>(args)...));
    }

    void discard_time_series(const std::string& name);

}; // class GlobalTimes

} // namespace aux
} // namespace wds
} // namespace bevarmejo
