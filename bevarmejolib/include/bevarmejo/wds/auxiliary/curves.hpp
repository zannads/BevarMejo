#ifndef BEVARMEJOLIB__WDS_ELEMENTS__CURVES_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__CURVES_HPP

#include <string>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/auxiliary/curve.hpp"

namespace bevarmejo {
namespace wds {

static const std::string l__VOLUME_CURVE = "VolumeCurve";
static const std::string l__PUMP_CURVE = "PumpCurve";
static const std::string l__EFFICIENCY_CURVE = "EfficiencyCurve";
static const std::string l__HEADLOSS_CURVE = "HeadlossCurve";
static const std::string l__GENERIC_CURVE = "GenericCurve";
static const unsigned int ec__CURVE = 400;

class GenericCurve : public SpecificCurve<double, double> {
    // WDS GenericCurve
    /*******************************************************************************
     * The wds::GenericCurve class represents a curve describing some property of 
     * the elements in the network (e.g. pump efficiency, valve headloss, etc.).
    ******************************************************************************/

/*--- Constructors ---*/
public:
    using inherited= SpecificCurve<double, double>;
    GenericCurve() = delete;
    GenericCurve(const std::string& id);
    GenericCurve(const GenericCurve& other);
    GenericCurve(GenericCurve&& rhs) noexcept;
    GenericCurve& operator=(const GenericCurve& rhs);
    GenericCurve& operator=(GenericCurve&& rhs) noexcept;
    virtual ~GenericCurve() {}
    std::unique_ptr<GenericCurve> clone() const { return std::unique_ptr<GenericCurve>(this->__clone()); }
private:
    virtual GenericCurve* __clone() const override { return new GenericCurve(*this); }

/*--- Pure virtual methods override---*/
public:
    const std::string& element_name() const override { return l__GENERIC_CURVE; }
    const unsigned int element_type() const override { return ec__CURVE+EN_GENERIC_CURVE; }

}; // class GenericCurve

// Assume these are more than simple aliases, but classes of the different units
// of measurement.
using Level = double;
using Volume = double;
using Flow = double;
using Head = double;
using Efficiency = double;
using Headloss = double;

class VolumeCurve : public SpecificCurve<Level, Volume> {
    // WDS VolumeCurve
    /*******************************************************************************
     * The wds::VolumeCurve class represents a curve describing the volume of a 
     * tank as a function of its level.
    ******************************************************************************/

/*--- Constructors ---*/
public:
    using inherited= SpecificCurve<Level, Volume>;
    VolumeCurve() = delete;
    VolumeCurve(const std::string& id);
    //VolumeCurve(const GenericCurve& other);
    VolumeCurve(const VolumeCurve& other);
    VolumeCurve(VolumeCurve&& rhs) noexcept;
    VolumeCurve& operator=(const VolumeCurve& rhs);
    VolumeCurve& operator=(VolumeCurve&& rhs) noexcept;
    virtual ~VolumeCurve() {}
    std::unique_ptr<VolumeCurve> clone() const { return std::unique_ptr<VolumeCurve>(this->__clone()); }
private:
    virtual VolumeCurve* __clone() const override { return new VolumeCurve(*this); }

/*--- Pure virtual methods override---*/
public:
    const std::string& element_name() const override { return l__VOLUME_CURVE; }
    const unsigned int element_type() const override { return ec__CURVE+EN_VOLUME_CURVE; }

}; // class VolumeCurve

class PumpCurve : public SpecificCurve<Flow, Head> {
    // WDS PumpCurve
    /*******************************************************************************
     * The wds::PumpCurve class represents a curve describing the head of a pump as 
     * a function of its flow.
    ******************************************************************************/

/*--- Constructors ---*/
public:
    using inherited= SpecificCurve<Flow, Head>;
    PumpCurve() = delete;
    PumpCurve(const std::string& id);
    PumpCurve(const GenericCurve& other);
    PumpCurve(const PumpCurve& other);
    PumpCurve(PumpCurve&& rhs) noexcept;
    PumpCurve& operator=(const PumpCurve& rhs);
    PumpCurve& operator=(PumpCurve&& rhs) noexcept;
    virtual ~PumpCurve() {}
    std::unique_ptr<PumpCurve> clone() const { return std::unique_ptr<PumpCurve>(this->__clone()); }
private:
    virtual PumpCurve* __clone() const override { return new PumpCurve(*this); }

/*--- Pure virtual methods override---*/
public:
    const std::string& element_name() const override { return l__PUMP_CURVE; }
    const unsigned int element_type() const override { return ec__CURVE+EN_PUMP_CURVE; }

}; // class PumpCurve

class EfficiencyCurve : public SpecificCurve<Flow, Efficiency> {
    // WDS EfficiencyCurve
    /*******************************************************************************
     * The wds::EfficiencyCurve class represents a curve describing the efficiency 
     * of a pump as a function of its flow.
    ******************************************************************************/

/*--- Constructors ---*/
public:
    using inherited= SpecificCurve<Flow, Efficiency>;
    EfficiencyCurve() = delete;
    EfficiencyCurve(const std::string& id);
    EfficiencyCurve(const EfficiencyCurve& other);
    EfficiencyCurve(EfficiencyCurve&& rhs) noexcept;
    EfficiencyCurve& operator=(const EfficiencyCurve& rhs);
    EfficiencyCurve& operator=(EfficiencyCurve&& rhs) noexcept;
    virtual ~EfficiencyCurve() {}
    std::unique_ptr<EfficiencyCurve> clone() const { return std::unique_ptr<EfficiencyCurve>(this->__clone()); }
private:
    virtual EfficiencyCurve* __clone() const override { return new EfficiencyCurve(*this); }

/*--- Pure virtual methods override---*/
public:
    const std::string& element_name() const override { return l__EFFICIENCY_CURVE; }
    const unsigned int element_type() const override { return ec__CURVE+EN_EFFIC_CURVE; }

}; // class EfficiencyCurve

class HeadlossCurve : public SpecificCurve<Flow, Headloss> {
    // WDS HeadlossCurve
    /*******************************************************************************
     * The wds::HeadlossCurve class represents a curve describing the headloss of a 
     * valve as a function of its flow.
    ******************************************************************************/

/*--- Constructors ---*/
public:
    using inherited= SpecificCurve<Flow, Headloss>;
    HeadlossCurve() = delete;
    HeadlossCurve(const std::string& id);
    HeadlossCurve(const HeadlossCurve& other);
    HeadlossCurve(HeadlossCurve&& rhs) noexcept;
    HeadlossCurve& operator=(const HeadlossCurve& rhs);
    HeadlossCurve& operator=(HeadlossCurve&& rhs) noexcept;
    virtual ~HeadlossCurve() {}
    std::unique_ptr<HeadlossCurve> clone() const { return std::unique_ptr<HeadlossCurve>(this->__clone()); }
private:
    virtual HeadlossCurve* __clone() const override { return new HeadlossCurve(*this); }

/*--- Pure virtual methods override---*/
public:
    const std::string& element_name() const override { return l__HEADLOSS_CURVE; }
    const unsigned int element_type() const override { return ec__CURVE+EN_HLOSS_CURVE; }

}; // class HeadlossCurve

//-----------------------------------------------------------------------------



} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__CURVES_HPP
