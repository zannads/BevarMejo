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

auto bevarmejo::eval::metrics::PaezFilion::mechanical_reliability_estimator(const WDS &a_wds) -> wds::aux::QuantitySeries<double>
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
    // Probability of no-pipe failure: product between the availability of each pipe
    double p0 = std::accumulate(avails.begin(), avails.end(), 1.0, std::multiplies<double>());

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


    
} // namespace bevarmejo::eval::metrics