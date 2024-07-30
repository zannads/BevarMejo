#ifndef BEVARMEJOLIB__QUANTITY_SERIES_HPP
#define BEVARMEJOLIB__QUANTITY_SERIES_HPP

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "bevarmejo/epanet_helpers/en_time_options.hpp"
#include "bevarmejo/wds/data_structures/time_series.hpp"

namespace bevarmejo {

// just a base class to be able to use the same pointer for different types
class QuantitySeriesBase { };

template <typename T>
class QuantitySeries : public QuantitySeriesBase {

/*--- Member types ---*/
public:
    using container= std::vector<T>;

    using key_type= time_t;
    using value_type= T;
    using instant_type= std::pair<key_type, T&>;
    using size_type= std::size_t;
    using difference_type= std::ptrdiff_t;
    using reference= T&;
    using const_reference= const T&;
    using pointer= T*;
    using const_pointer= const T*;

    enum class State {
        Invalid,        // For example, not enough time steps
        ValueFillable,  // ValueFillable if  TimeStep satisfies the condition of Valid but the values are not yet set
        Valid           // Valid if        TimeSteps.inner_size() == Values.size()
    };

    // If in valid case, it can be one of the followings:
    enum class Case {
        Constant,       // Valid && TimeSteps.length() == 2
        Regular,        // Valid && TimeSteps.length() > 2 && regular difference between time steps
        Flexible        // Valid && TimeSteps.length() > 2 && at least one irregular difference between time steps
    };

/*--- Attributes ---*/
protected:
    const TimeSeries& m__time_steps;
    container m__values;

/*--- Member methods ---*/
public:

/*--- Constructors ---*/
public:

    QuantitySeries() = delete;

    QuantitySeries( const TimeSeries& time_steps ) : 
        m__time_steps(time_steps), 
        m__values() {}

    QuantitySeries( const TimeSeries& time_steps, 
                    const_reference a_value) : 
        m__time_steps(time_steps),
        m__values() {
            // I have faith in the TimeSeries to be always well defined
            m__values.assign(m__time_steps.inner_size(), a_value);
        }

    QuantitySeries( const TimeSeries& time_steps, const container& values) : 
        m__time_steps(time_steps),
        m__values() {
            if (m__time_steps.inner_size() == values.size())
                m__values = values;
        }

    QuantitySeries(const QuantitySeries& other) = default;

    QuantitySeries(QuantitySeries&& other) noexcept = default;

    QuantitySeries& operator=(const QuantitySeries& other) = default;

    QuantitySeries& operator=(QuantitySeries&& other) noexcept = default;

    virtual ~QuantitySeries() = default;

/*--- Getters and setters ---*/
public:
    // get TimeSeries, you can't modify it as it is a const reference
    const TimeSeries& time_steps() const { return m__time_steps; }

    // No access allowed to the values, only through the methods. 
    const container& values() const { return m__values; }

/*--- Element access ---*/
public:

    instant_type at( size_type pos ) { 
        if (pos > m__time_steps.inner_size())
            throw std::out_of_range("QuantitySeries::at: pos out of range");

        if (pos < m__time_steps.inner_size())
            return {m__time_steps.at(pos), m__values.at(pos)}; // if they don't have the same length it's an error

        // We know pos == m__time_steps.inner_size().
        // You can restart from the beginning when you reach the end, but you need that both elements are full.
        if (m__time_steps.inner_size() == m__values.size())
            return {m__time_steps.back(), m__values.front()};
        else
            throw std::out_of_range("QuantitySeries::at: pos out of range");
     }
    const instant_type at( size_type pos ) const {
        if (pos > m__time_steps.inner_size())
            throw std::out_of_range("QuantitySeries::at: pos out of range");

        if (pos < m__time_steps.inner_size())
            return {m__time_steps.at(pos), m__values.at(pos)}; // if they don't have the same length it's an error

        // We know pos == m__time_steps.inner_size().
        // You can restart from the beginning when you reach the end, but you need that both elements are full.
        if (m__time_steps.inner_size() == m__values.size())
            return {m__time_steps.back(), m__values.front()};
        else
            throw std::out_of_range("QuantitySeries::at: pos out of range");
    }

    reference when_t( time_t time__s ) { 
        size_type pos= m__time_steps.find_pos(time__s);

        if (pos == m__time_steps.not_found())
            throw std::out_of_range("QuantitySeries::when_t: time__s out of range");

        if (pos < m__time_steps.inner_size())
            return m__values.at(pos);

        // We know pos == m__time_steps.inner_size().
        // You can restart from the beginning when you reach the end, but you need that both elements are full.
        if (m__time_steps.inner_size() == m__values.size())
            return m__values.front();
        else
            throw std::out_of_range("QuantitySeries::when_t: time__s out of range");
    }
    const_reference when_t( time_t time__s ) const {
        size_type pos= m__time_steps.find_pos(time__s);

        if (pos == m__time_steps.not_found()) 
            throw std::out_of_range("QuantitySeries::when_t: time__s out of range");

        if (pos < m__time_steps.inner_size())
            return m__values.at(pos);

        // We know pos == m__time_steps.inner_size().
        // You can restart from the beginning when you reach the end, but you need that both elements are full.
        if (m__time_steps.inner_size() == m__values.size())
            return m__values.front();
        else
            throw std::out_of_range("QuantitySeries::when_t: time__s out of range");
    }

    // No operator[] because it is not safe to use it with this setup

    instant_type front() { 
        if (m__time_steps.inner_size() == m__values.size())
            return {m__time_steps.front(), m__values.front()};
        else
            throw std::out_of_range("QuantitySeries::front: not enough values");
     }
    const instant_type front() const { 
        if (m__time_steps.inner_size() == m__values.size())
            return {m__time_steps.front(), m__values.front()};
        else
            throw std::out_of_range("QuantitySeries::front: not enough values");
     }

    reference when_t0() { return when_t(m__time_steps.front()); }
    const_reference when_t0() const { return when_t(m__time_steps.front()); }

    instant_type back() { 
        if (m__time_steps.inner_size() == m__values.size())
            return {m__time_steps.back(), m__values.front()};
        else
            throw std::out_of_range("QuantitySeries::back: not enough values");
    }
    const instant_type back() const { 
        if (m__time_steps.inner_size() == m__values.size())
            return {m__time_steps.back(), m__values.front()};
        else
            throw std::out_of_range("QuantitySeries::back: not enough values");
    }

    reference when_tH() { return when_t(m__time_steps.back()); }
    const_reference when_tH() const { return when_t(m__time_steps.back()); }

    // No direct access to the time series! 

/*--- Iterators ---*/
public: 
    // TODO: iterators are the  crucial part 
    class iterator {};
    
    class const_iterator {};
    
    class reverse_iterator {};

    class const_reverse_iterator {};

/*--- Capacity ---*/
public:

    bool empty() const noexcept { return m__values.empty(); }

    size_type n_missing_values() const noexcept { return m__time_steps.inner_size() - m__values.size(); }

    bool is_missing_values() const noexcept { return n_missing_values() > 0; }

    // No simple size, because I need to add 1 to the size of the time steps
    size_type inner_size() const noexcept { return m__values.size(); }

    // Length is the number of iterations you can do on the object.
    // If the quantity is not full, you can iterate over it but it is truncated.
    // Use n_missing_values() and is_missing() to check if the quantity is full.
    size_type length() const noexcept { return m__values.size()+1; }

    size_type max_size() const noexcept { 
        // report the minimum of the two sizes
        return ( (m__values.max_size() > m__time_steps.max_size()) ? m__time_steps.max_size() : m__values.max_size() ); 
    }

    void reserve(size_type new_cap) { m__values.reserve(new_cap); }

    size_type capacity() const noexcept { return m__values.capacity(); }

/*--- Modifiers ---*/
public:

    // Clear the values, but keep the time steps. (e.g., for a new simulation)
    void clear() noexcept { m__values.clear(); }
    // Bring everything to the initial state.
    void reset() noexcept { m__values.clear(); m__time_steps.reset(); }

    // for the insert, time__s works like a key, while the iterator is just a helper to know where to start looking for
    iterator insert( time_t time__s, const_reference value );
    iterator insert( time_t time__s, T&& value );
    iterator insert( const_iterator pos, time_t time__s, const_reference value );
    iterator insert( const_iterator pos, time_t time__s, T&& value );
    iterator insert( time_t time__s, size_type count, const_reference value ); // Automatically fill the values with the same value
    iterator insert( const_iterator pos, time_t time__s, size_type count, const_reference value );
    // I assume the iterator is some kind of iteratro that returns a pair of time_t and T
    template <class InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last );
    iterator insert(const_iterator pos, std::initializer_list<instant_type> ilist ); // A list of pairs of time_t and T

    // Erase the value at the given time
    iterator erase( time_t time__s );
    iterator erase( const_iterator pos );
    iterator erase( const_iterator first, const_iterator last );

    // Push back but with a name that makes sense for time series (simpler check than insert because it is always at the end)
    void commit( time_t time__s, const_reference value );
    void commit( time_t time__s, T&& value );

    // Pop back but with a name that makes sense for time series (simpler check than erase because it is always at the end)
    void rollback() { m__values.pop_back(); }

    // Resize should be called when the duration of the gto changes. 
    // Failing to call this method after changing the duration of the GTO will result in an invalid time series. 
    // And the values will be truncated to the new duration.
    void resize(); // Automatic resize based on duration and time steps size
    void resize( size_type new_size ); // Resize to a new size 

    template< class... Args >
    iterator emplace( time_t time__s, Args&&... args );
    template< class... Args >
    iterator emplace( const_iterator pos, time_t time__s, Args&&... args );

    // Differently from the standard, I return there also an iterator for coherence.
    template< class... Args >
    iterator emplace_back( Args&&... args );

    // No swap because I have a const reference to the time steps. 
    // but specialize: swap_values
    void swap_values( QuantitySeries& other ) noexcept;

    // No extract.

    // TODO: define useful operations for time series, for example: resample, integrate and differentiate


/*--- Lookup ---*/
public:

    // No count, each time should be unique.
    size_type count( time_t time__s ) const { return m__time_steps.count(time__s); }

    // Find, returns an iterator to the first time that is not less than the given time
    iterator find( time_t time__s );
    const_iterator find( time_t time__s ) const;

    // Find_pos, returns the array position to the first time that is not less than the given time
    size_type find_pos( time_t time__s ) const { return m__time_steps.find_pos(time__s); }

    // No equal_range, each time should be unique.

    QuantitySeries between_t( time_t start_time__s, time_t end_time__s ) const;

    // Lower_bound, returns an iterator to the first time that is not less than the given time
    iterator lower_bound( time_t time__s );
    const_iterator lower_bound( time_t time__s ) const;

    // Lower_bound_pos, returns the array position to the last time that is not greater than the given time
    size_type lower_bound_pos( time_t time__s ) const { return m__time_steps.lower_bound_pos(time__s); }
            
    // Upper_bound, returns an iterator to the first time that is greater than the given time
    iterator upper_bound( time_t time__s );
    const_iterator upper_bound( time_t time__s ) const;

    // Upper_bound_pos, returns the array position to the first time that is greater than the given time
    size_type upper_bound_pos( time_t time__s ) const { return m__time_steps.upper_bound_pos(time__s); }
};

} // namespace bevarmejo

#endif // BEVARMEJOLIB__QUANTITY_SERIES_HPP
