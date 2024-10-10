#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "bevarmejo/wds/auxiliary/global_times.hpp"

#include "time_series.hpp"

namespace bevarmejo {

namespace log {
namespace fname {
static const std::string check_valid = "check_valid";
} // namespace fname
} // namespace log

namespace wds {
namespace aux {

// Empty TimeSteps are always monotonic, starting after zero, and ending before t.

bool is_monotonic(const time::TimeSteps& time_steps) {
    if (time_steps.empty())
        return true;

    for (std::size_t i = 0; i < time_steps.size() - 1; ++i) {
        if (time_steps[i] >= time_steps[i + 1])
            return false;
    }
    return true;
}

bool starts_after_zero(const time::TimeSteps& time_steps) {
    return time_steps.empty() || time_steps.front() > 0;
}

bool ends_before_t(const time::TimeSteps& time_steps, time::Instant t) {
    return time_steps.empty() || time_steps.back() < t;
}



/*----------------------------------------------------------------------------*/

                            /*--- Constructors ---*/

/*----------------------------------------------------------------------------*/

TimeSeries::TimeSeries(const GlobalTimes& a_gto) : 
    TimeSeries(a_gto, time::TimeSteps()) 
{ }

TimeSeries::TimeSeries(const GlobalTimes& a_gto, const TimeSeries& other) : 
    m__gto(a_gto), m__time_steps(other.m__time_steps)
{ }

TimeSeries::TimeSeries(const GlobalTimes& a_gto, TimeSeries&& other) noexcept: 
    m__gto(a_gto), m__time_steps(std::move(other.m__time_steps))
{ }

TimeSeries& TimeSeries::operator=(const TimeSeries& other) {
    if (this == &other)
        return *this;

    // Can't copy the GTO because it's const reference.

    m__time_steps = other.m__time_steps;

    // No need to check for valid, I assume it is already.

    return *this;
}

TimeSeries& TimeSeries::operator=(TimeSeries&& other) noexcept{
    if (this == &other)
        return *this;

    // Can't move the GTO because it's const reference.

    m__time_steps = std::move(other.m__time_steps);

    // No need to check for valid, I assume it is already.

    return *this;
}

/*----------------------------------------------------------------------------*/

                            /*--- Element access ---*/

/*----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*/

                            /*--- Iterators ---*/

/*----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*/

                            /*--- Capacity ---*/

/*----------------------------------------------------------------------------*/

bool TimeSeries::empty() const noexcept { return false; }

TimeSeries::size_type TimeSeries::size() const noexcept { return m__time_steps.size() + 1; } // 0 is always present. 

TimeSeries::size_type TimeSeries::max_size() const noexcept { return m__time_steps.max_size(); }

void TimeSeries::reserve(size_type new_cap) { m__time_steps.reserve(new_cap); }

TimeSeries::size_type TimeSeries::capacity() const noexcept { return m__time_steps.capacity(); }

/*----------------------------------------------------------------------------*/

                            /*--- Modifiers ---*/

/*----------------------------------------------------------------------------*/

void TimeSeries::reset() noexcept { m__time_steps.clear(); }

/*----------------------------------------------------------------------------*/

                            /*--- Lookup ---*/

/*----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*/

                        /*--- Other methods ---*/

/*----------------------------------------------------------------------------*/

void TimeSeries::remove_leading_zero() {
    // Since with commit I can add 0 when m__time_steps is empty and when I
    // use the method time_steps also 0 is added, I need to remove eventual starting 0.
    // However, I only remove one because this is meant to be called only on construction
    // and if you put more than one leading 0, you are doing something wrong.
    if (!m__time_steps.empty() && m__time_steps.front() == 0)
        m__time_steps.erase(m__time_steps.begin());
}

// A time series is a monotonically increasing sequence of times, starting from 
// values greater than 0 (0 is treated as default value). The time steps can go 
// over the duration but I will print 
void TimeSeries::check_valid() const {

    if (!is_monotonic(m__time_steps))
        __format_and_throw<std::invalid_argument>(log::cname::time_series, log::fname::check_valid, 
                                                  "Time steps are not valid.", 
            "Time steps are not monotonically increasing.");

     if (!starts_after_zero(m__time_steps))
        __format_and_throw<std::invalid_argument>(log::cname::time_series, log::fname::check_valid, 
                                                "Time steps are not valid.", 
            "Time steps do not start after zero.",
            "First time step: ", m__time_steps.front());
        
        
}

} // namespace aux
} // namespace wds
} // namespace bevarmejo
