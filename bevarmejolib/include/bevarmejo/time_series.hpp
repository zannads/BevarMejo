#ifndef BEVARMEJOLIB__TIME_SERIES_HPP
#define BEVARMEJOLIB__TIME_SERIES_HPP

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "bevarmejo/epanet_helpers/en_time_options.hpp"

namespace bevarmejo {

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

    class iterator {
    private:
        TimeSeries* m__time_series;
        size_type m__index;

    public:
        using iterator_category= std::random_access_iterator_tag;
        using value_type= TimeSeries::value_type;
        using difference_type= TimeSeries::difference_type;
        using pointer= TimeSeries::const_pointer;
        using reference= TimeSeries::const_reference;

        iterator(TimeSeries* time_series, size_type index);

        value_type operator*() const;
        pointer operator->() const;
        value_type operator[](difference_type n) const;

        iterator& operator++();
        iterator operator++(int);
        iterator& operator--();
        iterator operator--(int);

        iterator& operator+=(difference_type n);
        iterator operator+(difference_type n) const;
        iterator& operator-=(difference_type n);
        iterator operator-(difference_type n) const;
        difference_type operator-(const iterator& other) const;

        bool operator==(const iterator& other) const;
        bool operator!=(const iterator& other) const;
        bool operator<(const iterator& other) const;
        bool operator>(const iterator& other) const;
        bool operator<=(const iterator& other) const;
        bool operator>=(const iterator& other) const;
    };

    class const_iterator {
    private:
        const TimeSeries* m__time_series;
        size_type m__index;

    public:
        using iterator_category= std::random_access_iterator_tag;
        using value_type= TimeSeries::value_type;
        using difference_type= TimeSeries::difference_type;
        using pointer= TimeSeries::const_pointer;
        using reference= TimeSeries::const_reference;

        const_iterator(const TimeSeries* time_series, size_type index);

        value_type operator*() const;
        pointer operator->() const;
        value_type operator[](difference_type n) const;

        const_iterator& operator++();
        const_iterator operator++(int);
        const_iterator& operator--();
        const_iterator operator--(int);

        const_iterator& operator+=(difference_type n);
        const_iterator operator+(difference_type n) const;
        const_iterator& operator-=(difference_type n);
        const_iterator operator-(difference_type n) const;
        difference_type operator-(const const_iterator& other) const;

        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
        bool operator<(const const_iterator& other) const;
        bool operator>(const const_iterator& other) const;
        bool operator<=(const const_iterator& other) const;
        bool operator>=(const const_iterator& other) const;
    };

    class reverse_iterator {
    private:
        TimeSeries* m__time_series;
        size_type m__index;

    public:
        using iterator_category= std::random_access_iterator_tag;
        using value_type= TimeSeries::value_type;
        using difference_type= TimeSeries::difference_type;
        using pointer= TimeSeries::const_pointer;
        using reference= TimeSeries::const_reference;

        reverse_iterator(TimeSeries* time_series, size_type index);

        value_type operator*() const;
        pointer operator->() const;
        value_type operator[](difference_type n) const;

        reverse_iterator& operator++();
        reverse_iterator operator++(int);
        reverse_iterator& operator--();
        reverse_iterator operator--(int);

        reverse_iterator& operator+=(difference_type n);
        reverse_iterator operator+(difference_type n) const;
        reverse_iterator& operator-=(difference_type n);
        reverse_iterator operator-(difference_type n) const;
        difference_type operator-(const reverse_iterator& other) const;

        bool operator==(const reverse_iterator& other) const;
        bool operator!=(const reverse_iterator& other) const;
        bool operator<(const reverse_iterator& other) const;
        bool operator>(const reverse_iterator& other) const;
        bool operator<=(const reverse_iterator& other) const;
        bool operator>=(const reverse_iterator& other) const;
    };

    class const_reverse_iterator {
    private:
        const TimeSeries* m__time_series;
        size_type m__index;

    public:
        using iterator_category= std::random_access_iterator_tag;
        using value_type= TimeSeries::value_type;
        using difference_type= TimeSeries::difference_type;
        using pointer= TimeSeries::const_pointer;
        using reference= TimeSeries::const_reference;

        const_reverse_iterator(const TimeSeries* time_series, size_type index);

        value_type operator*() const;
        pointer operator->() const;
        value_type operator[](difference_type n) const;

        const_reverse_iterator& operator++();
        const_reverse_iterator operator++(int);
        const_reverse_iterator& operator--();
        const_reverse_iterator operator--(int);

        const_reverse_iterator& operator+=(difference_type n);
        const_reverse_iterator operator+(difference_type n) const;
        const_reverse_iterator& operator-=(difference_type n);
        const_reverse_iterator operator-(difference_type n) const;
        difference_type operator-(const const_reverse_iterator& other) const;

        bool operator==(const const_reverse_iterator& other) const;
        bool operator!=(const const_reverse_iterator& other) const;
        bool operator<(const const_reverse_iterator& other) const;
        bool operator>(const const_reverse_iterator& other) const;
        bool operator<=(const const_reverse_iterator& other) const;
        bool operator>=(const const_reverse_iterator& other) const;
    };
    
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
    
    constexpr bool empty() const noexcept;

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

} // namespace bevarmejo

#endif // BEVARMEJOLIB__TIME_SERIES_HPP
