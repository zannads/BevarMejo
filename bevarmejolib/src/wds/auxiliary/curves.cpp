#include "curves.hpp"

namespace bevarmejo::wds
{

template <>
struct TypeTraits<GenericCurve>
{
    static constexpr const char* name = "GenericCurve";
    static constexpr const int code = 121;
    static constexpr bool is_EN_complete = true;
};

template <>
struct TypeTraits<VolumeCurve>
{
    static constexpr const char* name = "VolumeCurve";
    static constexpr const int code = 221;
    static constexpr bool is_EN_complete = true;
};

template <>
struct TypeTraits<PumpCurve>
{
    static constexpr const char* name = "PumpCurve";
    static constexpr const int code = 321;
    static constexpr bool is_EN_complete = true;
};

template <>
struct TypeTraits<EfficiencyCurve>
{
    static constexpr const char* name = "EfficiencyCurve";
    static constexpr const int code = 421;
    static constexpr bool is_EN_complete = true;
};

template <>
struct TypeTraits<HeadlossCurve>
{
    static constexpr const char* name = "HeadlossCurve";
    static constexpr const int code = 521;
    static constexpr bool is_EN_complete = true;
};

} // namespace bevarmejo::wds
