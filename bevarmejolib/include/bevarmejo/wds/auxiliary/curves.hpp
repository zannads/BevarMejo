#ifndef BEVARMEJOLIB__WDS_ELEMENTS__CURVES_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__CURVES_HPP

#include "bevarmejo/wds/auxiliary/curve.hpp"

namespace bevarmejo::wds
{

using GenericCurve = SpecificCurve<double, double>;
extern template struct TypeTraits<GenericCurve>;

using Level = double;
using Volume = double;
using VolumeCurve = SpecificCurve<Level, Volume>;
extern template struct TypeTraits<VolumeCurve>;

using Flow = double;
using Head = double;
using PumpCurve = SpecificCurve<Flow, Head>;
extern template struct TypeTraits<PumpCurve>;

using Efficiency = double;

using EfficiencyCurve = SpecificCurve<Flow, Efficiency>;
extern template struct TypeTraits<EfficiencyCurve>;

using HeadlossCurve = SpecificCurve<Flow, Head>;

} // namespace bevarmejo::wds

#endif // BEVARMEJOLIB__WDS_ELEMENTS__CURVES_HPP
