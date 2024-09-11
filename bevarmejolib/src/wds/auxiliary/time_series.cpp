#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "bevarmejo/wds/auxiliary/global_time_options.hpp"

#include "time_series.hpp"

namespace bevarmejo {
namespace wds {
namespace aux {

bool is_monotonic(const TimeSteps& time_steps) {
    for (std::size_t i = 0; i < time_steps.size() - 1; ++i) {
        if (time_steps[i] >= time_steps[i + 1])
            return false;
    }
    return true;
}

// A time series is a monotonically increasing sequence of times, starting from 
// values greater than 0 (0 is treated as default value). The time steps can go 
// over the duration but I will print 
void TimeSeries::check_valid() const {
    if (m__time_steps.empty())
        return; // An empty time series is valid. It can be used to represent a constant.

    if (m__time_steps.front() < 0)
        throw std::invalid_argument("TimeSeries::check_valid: Time steps do not start with a value greater than 0.");

    if (!is_monotonic(m__time_steps))
        throw std::invalid_argument("TimeSeries::check_valid: Time steps are not monotonic.");

    // Inserting time steps greater than the duration is allowed, but I will print a warning.
    // TODO: Print a warning here.
    // if (m__time_steps.size() > 1 && m__time_steps.back() >= m__gto.duration__s())
    // ?? how do I keep logs in this library?
}

TimeSeries::TimeSeries(const GlobalTimeOptions& a_gto) : 
    TimeSeries(a_gto, TimeSteps()) 
{ }

TimeSeries::~TimeSeries() {
    // you have to de-register from the GTO
    m__gto.remove_time_series(this);
}

TimeSeries::size_type TimeSeries::not_found() const noexcept { return std::numeric_limits<size_type>::max(); }

TimeSeries::value_type TimeSeries::at(size_type pos) {
    if (pos == 0)
        return 0l;

    if (pos < this->size())
        return m__time_steps[pos-1];

    if (pos == this->size())
        return m__gto.duration__s();

    throw std::out_of_range("TimeSeries::at: index out of range");
}

const TimeSeries::value_type TimeSeries::at(size_type pos) const {
    if (pos == 0)
        return 0l;

    if (pos < this->size())
        return m__time_steps[pos-1];

    if (pos == this->size())
        return m__gto.duration__s();

    throw std::out_of_range("TimeSeries::at: index out of range");
}

TimeSeries::value_type TimeSeries::front() { return 0l; }
const TimeSeries::value_type TimeSeries::front() const { return 0l; }

TimeSeries::value_type TimeSeries::back() { return m__gto.duration__s(); }
const TimeSeries::value_type TimeSeries::back() const { return m__gto.duration__s(); }

TimeSeries::iterator TimeSeries::begin() noexcept { return iterator(this, 0); }
TimeSeries::const_iterator TimeSeries::begin() const noexcept { return const_iterator(this, 0); }
TimeSeries::const_iterator TimeSeries::cbegin() const noexcept { return const_iterator(this, 0); }

TimeSeries::iterator TimeSeries::end() noexcept { return iterator(this, n_time_steps()); }
TimeSeries::const_iterator TimeSeries::end() const noexcept { return const_iterator(this, n_time_steps()); }
TimeSeries::const_iterator TimeSeries::cend() const noexcept { return const_iterator(this, n_time_steps()); }

TimeSeries::reverse_iterator TimeSeries::rbegin() noexcept { return reverse_iterator(this, n_time_steps()); }
TimeSeries::const_reverse_iterator TimeSeries::rbegin() const noexcept { return const_reverse_iterator(this, n_time_steps()); }
TimeSeries::const_reverse_iterator TimeSeries::crbegin() const noexcept { return const_reverse_iterator(this, n_time_steps()); }

TimeSeries::reverse_iterator TimeSeries::rend() noexcept { return reverse_iterator(this, 0); }
TimeSeries::const_reverse_iterator TimeSeries::rend() const noexcept { return const_reverse_iterator(this, 0); }
TimeSeries::const_reverse_iterator TimeSeries::crend() const noexcept { return const_reverse_iterator(this, 0); }

bool TimeSeries::empty() const noexcept { return false; }

bool TimeSeries::inner_empty() const noexcept { return m__time_steps.empty(); }

TimeSeries::size_type TimeSeries::inner_size() const noexcept { return m__time_steps.size(); }

// The true size is defined as the number of time steps less or equal than the duration plus one for zero time.
// It is also the number of values that a QuantitySeries will need to be valid.
TimeSeries::size_type TimeSeries::size() const { 
    // Find the index of the first time step greater than the duration
    size_type idx = 0;
    while (idx < m__time_steps.size() && m__time_steps[idx] <= m__gto.duration__s())
        ++idx;

    return idx + 1;
}

TimeSeries::size_type TimeSeries::n_time_steps() const noexcept { return size() + 1; }

TimeSeries::size_type TimeSeries::length() const noexcept { return n_time_steps(); }

TimeSeries::size_type TimeSeries::max_size() const noexcept { return m__time_steps.max_size(); }

void TimeSeries::reserve(size_type new_cap) { m__time_steps.reserve(new_cap); }

TimeSeries::size_type TimeSeries::capacity() const noexcept { return m__time_steps.capacity(); }

/*----------------------------------------------------------------------------*/

                            /*--- Modifiers ---*/

/*----------------------------------------------------------------------------*/

void TimeSeries::clear() { m__time_steps.clear(); }

void TimeSeries::reset() { clear(); }

void TimeSeries::commit(time_t time__s) {
    if (m__time_steps.empty() && time__s <= 0)
        throw std::invalid_argument("TimeSeries::commit: Time steps must be monotonic.");

    if (time__s <= m__time_steps.back())
        throw std::invalid_argument("TimeSeries::commit: Time steps must be monotonic.");

    // I can insert time steps greater than the duration, but I will print a warning.
    // if (time__s > m__gto.duration__s())
        // TODO: print warning

    m__time_steps.push_back(time__s);
}

void TimeSeries::rollback() {
    if (!m__time_steps.empty()) { // otherwise is UB
        m__time_steps.pop_back();
        return;
    }

    // TODO: print warning
}

void TimeSeries::shrink_to_duration() {
    auto sz = this->size(); 

    if (sz == 1) {
        m__time_steps.clear();
        return;
    }

    if (sz < m__time_steps.size()) {
        m__time_steps.resize(sz - 1);
        return;
    }

    // No problem, the time steps are already within the duration.
}

void TimeSeries::swap_time_steps(TimeSeries& other) noexcept { m__time_steps.swap(other.m__time_steps); }

/*----------------------------------------------------------------------------*/

                            /*--- Lookup ---*/

/*----------------------------------------------------------------------------*/
TimeSeries::size_type TimeSeries::count(time_t time__s) const { return find_pos(time__s) != not_found() ? 1 : 0; }

TimeSeries::iterator TimeSeries::find(time_t time__s) { return iterator(this, find_pos(time__s)); }

TimeSeries::const_iterator TimeSeries::find(time_t time__s) const { return const_iterator(this, find_pos(time__s)); }

TimeSeries::size_type TimeSeries::find_pos(time_t time__s) const {

    if (time__s < 0)
        return not_found();

    if (time__s == 0)
        return 0;

    size_type pos = 0;
    while (pos < m__time_steps.size() && m__time_steps[pos] <= m__gto.duration__s()) {
        if (m__time_steps[pos] == time__s)
            return pos+1;
        ++pos;
    }
    // It was not in the time steps before the duration, last chance is that it is the end time.
    if (time__s == gto().duration__s())
        return pos+1;

    return not_found();
}

} // namespace aux
} // namespace wds
} // namespace bevarmejo
