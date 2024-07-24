#ifndef BEVARMEJOLIB__WDS_DATA_STRUCTURES__TIME_SERIES_HPP
#define BEVARMEJOLIB__WDS_DATA_STRUCTURES__TIME_SERIES_HPP

#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

#include "bevarmejo/wds/data_structures/time_options.hpp"

namespace bevarmejo {
namespace wds {

static constexpr std::size_t base_value_pos = std::numeric_limits<std::size_t>::max();

template <typename T>
class TimeSeries {
/*--- Member types ---*/
public: 
    using value_type= T;
    using size_type= std::size_t;
    using difference_type= std::ptrdiff_t;
    using reference= T&;
    using const_reference= const T&;
    using pointer= T*;
    using const_pointer= const T*;

/*--- Attributes ---*/
protected:
    GlobalTimeOptions* p__glo_time_opts;

/*--- Constructors ---*/
// I need to define all the constructors and the functions that I want my derived
// classes to have as virtual functions. So that I make sure that they are
// implemented in the derived classes.
public:
    TimeSeries() : p__glo_time_opts(nullptr) {}

    TimeSeries(GlobalTimeOptions* ap_glo_time_opts) : p__glo_time_opts(ap_glo_time_opts) {}

    TimeSeries(const TimeSeries& other) : p__glo_time_opts(other.p__glo_time_opts) {}

    TimeSeries(TimeSeries&& other) noexcept : p__glo_time_opts(std::exchange(other.p__glo_time_opts, nullptr)) {}

    TimeSeries& operator=(const TimeSeries& other) {
        if (this != &other) {
            p__glo_time_opts = other.p__glo_time_opts;
        }
        return *this;
    }

    TimeSeries& operator=(TimeSeries&& other) noexcept {
        if (this != &other) {
            p__glo_time_opts = std::exchange(other.p__glo_time_opts, nullptr);
        }
        return *this;
    }

    virtual ~TimeSeries() = default;

/*--- Getters and setters ---*/
// TODO: ???

/*--- Time queries ---*/
public:
    time_t abs_t0__s() const {
        if (p__glo_time_opts == nullptr) {
            throw std::runtime_error("Global time options not set.");
        }
        return sim_t0__s() + p__glo_time_opts->shift_start_time__s;
    }

    time_t sim_t0__s() const { return 0l; }

    // For lovers of pythonic code
    time_t t0__s(bool absolute=false) const {
        return absolute ? abs_t0__s() : sim_t0__s();
    }

    time_t duration__s() const {
        if (p__glo_time_opts == nullptr) {
            throw std::runtime_error("Global time options not set.");
        }
        return p__glo_time_opts->duration__s;
    }

    time_t abs_tH__s() const {
        return abs_t0__s() + duration__s();
    }

    time_t sim_tH__s() const {
        return duration__s();
    }

    time_t tH__s(bool absolute=false) const {
        return absolute ? abs_tH__s() : sim_tH__s();
    }

/*--- Element access ---*/
public:
    virtual reference at( size_type pos ) = 0;
    virtual const_reference at( size_type pos ) const = 0;

    virtual const value_type when_sim_t( time_t time__s ) const = 0;

    const value_type when_abs_t( time_t time__s ) const {
        return this->at_sim_t(time__s - abs_t0__s());
    }
    
    const value_type when_t( time_t time__s, bool absolute=false ) const {
        return absolute ? this->when_abs_t(time__s) : this->when_sim_t(time__s);
    }

    // No operator[] because it is not safe to use it with this setup

    virtual reference front() = 0;
    virtual const_reference front() const = 0;

    virtual const value_type when_t0() const = 0;

    virtual reference back() = 0;
    virtual const_reference back() const = 0;

    virtual const value_type when_tH() const = 0;
    
    // No direct data access as it can vary depending on the derived class

/*--- Iterators ---*/
public:
    // TODO: this is the most important feature, I must make sure that I return alwasys pairs of time and value

/*--- Capacity ---*/
public:
    virtual bool empty() const noexcept = 0;

    virtual size_type size() const noexcept = 0; // Actual size of the object handling stuff under the hood
    
    virtual size_type length() const noexcept = 0; // Length of the time series (so when you iterate on it you do x steps)
    
    virtual size_type max_size() const noexcept = 0;
    
    virtual void reserve( size_type new_cap ) = 0;
    
    virtual size_type capacity() const noexcept = 0;
    
    // No shrink_to_fit

/*--- Modifiers ---*/
public:
    // I can't mark this as noexcept because the constructor of T in the
    // inherited classes could throw an exception
    virtual void clear() {
        p__glo_time_opts = nullptr;
    }

    // TODO: continue here with the modifiers and the lookup functions

}; // class TimeSeries

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_DATA_STRUCTURES__TIME_SERIES_HPP
