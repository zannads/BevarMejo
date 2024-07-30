#include <cassert>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "time_series.hpp"

namespace bevarmejo {

bool is_monotonic(const TimeSteps& time_steps) {
    for (std::size_t i = 0; i < time_steps.size() - 1; ++i) {
        if (time_steps[i] >= time_steps[i + 1])
            return false;
    }
    return true;
}

void TimeSeries::check_valid() const {
    if (m__time_steps.empty())
        throw std::invalid_argument("TimeSeries::check_valid: Time steps are empty.");

    if (m__time_steps.front() != 0)
        throw std::invalid_argument("TimeSeries::check_valid: Time steps do not start at 0.");

    if (!is_monotonic(m__time_steps))
        throw std::invalid_argument("TimeSeries::check_valid: Time steps are not monotonic.");

    if (m__time_steps.back() > m__gto.duration__s())
        throw std::invalid_argument("TimeSeries::check_valid: Time steps are not within the duration.");
}

TimeSeries::TimeSeries(const epanet::GlobalTimeOptions& a_gto) : m__gto(a_gto), m__time_steps({0l}) {
    check_valid();
}

template <typename... Args>
TimeSeries::TimeSeries(const epanet::GlobalTimeOptions& a_gto, Args&&... args) : m__gto(a_gto), m__time_steps(std::forward<Args>(args)...) {
    check_valid();
}

TimeSeries::size_type TimeSeries::not_found() const noexcept { return this->m__time_steps.size() + 1; }

TimeSeries::value_type TimeSeries::at(size_type pos)
{
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

constexpr bool TimeSeries::empty() const noexcept { return false; }

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

    if (time__s <= m__time_steps.back())
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

/*----------------------------------------------------------------------------*/

                            /*--- Iterators ---*/

/*----------------------------------------------------------------------------*/
TimeSeries::iterator::iterator(TimeSeries *time_series, size_type index) : 
    m__time_series(time_series), m__index(index) {}

TimeSeries::iterator::value_type TimeSeries::iterator::operator*() const {
    assert(m__index < m__time_series->length() && "Dereferencing end iterator");

    // Special case: index == size(), I return the end time.
    if (m__index == m__time_series->inner_size())
        return m__time_series->m__gto.duration__s();

    // Default case: 
    // Let dereference the end iterator be undefined behaviour in Release mode.
    return m__time_series->m__time_steps[m__index];
}

TimeSeries::iterator::pointer TimeSeries::iterator::operator->() const {
    assert(m__index < m__time_series->length() && "Dereferencing end iterator");

    if (m__index == m__time_series->inner_size())
        return m__time_series->m__gto.duration__s_ptr();

    return &m__time_series->m__time_steps[m__index];
}

TimeSeries::iterator::value_type TimeSeries::iterator::operator[](difference_type n) const {
    return *(*this + n);
}

TimeSeries::iterator& TimeSeries::iterator::operator++() {
    assert(m__index < m__time_series->length() && "Incrementing end iterator");

    if (m__index < m__time_series->length())
        ++m__index;
    return *this;
}

TimeSeries::iterator TimeSeries::iterator::operator++(int) {
    assert(m__index < m__time_series->length() && "Incrementing end iterator");

    iterator tmp = *this;
    if (m__index < m__time_series->length())
        ++m__index;
    return tmp;
}

TimeSeries::iterator& TimeSeries::iterator::operator--() {
    assert(m__index > 0 && "Decrementing begin iterator");

    if (m__index > 0)
        --m__index;
    return *this;
}

TimeSeries::iterator TimeSeries::iterator::operator--(int) {
    assert(m__index > 0 && "Decrementing begin iterator");

    iterator tmp = *this;
    if (m__index > 0)
        --m__index;
    return tmp;
}

TimeSeries::iterator& TimeSeries::iterator::operator+=(difference_type n) {
    assert(m__index + n <= m__time_series->length() && "Going out of bounds");
    
    // Overflow check
    if (n < 0 && m__index < -n)
        m__index = 0;
    else {
        size_type ub = m__time_series->length();
        m__index = (m__index + n > ub) ? ub : m__index + n;
    }   

    return *this;
}

TimeSeries::iterator TimeSeries::iterator::operator+(difference_type n) const {
    iterator tmp= *this;
    return tmp += n;
}

TimeSeries::iterator& TimeSeries::iterator::operator-=(difference_type n) {
    return (*this += -n);
}

TimeSeries::iterator TimeSeries::iterator::operator-(difference_type n) const {
    return (*this + -n);
}

TimeSeries::iterator::difference_type TimeSeries::iterator::operator-(const iterator& other) const {
    return m__index - other.m__index;
}

bool TimeSeries::iterator::operator==(const iterator& other) const {
    return m__index == other.m__index;
}

bool TimeSeries::iterator::operator!=(const iterator& other) const {
    return m__index != other.m__index;
}

bool TimeSeries::iterator::operator<(const iterator& other) const {
    return m__index < other.m__index;
}

bool TimeSeries::iterator::operator>(const iterator& other) const {
    return m__index > other.m__index;
}

bool TimeSeries::iterator::operator<=(const iterator& other) const {
    return m__index <= other.m__index;
}

bool TimeSeries::iterator::operator>=(const iterator& other) const {
    return m__index >= other.m__index;
}

/*----------------------------------------------------------------------------*/

                            /*--- Const Iterators ---*/

/*----------------------------------------------------------------------------*/

TimeSeries::const_iterator::const_iterator(const TimeSeries *time_series, size_type index) : 
    m__time_series(time_series), m__index(index) {}

TimeSeries::const_iterator::value_type TimeSeries::const_iterator::operator*() const {
    assert(m__index < m__time_series->length() && "Dereferencing end iterator");

    // Special case: index == size(), I return the end time.
    if (m__index == m__time_series->inner_size())
        return m__time_series->m__gto.duration__s();

    // Default case: 
    // Let dereference the end iterator be undefined behaviour in Release mode.
    return m__time_series->m__time_steps[m__index];
}

TimeSeries::const_iterator::pointer TimeSeries::const_iterator::operator->() const {
    assert(m__index < m__time_series->length() && "Dereferencing end iterator");

    if (m__index == m__time_series->inner_size())
        return m__time_series->m__gto.duration__s_ptr();

    return &m__time_series->m__time_steps[m__index];
}

TimeSeries::const_iterator::value_type TimeSeries::const_iterator::operator[](difference_type n) const {
    return *(*this + n);
}

TimeSeries::const_iterator& TimeSeries::const_iterator::operator++() {
    assert(m__index < m__time_series->length() && "Incrementing end iterator");

    if (m__index < m__time_series->length())
        ++m__index;
    return *this;
}

TimeSeries::const_iterator TimeSeries::const_iterator::operator++(int) {
    assert(m__index < m__time_series->length() && "Incrementing end iterator");

    const_iterator tmp = *this;
    if (m__index < m__time_series->length())
        ++m__index;
    return tmp;
}

TimeSeries::const_iterator& TimeSeries::const_iterator::operator--() {
    assert(m__index > 0 && "Decrementing begin iterator");

    if (m__index > 0)
        --m__index;
    return *this;
}

TimeSeries::const_iterator TimeSeries::const_iterator::operator--(int) {
    assert(m__index > 0 && "Decrementing begin iterator");

    const_iterator tmp = *this;
    if (m__index > 0)
        --m__index;
    return tmp;
}

TimeSeries::const_iterator& TimeSeries::const_iterator::operator+=(difference_type n) {
    assert(m__index + n <= m__time_series->length() && "Going out of bounds");
    
    // Overflow check
    if (n < 0 && m__index < -n)
        m__index = 0;
    else {
        size_type ub = m__time_series->length();
        m__index = (m__index + n > ub) ? ub : m__index + n;
    }   

    return *this;
}

TimeSeries::const_iterator TimeSeries::const_iterator::operator+(difference_type n) const {
    const_iterator tmp= *this;
    return tmp += n;
}

TimeSeries::const_iterator& TimeSeries::const_iterator::operator-=(difference_type n) {
    return (*this += -n);
}

TimeSeries::const_iterator TimeSeries::const_iterator::operator-(difference_type n) const {
    return (*this + -n);
}

TimeSeries::const_iterator::difference_type TimeSeries::const_iterator::operator-(const const_iterator& other) const {
    return m__index - other.m__index;
}

bool TimeSeries::const_iterator::operator==(const const_iterator& other) const {
    return m__index == other.m__index;
}

bool TimeSeries::const_iterator::operator!=(const const_iterator& other) const {
    return m__index != other.m__index;
}

bool TimeSeries::const_iterator::operator<(const const_iterator& other) const {
    return m__index < other.m__index;
}

bool TimeSeries::const_iterator::operator>(const const_iterator& other) const {
    return m__index > other.m__index;
}

bool TimeSeries::const_iterator::operator<=(const const_iterator& other) const {
    return m__index <= other.m__index;
}

bool TimeSeries::const_iterator::operator>=(const const_iterator& other) const {
    return m__index >= other.m__index;
}

/*----------------------------------------------------------------------------*/

                            /*--- Reverse Iterators ---*/

/*----------------------------------------------------------------------------*/
// Reverse iterator stores an iterator to the next element than the one it actually refers to.

TimeSeries::reverse_iterator::reverse_iterator(TimeSeries *time_series, size_type index) : 
    m__time_series(time_series), m__index(index) {}

TimeSeries::reverse_iterator::value_type TimeSeries::reverse_iterator::operator*() const {
    assert(m__index > 0 && m__index <= m__time_series->length() && "Dereferencing end iterator");

    if (m__index == m__time_series->inner_size()+1)
        return m__time_series->m__gto.duration__s();

    // Dereferencing the rend iterator (m__index == 0) is undefined behaviour.
    return m__time_series->m__time_steps[m__index-1];
}

TimeSeries::reverse_iterator::pointer TimeSeries::reverse_iterator::operator->() const {
    assert(m__index > 0 && m__index <= m__time_series->length() && "Dereferencing end iterator");

    if (m__index == m__time_series->inner_size()+1)
        return m__time_series->m__gto.duration__s_ptr();

    return &m__time_series->m__time_steps[m__index-1];
}

TimeSeries::reverse_iterator::value_type TimeSeries::reverse_iterator::operator[](difference_type n) const {
    return *(*this + n);
}

TimeSeries::reverse_iterator& TimeSeries::reverse_iterator::operator++() {
    assert(m__index > 0 && "Incrementing end iterator");

    if (m__index > 0)
        --m__index;
    return *this;
}

TimeSeries::reverse_iterator TimeSeries::reverse_iterator::operator++(int) {
    assert(m__index > 0 && "Incrementing end iterator");

    reverse_iterator tmp = *this;
    if (m__index > 0)
        --m__index;
    return tmp;
}

TimeSeries::reverse_iterator& TimeSeries::reverse_iterator::operator--() {
    assert(m__index < m__time_series->length() && "Decrementing begin iterator");

    if (m__index < m__time_series->length())
        ++m__index;
    return *this;
}

TimeSeries::reverse_iterator TimeSeries::reverse_iterator::operator--(int) {
    assert(m__index < m__time_series->length() && "Decrementing begin iterator");

    reverse_iterator tmp = *this;
    if (m__index < m__time_series->length())
        ++m__index;
    return tmp;
}

TimeSeries::reverse_iterator& TimeSeries::reverse_iterator::operator+=(difference_type n) {
    assert(m__index - n <= m__time_series->length() && "Going out of bounds");

    // Overflow check
    if (n>0 && m__index < n)
        m__index = 0;
    else {
        size_type upb = m__time_series->length();
        m__index = (m__index - n > upb) ? upb : m__index - n;
    }

    return *this;
}

TimeSeries::reverse_iterator TimeSeries::reverse_iterator::operator+(difference_type n) const {
    reverse_iterator tmp = *this;
    return tmp += n;
}

TimeSeries::reverse_iterator& TimeSeries::reverse_iterator::operator-=(difference_type n) {
    return (*this += -n);
}

TimeSeries::reverse_iterator TimeSeries::reverse_iterator::operator-(difference_type n) const {
    return (*this + -n);
}

TimeSeries::reverse_iterator::difference_type TimeSeries::reverse_iterator::operator-(const reverse_iterator& other) const {
    return m__index - other.m__index;
}

bool TimeSeries::reverse_iterator::operator==(const reverse_iterator& other) const {
    return m__index == other.m__index;
}

bool TimeSeries::reverse_iterator::operator!=(const reverse_iterator& other) const {
    return m__index != other.m__index;
}

// Reverse the logic of the comparison operators.
bool TimeSeries::reverse_iterator::operator<(const reverse_iterator& other) const {
    return m__index > other.m__index;
}

bool TimeSeries::reverse_iterator::operator>(const reverse_iterator& other) const {
    return m__index < other.m__index;
}

bool TimeSeries::reverse_iterator::operator<=(const reverse_iterator& other) const {
    return m__index >= other.m__index;
}

bool TimeSeries::reverse_iterator::operator>=(const reverse_iterator& other) const {
    return m__index <= other.m__index;
}

/*----------------------------------------------------------------------------*/

                            /*--- Const Reverse Iterators ---*/

/*----------------------------------------------------------------------------*/

TimeSeries::const_reverse_iterator::const_reverse_iterator(const TimeSeries *time_series, size_type index) : 
    m__time_series(time_series), m__index(index) {}

TimeSeries::const_reverse_iterator::value_type TimeSeries::const_reverse_iterator::operator*() const {
    assert(m__index > 0 && m__index <= m__time_series->length() && "Dereferencing end iterator");

    if (m__index == m__time_series->inner_size()+1)
        return m__time_series->m__gto.duration__s();

    // Dereferencing the rend iterator (m__index == 0) is undefined behaviour.
    return m__time_series->m__time_steps[m__index-1];
}

TimeSeries::const_reverse_iterator::pointer TimeSeries::const_reverse_iterator::operator->() const {
    assert(m__index > 0 && m__index <= m__time_series->length() && "Dereferencing end iterator");

    if (m__index == m__time_series->inner_size()+1)
        return m__time_series->m__gto.duration__s_ptr();

    return &m__time_series->m__time_steps[m__index-1];
}

TimeSeries::const_reverse_iterator::value_type TimeSeries::const_reverse_iterator::operator[](difference_type n) const {
    return *(*this + n);
}

TimeSeries::const_reverse_iterator& TimeSeries::const_reverse_iterator::operator++() {
    assert(m__index > 0 && "Incrementing end iterator");

    if (m__index > 0)
        --m__index;
    return *this;
}

TimeSeries::const_reverse_iterator TimeSeries::const_reverse_iterator::operator++(int) {
    assert(m__index > 0 && "Incrementing end iterator");

    const_reverse_iterator tmp = *this;
    if (m__index > 0)
        --m__index;
    return tmp;
}

TimeSeries::const_reverse_iterator& TimeSeries::const_reverse_iterator::operator--() {
    assert(m__index < m__time_series->length() && "Decrementing begin iterator");

    if (m__index < m__time_series->length())
        ++m__index;
    return *this;
}

TimeSeries::const_reverse_iterator TimeSeries::const_reverse_iterator::operator--(int) {
    assert(m__index < m__time_series->length() && "Decrementing begin iterator");

    const_reverse_iterator tmp = *this;
    if (m__index < m__time_series->length())
        ++m__index;
    return tmp;
}

TimeSeries::const_reverse_iterator& TimeSeries::const_reverse_iterator::operator+=(difference_type n) {
    assert(m__index -n != 0 && m__index - n <= m__time_series->length() && "Going out of bounds");

    // Overflow check
    if (n>0 && m__index < n)
        m__index = 0;
    else {
        size_type upb = m__time_series->length();
        m__index = (m__index - n > upb) ? upb : m__index - n;
    }

    return *this;
}

TimeSeries::const_reverse_iterator TimeSeries::const_reverse_iterator::operator+(difference_type n) const {
    const_reverse_iterator tmp = *this;
    return tmp += n;
}

TimeSeries::const_reverse_iterator& TimeSeries::const_reverse_iterator::operator-=(difference_type n) {
    return (*this += -n);
}

TimeSeries::const_reverse_iterator TimeSeries::const_reverse_iterator::operator-(difference_type n) const {
    return (*this + -n);
}

TimeSeries::const_reverse_iterator::difference_type TimeSeries::const_reverse_iterator::operator-(const const_reverse_iterator& other) const {
    return m__index - other.m__index;
}

bool TimeSeries::const_reverse_iterator::operator==(const const_reverse_iterator& other) const {
    return m__index == other.m__index;
}

bool TimeSeries::const_reverse_iterator::operator!=(const const_reverse_iterator& other) const {
    return m__index != other.m__index;
}

// Reverse the logic of the comparison operators.
bool TimeSeries::const_reverse_iterator::operator<(const const_reverse_iterator& other) const {
    return m__index > other.m__index;
}

bool TimeSeries::const_reverse_iterator::operator>(const const_reverse_iterator& other) const {
    return m__index < other.m__index;
}

bool TimeSeries::const_reverse_iterator::operator<=(const const_reverse_iterator& other) const {
    return m__index >= other.m__index;
}

bool TimeSeries::const_reverse_iterator::operator>=(const const_reverse_iterator& other) const {
    return m__index <= other.m__index;
}

} // namespace bevarmejo
