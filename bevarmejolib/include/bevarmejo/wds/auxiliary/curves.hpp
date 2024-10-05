#ifndef BEVARMEJOLIB__WDS_ELEMENTS__CURVES_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__CURVES_HPP

#include <string>
#include <memory>

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
    GenericCurve(const GenericCurve& other) = default;
    GenericCurve(GenericCurve&& rhs) noexcept = default;
    GenericCurve& operator=(const GenericCurve& rhs) = default;
    GenericCurve& operator=(GenericCurve&& rhs) noexcept = default;
    virtual ~GenericCurve() = default;
    std::unique_ptr<GenericCurve> clone() const;
private:
    virtual GenericCurve* __clone() const override;

/*--- Pure virtual methods override---*/
public:
    const std::string& element_name() const override;
    const unsigned int element_type() const override;

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
    VolumeCurve(const GenericCurve& other);
    VolumeCurve(const VolumeCurve& other) = default;
    VolumeCurve(VolumeCurve&& rhs) noexcept = default;
    VolumeCurve& operator=(const VolumeCurve& rhs) = default;
    VolumeCurve& operator=(VolumeCurve&& rhs) noexcept = default;
    virtual ~VolumeCurve() = default;
    std::unique_ptr<VolumeCurve> clone() const;
private:
    virtual VolumeCurve* __clone() const override;

/*--- Pure virtual methods override---*/
public:
    const std::string& element_name() const override;
    const unsigned int element_type() const override;

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
    PumpCurve(const PumpCurve& other) = default;
    PumpCurve(PumpCurve&& rhs) noexcept = default;
    PumpCurve& operator=(const PumpCurve& rhs) = default;
    PumpCurve& operator=(PumpCurve&& rhs) noexcept = default;
    virtual ~PumpCurve() = default;
    std::unique_ptr<PumpCurve> clone() const;
private:
    virtual PumpCurve* __clone() const override;

/*--- Pure virtual methods override---*/
public:
    const std::string& element_name() const override;
    const unsigned int element_type() const override;

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
    EfficiencyCurve(const GenericCurve& other);
    EfficiencyCurve(const EfficiencyCurve& other) = default;
    EfficiencyCurve(EfficiencyCurve&& rhs) noexcept = default;
    EfficiencyCurve& operator=(const EfficiencyCurve& rhs) = default;
    EfficiencyCurve& operator=(EfficiencyCurve&& rhs) noexcept = default;
    virtual ~EfficiencyCurve() = default;
    std::unique_ptr<EfficiencyCurve> clone() const;
private:
    virtual EfficiencyCurve* __clone() const override;

/*--- Pure virtual methods override---*/
public:
    const std::string& element_name() const override;
    const unsigned int element_type() const override;

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
    HeadlossCurve(const GenericCurve& other);
    HeadlossCurve(const HeadlossCurve& other) = default;
    HeadlossCurve(HeadlossCurve&& rhs) noexcept = default;
    HeadlossCurve& operator=(const HeadlossCurve& rhs) = default;
    HeadlossCurve& operator=(HeadlossCurve&& rhs) noexcept = default;
    virtual ~HeadlossCurve() = default;
    std::unique_ptr<HeadlossCurve> clone() const;
private:
    virtual HeadlossCurve* __clone() const override;

/*--- Pure virtual methods override---*/
public:
    const std::string& element_name() const override;
    const unsigned int element_type() const override;

}; // class HeadlossCurve

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__CURVES_HPP
