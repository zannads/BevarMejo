#ifndef BEVARMEJOLIB__WDS_DATA_STRUCTURES__REGULAR_TS_HPP
#define BEVARMEJOLIB__WDS_DATA_STRUCTURES__REGULAR_TS_HPP

#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

#include "bevarmejo/wds/data_structures/time_options.hpp"
#include "bevarmejo/wds/data_structures/time_series.hpp"
#include "bevarmejo/wds/data_structures/pattern.hpp"

namespace bevarmejo {
namespace wds {

template <typename T>
class RegularTS final : public TimeSeries<T> {
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
    PatternTimeOptions* p__pat_time_opts;
    std::shared_ptr<Pattern> p__pattern;
    T m__value;

/*--- Constructors ---*/
public:
    RegularTS() : 
        inherited(), 
        p__pat_time_opts(nullptr), 
        p__pattern(nullptr), 
        m__value() {}

    RegularTS(GlobalTimeOptions* a_glo_time_opts, PatternTimeOptions* a_pat_time_opts) :
        inherited(a_glo_time_opts), 
        p__pat_time_opts(a_pat_time_opts), 
        p__pattern(nullptr), 
        m__value() {}

    RegularTS(  GlobalTimeOptions* a_glo_time_opts,
                PatternTimeOptions* a_pat_time_opts,
                std::shared_ptr<Pattern> a_pattern) :
        inherited(a_glo_time_opts),
        p__pat_time_opts(a_pat_time_opts),
        p__pattern(a_pattern),
        m__value() {}

    RegularTS(  GlobalTimeOptions* a_glo_time_opts,
                PatternTimeOptions* a_pat_time_opts,
                std::shared_ptr<Pattern> a_pattern,
                T a_value) :
        inherited(a_glo_time_opts),
        p__pat_time_opts(a_pat_time_opts),
        p__pattern(a_pattern),
        m__value(a_value) {}

    // Variadic constructor
    template <typename... Args>
    RegularTS(  GlobalTimeOptions* a_glo_time_opts,
                PatternTimeOptions* a_pat_time_opts,
                std::shared_ptr<Pattern> a_pattern,
                Args&&... args) :
        inherited(a_glo_time_opts),
        p__pat_time_opts(a_pat_time_opts),
        p__pattern(a_pattern),
        m__value(std::forward<Args>(args)...) {}

    RegularTS(const RegularTS& other) = default;

    RegularTS(RegularTS&& other) noexcept = default;

    RegularTS& operator=(const RegularTS& other) = default;

    RegularTS& operator=(RegularTS&& other) noexcept = default;

    virtual ~RegularTS() = default;

/*--- Getters and setters ---*/
// TODO: ???

/*--- Element access ---*/
public:
    // The at function access the multiplier of the pattern and not the value of the time series
    reference at( std::size_t pos ) override {
        // Check all the elements point to something real, otherwise throw an exception
        if (inherited::p__glo_time_opts == nullptr) {
            throw std::runtime_error("Global time options not set.");
        }
        if (p__pat_time_opts == nullptr) {
            throw std::runtime_error("Pattern time options not set.");
        }
        if (p__pattern == nullptr) {
            throw std::runtime_error("Pattern not set.");
        }

        if (pos < p__pattern->size()) {
            return p__pattern->at(pos);
        } else if (pos == base_value_pos) {
            return m__value;
        } else {
            throw std::out_of_range("RegularTS::at: pos out of range (either between 0 and size for the multipliers or max for the value)");
        }
    }

    const_reference at(std::size_t pos ) const override {
        // Check all the elements point to something real, otherwise throw an exception
        if (inherited::p__glo_time_opts == nullptr) {
            throw std::runtime_error("Global time options not set.");
        }
        if (p__pat_time_opts == nullptr) {
            throw std::runtime_error("Pattern time options not set.");
        }
        if (p__pattern == nullptr) {
            throw std::runtime_error("Pattern not set.");
        }

        if (pos < p__pattern->size()) {
            return p__pattern->at(pos);
        } else if (pos == base_value_pos) {
            return m__value;
        } else {
            throw std::out_of_range("RegularTS::at: pos out of range (either between 0 and size for the multipliers or max for the value)");
        }
    }

    const value_type when_sim_t( long time__s ) const override {
        // Check all the elements point to something real, otherwise throw an exception
        if (inherited::p__glo_time_opts == nullptr) {
            throw std::runtime_error("Global time options not set.");
        }
        if (p__pat_time_opts == nullptr) {
            throw std::runtime_error("Pattern time options not set.");
        }
        if (p__pattern == nullptr) {
            throw std::runtime_error("Pattern not set.");
        }
        
        std::size_t steps = (time__s + p__pat_time_opts->shift_start_time__s) / p__pat_time_opts->timestep__s;

        std::size_t pos = steps % p__pattern->size();

        // Use the pattern at function for bound check (underlying vector)
        return m__value*p__pattern->at(pos);
    }

    // No operator []

   reference front() override {
        if (p__pattern == nullptr) {
            throw std::runtime_error("Pattern not set.");
        }
        return p__pattern->front();
    }

    const_reference front() const override {
        if (p__pattern == nullptr) {
            throw std::runtime_error("Pattern not set.");
        }
        return p__pattern->front();
    }

    const value_type when_t0() const override {
        return this->when_sim_t(0);
    }

    reference back() override {
        if (p__pattern == nullptr) {
            throw std::runtime_error("Pattern not set.");
        }
        return p__pattern->back();
    }

    const_reference back() const override {
        if (p__pattern == nullptr) {
            throw std::runtime_error("Pattern not set.");
        }
        return p__pattern->back();
    }

    const value_type when_tH() const override {
        return this->when_sim_t(inherited::sim_tH__s());
    }
    
    // No direct data access 

/*--- Iterators ---*/
public:
    // TODO: this is the most important feature, I must make sure that I return alwasys pairs of time and value

/*--- Capacity ---*/
public:
    bool empty() const noexcept override {
        if (p__pattern == nullptr) {
            return true;
        } else {
            return p__pattern->empty();
        }
    }

    std::size_t size() const noexcept override {
        if (p__pattern == nullptr) {
            return 0;
        } else {
            return p__pattern->size();
        }
    }

    std::size_t length() const noexcept override {
        return size() + 1; // Plus one for the last event at the end of the simulation
    }

    std::size_t max_size() const noexcept override {
        if (p__pattern == nullptr) {
            return 0;
        } else {
            return p__pattern->max_size();
        }
    }

    void reserve( std::size_t new_cap ) override {
        if (p__pattern != nullptr) {
            p__pattern->reserve(new_cap);
        }
        else {
            throw std::runtime_error("Pattern not set.");
        }
    }

    std::size_t capacity() const noexcept override {
        if (p__pattern == nullptr) {
            return 0;
        } else {
            return p__pattern->capacity();
        }
    }

 // No shrink_to_fit

/*--- Modifiers ---*/
public:
    void clear() override {
        inherited::clear();
        p__pat_time_opts = nullptr;
        p__pattern = nullptr;
        m__value = T();
    }

}; // class RegularTS

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_DATA_STRUCTURES__REGULAR_TS_HPP
