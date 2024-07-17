#ifndef BEVARMEJOLIB__WDS_DATA_STRUCTURES__REGULAR_TS_HPP
#define BEVARMEJOLIB__WDS_DATA_STRUCTURES__REGULAR_TS_HPP

#include <memory>

#include "bevarmejo/wds/data_structures/time_options.hpp"
#include "bevarmejo/wds/data_structures/time_series.hpp"
#include "bevarmejo/wds/data_structures/pattern.hpp"

namespace bevarmejo {
namespace wds {

template <typename T>
class RegularTS : public TimeSeries<T> {

/*--- Attributes ---*/
protected:
    PatternTimeOptions* p__pat_time_opts;
    std::shared_ptr<Pattern> p__pattern;
    T m__value;

/*--- Constructors ---*/

}; // class RegularTS

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_DATA_STRUCTURES__REGULAR_TS_HPP
