#ifndef BEVARMEJOLIB__TIME_SERIES_HPP
#define BEVARMEJOLIB__TIME_SERIES_HPP

#include <cassert>           // assert
#include <cstddef>           // std::size_t, std::ptrdiff_t
#include <initializer_list>  // std::initializer_list
#include <iterator>          // std::random_access_iterator_tag
#include <memory>            // std::shared_ptr
#include <stdexcept>         // std::out_of_range
#include <utility>           // std::move
#include <vector>            // std::vector

#include "bevarmejo/wds/epanet_helpers/en_time_options.hpp"

namespace bevarmejo {
namespace wds {
namespace aux {

using time_t= epanet::time_t;
using TimeSteps= std::vector<time_t>;

bool is_monotonic(const TimeSteps& time_steps);

class TimeSeries {

/*--- Member types ---*/
public:
    using container= std::vector<time_t>;

    using key_type= time_t;
    using value_type= time_t;
    using size_type= std::size_t;
    using difference_type= std::ptrdiff_t;
    using reference= time_t&;
    using const_reference= const time_t&;
    using pointer= time_t*;
    using const_pointer= const time_t*;

/*--- Attributes ---*/
private:
    const epanet::GlobalTimeOptions& m__gto; // Necessary to make sure that the time steps are within the duration.
    container m__time_steps;

/*--- Member methods ---*/
public:

    void check_valid() const;
    
/*--- Constructors ---*/
public:

    TimeSeries() = delete; // Not existing constructor because I have a const reference to the Global Time Options.

    TimeSeries( const epanet::GlobalTimeOptions& a_gto );

    TimeSeries( const epanet::GlobalTimeOptions& a_gto, std::initializer_list<time_t> ilist );

    // Variadic constructor
    template <typename... Args>
    TimeSeries( const epanet::GlobalTimeOptions& a_gto, Args&&... args );

    TimeSeries(const TimeSeries& other) = default;
    TimeSeries(TimeSeries&& other) = default;

    TimeSeries& operator=(const TimeSeries& other) = delete; // Not allowed to copy the GTO
    TimeSeries& operator=(TimeSeries&& other) = delete; // Not allowed to move the GTO

    ~TimeSeries() = default;

/*--- Getters and setters ---*/
public:
    // get Gto, you can't modify it as it is a const reference
    const epanet::GlobalTimeOptions& gto() const { return m__gto; }

    // No access allowed to the time steps, only through the time_t methods. But I can get a copy of the time steps.
    const container& time_steps() const { return m__time_steps; }

/*--- Element access ---*/
public:
    size_type not_found() const noexcept;

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

        value_type operator*() const;
        pointer operator->() const;
        value_type operator[](difference_type n) const { return *(*this + n); }

        Iterator& operator++() {
            assert(m__index < m__time_series->length() && "Incrementing end iterator");
            
            if (m__index < m__time_series->length())
                ++m__index;
            return *this;
        }
        Iterator operator++(int) { 
            assert(m__index < m__time_series->length() && "Incrementing end iterator");
            
            auto tmp= *this;
            if (m__index < m__time_series->length())
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

        Iterator& operator+=(difference_type n);
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

        value_type operator*() const;
        pointer operator->() const;
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
            assert(m__index < m__time_series->length() && "Decrementing rbegin iterator");

            if (m__index < m__time_series->length())
                ++m__index;
            return *this;
        }
        ReverseIterator operator--(int) {
            assert(m__index < m__time_series->length() && "Decrementing rbegin iterator");

            auto tmp= *this;
            if (m__index < m__time_series->length())
                ++m__index;
            return tmp;
        }

        ReverseIterator& operator+=(difference_type n);
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

    // No simple size, because I need to add 1 to the size of the time steps. (final time)
    size_type inner_size() const noexcept;

    // Length is the number of iterations that you can do on the time series.
    size_type length() const noexcept;

    size_type max_size() const noexcept;

    void reserve( size_type new_cap );

    size_type capacity() const noexcept;

    // No shrink_to_fit.

/*--- Modifiers ---*/
public:

    // No clear, because it empties the time steps and I need at least one time step. Also is noexcept.
    void reset();

    // Insert takes care automatically of the monotonicity of the time steps. It works like a insert or assign.
    // However, you can pass a position to insert the time step to be faster :)
    iterator insert( time_t time__s );
    iterator insert( const_iterator pos, time_t time__s );
    template < class InputIt >
    iterator insert( const_iterator pos, InputIt first, InputIt last );
    iterator insert( std::initializer_list<time_t> ilist );
    iterator insert( const_iterator pos, std::initializer_list<time_t> ilist );

    iterator erase( const_iterator pos );
    iterator erase( const_iterator first, const_iterator last );
    iterator erase( time_t time__s );

    // Push back has a different name more related to a time series.
    void commit( time_t time__s );

    // Pop back has a different name more related to a time series.
    void rollback();

    // Resize should be called when the duration of the gto changes. 
    // Failing to call this method after changing the duration of the GTO will result in an invalid time series.
    void shrink_to_duration();
    void resize( size_type count );

    // No emplace, try_emplace, emplace_hint, emplace_back as they are simple longs.

    // No swap, because I can't swap the GTO.
    // but specialize: swap_time_steps
    void swap_time_steps( TimeSeries& other ) noexcept;

    // No extract.

    // TODO: define merge and other useful operations for time series.

/*--- Lookup ---*/
public:

    // No count, each time should be unique.
    size_type count ( time_t time__s ) const;

    // Find, returns an iterator to the first time that is not less than the given time
    iterator find( time_t time__s );
    const_iterator find( time_t time__s ) const;

    // Find_pos, returns the array position to the first time that is not less than the given time
    size_type find_pos( time_t time__s ) const;

    bool contains( time_t time__s ) const;

    bool is_in_range( time_t time__s ) const; // Between front() and duration__s()

    // No equal_range, each time should be unique.

    TimeSeries sub_series( time_t start_time__s, time_t end_time__s ) const;

    // Lower_bound, returns an iterator to the first time that is not less than the given time
    iterator lower_bound( time_t time__s );
    const_iterator lower_bound( time_t time__s ) const;

    // Lower_bound_pos, returns the array position to the last time that is not greater than the given time
    size_type lower_bound_pos( time_t time__s ) const;
            
    // Upper_bound, returns an iterator to the first time that is greater than the given time
    iterator upper_bound( time_t time__s );
    const_iterator upper_bound( time_t time__s ) const;

    // Upper_bound_pos, returns the array position to the first time that is greater than the given time
    size_type upper_bound_pos( time_t time__s ) const;
};

/*----------------------------------------------------------------------------*/

                            /*--- Iterator ---*/

/*----------------------------------------------------------------------------*/
template <typename TS>
typename TimeSeries::Iterator<TS>::value_type TimeSeries::Iterator<TS>::operator*() const {
    assert(m__index < m__time_series->length() && "Dereferencing end iterator");

    // Special case: index == size(), I return the end time.
    if (m__index == m__time_series->inner_size())
        return m__time_series->m__gto.duration__s();

    // Default case: 
    // Let dereference the end iterator be undefined behaviour in Release mode.
    return m__time_series->m__time_steps[m__index];
}

template <typename TS>
typename TimeSeries::Iterator<TS>::pointer TimeSeries::Iterator<TS>::operator->() const {
    assert(m__index < m__time_series->length() && "Dereferencing end iterator");

    if (m__index == m__time_series->inner_size())
        return m__time_series->m__gto.duration__s_ptr();

    return &m__time_series->m__time_steps[m__index];
}

template <typename TS>
typename TimeSeries::Iterator<TS>& TimeSeries::Iterator<TS>::operator+=(difference_type n) {
    assert(m__index + n <= m__time_series->length() && "Going out of bounds");
    
    // Overflow check
    if (n < 0 && m__index < -n)
        m__index = 0;
    else {
        size_type upb = m__time_series->length();
        m__index = (m__index + n > upb) ? upb : m__index + n;
    }   

    return *this;
}

/*----------------------------------------------------------------------------*/

                            /*--- Reverse Iterator ---*/

/*----------------------------------------------------------------------------*/
// Reverse iterator stores an iterator to the next element than the one it actually refers to.
template <typename TS>
typename TimeSeries::ReverseIterator<TS>::value_type TimeSeries::ReverseIterator<TS>::operator*() const {
    assert(m__index > 0 && "Dereferencing rend iterator");

    if (m__index == m__time_series->inner_size()+1)
        return m__time_series->m__gto.duration__s();

    return m__time_series->m__time_steps[m__index-1];
}

template <typename TS>
typename TimeSeries::ReverseIterator<TS>::pointer TimeSeries::ReverseIterator<TS>::operator->() const {
    assert(m__index > 0 && "Dereferencing rend iterator");

    if (m__index == m__time_series->inner_size()+1)
        return m__time_series->m__gto.duration__s_ptr();

    return &m__time_series->m__time_steps[m__index-1];
}

template <typename TS>
typename TimeSeries::ReverseIterator<TS>& TimeSeries::ReverseIterator<TS>::operator+=(difference_type n) {
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

} // namespace aux
} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__TIME_SERIES_HPP
