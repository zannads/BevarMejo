#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "bevarmejo/wds/auxiliary/global_times.hpp"

#include "time_series.hpp"

namespace bevarmejo {
namespace wds {
namespace aux {

// Empty TimeSteps are always monotonic, starting after zero, and ending before t.

bool is_monotonic(const TimeSteps& time_steps) {
    for (std::size_t i = 0; i < time_steps.size() - 1; ++i) {
        if (time_steps[i] >= time_steps[i + 1])
            return false;
    }
    return true;
}

bool starts_after_zero(const TimeSteps& time_steps) {
    return time_steps.empty() || time_steps.front() > 0;
}

bool ends_before_t(const TimeSteps& time_steps, time_t t) {
    return time_steps.empty() || time_steps.back() < t;
}

// A time series is a monotonically increasing sequence of times, starting from 
// values greater than 0 (0 is treated as default value). The time steps can go 
// over the duration but I will print 
void TimeSeries::check_valid() const {
    if (!is_monotonic(m__time_steps) || !starts_after_zero(m__time_steps) || !ends_before_t(m__time_steps, m__gto.duration__s()))
        throw std::invalid_argument("TimeSeries::check_valid: Time steps are not valid.");
        
}

/*----------------------------------------------------------------------------*/

                            /*--- Constructors ---*/

/*----------------------------------------------------------------------------*/

TimeSeries::TimeSeries(const GlobalTimes& a_gto) : 
    TimeSeries(a_gto, TimeSteps()) 
{ }

/*----------------------------------------------------------------------------*/

                            /*--- Get and Set ---*/

/*----------------------------------------------------------------------------*/

const TimeSeries::container& TimeSeries::time_steps() const { return m__time_steps; }

/*----------------------------------------------------------------------------*/

                            /*--- Element access ---*/

/*----------------------------------------------------------------------------*/

TimeSeries::value_type TimeSeries::at(size_type pos) {
    return const_cast<const TimeSeries*>(this)->at(pos);
}

const TimeSeries::value_type TimeSeries::at(size_type pos) const {
    if (pos == 0)
        return 0l;

    if (pos < this->size()-1)
        return m__time_steps[pos-1];

    if (pos == this->size()-1)
        return m__gto.duration__s();

    throw std::out_of_range("TimeSeries::at: index out of range");
}

TimeSeries::value_type TimeSeries::front() { return 0l; }
const TimeSeries::value_type TimeSeries::front() const { return 0l; }

TimeSeries::value_type TimeSeries::back() { return m__gto.duration__s(); }
const TimeSeries::value_type TimeSeries::back() const { return m__gto.duration__s(); }

/*----------------------------------------------------------------------------*/

                            /*--- Iterators ---*/

/*----------------------------------------------------------------------------*/

TimeSeries::iterator TimeSeries::begin() noexcept { return iterator(this, 0); }
TimeSeries::const_iterator TimeSeries::begin() const noexcept { return const_iterator(this, 0); }
TimeSeries::const_iterator TimeSeries::cbegin() const noexcept { return const_iterator(this, 0); }

TimeSeries::iterator TimeSeries::end() noexcept { return iterator(this, this->size()); }
TimeSeries::const_iterator TimeSeries::end() const noexcept { return const_iterator(this, this->size()); }
TimeSeries::const_iterator TimeSeries::cend() const noexcept { return const_iterator(this, this->size()); }

TimeSeries::reverse_iterator TimeSeries::rbegin() noexcept { return reverse_iterator(this, this->size()); }
TimeSeries::const_reverse_iterator TimeSeries::rbegin() const noexcept { return const_reverse_iterator(this, this->size()); }
TimeSeries::const_reverse_iterator TimeSeries::crbegin() const noexcept { return const_reverse_iterator(this, this->size()); }

TimeSeries::reverse_iterator TimeSeries::rend() noexcept { return reverse_iterator(this, 0); }
TimeSeries::const_reverse_iterator TimeSeries::rend() const noexcept { return const_reverse_iterator(this, 0); }
TimeSeries::const_reverse_iterator TimeSeries::crend() const noexcept { return const_reverse_iterator(this, 0); }

/*----------------------------------------------------------------------------*/

                            /*--- Capacity ---*/

/*----------------------------------------------------------------------------*/

bool TimeSeries::empty() const noexcept { return false; }

TimeSeries::size_type TimeSeries::size() const noexcept { return m__time_steps.size() + 2; } // 0 and duration__s() are not stored.    

TimeSeries::size_type TimeSeries::max_size() const noexcept { return m__time_steps.max_size(); }

void TimeSeries::reserve(size_type new_cap) { m__time_steps.reserve(new_cap); }

TimeSeries::size_type TimeSeries::capacity() const noexcept { return m__time_steps.capacity(); }

void TimeSeries::shrink_to_duration() {
    
    // Remove all time steps that are greater than the duration. Equal to duration is allowed.
    while (!m__time_steps.empty() && m__time_steps.back() > m__gto.duration__s())
        m__time_steps.pop_back();
}

void TimeSeries::shrink_before_duration() {
    
    // Remove all time steps that are greater than the duration. Equal to duration is not allowed.
    while (!m__time_steps.empty() && m__time_steps.back() >= m__gto.duration__s())
        m__time_steps.pop_back();
}

/*----------------------------------------------------------------------------*/

                            /*--- Modifiers ---*/

/*----------------------------------------------------------------------------*/

void TimeSeries::reset() noexcept { m__time_steps.clear(); }

void TimeSeries::commit(time_t time__s) {
    if ( time__s < 0 || time__s > m__gto.duration__s() )
        throw std::invalid_argument("TimeSeries::commit: Time steps must be >= 0 && <= duration of the simulation.");

    if (m__time_steps.empty() && time__s == 0)
        return; // No problem if you commit the zero time as the first time step. As I expect to commit the zero in every simulation.

    if (!m__time_steps.empty() && time__s <= m__time_steps.back())
        throw std::invalid_argument("TimeSeries::commit: Time steps must be monotonic.");

    m__time_steps.push_back(time__s);
}

/*----------------------------------------------------------------------------*/

                            /*--- Lookup ---*/

/*----------------------------------------------------------------------------*/

TimeSeries::iterator TimeSeries::find(time_t time__s) { return iterator(this, find_pos(time__s)); }

TimeSeries::const_iterator TimeSeries::find(time_t time__s) const { return const_iterator(this, find_pos(time__s)); }

TimeSeries::size_type TimeSeries::find_pos(time_t time__s) const {

    if (time__s < 0)
        return size();

    if (time__s == 0)
        return 0;

    size_type pos = 0;
    while (pos < m__time_steps.size() && m__time_steps[pos] <= m__gto.duration__s()) {
        if (m__time_steps[pos] == time__s)
            return pos+1;
        ++pos;
    }
    // It was not in the time steps before the duration, last chance is that it is the end time.
    if (time__s == m__gto.duration__s())
        return pos+1;

    return size();
}

} // namespace aux
} // namespace wds
} // namespace bevarmejo
