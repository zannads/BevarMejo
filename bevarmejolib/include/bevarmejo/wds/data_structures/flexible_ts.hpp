#ifndef BEVARMEJOLIB__WDS_DATA_STRUCTURES__FLEXIBLE_TS_HPP
#define BEVARMEJOLIB__WDS_DATA_STRUCTURES__FLEXIBLE_TS_HPP

#include <memory>

#include "bevarmejo/wds/data_structures/time_options.hpp"
#include "bevarmejo/wds/data_structures/time_series.hpp"

namespace bevarmejo {
namespace wds {

template <typename T>
class FlexibleTS : public TimeSeries<T> {

/*--- Attributes ---*/
protected:
    std::shared_ptr<TimeSteps> p__time_steps;
    std::vector<T> m__values;

/*--- Constructors ---*/

}; // class FlexibleTS

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_DATA_STRUCTURES__FLEXIBLE_TS_HPP
