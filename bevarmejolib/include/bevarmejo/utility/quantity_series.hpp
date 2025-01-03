#ifndef BEVARMEJO__UTILITY__QUANTITY_SERIES_HPP
#define BEVARMEJO__UTILITY__QUANTITY_SERIES_HPP

#include <vector>

#include "bevarmejo/utility/time.hpp"

// Quantity Series is the detachted representation of a time series of quantities.
// Differently from the combo wds::aux::TimeSeries and wds::aux::QuantitySeries,
// this class is not bound to a specific time series, but it is a general representation
// that can live on its own.

namespace bevarmejo
{

template <typename T>
class QuantitySeries
{
/*----------------------------------------------------------------------------*/
/*---------------------------- Member types ----------------------------------*/
/*----------------------------------------------------------------------------*/
public:
    using mapped_type = T;
    using key_type = bevarmejo::time_t;
private:
    struct instance
    {
        const key_type time;
        mapped_type value;
    };
    using Container = std::vector<instance>;
public:
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using value_type = instance;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = typename Container::iterator;
    using const_iterator = typename Container::const_iterator;
    using reverse_iterator = typename Container::reverse_iterator;
    using const_reverse_iterator = typename Container::const_reverse_iterator;
    struct insert_return_type
    {
        iterator iterator;
        size_type n_inserted;
    };

/*----------------------------------------------------------------------------*/
/*---------------------------- Member objects --------------------------------*/
/*----------------------------------------------------------------------------*/
private:
    Container m__data;

/*----------------------------------------------------------------------------*/
/*--------------------------- Member functions -------------------------------*/
/*----------------------------------------------------------------------------*/
// (constructor)
public:
    QuantitySeries() = default;
    QuantitySeries(const QuantitySeries& other) = default;
    QuantitySeries(QuantitySeries&& other) noexcept = default;
    
// (destructor)
public:
    ~QuantitySeries() = default;

// operator=
public:
    QuantitySeries& operator=(const QuantitySeries& other) = default;
    QuantitySeries& operator=(QuantitySeries&& other) noexcept = default;

// assign
public:

// ---------------------------- Element access --------------------------------
public:
    // access specific element with bounds checking
    reference at( size_type pos );
    // access specific element with bounds checking
    const_reference at( size_type pos ) const;

    // No operator[] 
    
    // access the first element
    reference front();
    // access the first element
    const_reference front() const;

    // access the last element
    reference back();
    // access the last element
    const_reference back() const;

    // get a copy of the underlying data
    Container data() const;

// ---------------------------- Iterators -------------------------------------
public:
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

// ---------------------------- Capacity --------------------------------------
public:
    // checks whether the container is empty
    bool empty() const noexcept;

    // returns the number of elements
    size_type size() const noexcept;

    // returns the maximum possible number of elements
    size_type max_size() const noexcept;

    // reserves storage
    void reserve( size_type new_cap );

    // returns the number of elements that can be held in currently allocated storage
    size_type capacity() const noexcept;

    // reduces memory usage by freeing unused memory
    void shrink_to_fit();

// ---------------------------- Modifiers -------------------------------------
public:
    // clears the contents
    void clear() noexcept;

    // inserts elements
    insert_return_type insert( const value_type& value );
    // inserts elements
    insert_return_type insert( value_type&& value );

    // constructs element in-place
    template <class... Args>
    insert_return_type emplace( Args&&... args );

    // erases elements
    size_type erase( const_iterator pos );
    // erases elements
    size_type erase( const key_type& key );

    // no push_back

    // no pop_back

    // no resize

    // swaps the contents
    void swap ( QuantitySeries& other ) noexcept;
    
    // extract the underlying data
    instance extract( const_iterator pos );
    // extract the underlying data
    instance extract( const key_type& key );

    // merge two quantity series
    size_type merge( QuantitySeries& source );
    // merge two quantity series
    size_type merge( QuantitySeries&& source );

// ---------------------------- Lookup ----------------------------------------
// count
public:
    // returns the number of elements with specific key (noexcept because key comparison can not throw)
    size_type count( const key_type& key ) const noexcept
    {
        return std::count_if(m__data.begin(), m__data.end(), [&key](const instance& inst){ return inst.time == key; });
    }

// find
public:
    // finds an element with specific key (noexcept because key comparison can not throw)
    iterator find( const key_type& key ) noexcept;
    // finds an element with specific key (noexcept because key comparison can not throw)
    const_iterator find( const key_type& key ) const noexcept;

// find index
private:
    // finds the index of an element with specific key (noexcept because key comparison can not throw)
    size_type find_index( const key_type& key ) const noexcept;

// contains
public:
    // checks if an element with specific key exists (noexcept because key comparison can not throw)
    bool contains( const key_type& key ) const noexcept;

// equal range
public:
    // returns a range containing all elements with specific key
    std::pair<iterator, iterator> equal_range( const key_type& key );
    // returns a range containing all elements with specific key
    std::pair<const_iterator, const_iterator> equal_range( const key_type& key ) const;

// lower bound
public:
    // returns an iterator to the first element with key not less than specific key
    iterator lower_bound( const key_type& key );
    // returns an iterator to the first element with key not less than specific key
    const_iterator lower_bound( const key_type& key ) const;
private:
    // return the index of the first element with key not less than specific key
    size_type lower_bound_index( const key_type& key ) const;

// upper bound
public:
    // returns an iterator to the first element with key greater than specific key
    iterator upper_bound( const key_type& key );
    // returns an iterator to the first element with key greater than specific key
    const_iterator upper_bound( const key_type& key ) const;
private:
    // return the index of the first element with key greater than specific key
    size_type upper_bound_index( const key_type& key ) const;

// when
public:
    // get a copy of a quantity at a specific time
    mapped_type when( const key_type& key ) const;

}; // class QuantitySeries<T>

} // namespace bevarmejo

#endif // BEVARMEJO__UTILITY__QUANTITY_SERIES_HPP
