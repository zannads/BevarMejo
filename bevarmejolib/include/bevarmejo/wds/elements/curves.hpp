#ifndef BEVARMEJOLIB__WDS_ELEMENTS__CURVES_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__CURVES_HPP

#include "bevarmejo/wds/elements/curve.hpp"

namespace bevarmejo::wds
{

struct __GC__ { };
using GenericCurve = SpecificCurve<double, double, __GC__>;
template <>
struct TypeTraits<GenericCurve>
{
    static constexpr const char* name = "GenericCurve";
    static constexpr const int code = 121;
    static constexpr bool is_EN_complete = true;
};

using Level = double;
using Volume = double;
struct __VC__ { };
using VolumeCurve = SpecificCurve<Level, Volume, __VC__>;
template <>
struct TypeTraits<VolumeCurve>
{
    static constexpr const char* name = "VolumeCurve";
    static constexpr const int code = 221;
    static constexpr bool is_EN_complete = true;
};

using Flow = double;
using Head = double;
struct __PHC__ { };
using PumpCurve = SpecificCurve<Flow, Head, __PHC__>;
template <>
struct TypeTraits<PumpCurve>
{
    static constexpr const char* name = "PumpCurve";
    static constexpr const int code = 321;
    static constexpr bool is_EN_complete = true;
};

using Efficiency = double;
struct __EC__ { };
using EfficiencyCurve = SpecificCurve<Flow, Efficiency, __EC__>;
template <>
struct TypeTraits<EfficiencyCurve>
{
    static constexpr const char* name = "EfficiencyCurve";
    static constexpr const int code = 421;
    static constexpr bool is_EN_complete = true;
};

struct __HLC__ { };
using HeadlossCurve = SpecificCurve<Flow, Head, __HLC__>;
template <>
struct TypeTraits<HeadlossCurve>
{
    static constexpr const char* name = "HeadlossCurve";
    static constexpr const int code = 521;
    static constexpr bool is_EN_complete = true;
};
} // namespace bevarmejo::wds

#endif // BEVARMEJOLIB__WDS_ELEMENTS__CURVES_HPP
