#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>

#include "bevarmejo/wds/auxiliary/time_series.hpp"

#include "en_time_options.hpp"

namespace bevarmejo {
namespace epanet {

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
    m__constant(std::make_unique<const wds::aux::TimeSeries>(*this)),
    m__time_series()
{
    if (m__duration__s < 0)
        throw std::invalid_argument("GlobalTimeOptions::GlobalTimeOptions: Duration must be greater than or equal to 0.");
}

std::unique_ptr<GlobalTimeOptions> GlobalTimeOptions::clone() const {
    return std::make_unique<GlobalTimeOptions>(m__shift_start_time__s, m__duration__s);
}


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
    return *m__constant; 
}

std::size_t GlobalTimeOptions::n_time_series() const { 
    return m__time_series.size(); 
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

void GlobalTimeOptions::remove_time_series(const wds::aux::TimeSeries* const ap_time_series) const {
    auto it= std::find(m__time_series.begin(), m__time_series.end(), ap_time_series);
    if (it != m__time_series.end())
        m__time_series.erase(it);
}

void GlobalTimeOptions::notify_time_series() {
    for (auto p_time_series : m__time_series) {
        
        if (p_time_series)
            p_time_series->shrink_to_duration();
    }
}

} // namespace epanet
} // namespace bevarmejo
