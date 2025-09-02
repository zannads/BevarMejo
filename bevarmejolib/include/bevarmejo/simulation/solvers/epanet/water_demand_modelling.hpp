#pragma once
#include <memory>

namespace bevarmejo::sim::solvers::epanet
{

class WaterDemandModel
{
/*----------------------------------------------------------------------------*/
/*--------------------------- Member functions -------------------------------*/
/*----------------------------------------------------------------------------*/
// (constructor)
public:
    WaterDemandModel() noexcept = default;
    WaterDemandModel(const WaterDemandModel& other) noexcept = default;
    WaterDemandModel(WaterDemandModel&& other) noexcept = default;

// (destructor)
public:
    virtual ~WaterDemandModel() noexcept = default;

// operator=
public:
    WaterDemandModel& operator=(const WaterDemandModel& other) noexcept = default;
    WaterDemandModel& operator=(WaterDemandModel&& other) noexcept = default;

// (clone)
public:
    std::unique_ptr<WaterDemandModel> clone() const;
protected:
    virtual WaterDemandModel* clone_impl() const = 0;
}; // class WaterDemandModel

class DemandDrivenAnalysis final : public WaterDemandModel
{
/*----------------------------------------------------------------------------*/
/*--------------------------- Member functions -------------------------------*/
/*----------------------------------------------------------------------------*/
// (constructor)
public:
    DemandDrivenAnalysis() noexcept = default;
    DemandDrivenAnalysis(const DemandDrivenAnalysis& other) noexcept = default;
    DemandDrivenAnalysis(DemandDrivenAnalysis&& other) noexcept = default;

// (destructor)
public:
    ~DemandDrivenAnalysis() noexcept override = default;

// operator=
public:
    DemandDrivenAnalysis& operator=(const DemandDrivenAnalysis& other) noexcept = default;
    DemandDrivenAnalysis& operator=(DemandDrivenAnalysis&& other) noexcept = default;

// (clone)
public:
    std::unique_ptr<DemandDrivenAnalysis> clone() const;
protected:
    DemandDrivenAnalysis* clone_impl() const override;
}; // class DemandDrivenAnalysis

class PressureDrivenAnalysis final : public WaterDemandModel
{
/*----------------------------------------------------------------------------*/
/*--------------------------- Member functions -------------------------------*/
/*----------------------------------------------------------------------------*/
// (constructor)
public:
    PressureDrivenAnalysis() noexcept;
    PressureDrivenAnalysis(
        const double a_minimum_pressure__m,
        const double a_required_pressure__m,
        const double a_pressure_exponent);
    PressureDrivenAnalysis(const PressureDrivenAnalysis& other) noexcept = default;
    PressureDrivenAnalysis(PressureDrivenAnalysis&& other) noexcept = default;

// (destructor)
public:
    ~PressureDrivenAnalysis() noexcept override = default;

// operator=
public:
    PressureDrivenAnalysis& operator=(const PressureDrivenAnalysis& other) noexcept = default;
    PressureDrivenAnalysis& operator=(PressureDrivenAnalysis&& other) noexcept = default;

// (clone)
public:
    std::unique_ptr<PressureDrivenAnalysis> clone() const;
protected:
    PressureDrivenAnalysis* clone_impl() const override;

// Element access
public:
    auto minimum_pressure__m() const noexcept -> double;
    auto required_pressure__m() const noexcept -> double;
    auto pressure_exponent() const noexcept -> double;

// Modifiers
public:
    auto minimum_pressure__m(const double a_pressure) -> PressureDrivenAnalysis&;
    auto required_pressure__m(const double a_pressure) -> PressureDrivenAnalysis&;
    auto pressure_exponent(const double a_exponent) -> PressureDrivenAnalysis&;

/*----------------------------------------------------------------------------*/
/*---------------------------- Member objects --------------------------------*/
/*----------------------------------------------------------------------------*/
private:
    double m__minimum_pressure__m;  // Pressure below which there is no flow to the customers.
    double m__required_pressure__m; // Pressure required to deliver the full customer demand.
    double m__pressure_exponent;    // Pressure exponent in demand function.
}; // class PressureDrivenAnalysis

} // namespace bevarmejo::sim::solvers::epanet