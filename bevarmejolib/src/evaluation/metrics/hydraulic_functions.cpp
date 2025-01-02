
#include "bevarmejo/utility/bemexcept.hpp"
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
    
} // namespace bevarmejo::eval::metrics