#ifndef BEVARMEJOLIB__QUANTITY_SERIES_HPP
#define BEVARMEJOLIB__QUANTITY_SERIES_HPP

#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "bevarmejo/io.hpp"

#include "bevarmejo/wds/epanet_helpers/en_time_options.hpp"
#include "bevarmejo/wds/auxiliary/time_series.hpp"

namespace bevarmejo {
namespace wds {
namespace aux {

// just a base class to be able to use the same pointer for different types
class QuantitySeriesBase {

/*--- Attributes ---*/
protected:
    const TimeSeries& m__time_series;

public:
// Default all constructors and destructor
    QuantitySeriesBase() = delete;
    QuantitySeriesBase(const TimeSeries& time_steps) : m__time_series(time_steps) { }
    QuantitySeriesBase(const QuantitySeriesBase& other) = default;
    QuantitySeriesBase(QuantitySeriesBase&& other) noexcept = default;
    QuantitySeriesBase& operator=(const QuantitySeriesBase& other) = delete;
    QuantitySeriesBase& operator=(QuantitySeriesBase&& other) noexcept = delete;
    virtual ~QuantitySeriesBase() = default;

    // Clone method to be able to copy the objects.
    std::unique_ptr<QuantitySeriesBase> clone() const {
        return std::unique_ptr<QuantitySeriesBase>(__clone());
    }

protected:
    virtual QuantitySeriesBase* __clone() const = 0;

/*--- Getters and setters ---*/
public:
    // get TimeSeries, you can't modify it as it is a const reference
    const TimeSeries& time_steps() const { return m__time_series; }
    
}; // class QuantitySeriesBase

template <typename T>
class QuantitySeries final : public QuantitySeriesBase {

/*--- Member types ---*/
public:
    using container= std::vector<T>;
    using inherited= QuantitySeriesBase;

    using key_type= time_t;
    using value_type= T;
    using instant_type= std::pair<key_type, T&>;
    using const_instant_type= std::pair<key_type, const T&>;
    using size_type= std::size_t;
    using difference_type= std::ptrdiff_t;
    using reference= T&;
    using const_reference= const T&;
    using pointer= T*;
    using const_pointer= const T*;

/*--- Attributes ---*/
protected:
    container m__values;

/*--- Member methods ---*/
public:
    // The quantity series is allowed to be accessed only if the values are as long as the time steps
    // or one value more (for the end time).
    bool is_accessible() const noexcept { 
        return ((m__time_series.inner_size() == m__values.size()) ||
                (m__time_series.inner_size()+1 == m__values.size()) );
    }

    void check_access() const {
        if ((m__time_series.inner_size() == m__values.size()) ||
            (m__time_series.inner_size()+1 == m__values.size()) ) {
            std::ostringstream errmessage;
            bevarmejo::io::stream_out(errmessage, 
                "QuantitySeries::check_access: Impossible to access the values with the current state.",
                "\tExpected value size to be:\n\t\t", m__time_series.inner_size(),
                "\tActual value size is:\n\t\t", m__values.size()
            );
            
            throw std::logic_error(errmessage.str());
        }
    }
            

/*--- Constructors ---*/
public:

    QuantitySeries() = delete;

    QuantitySeries( const TimeSeries& time_steps ) : 
        inherited(time_steps), 
        m__values() {
            this->reserve();
        }

    QuantitySeries( const TimeSeries& time_steps, 
                    const_reference a_value) : 
        inherited(time_steps),
        m__values() {
            // I have faith in the TimeSeries to be always well defined
            m__values.assign(m__time_series.inner_size(), a_value);
        }

    QuantitySeries( const TimeSeries& time_steps, const container& values) : 
        inherited(time_steps),
        m__values(values) {
            this->reserve();
        }

    QuantitySeries(const QuantitySeries& other) = default;
    QuantitySeries(QuantitySeries&& other) noexcept = default;
    QuantitySeries& operator=(const QuantitySeries& other) {
        if (this != &other) {
            // You can't reassign the TimeSeries of the inherited class because is const reference
            m__values= other.m__values;
        }
        return *this;
    }
    QuantitySeries& operator=(QuantitySeries&& other) noexcept {
        if (this != &other) {
            // You can't reassign the TimeSeries of the inherited class because is const reference
            m__values= std::move(other.m__values);
        }
        return *this;
    }

    virtual ~QuantitySeries() = default;

protected:
    virtual QuantitySeriesBase* __clone() const override {
        return new QuantitySeries(*this);
    }

/*--- Getters and setters ---*/
public:
    // No access allowed to the values, only through the methods. 
    const container& values() const { return m__values; }
    // Let's make it private so that friend classes, like the iterator can access it.
private:
    container& values() { return m__values; }

/*--- Element access ---*/
// In general, it is not possible to access the values unless the values series is 
// as long as the time series. It is still possible to modify it with insert, commit
// etc., but access (read only or not) will throw an error. 
// Asking for an iterator will automatically return the end iterator.
public:

    instant_type at( size_type pos ) {
        check_access();

        // Only special case, is full quantity series and you want the last element (Tend).
        if (pos == m__time_series.inner_size() && m__time_series.inner_size() == m__values.size())
            return {m__time_series.back(), m__values.front()};

        // Else let the at functions throw the exceptions.
        return {m__time_series.at(pos), m__values.at(pos)};
    }

    const_instant_type at( size_type pos ) const {
        check_access();
        
        // Only special case, is full quantity series and you want the last element (Tend).
        if (pos == m__time_series.inner_size() && m__time_series.inner_size() == m__values.size())
            return {m__time_series.back(), m__values.front()};

        // Else let the at functions throw the exceptions.
        return {m__time_series.at(pos), m__values.at(pos)};
    }

    reference when_t( time_t time__s ) { 
        check_access();

        size_type pos= m__time_series.find_pos(time__s);

        if (pos == m__time_series.not_found())
            throw std::out_of_range("QuantitySeries::when_t: time__s out of range");

        if (pos == m__time_series.inner_size() && m__time_series.inner_size() == m__values.size())
            return m__values.front();

        return m__values.at(pos);
    }
    const_reference when_t( time_t time__s ) const {
        check_access();

        size_type pos= m__time_series.find_pos(time__s);

        if (pos == m__time_series.not_found()) 
            throw std::out_of_range("QuantitySeries::when_t: time__s out of range");

        if (pos == m__time_series.inner_size() && m__time_series.inner_size() == m__values.size())
            return m__values.front();

        return m__values.at(pos);
    }

    // No operator[] because it is not safe to use it with this setup

    instant_type front() { 
        check_access();

        return {m__time_series.front(), m__values.front()};
    }
    const_instant_type front() const { 
        check_access();
    
        return {m__time_series.front(), m__values.front()};
    }

    reference when_t0() { return when_t(0l); }
    const_reference when_t0() const { return when_t(0l); }

    instant_type back() { 
        check_access();

        if (m__time_series.inner_size() == m__values.size())
            return {m__time_series.back(), m__values.front()};

        // else m__time_series.inner_size()+1 == m__values.size()
        return {m__time_series.back(), m__values.back()};
    }
    const_instant_type back() const { 
        check_access();

        if (m__time_series.inner_size() == m__values.size())
            return {m__time_series.back(), m__values.front()};

        // else m__time_series.inner_size()+1 == m__values.size()
        return {m__time_series.back(), m__values.back()};
    }

    reference when_tH() { return when_t(m__time_series.back()); }
    const_reference when_tH() const { return when_t(m__time_series.back()); }

    // No direct access to the time series! 

/*--- Iterators ---*/
// Iterators follow the same behaviour of the TimeSeries, so from 0 to m__time_series.size()-1 
// access m__time_series and m__values at position.
// At index = m__time_series.size() and if m__time_series.size()+1 == m_values:
//       access duration__s and m__values[index].
// At index = m__time_series.size() and if m__time_series.size() == m_values:
//       access duration__s and m__values[0].
// Dereferencing the end iterator is UB.
private:
    template <typename TS>
    class Iterator {
    public:
        using iterator_category= std::random_access_iterator_tag;
        using value_type= typename TS::instant_type;
        using size_type= typename TS::size_type;
        using difference_type= typename TS::difference_type;
        using pointer= typename TS::instant_type*;
    private:
        TS* m__qs;
        size_type m__index;
        mutable value_type __temp__; // Just for the operator->

    public:
        Iterator(TS* qs, size_type index) : m__qs(qs), m__index(index) { }

        value_type operator*() const {
            assert(m__index < m__qs->length() && "Dereferencing the end iterator.");

            if (m__index == m__qs->length())
                return {m__qs->time_steps().back(), m__qs->values().front()};

            return {m__qs->time_steps().at(m__index), m__qs->values()[m__index]};
        }
        pointer operator->() const { __temp__= **this; return &__temp__; }
        value_type operator[](difference_type n) const { return *(*this + n); }

        Iterator& operator++() { 
            assert(m__index < m__qs->length() && "Incrementing the end iterator.");

            if (m__index < m__qs->length())
                ++m__index;
            return *this;
        }
        Iterator operator++(int) {
            assert(m__index < m__qs->length() && "Incrementing the end iterator.");

            Iterator tmp= *this;
            if (m__index < m__qs->length())
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
            assert(m__index + n <= m__qs->length() && "Going out of bounds.");

            // Overflow check
            if (n < 0 && m__index < -n)
                m__index= 0;
            else {
                size_type upb= m__qs->length();
                m__index= (m__index+n > upb) ? upb : m__index+n;
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
        using value_type= typename TS::instant_type;
        using size_type= typename TS::size_type;
        using difference_type= typename TS::difference_type;
        using pointer= typename TS::instant_type*;
    private:
        TS* m__qs;
        size_type m__index;
        mutable value_type __temp__; // Just for the operator->

    public:
        ReverseIterator(TS* qs, size_type index) : m__qs(qs), m__index(index) { }

        value_type operator*() const {
            assert(m__index > 0 && "Dereferencing the rend iterator.");

            if (m__index == m__qs->length())
                return {m__qs->time_steps().back(), m__qs->values().front()};

            return {m__qs->time_steps().at(m__index - 1), m__qs->values()[m__index - 1]};
        }
        pointer operator->() const { __temp__= **this; return &__temp__; }
        value_type operator[](difference_type n) const { return *(*this + n); }

        ReverseIterator& operator++() { 
            assert(m__index > 0 && "Incrementing the rend iterator");

            if (m__index > 0)
                --m__index;
            return *this;
        }
        ReverseIterator operator++(int) {
            assert(m__index > 0 && "Incrementing the rend iterator");

            auto tmp= *this;
            if (m__index > 0)
                --m__index;
            return tmp;
        }
        ReverseIterator& operator--() {
            assert(m__index < m__qs->length() && "Decrementing the rbegin iterator");

            if (m__index < m__qs->length())
                ++m__index;
            return *this;
        }
        ReverseIterator operator--(int) {
            assert(m__index < m__qs->length() && "Decrementing the rbegin iterator");

            auto tmp= *this;
            if (m__index < m__qs->length())
                ++m__index;
            return tmp;
        }

        ReverseIterator& operator+=(difference_type n) {
            assert(m__index - n <= m__qs->length() && "Going out of bounds.");

            // Overflow check
            if (n > 0 && m__index < n)
                m__index= 0;
            else {
                size_type upb= m__qs->length();
                m__index= (m__index-n > upb) ? upb : m__index-n;
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
    using iterator= Iterator<QuantitySeries>;
    using const_iterator= Iterator<const QuantitySeries>;
    using reverse_iterator= ReverseIterator<QuantitySeries>;
    using const_reverse_iterator= ReverseIterator<const QuantitySeries>;
private:
    // Declare the iterator classes as friends
    friend class Iterator<QuantitySeries>;
    friend class Iterator<const QuantitySeries>;
    friend class ReverseIterator<QuantitySeries>;
    friend class ReverseIterator<const QuantitySeries>;
public:

    iterator begin() noexcept { 
        if (is_accessible())    return iterator(this, 0);
        else                    return end();
    }
    const_iterator begin() const noexcept { 
        if (is_accessible())    return const_iterator(this, 0);
        else                    return cend();
    }
    const_iterator cbegin() const noexcept { 
        if (is_accessible())    return const_iterator(this, 0);
        else                    return cend();
    }
    
    iterator end() noexcept { return iterator(this, m__time_series.length()); }
    const_iterator end() const noexcept { return const_iterator(this, m__time_series.length()); }
    const_iterator cend() const noexcept { return const_iterator(this, m__time_series.length()); }

    reverse_iterator rbegin() noexcept { 
        if (is_accessible())    return reverse_iterator(this, m__time_series.length());
        else                    return rend();
    }
    const_reverse_iterator rbegin() const noexcept { 
        if (is_accessible())    return const_reverse_iterator(this, m__time_series.length());
        else                    return crend();
    }
    const_reverse_iterator crbegin() const noexcept { 
        if (is_accessible())    return const_reverse_iterator(this, m__time_series.length());
        else                    return crend();
    }

    reverse_iterator rend() noexcept { return reverse_iterator(this, 0); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(this, 0); }
    const_reverse_iterator crend() const noexcept { return const_reverse_iterator(this, 0); }

/*--- Capacity ---*/
public:

    bool empty() const noexcept { return m__values.empty(); }

    size_type n_missing_values() const noexcept { 
        if (m__time_series.inner_size() == m__values.size() || m__time_series.inner_size()+1 == m__values.size())
            return 0ul;
        
        // else 
        difference_type diff= m__time_series.inner_size() - m__values.size();
        return (diff > 0) ? static_cast<size_type>(diff) : 0ul;
    }

    bool is_missing_values() const noexcept { return n_missing_values() > 0ul; }

    // No simple size, because I need to add 1 to the size of the time steps
    size_type inner_size() const noexcept { return m__values.size(); }

    // Length is the number of iterations you can do on the object.
    // If the quantity is not full, you can not iterate over it.
    // Use n_missing_values() and is_missing() to check if the quantity is full.
    size_type length() const noexcept { return m__time_series.length(); }

    size_type max_size() const noexcept { 
        // report the minimum of the two sizes
        return ( (m__values.max_size() > m__time_series.max_size()) ? m__time_series.max_size() : m__values.max_size() ); 
    }

    // Reserve should be called to reserve space based on the time steps size. 
    void reserve() { m__values.reserve(m__time_series.capacity()); }

    // Minimum between the capacity of the container and how many values we are missing for the given time steps.
    size_type capacity() const noexcept { return m__values.capacity() > n_missing_values() ? n_missing_values() : m__values.capacity(); }

/*--- Modifiers ---*/
public:

    // Clear the values, but keep the time steps. (e.g., for a new simulation)
    void clear() noexcept { m__values.clear(); reserve(); }

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
    void commit( time_t time__s, const_reference value ) {
        auto pos = m__values.size();

        if (pos > m__time_series.inner_size())
            throw std::out_of_range("QuantitySeries::commit: not enough time steps");

        if (time__s != m__time_series.at(pos))
            throw std::invalid_argument("QuantitySeries::commit: time__s must be equal to the next time step");

        m__values.push_back(value);
    }
    void commit( time_t time__s, T&& value ) {
        auto pos = m__values.size(); 

        if (pos > m__time_series.inner_size())
            throw std::out_of_range("QuantitySeries::commit: not enough time steps");

        if (time__s != m__time_series.at(pos))
            throw std::invalid_argument("QuantitySeries::commit: time__s must be equal to the next time step");

        m__values.push_back(std::move(value));
    }

    // Pop back but with a name that makes sense for time series (simpler check than erase because it is always at the end)
    void rollback() { m__values.pop_back(); }

    // Resize should be called when the duration of the gto changes. 
    // Failing to call this method after changing the duration of the GTO will result in an invalid time series. 
    // And the values will be truncated to the new duration.
    void resize(); // Automatic resize based on duration and time steps size
    void resize( size_type new_size ); // Resize to a new size 
    // Resize to the inner size of the time steps
    void fit_to_time_steps() {
        if (m__time_series.inner_size() < m__values.size())
            m__values.resize(m__time_series.inner_size());
    } 

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
    size_type count( time_t time__s ) const { return m__time_series.count(time__s); }

    // Find, returns an iterator to the first time that is not less than the given time
    iterator find( time_t time__s );
    const_iterator find( time_t time__s ) const;

    // Find_pos, returns the array position to the first time that is not less than the given time
    size_type find_pos( time_t time__s ) const { return m__time_series.find_pos(time__s); }

    // No equal_range, each time should be unique.

    QuantitySeries between_t( time_t start_time__s, time_t end_time__s ) const;

    // Lower_bound, returns an iterator to the first time that is not less than the given time
    iterator lower_bound( time_t time__s );
    const_iterator lower_bound( time_t time__s ) const;

    // Lower_bound_pos, returns the array position to the last time that is not greater than the given time
    size_type lower_bound_pos( time_t time__s ) const { return m__time_series.lower_bound_pos(time__s); }
            
    // Upper_bound, returns an iterator to the first time that is greater than the given time
    iterator upper_bound( time_t time__s );
    const_iterator upper_bound( time_t time__s ) const;

    // Upper_bound_pos, returns the array position to the first time that is greater than the given time
    size_type upper_bound_pos( time_t time__s ) const { return m__time_series.upper_bound_pos(time__s); }
};

using QuantitiesMap= std::unordered_map<std::string, std::unique_ptr<QuantitySeriesBase>>;

template <typename T>
QuantitySeries<T>& extract_quantity_series(QuantitiesMap& quantities, const std::string& key) {
    auto it= quantities.find(key);
    if (it == quantities.end())
        throw std::out_of_range("QuantitySeries::extract_quantity_series: key not found");

    return dynamic_cast<QuantitySeries<T>&>(*it->second);
}

} // namespace aux
} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__QUANTITY_SERIES_HPP
