#pragma once

#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "bevarmejo/bemexcept.hpp"

#include "bevarmejo/wds/auxiliary/global_times.hpp"

namespace bevarmejo {

namespace log {

namespace cname {
static const std::string time_series= "TimeSeries";
} // namespace cname

namespace fname {
static const std::string commit= "commit";
static const std::string rollback= "rollback";
} // namespace fname

} // namespace log


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
    template <typename T>
    friend class QuantitySeries;
    
/*--- Constructors ---*/
public:

    TimeSeries() = delete; // No default constructor because I have a const reference to the Global Time Options.

    TimeSeries( const GlobalTimes& a_gto );

    // Variadic constructor
    template <typename... Args,
        std::enable_if_t<!std::is_same_v<std::decay_t<Args>..., TimeSeries>, int> = 0>
    TimeSeries( const GlobalTimes& a_gto, Args&&... args ) : 
        m__gto(a_gto),
        m__time_steps(std::forward<Args>(args)...)
    {
        remove_leading_zero();
        check_valid(); 
    }

    // No normal copy and move constructor because the const reference must be 
    // assigned by a GlobalTimes object (since the TimeSeries never exposes its 
    // GT c-reference there is no risk that a TimeSeries is created outside a GT).
    // If there is a need for a new copy or to move a TS, use the extra constructors, 
    TimeSeries(const TimeSeries& other) = delete;
    TimeSeries(const GlobalTimes& a_gto, const TimeSeries& other); // Copy constructor.
    
    TimeSeries(TimeSeries&& other) noexcept = delete;
    TimeSeries(const GlobalTimes& a_gto, TimeSeries&& other) noexcept; // Move constructor.

    // Copy and move assignment operators are ok because it is assumed that only
    // the time steps are copied and not the GT reference, meaning that the TS 
    // remains linked always to the same GT for the entirety of its lifetime.
    TimeSeries& operator=(const TimeSeries& other);
    TimeSeries& operator=(TimeSeries&& other) noexcept;

    ~TimeSeries() = default;
    
/*--- Element access ---*/
public:
    // All access, is by value, so I can't modify the time steps.
    // This is done to avoid the time steps to be modified without checking the validity of the time series.
    // Functions are templatised to allow to switch between absolute and relative time.
    // Since the two are technically two different things and should not be mixed, 
    // you should know at compile time which one you are using. 
    // Default is always relative time.
    using DefaultTime= time::RelativeTime;

    template <typename RTT = DefaultTime>
    value_type at( size_type pos ) {
        return const_cast<const TimeSeries*>(this)->at<RTT>(pos);
    }

    template <typename RTT = DefaultTime>
    const value_type at( size_type pos ) const {

        static_assert(std::is_same<RTT, time::RelativeTime>::value || std::is_same<RTT, time::AbsoluteTime>::value, 
            "TimeSeries::at<T>: T must be time::RelativeTime or time::AbsoluteTime.");

        if constexpr (std::is_same_v<RTT, time::AbsoluteTime>) 
            return this->at<time::RelativeTime>(pos) + m__gto.shift_start_time__s();
        

    // ======== Actual Implementation (hyp: RTT == time::RelativeTime)========

        if (pos == 0)
            return value_type{0};

        if (pos <= m__time_steps.size() )
            return m__time_steps[pos-1];

        __format_and_throw<std::out_of_range>(log::cname::time_series, log::fname::at, 
                                                "Index out of range.", 
            "Valid index range: [0, ", m__time_steps.size(), "]\n",
            "Index = ", pos);
    }

    // No operator[] because it is not safe to use it with this setup

    template <typename RTT = DefaultTime>
    value_type front() {
        return const_cast<const TimeSeries*>(this)->front<RTT>();
    }

    template <typename RTT = DefaultTime>
    const value_type front() const {
        
        static_assert(std::is_same<RTT, time::RelativeTime>::value || std::is_same<RTT, time::AbsoluteTime>::value, 
            "TimeSeries::front<T>: T must be time::RelativeTime or time::AbsoluteTime.");

        if constexpr (std::is_same_v<RTT, time::AbsoluteTime>) 
            return this->front<time::RelativeTime>() + m__gto.shift_start_time__s();

    // ======== Actual Implementation (hyp: RTT == time::RelativeTime) ========

        return value_type{0};
    }

    template <typename RTT = DefaultTime>
    value_type back() {
        return const_cast<const TimeSeries*>(this)->back<RTT>();
    }

    template <typename RTT = DefaultTime>
    const value_type back() const {
        
        static_assert(std::is_same<RTT, time::RelativeTime>::value || std::is_same<RTT, time::AbsoluteTime>::value, 
            "TimeSeries::back<T>: T must be time::RelativeTime or time::AbsoluteTime.");

        if constexpr (std::is_same_v<RTT, time::AbsoluteTime>) 
            return this->back<time::RelativeTime>() + m__gto.shift_start_time__s();
            
        // ======== Actual Implementation (hyp: RTT == time::RelativeTime) ========

        if (m__time_steps.empty())
            return value_type{0};

        return m__time_steps.back();
    }

    // No direct access to the time series! But they can be read. I return a copy.

    template <typename RTT = DefaultTime>
    container time_steps() const {
        static_assert(std::is_same<RTT, time::RelativeTime>::value || std::is_same<RTT, time::AbsoluteTime>::value, 
            "TimeSeries::time_steps<T>: T must be time::RelativeTime or time::AbsoluteTime.");

        container out_time_steps(m__time_steps.size()+1);

        if constexpr (std::is_same_v<RTT, time::AbsoluteTime>) {
            out_time_steps[0]= m__gto.shift_start_time__s();

            for (size_type i=0; i<m__time_steps.size(); ++i)
                out_time_steps[i+1]= m__time_steps[i] + m__gto.shift_start_time__s();
        }
        else {
            out_time_steps[0]= 0;

            for (size_type i=0; i<m__time_steps.size(); ++i)
                out_time_steps[i+1]= m__time_steps[i];
        }

        return out_time_steps;
    }

/*--- Iterators ---*/
public: 

// As said before, there are different type of iterators:
// - Normal iterators go through all the time steps, independently from what durations is.
// - Truncated iterators go through the time steps, but they stop at the duration of the GT.
// - Special iterators for the QuantitySeries are available and they return time and the index 
//      for the quantity series from which to extract the value.
// As the normal access methods, the iterators are templatised to allow to switch between
// absolute and relative time. Default is always relative time.
// The normal iterator can be seen as a truncated iterator with infinite duration (or for practical reasons the last time step).
private:
    template <typename TS, typename RTT= DefaultTime>
    class Iterator {
    public:
        using iterator_category= std::random_access_iterator_tag;
        using value_type= typename TS::value_type;
        using difference_type= typename TS::difference_type;
        using pointer= typename TS::const_pointer;
        using reference= typename TS::const_reference;

        static constexpr size_type duration_index() { return std::numeric_limits<size_type>::max(); }
        
    private:
        TS* m__time_series;
        size_type m__index;
        const size_type m__end_index;
        const time::Instant m__end_t;

    public:
        Iterator(TS* time_series, size_type index) : 
            Iterator<TS, RTT>(time_series, index, time_series->template back<RTT>()) {}
        Iterator(TS* time_series, size_type index, time::Instant a_end_t) : 
            m__time_series(time_series), m__index(index),
            m__end_index(time_series->template lower_bound_pos<RTT>(a_end_t)), m__end_t(a_end_t) {}

        value_type operator*() const {
            if (m__index == duration_index())
                return m__end_t; // I am assuming that it was already passed as the correct type of time.

            return m__time_series->template at<RTT>(m__index); 
        }

        pointer operator->() const = delete; // We only return by value.
        value_type operator[](difference_type n) const { return *(*this + n); }

        Iterator& operator++() {
            if (m__index < m__end_index) {
                ++m__index;
            }
            else if (m__index == m__end_index) {
                m__index = duration_index();
            }
            else if (m__index == duration_index()) {
                m__index = m__time_series->size();
            }
            else {
                assert(false && "Incrementing end iterator");
            }

            return *this;
        }
        Iterator operator++(int) { auto tmp= *this; ++(*this); return tmp; }

        Iterator& operator--() {
            if (m__index == m__time_series->size()) {
                m__index = duration_index();
            }
            else if (m__index == duration_index()) {
                m__index = m__end_index;
            }
            else if (m__index > 0) {
                --m__index;
            }
            else {
                assert(false && "Decrementing begin iterator");
            }

            return *this;
        }
        Iterator operator--(int) { auto tmp= *this; --(*this); return tmp; }

        Iterator& operator+=(difference_type n)
        {
            while (n)
            {
                if (n > 0)
                {
                    ++(*this);
                    --n;
                }
                else
                {
                    --(*this);
                    ++n;
                }
            }

            return *this;
        }
        Iterator operator+(difference_type n) const { auto tmp= *this; return tmp += n; }
        Iterator& operator-=(difference_type n) { return (*this += (-n)); }
        Iterator operator-(difference_type n) const { return (*this + (-n)); }
        difference_type operator-(const Iterator& other) const { return m__index - other.m__index; }

        bool operator==(const Iterator& other) const { return m__index == other.m__index; }
        bool operator!=(const Iterator& other) const { return m__index != other.m__index; }
        bool operator<(const Iterator& other) const { return m__index < other.m__index; }
        bool operator>(const Iterator& other) const { return m__index > other.m__index; }
        bool operator<=(const Iterator& other) const { return m__index <= other.m__index; }
        bool operator>=(const Iterator& other) const { return m__index >= other.m__index; }
    };
    
    template <typename TS, typename RTT= DefaultTime>
    class ReverseIterator {
    public:
        using iterator_category= std::random_access_iterator_tag;
        using value_type= typename TS::value_type;
        using difference_type= typename TS::difference_type;
        using pointer= typename TS::const_pointer;
        using reference= typename TS::const_reference;

        static constexpr size_type duration_index() { return std::numeric_limits<size_type>::max(); }

    private:
        TS* m__time_series;
        size_type m__index;
        const size_type m__end_index;
        const time::Instant m__end_t;

    public:
        ReverseIterator(TS* time_series, size_type index) : 
            ReverseIterator<TS, RTT>(time_series, index, time_series->template back<RTT>()) {}
        ReverseIterator(TS* time_series, size_type index, time::Instant a_end_t) : 
            m__time_series(time_series), m__index(index),
            m__end_index(time_series->template lower_bound_pos<RTT>(a_end_t)+1), m__end_t(a_end_t) {}

        value_type operator*() const { 
            if (m__index == duration_index())
                return m__end_t; 

            return m__time_series->template at<RTT>(m__index-1);
        }
        pointer operator->() const = delete; // We only return by value.
        value_type operator[](difference_type n) const { return *(*this + n); }

        ReverseIterator& operator++() {
            if (m__index == duration_index()) {
                m__index = m__end_index;
            }
            else if (m__index > 0) {
                --m__index;
            }
            else {
                assert(false && "Incrementing rend iterator");
            }

            return *this;
        }
        ReverseIterator operator++(int) { auto tmp= *this; ++(*this); return tmp; }

        ReverseIterator& operator--() {
            if (m__index < m__end_index) {
                ++m__index;
            }
            else if (m__index == m__end_index) {
                m__index = duration_index();
            }
            else { // if (m__index == duration_index()) 
                assert(false && "Decrementing rbegin iterator");
            }

            return *this;
        }
        ReverseIterator operator--(int) { auto tmp= *this; --(*this); return tmp; }

        ReverseIterator& operator+=(difference_type n)
        {
            
            while (n)
            {
                if (n > 0)
                {
                    ++(*this);
                    --n;
                }
                else
                {
                    --(*this);
                    ++n;
                }
            }

            return *this;
        }
        ReverseIterator operator+(difference_type n) const { auto tmp= *this; return tmp += n; }
        ReverseIterator& operator-=(difference_type n) { return (*this += (-n)); }
        ReverseIterator operator-(difference_type n) const { return (*this + (-n)); }
        // Reverse the logic of the comparison operators.
        difference_type operator-(const ReverseIterator& other) const { return other.m__index - m__index; }

        bool operator==(const ReverseIterator& other) const { return m__index == other.m__index; }
        bool operator!=(const ReverseIterator& other) const { return m__index != other.m__index; }
        // Reverse the logic of the comparison operators.
        bool operator<(const ReverseIterator& other) const { return m__index > other.m__index; }
        bool operator>(const ReverseIterator& other) const { return m__index < other.m__index; }
        bool operator<=(const ReverseIterator& other) const { return m__index >= other.m__index; }
        bool operator>=(const ReverseIterator& other) const { return m__index <= other.m__index; }
    };

public:
    // all iterators are const because the time series can not be modified directly but only through the methods.
    template <typename RTT = DefaultTime>
    using iterator= Iterator<const TimeSeries, RTT>;
    template <typename RTT = DefaultTime>
    using const_iterator= Iterator<const TimeSeries, RTT>;
    template <typename RTT = DefaultTime>
    using reverse_iterator= ReverseIterator<const TimeSeries, RTT>;
    template <typename RTT = DefaultTime>
    using const_reverse_iterator= ReverseIterator<const TimeSeries, RTT>;

    template <typename RTT = DefaultTime>
    iterator<RTT> begin() noexcept { return iterator<RTT>(this, 0); }
    template <typename RTT = DefaultTime>
    const_iterator<RTT> begin() const noexcept { return const_iterator<RTT>(this, 0); }
    template <typename RTT = DefaultTime>
    const_iterator<RTT> cbegin() const noexcept { return const_iterator<RTT>(this, 0); }

    template <typename RTT = DefaultTime>
    iterator<RTT> end() noexcept { return iterator<RTT>(this, this->size()); }
    template <typename RTT = DefaultTime>
    const_iterator<RTT> end() const noexcept { return const_iterator<RTT>(this, this->size()); }
    template <typename RTT = DefaultTime>
    const_iterator<RTT> cend() const noexcept { return const_iterator<RTT>(this, this->size()); }

    template <typename RTT = DefaultTime>
    reverse_iterator<RTT> rbegin() noexcept { return reverse_iterator<RTT>(this, this->size()); }
    template <typename RTT = DefaultTime>
    const_reverse_iterator<RTT> rbegin() const noexcept { return const_reverse_iterator<RTT>(this, this->size()); }
    template <typename RTT = DefaultTime>
    const_reverse_iterator<RTT> crbegin() const noexcept { return const_reverse_iterator<RTT>(this, this->size()); }

    template <typename RTT = DefaultTime>
    reverse_iterator<RTT> rend() noexcept { return reverse_iterator<RTT>(this, 0); }
    template <typename RTT = DefaultTime>
    const_reverse_iterator<RTT> rend() const noexcept { return const_reverse_iterator<RTT>(this, 0); }
    template <typename RTT = DefaultTime>
    const_reverse_iterator<RTT> crend() const noexcept { return const_reverse_iterator<RTT>(this, 0); }

    // Truncated iterators
    template <typename RTT = DefaultTime>
    iterator<RTT> tbegin(time::Instant end_t) noexcept { return iterator<RTT>(this, 0, end_t); }
    template <typename RTT = DefaultTime>
    const_iterator<RTT> tbegin(time::Instant end_t) const noexcept { return const_iterator<RTT>(this, 0, end_t); }
    template <typename RTT = DefaultTime>
    const_iterator<RTT> tcbegin(time::Instant end_t) const noexcept { return const_iterator<RTT>(this, 0, end_t); }

    template <typename RTT = DefaultTime>
    iterator<RTT> tend(time::Instant end_t) noexcept { return iterator<RTT>(this, this->size(), end_t); }
    template <typename RTT = DefaultTime>
    const_iterator<RTT> tend(time::Instant end_t) const noexcept { return const_iterator<RTT>(this, this->size(), end_t); }
    template <typename RTT = DefaultTime>
    const_iterator<RTT> tcend(time::Instant end_t) const noexcept { return const_iterator<RTT>(this, this->size(), end_t); }

    template <typename RTT = DefaultTime>
    reverse_iterator<RTT> trbegin(time::Instant end_t) noexcept { return reverse_iterator<RTT>(this, this->size(), end_t); }
    template <typename RTT = DefaultTime>
    const_reverse_iterator<RTT> trbegin(time::Instant end_t) const noexcept { return const_reverse_iterator<RTT>(this, this->size(), end_t); }
    template <typename RTT = DefaultTime>
    const_reverse_iterator<RTT> tcrbegin(time::Instant end_t) const noexcept { return const_reverse_iterator<RTT>(this, this->size(), end_t); }

    template <typename RTT = DefaultTime>
    reverse_iterator<RTT> trend(time::Instant end_t) noexcept { return reverse_iterator<RTT>(this, 0, end_t); }
    template <typename RTT = DefaultTime>
    const_reverse_iterator<RTT> trend(time::Instant end_t) const noexcept { return const_reverse_iterator<RTT>(this, 0, end_t); }
    template <typename RTT = DefaultTime>
    const_reverse_iterator<RTT> tcrend(time::Instant end_t) const noexcept { return const_reverse_iterator<RTT>(this, 0, end_t); }

/*--- Capacity ---*/
public:
    
    bool empty() const noexcept;

    size_type size() const noexcept;

    size_type max_size() const noexcept;

    void reserve( size_type new_cap );

    size_type capacity() const noexcept;

/*--- Modifiers ---*/
public:

    void reset() noexcept;

    // Insert takes care automatically of the monotonicity of the time steps. It works like a insert or assign.
    // However, you can pass a position to insert the time step to be faster :)
    template <typename RTT = DefaultTime>
    iterator<RTT> insert( time::Instant time__s );
    template <typename RTT = DefaultTime>
    iterator<RTT> insert( const_iterator<RTT> pos, time::Instant time__s );
    template <class InputIt, typename RTT = DefaultTime>
    iterator<RTT> insert( const_iterator<RTT> pos, InputIt first, InputIt last );
    template <typename RTT = DefaultTime>
    iterator<RTT> insert( std::initializer_list<time::Instant> ilist );
    template <typename RTT = DefaultTime>
    iterator<RTT> insert( const_iterator<RTT> pos, std::initializer_list<time::Instant> ilist );


    template <typename RTT = DefaultTime>
    iterator<RTT> erase( const_iterator<RTT> pos );
    template <typename RTT = DefaultTime>
    iterator<RTT> erase( const_iterator<RTT> first, const_iterator<RTT> last );
    template <typename RTT = DefaultTime>
    iterator<RTT> erase( time::Instant time__s );

    // Push back has a different name more related to a time series.
    template <typename RTT = DefaultTime>
    void commit( time::Instant time__s ) {
        static_assert(std::is_same<RTT, time::RelativeTime>::value || std::is_same<RTT, time::AbsoluteTime>::value, 
            "TimeSeries::commit<T>: T must be time::RelativeTime or time::AbsoluteTime.");

        if constexpr (std::is_same_v<RTT, time::AbsoluteTime>)
            this->commit<time::RelativeTime>(time__s - m__gto.shift_start_time__s());

        // ======== Actual Implementation (hyp: RTT == time::RelativeTime) ========

        if ( time__s < 0 )
            __format_and_throw<std::invalid_argument>(log::cname::time_series, log::fname::commit, 
                                                        "Time steps must be >= 0.",
                "Time step = ", time__s);
                
        if (m__time_steps.empty() && time__s == 0)
            return; // No problem if you commit the zero time as the first time step. As I expect to commit the zero at the beginning of every simulation.

        if (!m__time_steps.empty() && time__s <= m__time_steps.back())
            __format_and_throw<std::invalid_argument>(log::cname::time_series, log::fname::commit, 
                                                        "Time steps must be monotonic.",
                "Back of the TimeSeries = ", m__time_steps.back(), "\n",
                "Time step = ", time__s);
                
        m__time_steps.push_back(time__s);
    }

    // Pop back has a different name more related to a time series.
    void rollback();

    // Resize should be called when the duration of the gto changes. 
    // Failing to call this method after changing the duration of the GTO will result in an invalid time series.
    
    void resize( size_type count );

    // No emplace, try_emplace, emplace_hint, emplace_back as they are simple longs.

    // No swap, because I can't swap the GTO.

    // No extract.

    // TODO: define merge and other useful operations for time series.

/*--- Lookup ---*/
public:
    // Like access methods, lookup methods are templatised to allow to switch between absolute and relative time.

    // Each time should be unique or non existing. Only duration can be repeated and return 2.
    template <typename RTT = DefaultTime>
    size_type count ( time::Instant time__s ) const;

    // Find, returns an iterator to the instant that is exactly the given time
    template <typename RTT = DefaultTime>
    iterator<RTT> find( time::Instant time__s ) { return iterator<RTT>(this, find_pos<RTT>(time__s)); }
    template <typename RTT = DefaultTime>
    const_iterator<RTT> find( time::Instant time__s ) const { return const_iterator<RTT>(this, find_pos<RTT>(time__s)); }

    // Find_pos, returns the array position to the instant that is exactly the given time
    template <typename RTT = DefaultTime>
    size_type find_pos( time::Instant time__s ) const {

        static_assert(std::is_same<RTT, time::RelativeTime>::value || std::is_same<RTT, time::AbsoluteTime>::value, 
            "TimeSeries::find_pos<T>: T must be time::RelativeTime or time::AbsoluteTime.");

        if constexpr (std::is_same_v<RTT, time::AbsoluteTime>) 
            return this->find_pos<time::RelativeTime>(time__s - m__gto.shift_start_time__s());

        // ======== Actual Implementation (hyp: RTT == time::RelativeTime) ========

        if (time__s < 0)
            return this->size();

        if (time__s == 0)
            return 0;

        size_type pos = 0;
        while (pos < m__time_steps.size() ) {
            if (m__time_steps[pos] == time__s)
                return pos+1;
            ++pos;
        }

        return this->size();
    }

    // Contains, returns true if the time is in the time series.
    template <typename RTT = DefaultTime>
    bool contains( time::Instant time__s ) const;

    // Lower_bound, returns an iterator to the last time that is less than the given time.
    template <typename RTT = DefaultTime>
    iterator<RTT> lower_bound( time::Instant time__s ) { return iterator<RTT>(this, lower_bound_pos<RTT>(time__s)); }
    template <typename RTT = DefaultTime>
    const_iterator<RTT> lower_bound( time::Instant time__s ) const { return const_iterator<RTT>(this, lower_bound_pos<RTT>(time__s)); }

    // Lower_bound_pos, returns the array position to the last time that is less than the given time
    template <typename RTT = DefaultTime>
    size_type lower_bound_pos( time::Instant time__s ) const {

        static_assert(std::is_same<RTT, time::RelativeTime>::value || std::is_same<RTT, time::AbsoluteTime>::value, 
            "TimeSeries::find_pos<T>: T must be time::RelativeTime or time::AbsoluteTime.");

        if constexpr (std::is_same_v<RTT, time::AbsoluteTime>) 
            return this->find_pos<time::RelativeTime>(time__s - m__gto.shift_start_time__s());

        // ======== Actual Implementation (hyp: RTT == time::RelativeTime) ========

        if (time__s <= 0)
            return this->size();

        size_type pos = 0;
        while (pos < m__time_steps.size() ) {
            if (m__time_steps[pos] >= time__s)
                return pos; // No plus one because I want the first time that is not less than the given time. And the previous is less one, but plus one of the zero.
            ++pos;
        }

        return pos; // If I reach the end, the lower bound is the last.   
    }
            
    // Upper_bound, returns an iterator to the first time that is greater than the given time
    template <typename RTT = DefaultTime>
    iterator<RTT> upper_bound( time::Instant time__s );
    template <typename RTT = DefaultTime>
    const_iterator<RTT> upper_bound( time::Instant time__s ) const;

    // Upper_bound_pos, returns the array position to the first time that is greater than the given time
    template <typename RTT = DefaultTime>
    size_type upper_bound_pos( time::Instant time__s ) const;

/*--- Other methods ---*/
private:

    // Remove the first leading zero.
    void remove_leading_zero();

    // Check if the time series is valid.
    void check_valid() const;

};

} // namespace aux
} // namespace wds
} // namespace bevarmejo
