#include <string>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/curve.hpp"

#include "curves.hpp"

namespace bevarmejo {
namespace wds {

VolumeCurve::VolumeCurve(const std::string& id) :
    inherited(id) {}

VolumeCurve::VolumeCurve(const VolumeCurve& other) :
    inherited(other) {}

VolumeCurve::VolumeCurve(VolumeCurve&& rhs) noexcept :
    inherited(std::move(rhs)) {}

VolumeCurve& VolumeCurve::operator=(const VolumeCurve& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
    }
    return *this;
}

VolumeCurve& VolumeCurve::operator=(VolumeCurve&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
    }
    return *this;
}

//-----------------------------------------------------------------------------

PumpCurve::PumpCurve(const std::string& id) :
    inherited(id) {}
/*
PumpCurve::PumpCurve(const GenericCurve& other) :
    inherited(other)
    {
        for (const auto& [x, y] : other.curve()) {
            _curve_.insert(x, y);
        }
    }*/

PumpCurve::PumpCurve(const PumpCurve& other) :
    inherited(other) {}

PumpCurve::PumpCurve(PumpCurve&& rhs) noexcept :
    inherited(std::move(rhs)) {}

PumpCurve& PumpCurve::operator=(const PumpCurve& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
    }
    return *this;
}

PumpCurve& PumpCurve::operator=(PumpCurve&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
    }
    return *this;
}

//-----------------------------------------------------------------------------

EfficiencyCurve::EfficiencyCurve(const std::string& id) :
    inherited(id) {}

EfficiencyCurve::EfficiencyCurve(const EfficiencyCurve& other) :
    inherited(other) {}

EfficiencyCurve::EfficiencyCurve(EfficiencyCurve&& rhs) noexcept :
    inherited(std::move(rhs)) {}

EfficiencyCurve& EfficiencyCurve::operator=(const EfficiencyCurve& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
    }
    return *this;
}

EfficiencyCurve& EfficiencyCurve::operator=(EfficiencyCurve&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
    }
    return *this;
}

//-----------------------------------------------------------------------------

HeadlossCurve::HeadlossCurve(const std::string& id) :
    inherited(id) {}

HeadlossCurve::HeadlossCurve(const HeadlossCurve& other) :
    inherited(other) {}

HeadlossCurve::HeadlossCurve(HeadlossCurve&& rhs) noexcept :
    inherited(std::move(rhs)) {}

HeadlossCurve& HeadlossCurve::operator=(const HeadlossCurve& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
    }
    return *this;
}

HeadlossCurve& HeadlossCurve::operator=(HeadlossCurve&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
    }
    return *this;
}

//-----------------------------------------------------------------------------

GenericCurve::GenericCurve(const std::string& id) :
    inherited(id) {}

GenericCurve::GenericCurve(const GenericCurve& other) :
    inherited(other) {}

GenericCurve::GenericCurve(GenericCurve&& rhs) noexcept :
    inherited(std::move(rhs)) {}

GenericCurve& GenericCurve::operator=(const GenericCurve& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
    }
    return *this;
}

GenericCurve& GenericCurve::operator=(GenericCurve&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
    }
    return *this;
}

} // namespace wds
} // namespace bevarmejo
