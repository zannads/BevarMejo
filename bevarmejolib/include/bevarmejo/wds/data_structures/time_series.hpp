#ifndef BEVARMEJOLIB__WDS_DATA_STRUCTURES__TIME_SERIES_HPP
#define BEVARMEJOLIB__WDS_DATA_STRUCTURES__TIME_SERIES_HPP

#include <vector>

#include "bevarmejo/wds/data_structures/time_options.hpp"

namespace bevarmejo {
namespace wds {

template <typename T>
class TimeSeries {

/*--- Attributes ---*/
protected:
    GlobalTimeOptions* p__glo_time_opts;

/*--- Constructors ---*/
// I need to define all the constructors and the functions that I want my derived
// classes to have as virtual functions. So that I make sure that they are
// implemented in the derived classes.
public:
    TimeSeries() : p__glo_time_opts(nullptr) {}

    TimeSeries(GlobalTimeOptions* a_glo_time_opts) : p__glo_time_opts(a_glo_time_opts) {}

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

/*--- Queries ---*/
public:
    long start_time__s(bool absolute = false) const  override {
        return absolute ? p__glo_time_opts->shift_start_time__s : 0;
    }

    long duration__s() const  override {
        return p__glo_time_opts->duration__s;
    }

    long end_time__s(bool absolute = false) const  override {
        return absolute ? p__glo_time_opts->shift_start_time__s + p__glo_time_opts->duration__s : p__glo_time_opts->duration__s;
    }

/*--- Element access ---*/
public:
    virtual T& at( std::size_t pos ) = 0;
    virtual const T& at( std::size_t pos ) const = 0;
    virtual T& at_t( const long time__s ) = 0;
    virtual const T& at_t( const long time__s ) const = 0;

    // No operator []

    virtual T& front() = 0;
    virtual const T& front() const = 0;
    virtual T& at_start_t() = 0;
    virtual const T& at_start_t() const = 0;

    virtual T& back() = 0;
    virtual const T& back() const = 0;
    virtual T& at_end_t() = 0;
    virtual const T& at_end_t() const = 0;
    
    // No direct data access as it can vary depending on the derived class

/*--- Iterators ---*/
public:
    // TODO: this is the most important feature, I must make sure that I return alwasys pairs of time and value

/*--- Capacity ---*/
public:
    virtual bool empty() const = 0;
    virtual std::size_t size() const = 0; // Actual size of the object handling stuff under the hood
    virtual std::size_t length() const = 0; // Length of the time series (so when you iterate on it you do x steps)
    virtual std::size_t max_size() const noexcept = 0;
    virtual void reserve( std::size_t new_cap ) = 0;
    virtual std::size_t capacity() const noexcept = 0;
    
    // No shrink_to_fit

/*--- Modifiers ---*/
public:
    virtual void clear() noexcept = 0;

    // TODO: continue here with the modifiers and the lookup functions

}; // class TimeSeries

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_DATA_STRUCTURES__TIME_SERIES_HPP
