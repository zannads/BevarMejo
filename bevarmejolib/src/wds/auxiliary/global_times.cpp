#include <cassert>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

#include "global_times.hpp"

#include "bevarmejo/wds/auxiliary/time_series.hpp"

namespace bevarmejo {

namespace log {
namespace fname {
static const std::string time_series= "time_series";
static const std::string duration= "duration__s";
static const std::string discard_time_series= "discard_time_series";
} // namespace fname 
} // namespace log 
 

namespace wds {
namespace aux {

// Constructors 

GlobalTimes::GlobalTimes() : 
    GlobalTimes(0l, 0l)
{ }

GlobalTimes::GlobalTimes(time::Instant a_duration__s) : 
    GlobalTimes(0l, a_duration__s) 
{ }

GlobalTimes::GlobalTimes(time::Instant a_shift_start_time__s, time::Instant a_duration__s) :
    m__shift_start_time__s(a_shift_start_time__s),
    m__duration__s(a_duration__s),
    m__constant(std::make_unique<TimeSeries>(*this)),
    m__results(std::make_unique<TimeSeries>(*this)),
    m__ud_time_series()
{
    beme_throw_if(m__duration__s < 0, std::invalid_argument,
        "Impossible to create the GlobalTimes object.",
        "Duration must be greater than or equal to 0.",
        "Duration: ", a_duration__s);
}

GlobalTimes::GlobalTimes(const GlobalTimes& other) :
    m__shift_start_time__s(other.m__shift_start_time__s),
    m__duration__s(other.m__duration__s),
    m__constant(std::make_unique<TimeSeries>(*this)),
    m__results(std::make_unique<TimeSeries>(*this, *other.m__results)),
    m__ud_time_series()
{
    for (const auto& [key, p_time_series] : other.m__ud_time_series) {
        m__ud_time_series[key] = std::make_unique<TimeSeries>(*this, *p_time_series);
    }
}

GlobalTimes::GlobalTimes(GlobalTimes&& other) :
    m__shift_start_time__s(other.m__shift_start_time__s),
    m__duration__s(other.m__duration__s),
    m__constant(std::make_unique<TimeSeries>(*this)),
    m__results(std::make_unique<TimeSeries>(*this, std::move(*other.m__results))),
    m__ud_time_series()
{
    for (auto& [key, p_time_series] : other.m__ud_time_series) {
        m__ud_time_series[key] = std::make_unique<TimeSeries>(*this, std::move(*p_time_series));
    }
}

GlobalTimes& GlobalTimes::operator=(const GlobalTimes& other) { 
    if (this == &other)
        return *this;

    m__shift_start_time__s = other.m__shift_start_time__s;

    m__duration__s = other.m__duration__s;

    // m__constant no need to copy because it's always the same
    
    // I can simply copy the results but creating a new object so that the old is destroyed with the unique_ptr substitution.
    m__results = std::make_unique<TimeSeries>(*this, other.m__results->time_steps());

    // Copying the others, I could reassign the timesteps to the same ones, delete the non existing in other and create 
    // new ones for the non existing in this, but it's easier to just clear and copy.
    m__ud_time_series.clear();
    m__ud_time_series.reserve(other.m__ud_time_series.size());

    for (const auto& [key, p_time_series] : other.m__ud_time_series) {
        // First allocate and then actully copy the TimeSteps.
        m__ud_time_series[key] = std::make_unique<TimeSeries>(*this);
        *m__ud_time_series[key] = *p_time_series;
    }

    return *this;
}

GlobalTimes& GlobalTimes::operator=(GlobalTimes&& other) { 
    if (this == &other)
        return *this;

    m__shift_start_time__s = other.m__shift_start_time__s;

    m__duration__s = other.m__duration__s;

    // m__constant no need to move because it's always the same

    *m__results = std::move(*other.m__results);

    // Moving the other time series, I can't simply move the map because the uniqeu_ptr<TimeSeries> would still have 
    // the old reference to the other GlobalTimes object. As in the copy assignment, I will clear and copy.

    m__ud_time_series.clear();
    m__ud_time_series.reserve(other.m__ud_time_series.size());

    for (auto& [key, p_time_series] : other.m__ud_time_series) {
        // First allocate and then actully move the TimeSteps.
        m__ud_time_series[key] = std::make_unique<TimeSeries>(*this);
        *m__ud_time_series[key] = std::move(*p_time_series);
    }

    return *this;
} 

// Getters 

time::Instant GlobalTimes::shift_start_time__s() const { 
    return m__shift_start_time__s;
}

time::Instant GlobalTimes::duration__s() const { 
    return m__duration__s; 
}

const wds::aux::TimeSeries& GlobalTimes::constant() const { 
    return *m__constant; 
}

wds::aux::TimeSeries& GlobalTimes::results() { 
    return *m__results; 
}

const wds::aux::TimeSeries& GlobalTimes::results() const { 
    return *m__results; 
}

std::size_t GlobalTimes::n_time_series() const { 
    return m__ud_time_series.size(); 
}

const wds::aux::TimeSeries& GlobalTimes::time_series(const std::string& name) const
{

    if (name == label::__CONSTANT_TS)
        return constant();

    if (name == label::__RESULTS_TS)
        return results();

    auto it = m__ud_time_series.find(name);
    beme_throw_if(it == m__ud_time_series.end(), std::invalid_argument,
        "Impossible to access the requested time series.",
        "Time series not found.",
        "TimeSeries name: ", name);
    
    return *it->second;
}

wds::aux::TimeSeries& GlobalTimes::time_series(const std::string& name)
{
    beme_throw_if(name == label::__CONSTANT_TS, std::invalid_argument,
        "Impossible to access the requested time series.",
        "Cannot access the constant time series from a non const GlobalTimes object.");
    
    if (name == label::__RESULTS_TS)
        return results();

    auto it= m__ud_time_series.find(name);
    beme_throw_if(it == m__ud_time_series.end(), std::invalid_argument,
        "Impossible to access the requested time series.",
        "Time series not found.",
        "TimeSeries name: ", name);
    
    return *it->second;
}

// Setters

void GlobalTimes::shift_start_time__s(const time::Instant a_shift_start_time__s)
{
    m__shift_start_time__s = a_shift_start_time__s;
}

void GlobalTimes::duration__s(const time::Instant a_duration__s)
{
    beme_throw_if(a_duration__s < 0, std::invalid_argument,
        "Impossible to set the duration of the GlobalTimes object.",
        "Duration must be greater than or equal to 0.",
        "Duration: ", a_duration__s);
                                                    
    m__duration__s = a_duration__s;
}

void GlobalTimes::discard_time_series(const std::string& name)
{
    beme_throw_if(name == label::__CONSTANT_TS || name == label::__RESULTS_TS, std::invalid_argument,
        "Impossible to discard the requested time series.",
        "Time series name is reserved and cannot be used.",
        "TimeSeries name: ", name);
                     
    auto it= m__ud_time_series.find(name);
    if (it == m__ud_time_series.end())
        return; // Nothing to do.

    m__ud_time_series.erase(it);
}

} // namespace aux
} // namespace wds
} // namespace bevarmejo
