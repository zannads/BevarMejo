#pragma once

#include <memory>
#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>

#include "bevarmejo/utility/except.hpp"
#include "bevarmejo/utility/quantity_series.hpp"
#include "bevarmejo/utility/type_traits.hpp"
#include "bevarmejo/wds/water_distribution_system.hpp"

namespace bevarmejo
{

// Forward declaration of the evaluator
class WDSEvaluator;

namespace eval
{
namespace detail
{
// A metric can be constructed if the passed type is a callable object with as 
// input a const reference to a WDS or a const reference to a network element type 
// of the wds (specifically: node, link, tank, reservoir, junction, pipe, pump, valve)
// and it returns a quantity series of double.

// Helper to check if a callable is valid for a specific input type
template <typename Callable, typename ArgType>
using is_metric_callable__ret_type__and__inp_arg = std::is_invocable_r<bevarmejo::wds::aux::QuantitySeries<double>, Callable, const ArgType&>;

// Trait to check if a callable is valid for any input argument in a tuple
template <typename Callable, typename Tuple, std::size_t Index = 0, std::size_t Size = std::tuple_size_v<Tuple>>
struct is_metric_callable__ret_type__and__inp_tuple
{
    static constexpr bool value = (
                                    (Index < Size) &&
                                    (
                                     is_metric_callable__ret_type__and__inp_arg<Callable, std::tuple_element_t<Index, Tuple>>::value ||
                                     is_metric_callable__ret_type__and__inp_tuple<Callable, Tuple, Index + 1>::value
                                    )
    );
};

// Specialization for the end of the tuple
template <typename Callable, typename Tuple, std::size_t Size>
struct is_metric_callable__ret_type__and__inp_tuple<Callable, Tuple, Size, Size>
{
    static constexpr bool value = false;
};

template <typename Callable>
constexpr bool is_metric_callable_on_wds_v = is_metric_callable__ret_type__and__inp_arg<Callable, WDS>::value;

template <typename Callable>
constexpr bool is_metric_callable_on_wds_net_elements_v = is_metric_callable__ret_type__and__inp_tuple<Callable, WDS::NetworkElementsTypes>::value;

} // namespace detail
} // namespace eval


class Metric final
{
/*------- Member types -------*/
public:
    using ModeSelector = TagsSelector<ViewModes>;

/*------- Member objects -------*/
private:
    std::string m__sys_selector__name;
    ModeSelector m__sys_selector__mode;

    // Type-erased callable for the WDS 
    std::function<wds::aux::QuantitySeries<double>(const WDS&)> m__func;

/*------- Member functions -------*/
// (constructor)
public:
    Metric() = delete; // No default constructor
    Metric(const Metric& rhs); // Copy constructor
    Metric(Metric&& rhs) noexcept = default; // Move constructor
    
    // Constructor template to accept any callable that can be used with the evaluator
    // that gets a WDS as input.
    template <typename Callable,
        typename M = RVMode::Exclude, // Mode for the system selector
        typename = std::enable_if_t<eval::detail::is_metric_callable_on_wds_v<Callable>>>
    explicit Metric(Callable&& callable, M sys_selector_mode = M{}, std::string sys_selector_name = "None") :
        m__sys_selector__name(std::move(sys_selector_name)),
        m__sys_selector__mode(make_selector<ViewModes, M>())
    {
        m__func = [callable = std::move(callable)](const WDS& a_wds) -> wds::aux::QuantitySeries<double>
        {
            // Nothing to do when the callable wants a WDS
            return callable(a_wds);
        };
    }

    // Templated constructor to accept any callable that can be used with the evaluator
    // that gets a WDS element as input.
    template <typename Callable,
        typename M1 = RVMode::Exclude, typename M2 = RVMode::Exclude, // Modes for the system and the subnetwork selectors
        typename = std::enable_if_t<eval::detail::is_metric_callable_on_wds_net_elements_v<Callable>>>
    explicit Metric(Callable&& callable,
                    M1 sys_selector_mode = M1{}, std::string sys_selector_name = "None",
                    M2 subnet_selector_mode = M2{}, std::string subnet_selector_name = "None") :
        m__sys_selector__name(std::move(sys_selector_name)),
        m__sys_selector__mode(make_selector<ViewModes, M1>())
    {
        m__func = [callable = std::move(callable), subnet_name = std::move(subnet_selector_name)](const WDS& a_wds) -> wds::aux::QuantitySeries<double>
        {
            // Based on the subnet mode and the subnet selector name extract the view for the type requied by the callable
            // Then iterate on that view and call the callable on each element.
            // At the end of this we have a vector of wds::aux::QuantitySeries<double> that we can merge.
            // As of now, merging is the average or comulative(?) of the series.
            using CallableArgType = typename wds::Junction; // Placeholder for the type of the callable argument to be extracted from the callable

            auto view = a_wds.network_elements_view<CallableArgType, M2>(subnet_name);

            std::vector<wds::aux::QuantitySeries<double>> series;
            for (const auto& [name, element] : a_view)
            {
                series.emplace_back(callable(element));
            }
            
            wds::aux::QuantitySeries<double> merged_series(series.front().time_series());
            for (const auto& element_series : series)
            {
                // merged_series.merge(element_series);
            }

            return merged_series;
        };
    }

// (destructor)
public:
    ~Metric() = default;

// operator=
public:
    Metric& operator=(const Metric& rhs);
    Metric& operator=(Metric&& rhs) noexcept = default;
    
// Actually only one method to compute the metric
public:
    QuantitySeries<double> compute(const WDSEvaluator& evaluator) const;

    wds::aux::QuantitySeries<double> operator()(const WaterDistributionSystem& a_wds) const
    {
        return m__func(a_wds);
    }

private:
    QuantitySeries<double> operator()(const WDSEvaluator& evaluator) const;

}; // class Metric

} // namespace bevarmejo
