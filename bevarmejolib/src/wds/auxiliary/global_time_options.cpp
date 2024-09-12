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

GlobalTimeOptions::GlobalTimeOptions() : 
    GlobalTimeOptions(0l, 0l)
{ }

GlobalTimeOptions::GlobalTimeOptions(time_t a_duration__s) : 
    GlobalTimeOptions(0l, a_duration__s) 
{ }

GlobalTimeOptions::GlobalTimeOptions(time_t a_shift_start_time__s, time_t a_duration__s) :
    m__shift_start_time__s(a_shift_start_time__s),
    m__duration__s(a_duration__s),
    m__constant(*this),
    m__results(*this),
    m__ud_time_series()
{
    if (m__duration__s < 0)
        throw std::invalid_argument("GlobalTimeOptions::GlobalTimeOptions: Duration must be greater than or equal to 0.");
}

GlobalTimeOptions::GlobalTimeOptions(const GlobalTimeOptions& other) :
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

GlobalTimeOptions::GlobalTimeOptions(GlobalTimeOptions&& other) :
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

time_t GlobalTimeOptions::shift_start_time__s() const { 
    return m__shift_start_time__s;
}

time_t GlobalTimeOptions::duration__s() const { 
    return m__duration__s; 
}
const time_t* GlobalTimeOptions::duration__s_ptr() const { 
    return &m__duration__s; 
}

const wds::aux::TimeSeries& GlobalTimeOptions::constant() const { 
    return m__constant; 
}

wds::aux::TimeSeries& GlobalTimeOptions::results() { 
    return m__results; 
}

const wds::aux::TimeSeries& GlobalTimeOptions::results() const { 
    return m__results; 
}

std::size_t GlobalTimeOptions::n_time_series() const { 
    return m__ud_time_series.size(); 
}

const wds::aux::TimeSeries& GlobalTimeOptions::time_series(const std::string& name) const {
    auto it= m__ud_time_series.find(name);
    if (it == m__ud_time_series.end())
        throw std::invalid_argument("GlobalTimeOptions::time_series: Time series not found.");
    
    return *it->second;
}

wds::aux::TimeSeries& GlobalTimeOptions::time_series(const std::string& name) {
    auto it= m__ud_time_series.find(name);
    if (it == m__ud_time_series.end())
        throw std::invalid_argument("GlobalTimeOptions::time_series: Time series not found.");
    
    return *it->second;
}

// Setters

void GlobalTimeOptions::shift_start_time__s(const time_t a_shift_start_time__s) { 
    m__shift_start_time__s = a_shift_start_time__s;
}

void GlobalTimeOptions::duration__s(const time_t a_duration__s) {
    if (a_duration__s < 0)
        throw std::invalid_argument("GlobalTimeOptions::duration__s: Duration must be greater than or equal to 0.");
    
    m__duration__s = a_duration__s;

    notify_time_series();
}

void GlobalTimeOptions::discard_time_series(const std::string& name) {
    auto it= m__ud_time_series.find(name);
    if (it == m__ud_time_series.end())
        throw std::invalid_argument("GlobalTimeOptions::discard_time_series: Time series not found.");
    
    m__ud_time_series.erase(it);
}

void GlobalTimeOptions::notify_time_series() {
    for (auto& [key, p_time_series] : m__ud_time_series) {
        // Should never be nullptr, but I will check anyway.
        assert(p_time_series != nullptr && "GlobalTimeOptions::notify_time_series: Time series is nullptr.");
        
        p_time_series->shrink_to_duration();
    }
}

} // namespace aux
} // namespace wds
} // namespace bevarmejo
