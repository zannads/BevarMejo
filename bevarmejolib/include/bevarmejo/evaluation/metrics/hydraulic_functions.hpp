#pragma once

#include "bevarmejo/wds/water_distribution_system.hpp"

namespace bevarmejo::eval::metrics
{

class HeadDeficiency
{
/*------- Member objects -------*/
private:
    double m__min_head = 14.0;
    bool m__relative = false;

/*------- Member functions -------*/
public:
    HeadDeficiency() = default;
    HeadDeficiency(const double min_head, const bool relative);
    HeadDeficiency(const HeadDeficiency& other) = default;
    HeadDeficiency(HeadDeficiency&& other) noexcept = default;
    HeadDeficiency& operator=(const HeadDeficiency& other) = default;
    HeadDeficiency& operator=(HeadDeficiency&& other) noexcept = default;
    ~HeadDeficiency() = default;

/*------- Compute method -------*/
public:
    wds::aux::QuantitySeries<double> operator()(const wds::Junction& a_junction) const;
};

auto total_water_demand(const WDS& a_wds) -> wds::aux::QuantitySeries<double>;

auto total_water_consumption(const WDS& a_wds) -> wds::aux::QuantitySeries<double>;


namespace PaezFilion
{
auto mechanical_reliability_estimator(const WDS& a_wds) -> wds::aux::QuantitySeries<double>;

auto mechanical_reliability_estimator_impl(
    const std::vector<double>& availability_pipes,
    const std::vector<double>& flow_pipes,
    const double total_demand,
    const double total_consumption
) -> double;

} // PaezFilion

namespace CullinaneEtAl
{
auto pipe_mechanical_availability(const WDS::Pipe& a_pipe) -> double;
}


} // namespace bevarmejo::eval::metrics
