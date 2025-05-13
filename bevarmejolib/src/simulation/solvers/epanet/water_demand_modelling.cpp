#include <memory>

// From EPANET
#include "types.h"

#include "bevarmejo/utility/exceptions.hpp"

#include "bevarmejo/simulation/solvers/epanet/water_demand_modelling.hpp"

namespace bevarmejo::sim::solvers::epanet
{

/*----------------------------------------------------------------------------*/
/*--------------------------- WaterDemandModel -------------------------------*/
/*----------------------------------------------------------------------------*/
std::unique_ptr<WaterDemandModel> WaterDemandModel::clone() const
{
    return std::unique_ptr<WaterDemandModel>(this->clone_impl());
}

/*----------------------------------------------------------------------------*/
/*------------------------- DemandDrivenAnalysis -----------------------------*/
/*----------------------------------------------------------------------------*/
std::unique_ptr<DemandDrivenAnalysis> DemandDrivenAnalysis::clone() const
{
    return std::unique_ptr<DemandDrivenAnalysis>(this->clone_impl());
}

DemandDrivenAnalysis* DemandDrivenAnalysis::clone_impl() const
{
    return new DemandDrivenAnalysis(*this);
}


/*----------------------------------------------------------------------------*/
/*------------------------- DemandDrivenAnalysis -----------------------------*/
/*----------------------------------------------------------------------------*/
// (constructor)
PressureDrivenAnalysis::PressureDrivenAnalysis() noexcept :
    m__minimum_pressure__m(0.0),
    m__required_pressure__m(30.0),
    m__pressure_exponent(0.5)
{ }

PressureDrivenAnalysis::PressureDrivenAnalysis(
    const double a_minimum_pressure__m,
    const double a_required_pressure__m,
    const double a_pressure_exponent)
{
    beme_throw_if(a_minimum_pressure__m < 0.0,
        std::invalid_argument,
        "Impossible to construct a Pressure Driven Analysis object.",
        "The provided minimum pressure must be non-negative.\n",
        "Minimum pressure: ", a_minimum_pressure__m, " m.\n"
    );

    beme_throw_if(a_required_pressure__m - a_minimum_pressure__m < MINPDIFF,
        std::invalid_argument,
        "Impossible to construct a Pressure Driven Analysis object.",
        "The provided pressures differ by at least 0.1 m.\n",
        "Minimum pressure: ", a_minimum_pressure__m, " m.\n",
        "Required pressure: ", a_required_pressure__m, " m."
    );

    m__minimum_pressure__m = a_minimum_pressure__m;
    m__required_pressure__m = a_required_pressure__m;
    
    pressure_exponent(a_pressure_exponent);
}

// (clone)
std::unique_ptr<PressureDrivenAnalysis> PressureDrivenAnalysis::clone() const
{
    return std::unique_ptr<PressureDrivenAnalysis>(this->clone_impl());
}

PressureDrivenAnalysis* PressureDrivenAnalysis::clone_impl() const
{
    return new PressureDrivenAnalysis(*this);
}


auto PressureDrivenAnalysis::minimum_pressure__m() const noexcept -> double
{
    return m__minimum_pressure__m;
}

auto PressureDrivenAnalysis::required_pressure__m() const noexcept -> double
{
    return m__required_pressure__m;
}

auto PressureDrivenAnalysis::pressure_exponent() const noexcept -> double
{
    return m__pressure_exponent;
}

auto PressureDrivenAnalysis::minimum_pressure__m(const double a_minimum_pressure) -> PressureDrivenAnalysis&
{
    beme_throw_if(a_minimum_pressure < 0.0 || m__required_pressure__m - a_minimum_pressure < MINPDIFF,
        std::invalid_argument,
        "Impossible to set the minimum pressure for the Pressure Driven Analysis.",
        "The provided value is not in the current valid range (non-negative and less than the required pressure minus 0.1).\n",
        "Minimum pressure: ", a_minimum_pressure, " m.\n",
        "Valid minimum pressure range: [", 0.0, ", ", m__required_pressure__m-MINPDIFF, "] m."
    );

    m__minimum_pressure__m = a_minimum_pressure;

    return *this;
}

auto PressureDrivenAnalysis::required_pressure__m(const double a_required_pressure) -> PressureDrivenAnalysis&
{
    beme_throw_if(a_required_pressure - m__minimum_pressure__m < MINPDIFF,
        std::invalid_argument,
        "Impossible to set the required pressure for the Pressure Driven Analysis.",
        "The provided value must be greater than the minimum pressure plus 0.1.\n",
        "Required pressure: ", a_required_pressure, " m.\n",
        "Threshold: ", m__minimum_pressure__m+MINPDIFF, "m."
    );

    m__required_pressure__m = a_required_pressure;

    return *this;
}

auto PressureDrivenAnalysis::pressure_exponent(const double a_exponent) -> PressureDrivenAnalysis&
{
    beme_throw_if(a_exponent <= 0.0, std::invalid_argument,
        "Impossible to set the exponent for the Pressure Driven Analysis.",
        "The provided value must be positive.\n",
        "Value: ", a_exponent, "."
    );

    m__pressure_exponent = a_exponent;

    return *this;
}

} // namespace bevarmejo::sim::solvers::epanet
