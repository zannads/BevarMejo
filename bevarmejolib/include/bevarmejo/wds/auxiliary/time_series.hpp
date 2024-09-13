#pragma once

#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

#include "bevarmejo/wds/auxiliary/global_times.hpp"

namespace bevarmejo {
namespace wds {
namespace aux {

template <typename T>
class QuantitySeries;

bool is_monotonic(const time::TimeSteps& time_steps);

bool starts_after_zero(const time::TimeSteps& time_steps);

bool ends_before_t(const time::TimeSteps& time_steps, time::Instant t);

class TimeSeries {

// The TimeSeries class is a monotonically increasing sequence of time steps 
// representing the time shift with respect to the absolute start time of the
// simulation.
// The class behaves like a set to mantain the time steps unique and in order. 
// Therefore, accessing returns by value and not by reference and to modify the
// time steps, you need to use the insert and erase methods. 
// Commit and rollback work like push_back and pop_back.
// Basically it's a mix between a vector and an ordered set.
// Access and modification can be done in relative (default) or absolute time.
// Normal iterators are available, but there are extra type of iterators and 
// methods to get these.
// Normal iterators goes through the time steps, but starting from the always 
// present zero time step.
// Truncated iterators goes through the time steps, starting from the always 
// present zero time step, but ending at the duration of the GT.
// Special iterators for the QuantitySeries are available and, along with time, 
// they return the index of the time step to access the value associated with time.

/*--- Member types ---*/
public:
    using container= time::TimeSteps;

    using key_type= time::Instant;
    using value_type= time::Instant;
    using size_type= std::size_t;
    using difference_type= std::ptrdiff_t;
    using reference= time::Instant&;
    using const_reference= const time::Instant&;
    using pointer= time::Instant*;
    using const_pointer= const time::Instant*;

/*--- Attributes ---*/
private:
    const GlobalTimes& m__gto; 
    container m__time_steps; 

/*--- Support ---*/
private:
    friend class GlobalTimes;
    template <typename T>
    friend class QuantitySeries;
    
/*--- Constructors ---*/
public:

    TimeSeries() = delete; // No default constructor because I have a const reference to the Global Time Options.

    TimeSeries( const GlobalTimes& a_gto );

    // Variadic constructor
    template <typename... Args>
    TimeSeries( const GlobalTimes& a_gto, Args&&... args ) : 
        m__gto(a_gto),
        m__time_steps(std::forward<Args>(args)...)
    {
        check_valid();
    }

    TimeSeries(const TimeSeries& other) = delete;
    TimeSeries& operator=(const TimeSeries& other) = delete;

    TimeSeries(TimeSeries&& other) noexcept = delete;
    TimeSeries& operator=(TimeSeries&& other) noexcept = delete;

    ~TimeSeries() = default;

/*--- Getters and setters ---*/
public:

    // No access allowed to the time steps, only through the time::Instant methods. But I can get a copy of the time steps.
    const container& time_steps() const;
    
/*--- Element access ---*/
public:
    // All access, is by value, so I can't modify the time steps.
    // This is done to avoid the time steps to be modified without checking the validity of the time series.
    value_type at( size_type pos );
    const value_type at( size_type pos ) const;

    // No operator[] because it is not safe to use it with this setup

    value_type front();
    const value_type front() const;

    value_type back();
    const value_type back() const;

    // No direct access to the time series! 

/*--- Iterators ---*/
public: 

// Forward iterators go from 0 to m__time_steps.size()-1 to access time size.  
// When at m__time_steps.size() it is the final time of the GTO. 
// When at m__time_steps.size()+1 it is not found, dereferencing is undefined behavior.

// The reverse iterators behave the same, but I access index-1 to get the time step.
// So that the end operator has index 0 and I can use a size_type for both forward and reverse iterators.
private:
    template <typename TS>
    class Iterator {
    public:
        using iterator_category= std::random_access_iterator_tag;
        using value_type= typename TS::value_type;
        using difference_type= typename TS::difference_type;
        using pointer= typename TS::const_pointer;
        using reference= typename TS::const_reference;

    private:
        TS* m__time_series;
        size_type m__index;

    public:
        Iterator(TS* time_series, size_type index) : m__time_series(time_series), m__index(index) {}

        value_type operator*() const { return m__time_series->at(m__index); }
        pointer operator->() const = delete; // We only return by value.
        value_type operator[](difference_type n) const { return *(*this + n); }

        Iterator& operator++() {
            assert(m__index < m__time_series->size() && "Incrementing end iterator");
            
            if (m__index < m__time_series->size())
                ++m__index;
            return *this;
        }
        Iterator operator++(int) { 
            assert(m__index < m__time_series->size() && "Incrementing end iterator");
            
            auto tmp= *this;
            if (m__index < m__time_series->size())
                ++m__index;
            return tmp;
        }
        Iterator& operator--() {
            assert(m__index > 0 && "Decrementing begin iterator");

            if (m__index > 0)
                --m__index;
            return *this;
        }
        Iterator operator--(int) {
            assert(m__index > 0 && "Decrementing begin iterator");

            auto tmp= *this;
            if (m__index > 0)
                --m__index;
            return tmp;
        }

        Iterator& operator+=(difference_type n) {
            assert(m__index + n <= m__time_series->size() && "Going out of bounds");
            
            // Overflow check
            if (n < 0 && m__index < -n)
                m__index = 0;
            else {
                size_type upb = m__time_series->size();
                m__index = (m__index + n > upb) ? upb : m__index + n;
            }   

            return *this;
        }
        Iterator operator+(difference_type n) const { auto tmp= *this; return tmp += n; }
        Iterator& operator-=(difference_type n) { return (*this += -n); }
        Iterator operator-(difference_type n) const { return (*this + -n); }
        difference_type operator-(const Iterator& other) const { return m__index - other.m__index; }

        bool operator==(const Iterator& other) const { return m__index == other.m__index; }
        bool operator!=(const Iterator& other) const { return m__index != other.m__index; }
        bool operator<(const Iterator& other) const { return m__index < other.m__index; }
        bool operator>(const Iterator& other) const { return m__index > other.m__index; }
        bool operator<=(const Iterator& other) const { return m__index <= other.m__index; }
        bool operator>=(const Iterator& other) const { return m__index >= other.m__index; }
    };
    
    template <typename TS>
    class ReverseIterator {
    public:
        using iterator_category= std::random_access_iterator_tag;
        using value_type= typename TS::value_type;
        using difference_type= typename TS::difference_type;
        using pointer= typename TS::const_pointer;
        using reference= typename TS::const_reference;

    private:
        TS* m__time_series;
        size_type m__index;

    public:
        ReverseIterator(TS* time_series, size_type index) : m__time_series(time_series), m__index(index) {}

        value_type operator*() const { return m__time_series->at(m__index-1); }
        pointer operator->() const = delete; // We only return by value.
        value_type operator[](difference_type n) const { return *(*this + n); }

        ReverseIterator& operator++() {
            assert(m__index > 0 && "Incrementing rend iterator");

            if (m__index > 0)
                --m__index;
            return *this;
        }
        ReverseIterator operator++(int) {
            assert(m__index > 0 && "Incrementing rend iterator");

            auto tmp= *this;
            if (m__index > 0)
                --m__index;
            return tmp;
        }
        ReverseIterator& operator--() {
            assert(m__index < m__time_series->size() && "Decrementing rbegin iterator");

            if (m__index < m__time_series->size())
                ++m__index;
            return *this;
        }
        ReverseIterator operator--(int) {
            assert(m__index < m__time_series->size() && "Decrementing rbegin iterator");

            auto tmp= *this;
            if (m__index < m__time_series->size())
                ++m__index;
            return tmp;
        }

        ReverseIterator& operator+=(difference_type n) {
            assert(m__index - n <= m__time_series->size() && "Going out of bounds");

            // Overflow check
            if (n>0 && m__index < n)
                m__index = 0;
            else {
                size_type upb = m__time_series->size();
                m__index = (m__index - n > upb) ? upb : m__index - n;
            }

            return *this;
        }
        ReverseIterator operator+(difference_type n) const { auto tmp= *this; return tmp += n; }
        ReverseIterator& operator-=(difference_type n) { return (*this += -n); }
        ReverseIterator operator-(difference_type n) const { return (*this + -n); }
        // Reverse the logic of the comparison operators.
        difference_type operator-(const ReverseIterator& other) const { return -(m__index - other.m__index); }

        bool operator==(const ReverseIterator& other) const { return m__index == other.m__index; }
        bool operator!=(const ReverseIterator& other) const { return m__index != other.m__index; }
        // Reverse the logic of the comparison operators.
        bool operator<(const ReverseIterator& other) const { return m__index > other.m__index; }
        bool operator>(const ReverseIterator& other) const { return m__index < other.m__index; }
        bool operator<=(const ReverseIterator& other) const { return m__index >= other.m__index; }
        bool operator>=(const ReverseIterator& other) const { return m__index <= other.m__index; }
    };

public:
    using iterator= Iterator<TimeSeries>;
    using const_iterator= Iterator<const TimeSeries>;
    using reverse_iterator= ReverseIterator<TimeSeries>;
    using const_reverse_iterator= ReverseIterator<const TimeSeries>;

    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    const_iterator cbegin() const noexcept;

    iterator end() noexcept;
    const_iterator end() const noexcept;
    const_iterator cend() const noexcept;

    reverse_iterator rbegin() noexcept;
    const_reverse_iterator rbegin() const noexcept;
    const_reverse_iterator crbegin() const noexcept;

    reverse_iterator rend() noexcept;
    const_reverse_iterator rend() const noexcept;
    const_reverse_iterator crend() const noexcept;

/*--- Capacity ---*/
public:
    
    bool empty() const noexcept;

    size_type size() const noexcept;

    size_type max_size() const noexcept;

    void reserve( size_type new_cap );

    size_type capacity() const noexcept;

    // shrink_to_fit is updated to shrink_to_duration because the time steps should be within the duration.
    // Equality to duration is allowed, but greater than duration is not allowed.
    void shrink_to_duration();

    // Same as shrink_to_duration, but equal to duration is not allowed.
    void shrink_before_duration();

private:
    
    bool inner_empty() const noexcept;

    size_type inner_size() const noexcept;

/*--- Modifiers ---*/
public:

    void reset() noexcept;

    // Insert takes care automatically of the monotonicity of the time steps. It works like a insert or assign.
    // However, you can pass a position to insert the time step to be faster :)
    iterator insert( time::Instant time__s );
    iterator insert( const_iterator pos, time::Instant time__s );
    template < class InputIt >
    iterator insert( const_iterator pos, InputIt first, InputIt last );
    iterator insert( std::initializer_list<time::Instant> ilist );
    iterator insert( const_iterator pos, std::initializer_list<time::Instant> ilist );

    iterator erase( const_iterator pos );
    iterator erase( const_iterator first, const_iterator last );
    iterator erase( time::Instant time__s );

    // Push back has a different name more related to a time series.
    void commit( time::Instant time__s );

    // Pop back has a different name more related to a time series.
    void rollback();

    // Resize should be called when the duration of the gto changes. 
    // Failing to call this method after changing the duration of the GTO will result in an invalid time series.
    
    void resize( size_type count );

    // No emplace, try_emplace, emplace_hint, emplace_back as they are simple longs.

    // No swap, because I can't swap the GTO.

    // No extract.

    // TODO: define merge and other useful operations for time series.

private:
    void inner_clear() noexcept;

    void inner_swap( TimeSeries& other ) noexcept;

/*--- Lookup ---*/
public:

    // Each time should be unique or non existing. Only duration can be repeated and return 2.
    size_type count ( time::Instant time__s ) const;

    // Find, returns an iterator to the instant that is exactly the given time
    iterator find( time::Instant time__s );
    const_iterator find( time::Instant time__s ) const;

    // Find_pos, returns the array position to the instant that is exactly the given time
    size_type find_pos( time::Instant time__s ) const;

    // Contains, returns true if the time is in the time series.
    bool contains( time::Instant time__s ) const;

    // Can be inserted, returns true if the time can be inserted in the time series (check for monotonicity and duration).
    bool can_be_inserted( time::Instant time__s ) const;

    // No equal_range, each time should be unique.

    TimeSeries sub_series( time::Instant start_time__s, time::Instant end_time__s ) const;

    // Lower_bound, returns an iterator to the last time that is less than the given time.
    iterator lower_bound( time::Instant time__s );
    const_iterator lower_bound( time::Instant time__s ) const;

    // Lower_bound_pos, returns the array position to the last time that is less than the given time
    size_type lower_bound_pos( time::Instant time__s ) const;
            
    // Upper_bound, returns an iterator to the first time that is greater than the given time
    iterator upper_bound( time::Instant time__s );
    const_iterator upper_bound( time::Instant time__s ) const;

    // Upper_bound_pos, returns the array position to the first time that is greater than the given time
    size_type upper_bound_pos( time::Instant time__s ) const;

/*--- Other methods ---*/
public:

    // Check if the time series is valid.
    void check_valid() const;

};

} // namespace aux
} // namespace wds
} // namespace bevarmejo
