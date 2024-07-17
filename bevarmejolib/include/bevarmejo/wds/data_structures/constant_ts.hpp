#ifndef BEVARMEJOLIB__WDS_DATA_STRUCTURES__CONSTANT_TS_HPP
#define BEVARMEJOLIB__WDS_DATA_STRUCTURES__CONSTANT_TS_HPP

#include <stdexcept>
#include <vector>

#include "bevarmejo/wds/data_structures/time_options.hpp"
#include "bevarmejo/wds/data_structures/time_series.hpp"

namespace bevarmejo {
namespace wds {

template <typename T>
class ConstantTS final : public TimeSeries<T> {
public:
    using inherited = TimeSeries<T>;

/*--- Attributes ---*/
protected:
    T m__value;

/*--- Constructors ---*/
public:
    ConstantTS() : inherited(), m__value() {}

    ConstantTS(GlobalTimeOptions* a_glo_time_opts, T a_value) :
        inherited(a_glo_time_opts), m__value(a_value) {}

    // Variadic constructor
    template <typename... Args>
    ConstantTS(GlobalTimeOptions* a_glo_time_opts, Args&&... args) :
        inherited(a_glo_time_opts), m__value(std::forward<Args>(args)...) {}

    ConstantTS(const ConstantTS& other) : inherited(other), m__value(other.m__value) {}

    ConstantTS(ConstantTS&& other) noexcept :
        inherited(std::exchange(other.p__glo_time_opts, nullptr)), m__value(std::move(other.m__value)) {}

    ConstantTS& operator=(const ConstantTS& other) {
        if (this != &other) {
            inherited::operator=(other);
            m__value = other.m__value;
        }
        return *this;
    }

    ConstantTS& operator=(ConstantTS&& other) noexcept {
        if (this != &other) {
            inherited::operator=(std::exchange(other.p__glo_time_opts, nullptr));
            m__value = std::move(other.m__value);
        }
        return *this;
    }

    ~ConstantTS() = default;

/*--- Element access ---*/
public:
    T& at( std::size_t pos )  override {
        if (pos == 0) {
            return m__value;
        } else {
            throw std::out_of_range("ConstantTS::at: pos out of range");
        }
    }

    const T& at( std::size_t pos ) const override {
        if (pos == 0) {
            return m__value;
        } else {
            throw std::out_of_range("ConstantTS::at: pos out of range");
        }
    }

    T& at_t( const long time__s ) override {
        if (time__s >= start_time__s() && time__s <= end_time__s()) {
            return m__value;
        } else {
            throw std::out_of_range("ConstantTS::at_t: time__s out of range");
        }
    }

    const T& at_t( const long time__s ) const override {
        if (time__s >= start_time__s() && time__s <= end_time__s()) {
            return m__value;
        } else {
            throw std::out_of_range("ConstantTS::at_t: time__s out of range");
        }
    }

    // No operator []

    T& front() override {
        return m__value;
    }

    const T& front() const override {
        return m__value;
    }

    T& at_start_t() override {
        return m__value;
    }

    const T& at_start_t() const override {
        return m__value;
    }

    T& back() override {
        return m__value;
    }

    const T& back() const override {
        return m__value;
    }

    T& at_end_t() override {
        return m__value;
    }

    const T& at_end_t() const override {
        return m__value;
    }

    // No direct data access as it can vary depending on the derived class

/*--- Iterators ---*/

/*--- Capacity ---*/
public:
    bool empty() const override {
        return false;
    }

    std::size_t size() const  override {
        return 1; // The element is a constant and has size one if you try to access it with at()
    }

    std::size_t length() const  override {
        // The number of iterations you get when you iterate over the time series: 
        // Start time, end time. 
        // So, 2. This is necessary if you need to compute energy or things were you need to integrate.
        return 2; 
    }

    std::size_t max_size() const noexcept override {
        return 1;
    }

    void reserve( std::size_t new_cap ) override {
        if (new_cap > 1) {
            //TODO: print an error that you are requesting space for a constant time series
        }
    }

    std::size_t capacity() const noexcept override{
        return 1;
    }
    
    // No shrink_to_fit

/*--- Modifiers ---*/
public:
    void clear() noexcept override {
        // Do nothing
    }

}; // class ConstantTS

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_DATA_STRUCTURES__CONSTANT_TS_HPP
