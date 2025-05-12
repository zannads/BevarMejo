#include <numeric>
#include <vector>

#include "bevarmejo/utility/exceptions.hpp"
#include "bevarmejo/wds/utility/quantity_series.hpp"
#include "bevarmejo/wds/water_distribution_system.hpp"

#include "bevarmejo/evaluation/metrics/hydraulic_functions.hpp"

namespace bevarmejo::eval::metrics
{

HeadDeficiency::HeadDeficiency(const double min_head, const bool relative)
    : m__min_head(min_head), m__relative(relative)
{
    beme_throw_if(m__min_head <= 0.0, std::runtime_error,
        "Impossible to build head deficiency metric.",
        "The minimum head must be greater than zero.",
        "Minimum head: ", min_head);
}

auto HeadDeficiency::operator()(const wds::Junction& a_junction) const -> wds::aux::QuantitySeries<double>
{
    wds::aux::QuantitySeries<double> deficiency(a_junction.head().time_series());

    double weight = 1.0;
    if (m__relative) weight = 1 / m__min_head;

    for (const auto& [t, head] : a_junction.head())
    {
        double deficit = head < m__min_head ? (m__min_head - head) * weight : 0.0;

        deficiency.commit(t, deficit);
    }

    return deficiency;
}

auto total_water_demand(const WDS& a_wds) -> wds::aux::QuantitySeries<double>
{
    wds::aux::QuantitySeries<double> tot(a_wds.result_time_series());
    for (const auto t : a_wds.result_time_series())
    {
        auto junctions = a_wds.junctions();
        tot.commit(t,
            std::accumulate(junctions.begin(), junctions.end(),
            0.0,
            [t](double c, auto j) { return std::move(c) + j.value.demand().when_t(t); })
        );
    }

    return tot;
}

auto total_water_consumption(const WDS& a_wds) -> wds::aux::QuantitySeries<double>
{
    wds::aux::QuantitySeries<double> tot(a_wds.result_time_series());
    for (const auto t : a_wds.result_time_series())
    {
        auto junctions = a_wds.junctions();
        tot.commit(t,
            std::accumulate(junctions.begin(), junctions.end(),
            0.0,
            [t](double c, auto j) { return std::move(c) + j.value.consumption().when_t(t); })
        );
    }

    return tot;
}

auto PaezFilion::mechanical_reliability_estimator(const WDS &a_wds) -> wds::aux::QuantitySeries<double>
{
    // Quite simple function, we extract the 4 parameters that are required:
    // availability and flow in pipes, total demand and actual total consumption
    // from the WDS.

    // Pre allocations:
    // results
    wds::aux::QuantitySeries<double> mre(a_wds.result_time_series());
    auto n_pipes = a_wds.n_pipes();

    // availability of pipes doesn't depend on the simulation so we compute it once
    auto avails = std::vector<double>();
    avails.reserve(n_pipes);
    for (const auto&& [name, pipe] : a_wds.pipes())
    {
        avails.push_back(CullinaneEtAl::pipe_mechanical_availability(pipe));
    }

    auto total_d = eval::metrics::total_water_demand(a_wds);
    auto total_c = eval::metrics::total_water_consumption(a_wds);

    // The flows change at each time step, but the number of pipes is constant
    // so we pre allocate
    auto flows = std::vector<double>();
    flows.reserve(n_pipes);

    for (const auto t : a_wds.result_time_series())
    {
        std::size_t i = 0;
        for (auto it = a_wds.pipes().begin(); it < a_wds.pipes().end(); ++i, ++it)
        {
            flows[i] = it->flow().when_t(t);
        }

        mre.commit(t, 
            mechanical_reliability_estimator_impl(
                avails,
                flows,
                total_d.when_t(t),
                total_c.when_t(t)
            )
        );
    }

    return mre;
}

auto PaezFilion::mechanical_reliability_estimator_impl(
    const std::vector<double>& availability_pipes,
    const std::vector<double>& flow_pipes,
    const double total_demand,
    const double total_consumption
) -> double
{
     // Probability of no-pipe failure: product between the availability of each pipe
    double p0 = std::accumulate(availability_pipes.begin(), availability_pipes.end(), 1.0, std::multiplies<double>());

    double mre = p0;
    for (auto i = 0; i < availability_pipes.size(); ++i)
    {
        mre += p0*(1-availability_pipes[i])/availability_pipes[i]*(total_consumption-flow_pipes[i])/total_demand;
    }

    return mre;
}

auto CullinaneEtAl::pipe_mechanical_availability(const WDS::Pipe& a_pipe) -> double
{
    // get the diam but in inches... 
    double diam__mm = a_pipe.diameter().value();
    double diam__in = diam__mm/1000.0/MperFT*12.0;

    constexpr double c1 = 0.21218;
    constexpr double c2 = 0.00074;
    constexpr double e1 = 1.462131;
    constexpr double e2 = 0.285;

    double num = c1*std::pow(diam__in, e1);
    double den = c2*std::pow(diam__in, e2) + num;
    return num/den;
}

    
} // namespace bevarmejo::eval::metrics