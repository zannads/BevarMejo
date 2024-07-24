#ifndef BEVARMEJOLIB__WDS_DATA_STRUCTURES__DATA_ENTITY_HPP
#define BEVARMEJOLIB__WDS_DATA_STRUCTURES__DATA_ENTITY_HPP

#include <memory>

#include "bevarmejo/wds/data_structures/time_options.hpp"
#include "bevarmejo/wds/data_structures/time_series.hpp"
#include "bevarmejo/wds/data_structures/constant_ts.hpp"
#include "bevarmejo/wds/data_structures/regular_ts.hpp"
#include "bevarmejo/wds/data_structures/flexible_ts.hpp"

namespace bevarmejo {
namespace wds {

template <typename T>
class DataEntity {

/*--- Attributes ---*/
protected:
    std::unique_ptr<TimeSeries<T>> p__time_series;

/*--- Constructors ---*/
public:
    DataEntity() = default;

}; // class DataEntity

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_DATA_STRUCTURES__DATA_ENTITY_HPP
