#ifndef BEVARMEJOLIB__WDS_ELEMENTS__CURVES_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__CURVES_HPP

#include "bevarmejo/wds/auxiliary/curve.hpp"

namespace bevarmejo::wds
{
    using GenericCurve = SpecificCurve<double, double>;

    template <>
    struct TypeTraits<GenericCurve>
    {
        static constexpr const char* name = "GenericCurve";
        static constexpr const int code = 121;
        static constexpr bool is_EN_complete = true;
    };

    using Level = double;
    using Volume = double;
    using Flow = double;
    using Head = double;
    using Efficiency = double;
    using Headloss = double;

    using VolumeCurve = SpecificCurve<Level, Volume>;

    template <>
    struct TypeTraits<VolumeCurve>
    {
        static constexpr const char* name = "VolumeCurve";
        static constexpr const int code = 221;
        static constexpr bool is_EN_complete = true;
    };

    using PumpCurve = SpecificCurve<Flow, Head>;

    template <>
    struct TypeTraits<PumpCurve>
    {
        static constexpr const char* name = "PumpCurve";
        static constexpr const int code = 321;
        static constexpr bool is_EN_complete = true;
    };

    using EfficiencyCurve = SpecificCurve<Flow, Efficiency>;

    template <>
    struct TypeTraits<EfficiencyCurve>
    {
        static constexpr const char* name = "EfficiencyCurve";
        static constexpr const int code = 421;
        static constexpr bool is_EN_complete = true;
    };

    using HeadlossCurve = SpecificCurve<Flow, Headloss>;

    template <>
    struct TypeTraits<HeadlossCurve>
    {
        static constexpr const char* name = "HeadlossCurve";
        static constexpr const int code = 521;
        static constexpr bool is_EN_complete = true;
    };
} // namespace bevarmejo::wds

#endif // BEVARMEJOLIB__WDS_ELEMENTS__CURVES_HPP
