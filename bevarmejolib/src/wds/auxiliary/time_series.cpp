#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "bevarmejo/wds/epanet_helpers/en_time_options.hpp"

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

// A time series is a monotonically increasing sequence of times, starting from 0
// and less than the duration of the simulation.
void TimeSeries::check_valid() const {
    if (m__time_steps.empty())
        throw std::invalid_argument("TimeSeries::check_valid: Time steps are empty.");

    if (m__time_steps.front() != 0)
        throw std::invalid_argument("TimeSeries::check_valid: Time steps do not start at 0.");

    if (!is_monotonic(m__time_steps))
        throw std::invalid_argument("TimeSeries::check_valid: Time steps are not monotonic.");

    if (m__time_steps.size() > 1 && m__time_steps.back() >= m__gto.duration__s())
        throw std::invalid_argument("TimeSeries::check_valid: Time steps are not within the duration.");
}

TimeSeries::TimeSeries(const epanet::GlobalTimeOptions& a_gto) : 
    m__gto(a_gto), 
    m__time_steps({0l}) 
{
    check_valid();
}

TimeSeries::TimeSeries(const epanet::GlobalTimeOptions &a_gto, std::initializer_list<time_t> ilist) :
    m__gto(a_gto),
    m__time_steps(ilist) 
{
    check_valid();
}

template <typename... Args>
TimeSeries::TimeSeries(const epanet::GlobalTimeOptions &a_gto, Args &&...args) : 
    m__gto(a_gto),
    m__time_steps(std::forward<Args>(args)...)
{
    check_valid();
}

TimeSeries& TimeSeries::operator= (const TimeSeries& other) {
    if (this != &other) {
        // m__gto = other.m__gto; // const reference cannot be assigned
        m__time_steps = other.m__time_steps;
        check_valid();
    }
    return *this;
}

TimeSeries& TimeSeries::operator= (TimeSeries&& other) noexcept {
    if (this != &other) {
        // m__gto = std::move(other.m__gto); // const reference cannot be assigned
        m__time_steps = std::move(other.m__time_steps);
        check_valid();
    }
    return *this;
}

TimeSeries::size_type TimeSeries::not_found() const noexcept { return this->m__time_steps.size() + 1; }

TimeSeries::value_type TimeSeries::at(size_type pos) {
    if (pos < m__time_steps.size())
        return m__time_steps[pos];

    if (pos == m__time_steps.size())
        return m__gto.duration__s();

    throw std::out_of_range("TimeSeries::at: index out of range");
}

const TimeSeries::value_type TimeSeries::at(size_type pos) const {
    if (pos < m__time_steps.size())
        return m__time_steps[pos];

    if (pos == m__time_steps.size())
        return m__gto.duration__s();

    throw std::out_of_range("TimeSeries::at: index out of range");
}

TimeSeries::value_type TimeSeries::front() { return m__time_steps.front(); }
const TimeSeries::value_type TimeSeries::front() const { return m__time_steps.front(); }

TimeSeries::value_type TimeSeries::back() { return m__gto.duration__s(); }
const TimeSeries::value_type TimeSeries::back() const { return m__gto.duration__s(); }

TimeSeries::iterator TimeSeries::begin() noexcept { return iterator(this, 0); }
TimeSeries::const_iterator TimeSeries::begin() const noexcept { return const_iterator(this, 0); }
TimeSeries::const_iterator TimeSeries::cbegin() const noexcept { return const_iterator(this, 0); }

TimeSeries::iterator TimeSeries::end() noexcept { return iterator(this, m__time_steps.size() + 1); }
TimeSeries::const_iterator TimeSeries::end() const noexcept { return const_iterator(this, m__time_steps.size() + 1); }
TimeSeries::const_iterator TimeSeries::cend() const noexcept { return const_iterator(this, m__time_steps.size() + 1); }

TimeSeries::reverse_iterator TimeSeries::rbegin() noexcept { return reverse_iterator(this, m__time_steps.size() + 1); }
TimeSeries::const_reverse_iterator TimeSeries::rbegin() const noexcept { return const_reverse_iterator(this, m__time_steps.size() + 1); }
TimeSeries::const_reverse_iterator TimeSeries::crbegin() const noexcept { return const_reverse_iterator(this, m__time_steps.size() + 1); }

TimeSeries::reverse_iterator TimeSeries::rend() noexcept { return reverse_iterator(this, 0); }
TimeSeries::const_reverse_iterator TimeSeries::rend() const noexcept { return const_reverse_iterator(this, 0); }
TimeSeries::const_reverse_iterator TimeSeries::crend() const noexcept { return const_reverse_iterator(this, 0); }

bool TimeSeries::empty() const noexcept { return m__time_steps.empty(); }

TimeSeries::size_type TimeSeries::inner_size() const noexcept { return m__time_steps.size(); }

TimeSeries::size_type TimeSeries::length() const noexcept { return m__time_steps.size() + 1; }

TimeSeries::size_type TimeSeries::max_size() const noexcept { return m__time_steps.max_size(); }

void TimeSeries::reserve(size_type new_cap) { m__time_steps.reserve(new_cap); }

TimeSeries::size_type TimeSeries::capacity() const noexcept { return m__time_steps.capacity(); }

/*----------------------------------------------------------------------------*/

                            /*--- Modifiers ---*/

/*----------------------------------------------------------------------------*/

void TimeSeries::reset() { m__time_steps.assign({0l}); }

void TimeSeries::commit(time_t time__s) {
    if (time__s > m__gto.duration__s())
        throw std::invalid_argument("TimeSeries::commit: Can not add time steps greater than the duration.");

    if (time__s == m__time_steps.back())
        return; // No need to commit anything

    if (time__s < m__time_steps.back())
        throw std::invalid_argument("TimeSeries::commit: Time steps must be monotonic.");

    m__time_steps.push_back(time__s);
}

void TimeSeries::rollback() {
    if (m__time_steps.size() > 1)
        m__time_steps.pop_back();
    else
        m__time_steps.assign({0l});
}

void TimeSeries::shrink_to_duration() {
    auto pos = this->lower_bound_pos(m__gto.duration__s());

    if (pos == 0) { // This should happen when the duration is 0 or less than the first time step.
        m__time_steps.assign({0l}); 
        return;
    }
    
    // If there is a time step equal to the duration, I need to keep it too.
    if (m__time_steps[pos] == m__gto.duration__s())
        ++pos;

    if (pos < m__time_steps.size())
        m__time_steps.resize(pos);

    // If pos == m__time_steps.size(), it means that the duration is bigger than the last time step.
    // In this case, I do nothing.
}

void TimeSeries::swap_time_steps(TimeSeries& other) noexcept { m__time_steps.swap(other.m__time_steps); }

/*----------------------------------------------------------------------------*/

                            /*--- Lookup ---*/

/*----------------------------------------------------------------------------*/
TimeSeries::size_type TimeSeries::count(time_t time__s) const { return find_pos(time__s) != not_found() ? 1 : 0; }

TimeSeries::iterator TimeSeries::find(time_t time__s) {
    size_type pos = find_pos(time__s);
    return iterator(this, pos);
}

TimeSeries::const_iterator TimeSeries::find(time_t time__s) const {
    size_type pos = find_pos(time__s);
    return const_iterator(this, pos);
}

TimeSeries::size_type TimeSeries::find_pos(time_t time__s) const {

    size_type pos = 0;
    while (pos < m__time_steps.size() ) {
        if (m__time_steps[pos] == time__s)
            return pos;
        ++pos;
    }
    // It was not in the time steps, last chance is that it is the end time.
    if (time__s == gto().duration__s())
        return m__time_steps.size();

    return not_found(); // Not found
}

TimeSeries::size_type TimeSeries::lower_bound_pos(time_t time__s) const {
    if (time__s < m__time_steps.front())
        return not_found();

    size_type pos = 0;
    while (pos < m__time_steps.size() - 1) {
        if (m__time_steps[pos+1] >= time__s)
            return pos;
        ++pos;
    }

    if (gto().duration__s() >= time__s)
        return m__time_steps.size();

    return pos;
}

TimeSeries::size_type TimeSeries::upper_bound_pos(time_t time__s) const {
    if (time__s < m__time_steps.front())
        return 0;

    return lower_bound_pos(time__s) + 1;
}

} // namespace aux
} // namespace wds
} // namespace bevarmejo
