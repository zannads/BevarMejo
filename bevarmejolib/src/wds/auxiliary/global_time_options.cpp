#include <cassert>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

#include "bevarmejo/wds/auxiliary/time_series.hpp"

#include "global_time_options.hpp"

namespace bevarmejo {
namespace wds {
namespace aux {

// Constructors 

GlobalTimes::GlobalTimes() : 
    GlobalTimes(0l, 0l)
{ }

GlobalTimes::GlobalTimes(time_t a_duration__s) : 
    GlobalTimes(0l, a_duration__s) 
{ }

GlobalTimes::GlobalTimes(time_t a_shift_start_time__s, time_t a_duration__s) :
    m__shift_start_time__s(a_shift_start_time__s),
    m__duration__s(a_duration__s),
    m__constant(*this),
    m__results(*this),
    m__ud_time_series()
{
    if (m__duration__s < 0)
        throw std::invalid_argument("GlobalTimes::GlobalTimes: Duration must be greater than or equal to 0.");
}

GlobalTimes::GlobalTimes(const GlobalTimes& other) :
    m__shift_start_time__s(other.m__shift_start_time__s),
    m__duration__s(other.m__duration__s),
    m__constant(*this),
    m__results(*this, other.m__results.time_steps()),
    m__ud_time_series()
{
    for (const auto& [key, p_time_series] : other.m__ud_time_series) {
        m__ud_time_series[key] = std::make_unique<TimeSeries>(*this, p_time_series->time_steps());
    }
}

GlobalTimes::GlobalTimes(GlobalTimes&& other) :
    m__shift_start_time__s(other.m__shift_start_time__s),
    m__duration__s(other.m__duration__s),
    m__constant(*this),
    m__results(*this, std::move(other.m__results.m__time_steps)),
    m__ud_time_series()
{
    for (auto& [key, p_time_series] : other.m__ud_time_series) {
        m__ud_time_series[key] = std::make_unique<TimeSeries>(*this, std::move(p_time_series->m__time_steps));
    }
}

// TODO: Implement the copy and move assignment operators.

// Getters 

time_t GlobalTimes::shift_start_time__s() const { 
    return m__shift_start_time__s;
}

time_t GlobalTimes::duration__s() const { 
    return m__duration__s; 
}
const time_t* GlobalTimes::duration__s_ptr() const { 
    return &m__duration__s; 
}

const wds::aux::TimeSeries& GlobalTimes::constant() const { 
    return m__constant; 
}

wds::aux::TimeSeries& GlobalTimes::results() { 
    return m__results; 
}

const wds::aux::TimeSeries& GlobalTimes::results() const { 
    return m__results; 
}

std::size_t GlobalTimes::n_time_series() const { 
    return m__ud_time_series.size(); 
}

const wds::aux::TimeSeries& GlobalTimes::time_series(const std::string& name) const {
    auto it= m__ud_time_series.find(name);
    if (it == m__ud_time_series.end())
        throw std::invalid_argument("GlobalTimes::time_series: Time series not found.");
    
    return *it->second;
}

wds::aux::TimeSeries& GlobalTimes::time_series(const std::string& name) {
    auto it= m__ud_time_series.find(name);
    if (it == m__ud_time_series.end())
        throw std::invalid_argument("GlobalTimes::time_series: Time series not found.");
    
    return *it->second;
}

// Setters

void GlobalTimes::shift_start_time__s(const time_t a_shift_start_time__s) { 
    m__shift_start_time__s = a_shift_start_time__s;
}

void GlobalTimes::duration__s(const time_t a_duration__s) {
    if (a_duration__s < 0)
        throw std::invalid_argument("GlobalTimes::duration__s: Duration must be greater than or equal to 0.");
    
    m__duration__s = a_duration__s;

    notify_time_series();
}

void GlobalTimes::discard_time_series(const std::string& name) {
    auto it= m__ud_time_series.find(name);
    if (it == m__ud_time_series.end())
        throw std::invalid_argument("GlobalTimes::discard_time_series: Time series not found.");
    
    m__ud_time_series.erase(it);
}

void GlobalTimes::notify_time_series() {
    for (auto& [key, p_time_series] : m__ud_time_series) {
        // Should never be nullptr, but I will check anyway.
        assert(p_time_series != nullptr && "GlobalTimes::notify_time_series: Time series is nullptr.");
        
        p_time_series->shrink_to_duration();
    }
}

} // namespace aux
} // namespace wds
} // namespace bevarmejo
