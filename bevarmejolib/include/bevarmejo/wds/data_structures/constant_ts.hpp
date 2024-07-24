#ifndef BEVARMEJOLIB__WDS_DATA_STRUCTURES__CONSTANT_TS_HPP
#define BEVARMEJOLIB__WDS_DATA_STRUCTURES__CONSTANT_TS_HPP

#include <stdexcept>
#include <utility>
#include <vector>

#include "bevarmejo/wds/data_structures/time_options.hpp"
#include "bevarmejo/wds/data_structures/time_series.hpp"

namespace bevarmejo {
namespace wds {

template <typename T>
class ConstantTS final : public TimeSeries<T> {
/*--- Member types ---*/
public:
    using inherited = TimeSeries<T>;
    using value_type = T;
    using size_type = typename inherited::size_type;
    using difference_type = typename inherited::difference_type;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;

/*--- Attributes ---*/
protected:
    T m__value;

/*--- Constructors ---*/
public:
    ConstantTS() : 
        inherited(),
        m__value() {}

    ConstantTS(GlobalTimeOptions* a_glo_time_opts) : 
        inherited(a_glo_time_opts),
        m__value() {}

    ConstantTS(GlobalTimeOptions* a_glo_time_opts, T a_value) :
        inherited(a_glo_time_opts),
        m__value(a_value) {}

    // Variadic constructor
    template <typename... Args>
    ConstantTS(GlobalTimeOptions* a_glo_time_opts, Args&&... args) :
        inherited(a_glo_time_opts),
        m__value(std::forward<Args>(args)...) {}

    ConstantTS(const ConstantTS& other) = default;

    ConstantTS(ConstantTS&& other) noexcept = default;

    ConstantTS& operator=(const ConstantTS& other) = default;

    ConstantTS& operator=(ConstantTS&& other) noexcept = default;

    virtual ~ConstantTS() = default;

/*--- Getters and setters ---*/
// TODO: ???

/*--- Element access ---*/
public:
    reference at( size_type pos ) override {
        if (pos == 0 || pos == base_value_pos) {
            return m__value;
        } else {
            throw std::out_of_range("ConstantTS::at: pos out of range");
        }
    }

    const_reference at( size_type pos ) const override {
        if (pos == 0 || pos == base_value_pos) {
            return m__value;
        } else {
            throw std::out_of_range("ConstantTS::at: pos out of range");
        }
    }

    const value_type when_sim_t( long time__s ) const override {
        if (time__s >= this->sim_t0__s() && time__s <= this->sim_tH__s()) {
            return m__value;
        } else {
            throw std::out_of_range("ConstantTS::at_sim_t: time__s out of range");
        }
    }

    // No operator []

    reference front() override { return m__value; }
    const_reference front() const override { return m__value; }

    const value_type when_t0() const override { return m__value; }

    reference back() override { return m__value; }
    const_reference back() const override { return m__value; }

    const value_type when_tH() const override { return m__value; }

    // No direct data access as it can vary depending on the derived class

/*--- Iterators ---*/
public:
    // TODO: this is the most important feature, I must make sure that I return alwasys pairs of time and value

/*--- Capacity ---*/
public:
    bool empty() const noexcept override { return false; }

    // The element is a constant and has size one if you try to access it with at()
    size_type size() const noexcept override{ return 1; }

    // The number of iterations you get when you iterate over the time series: 
        // Start time, end time. 
    // So, 2. This is necessary if you need to compute energy or things were you need to integrate.
    size_type length() const noexcept override { return 2; }

    size_type max_size() const noexcept override { return 1; }

    void reserve( size_type new_cap ) override { return; } // Do nothing

    size_type capacity() const noexcept override{ return 1; }
    
    // No shrink_to_fit

/*--- Modifiers ---*/
public:
    void clear() override {
        inherited::clear();
        m__value = T();
    } 

}; // class ConstantTS

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_DATA_STRUCTURES__CONSTANT_TS_HPP
