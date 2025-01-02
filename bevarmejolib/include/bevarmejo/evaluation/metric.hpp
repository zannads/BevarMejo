#pragma once

#include <memory>
#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>

#include "bevarmejo/utility/bemexcept.hpp"
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
template <typename Callable, typename Tuple, std::size_t Index = 0>
struct is_metric_callable__ret_type__and__inp_tuple
{
    static constexpr bool value = (
                                    (Index < std::tuple_size_v<Tuple>) &&
                                    (
                                     is_metric_callable__ret_type__and__inp_arg<Callable, std::tuple_element_t<Index, Tuple>>::value ||
                                     is_metric_callable__ret_type__and__inp_tuple<Callable, Tuple, Index + 1>::value
                                    )
    );
};

// Specialization for the end of the tuple
template <typename Callable, typename Tuple>
struct is_metric_callable__ret_type__and__inp_tuple<Callable, Tuple, std::tuple_size_v<Tuple>>
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
    // Constructor template to accept any callable that can be used with the evaluator
    // that gets a WDS as input.
    template <typename Callable,
        typename M = RVMode::Exclude, // Mode for the system selector
        typename = std::enable_if_t<eval::detail::is_metric_callable_on_wds_v<Callable>>>
    explicit Metric(Callable callable, std::string sys_selector_name = "None") :
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
        typename M1, typename M2, // Modes for the system and the subnetwork selectors
        typename = std::enable_if_t<eval::detail::is_metric_callable_on_wds_net_elements_v<Callable>>>
    explicit Metric(Callable callable, std::string sys_selector_name = "None", std::string subnet_selector_name = "None") :
        m__sys_selector__name(std::move(sys_selector_name)),
        m__sys_selector__mode(make_selector<ViewModes, M1>())
    {
        m__func = [callable = std::move(callable), subnet_name = std::move(subnet_selector_name)](const WDS& a_wds) -> wds::aux::QuantitySeries<double>
        {
            // Based on the subnet mode and the subnet selector name extract the view for the type requied by the callable
            // Then iterate on that view and call the callable on each element.
            // At the end of this we have a vector of wds::aux::QuantitySeries<double> that we can merge.
            // As of now, merging is the average or comulative(?) of the series.
            using CallableArgType = std::tuple_element_t<0, a_wds::NetworkElementsTypes>; // TODO: extract the type from the callable

            auto compute_on_view = [&callable](auto a_view) -> std::vector<wds::aux::QuantitySeries<double>>
            {
                std::vector<wds::aux::QuantitySeries<double>> series;
                for (const auto& [name, element] : a_view)
                {
                    series.push_back(callable(element));
                }
                
                return series;
            };

            auto merge_elements_results = [](const std::vector<wds::aux::QuantitySeries<double>>& series) -> wds::aux::QuantitySeries<double>
            {
                wds::aux::QuantitySeries<double> merged_series;
                for (const auto& element_series : series)
                {
                    // merged_series.merge(element_series);
                }

                return merged_series;
            };

            // Get the view for the subnet
            if constexpr (std::is_same_v<M2, RVMode::Exclude>)
            {
                auto view = a_wds.subnetwork_excluding<CallableArgType>(subnet_name);

                return merge_elements_results(compute_on_view(view));
            }
            else if constexpr (std::is_same_v<M2, RVMode::Include>)
            {
                auto view = a_wds.subnetwork<CallableArgType>(subnet_name);

                return merge_elements_results(compute_on_view(view));
            }
            else if constexpr (std::is_same_v<M2, RVMode::OrderedInclude>)
            {
                auto view = a_wds.subnetwork_with_order<CallableArgType>(subnet_name);

                return merge_elements_results(compute_on_view(view));
            }
            else
            {
                static_assert(std::true_type::value, "Invalid mode for the subnet selector.");
            }
            
        };
    }

// (destructor)
public:
    ~Metric() = default;

// operator=
public:
    Metric& operator=(const Metric& rhs) = default;
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
