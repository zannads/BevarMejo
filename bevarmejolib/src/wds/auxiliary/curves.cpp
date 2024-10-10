#include <string>
#include <memory>

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/auxiliary/curve.hpp"

#include "curves.hpp"

namespace bevarmejo {
namespace wds {

GenericCurve::GenericCurve(const std::string& id) :
    inherited(id) {}

std::unique_ptr<GenericCurve> GenericCurve::clone() const {
    return std::unique_ptr<GenericCurve>(this->__clone());
}

GenericCurve* GenericCurve::__clone() const {
    return new GenericCurve(*this);
}

const std::string& GenericCurve::element_name() const {
    return l__GENERIC_CURVE;
}

const unsigned int GenericCurve::element_type() const {
    return ec__CURVE + EN_GENERIC_CURVE;
}

//-----------------------------------------------------------------------------

VolumeCurve::VolumeCurve(const std::string& id) :
    inherited(id) {}

std::unique_ptr<VolumeCurve> VolumeCurve::clone() const {
    return std::unique_ptr<VolumeCurve>(this->__clone());
}

VolumeCurve* VolumeCurve::__clone() const {
    return new VolumeCurve(*this);
}

const std::string& VolumeCurve::element_name() const {
    return l__VOLUME_CURVE;
}

const unsigned int VolumeCurve::element_type() const {
    return ec__CURVE + EN_VOLUME_CURVE;
}

//-----------------------------------------------------------------------------

PumpCurve::PumpCurve(const std::string& id) :
    inherited(id) {}

std::unique_ptr<PumpCurve> PumpCurve::clone() const {
    return std::unique_ptr<PumpCurve>(this->__clone());
}

PumpCurve* PumpCurve::__clone() const {
    return new PumpCurve(*this);
}

const std::string& PumpCurve::element_name() const {
    return l__PUMP_CURVE;
}

const unsigned int PumpCurve::element_type() const {
    return ec__CURVE + EN_PUMP_CURVE;
}

//-----------------------------------------------------------------------------

EfficiencyCurve::EfficiencyCurve(const std::string& id) :
    inherited(id) {}

std::unique_ptr<EfficiencyCurve> EfficiencyCurve::clone() const {
    return std::unique_ptr<EfficiencyCurve>(this->__clone());
}

EfficiencyCurve* EfficiencyCurve::__clone() const {
    return new EfficiencyCurve(*this);
}

const std::string& EfficiencyCurve::element_name() const {
    return l__EFFICIENCY_CURVE;
}

const unsigned int EfficiencyCurve::element_type() const {
    return ec__CURVE + EN_EFFIC_CURVE;
}

//-----------------------------------------------------------------------------

HeadlossCurve::HeadlossCurve(const std::string& id) :
    inherited(id) {}

std::unique_ptr<HeadlossCurve> HeadlossCurve::clone() const {
    return std::unique_ptr<HeadlossCurve>(this->__clone());
}

HeadlossCurve* HeadlossCurve::__clone() const {
    return new HeadlossCurve(*this);
}

const std::string& HeadlossCurve::element_name() const {
    return l__HEADLOSS_CURVE;
}

const unsigned int HeadlossCurve::element_type() const {
    return ec__CURVE + EN_HLOSS_CURVE;
}

} // namespace wds
} // namespace bevarmejo
