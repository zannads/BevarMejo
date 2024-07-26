#ifndef BEVARMEJOLIB__QUANTITY_SERIES_HPP
#define BEVARMEJOLIB__QUANTITY_SERIES_HPP

#include <memory>
#include <vector>

namespace bevarmejo {

using time_t= long; // Necessary to be consistent with the EPANET library
using TimeSteps= std::vector<time_t>;

template <typename T>
class QuantitySeries {

/*--- Member types ---*/
public:
    using container= std::vector<T>;

    using key_type= time_t;
    using value_type= T;
    using instant_type= std::pair<key_type, value_type>;
    using size_type= std::size_t;
    using difference_type= std::ptrdiff_t;
    using reference= T&;
    using const_reference= const T&;
    using pointer= T*;
    using const_pointer= const T*;
    // TODO: iterators
    // TODO: reverse iterators
    // TODO: const versions of the above

    enum class State {
        Invalid,        // For example, not enough time steps
        Transitional,   // For example, not all values are set
        Valid           // Values are conforming to the time steps
    };

    enum class Case {
        Undefined,      // Either in Invalid or Transitional state
        Constant,       // Valid && TimeSteps.size() == 2 && Values.size() == 1
        Regular,        // Valid && TimeSteps.size() == x && Values.size() == x-1
        Flexible        // Valid && TimeSteps.size() == x && Values.size() == x
    };

/*--- Attributes ---*/
protected:
    std::shared_ptr<TimeSteps> m__time_steps;
    container m__values;

/*--- Member methods ---*/
public:
    bool is_state(State a_state) const {
        switch (a_state) {

            case State::Invalid:
                return ( (m__time_steps == nullptr && !m__values.empty() ) ||
                         (m__time_steps != nullptr && m__time_steps->empty() && !m__values.empty() ) ||
                         (m__time_steps != nullptr && (m__values.size() > m__time_steps->size()) )
                        );

            case State::Transitional:
                return !is_state(State::Invalid) && !is_state(State::Valid);
                
            case State::Valid:
                return (m__time_steps != nullptr &&
                        m__time_steps->size()>=2 && 
                        (m__time_steps->size() == m__values.size() || m__time_steps->size() == m__values.size()+1));

            default:
                return false;
        }
    
    }

    void check_valid() const {
        if (!is_state(State::Valid))
            throw std::runtime_error("QuantitySeries::check_valid: Invalid or Transitional state.");
    }

    bool is_special_case(Case a_case) const {
        switch (a_case) {

            case Case::Undefined:
                return !is_state(State::Valid);

            case Case::Constant:
                return is_state(State::Valid) && m__time_steps->size() == 2 && m__values.size() == 1;

            case Case::Regular:
                return is_state(State::Valid) && m__time_steps->size() > 2 && m__values.size() == m__time_steps->size()-1;

            case Case::Flexible:
                return is_state(State::Valid) && m__time_steps->size() >= 2 && m__values.size() == m__time_steps->size();

            default:
                return false;
        }
    
    }
    
/*--- Constructors ---*/
public:

    QuantitySeries() : 
        m__time_steps(nullptr), 
        m__values() {}

    QuantitySeries(std::shared_ptr<TimeSteps> ap_time_steps) : 
        m__time_steps(ap_time_steps), 
        m__values() {}

    QuantitySeries( std::shared_ptr<TimeSteps> ap_time_steps, 
                    const_reference a_value,
                    bool force_flexible= false) : 
        m__time_steps(ap_time_steps),
        m__values() {
            if (m__time_steps != nullptr && !m__time_steps->empty()) {
                if (force_flexible)
                    m__values.resize(m__time_steps->size(), a_value);
                else
                    m__values.resize(m__time_steps->size()-1, a_value);
            }
            // else let the vector be empty and pointer null.
            // If time is empty, values is also empty.
        }

    QuantitySeries(std::shared_ptr<TimeSteps> ap_time_steps, const container& values) : 
        m__time_steps(ap_time_steps),
        m__values() {
            if (m__time_steps != nullptr && !m__time_steps->empty())
                m__values = values;
            // else let the vector be empty and pointer null.
            // No guarantee that the resulting object is in a valid state.
        }

    QuantitySeries(const QuantitySeries& other) = default;

    QuantitySeries(QuantitySeries&& other) noexcept = default;

    QuantitySeries& operator=(const QuantitySeries& other) = default;

    QuantitySeries& operator=(QuantitySeries&& other) noexcept = default;

    virtual ~QuantitySeries() = default;

/*--- Getters and setters ---*/
public:
    std::shared_ptr<TimeSteps> time_steps() { return m__time_steps; }
    const std::shared_ptr<TimeSteps> time_steps() const { return m__time_steps; }
    void time_steps(std::shared_ptr<TimeSteps> ap_time_steps) {
        m__time_steps = ap_time_steps;
        if (m__time_steps == nullptr)
            m__values.clear();
        // else m__values.resize(m__time_steps->size());
    }

    container& values() { return m__values; }
    const container& values() const { return m__values; }
    void values(const container& values) {
        if (m__time_steps != nullptr)
            m__values = values;
    }

/*--- Element access ---*/
public:

    reference at( size_type pos ) { check_valid(); return m__values.at(pos); }
    const_reference at( size_type pos ) const { check_valid(); return m__values.at(pos); }

    reference when_t( time_t time__s ) { return m__values.at(find_pos(time__s)); }
    const_reference when_t( time_t time__s ) const { return m__values.at(find_pos(time__s)); }

    // No operator[] because it is not safe to use it with this setup

    reference front() { check_valid(); return m__values.front(); }
    const_reference front() const { check_valid(); return m__values.front(); }

    reference when_t0() { check_valid(); return when_t(m__time_steps->front()); }
    const_reference when_t0() const { check_valid(); return when_t(m__time_steps->front()); }

    reference back() { check_valid(); return m__values.back(); }
    const_reference back() const { return m__values.back(); }

    reference when_tH() { check_valid(); return when_t(m__time_steps->back()); }
    const_reference when_tH() const { check_valid(); return when_t(m__time_steps->back()); }

    // No direct access to the time series! 

/*--- Iterators ---*/
public: 
    // TODO: iterators are the  crucial part 

/*--- Capacity ---*/
public:

/*--- Modifiers ---*/
public:

/*--- Lookup ---*/
public:

    // No count, each time should be unique.

    // Find, returns an iterator to the first time that is not less than the given time

    // Find_pos, returns the array position to the first time that is not less than the given time
    size_type find_pos( time_t time__s ) const {
        check_valid();
        if (time__s < m__time_steps->front() || time__s > m__time_steps->back())
            throw std::out_of_range("QuantitySeries::find_pos: time__s out of range");

        size_type pos= 0;
        while (pos < m__time_steps->size() -1) {
            if (m__time_steps->at(pos+1) > time__s)
                break;
            ++pos;
        }
        return pos;
    }

    // No equal_range, each time should be unique.

    // Lower_bound, returns an iterator to the first time that is not less than the given time

    // Lower_bound_pos, returns the array position to the last time that is not greater than the given time
    size_type lower_bound_pos( time_t time__s ) const {
        check_valid();
        if (time__s < m__time_steps->front() || time__s > m__time_steps->back())
            throw std::out_of_range("QuantitySeries::lower_bound_pos: time__s out of range");

        size_type pos= 0;
        while (pos < m__time_steps->size() -1) {
            if (m__time_steps->at(pos+1) >= time__s)
                break;
            ++pos;
        }
        return pos;
    }
            
    // Upper_bound, returns an iterator to the first time that is greater than the given time

    // Upper_bound_pos, returns the array position to the first time that is greater than the given time
    size_type upper_bound_pos( time_t time__s ) const {
        check_valid();
        if (time__s < m__time_steps->front() || time__s > m__time_steps->back())
            throw std::out_of_range("QuantitySeries::upper_bound_pos: time__s out of range");

        size_type pos= 1;
        while (pos < m__time_steps->size()-1) {
            if (m__time_steps->at(pos) > time__s)
                break;
            ++pos;
        }
        return pos;
    }

};

} // namespace bevarmejo

#endif // BEVARMEJOLIB__QUANTITY_SERIES_HPP
