#include <gtest/gtest.h>

#include "bevarmejo/evaluation/metric.hpp"
#include "bevarmejo/evaluation/metrics/hydraulic_functions.hpp"

#include "bevarmejo/hydraulic_functions.hpp"

namespace __bevarmejo
{

class MockWDSMetric
{
    public:
        bevarmejo::wds::aux::QuantitySeries<double> operator()(const bevarmejo::WDS& a_wds) const
        {
            return bevarmejo::wds::aux::QuantitySeries<double>(a_wds.result_time_series());
        }
};

template<typename T>
class MockNodeMetric
{
    public:
        bevarmejo::wds::aux::QuantitySeries<double> operator()(const T& a_element) const
        {
            return bevarmejo::wds::aux::QuantitySeries<double>(a_element.head().time_series());
        }
};

template<typename T>
class MockLinkMetric
{
    public:
        bevarmejo::wds::aux::QuantitySeries<double> operator()(const T& a_element) const
        {
            return bevarmejo::wds::aux::QuantitySeries<double>(a_element.flow().time_series());
        }
};

bevarmejo::wds::aux::QuantitySeries<double> MockFunction(const bevarmejo::WDS& a_wds)
{
    return bevarmejo::wds::aux::QuantitySeries<double>(a_wds.result_time_series());
}

// Tests for Metric
TEST(MetricTests, MetricTraits)
{
    static_assert(bevarmejo::eval::detail::is_metric_callable__ret_type__and__inp_arg<MockWDSMetric, bevarmejo::WDS>::value, "is_metric_callable__ret_type__and__inp_arg failed for MockWDSMetric");
    static_assert(bevarmejo::eval::detail::is_metric_callable_on_wds_v<MockWDSMetric>, "is_metric_callable_on_wds_v failed for MockWDSMetric");

    static_assert(bevarmejo::eval::detail::is_metric_callable__ret_type__and__inp_arg<decltype(MockFunction), bevarmejo::WDS>::value, "is_metric_callable__ret_type__and__inp_arg failed for MockFunction");
    static_assert(bevarmejo::eval::detail::is_metric_callable_on_wds_v<decltype(MockFunction)>, "is_metric_callable_on_wds_v failed for MockFunction");

    static_assert(bevarmejo::eval::detail::is_metric_callable__ret_type__and__inp_arg<MockNodeMetric<bevarmejo::wds::Node>, bevarmejo::wds::Node>::value, "is_metric_callable__ret_type__and__inp_arg failed for MockNodeMetric<bevarmejo::Node>");
    static_assert(bevarmejo::eval::detail::is_metric_callable_on_wds_net_elements_v<MockNodeMetric<bevarmejo::wds::Node>>, "is_metric_callable_on_wds_net_elements_v failed for MockNodeMetric<bevarmejo::Node>");

    static_assert(bevarmejo::eval::detail::is_metric_callable__ret_type__and__inp_arg<MockNodeMetric<bevarmejo::wds::Junction>, bevarmejo::wds::Junction>::value, "is_metric_callable__ret_type__and__inp_arg failed for MockNodeMetric<bevarmejo::Junction>");
    static_assert(bevarmejo::eval::detail::is_metric_callable_on_wds_net_elements_v<MockNodeMetric<bevarmejo::wds::Junction>>, "is_metric_callable_on_wds_net_elements_v failed for MockNodeMetric<bevarmejo::Junction>");

    static_assert(bevarmejo::eval::detail::is_metric_callable__ret_type__and__inp_arg<MockNodeMetric<bevarmejo::wds::Reservoir>, bevarmejo::wds::Reservoir>::value, "is_metric_callable__ret_type__and__inp_arg failed for MockNodeMetric<bevarmejo::Reservoir>");
    static_assert(bevarmejo::eval::detail::is_metric_callable_on_wds_net_elements_v<MockNodeMetric<bevarmejo::wds::Reservoir>>, "is_metric_callable_on_wds_net_elements_v failed for MockNodeMetric<bevarmejo::Reservoir>");

    static_assert(bevarmejo::eval::detail::is_metric_callable__ret_type__and__inp_arg<MockNodeMetric<bevarmejo::wds::Tank>, bevarmejo::wds::Tank>::value, "is_metric_callable__ret_type__and__inp_arg failed for MockNodeMetric<bevarmejo::Tank>");
    static_assert(bevarmejo::eval::detail::is_metric_callable_on_wds_net_elements_v<MockNodeMetric<bevarmejo::wds::Tank>>, "is_metric_callable_on_wds_net_elements_v failed for MockNodeMetric<bevarmejo::Tank>");

    static_assert(bevarmejo::eval::detail::is_metric_callable__ret_type__and__inp_arg<MockLinkMetric<bevarmejo::wds::Link>, bevarmejo::wds::Link>::value, "is_metric_callable__ret_type__and__inp_arg failed for MockLinkMetric<bevarmejo::Link>");
    static_assert(bevarmejo::eval::detail::is_metric_callable_on_wds_net_elements_v<MockLinkMetric<bevarmejo::wds::Link>>, "is_metric_callable_on_wds_net_elements_v failed for MockLinkMetric<bevarmejo::Link>");

    static_assert(bevarmejo::eval::detail::is_metric_callable__ret_type__and__inp_arg<MockLinkMetric<bevarmejo::wds::Pipe>, bevarmejo::wds::Pipe>::value, "is_metric_callable__ret_type__and__inp_arg failed for MockLinkMetric<bevarmejo::Pipe>");
    static_assert(bevarmejo::eval::detail::is_metric_callable_on_wds_net_elements_v<MockLinkMetric<bevarmejo::wds::Pipe>>, "is_metric_callable_on_wds_net_elements_v failed for MockLinkMetric<bevarmejo::Pipe>");

    static_assert(bevarmejo::eval::detail::is_metric_callable__ret_type__and__inp_arg<MockLinkMetric<bevarmejo::wds::Pump>, bevarmejo::wds::Pump>::value, "is_metric_callable__ret_type__and__inp_arg failed for MockLinkMetric<bevarmejo::Pump>");
    static_assert(bevarmejo::eval::detail::is_metric_callable_on_wds_net_elements_v<MockLinkMetric<bevarmejo::wds::Pump>>, "is_metric_callable_on_wds_net_elements_v failed for MockLinkMetric<bevarmejo::Pump>");

}

TEST(MetricTests, MetricConstructionWDS)
{
    bevarmejo::Metric metric1(MockWDSMetric{});

    bevarmejo::Metric metric2([](const bevarmejo::WDS& a_wds) -> bevarmejo::wds::aux::QuantitySeries<double>
    {
        return bevarmejo::wds::aux::QuantitySeries<double>(a_wds.result_time_series());
    });

    bevarmejo::Metric metric3(MockFunction);

    
    bevarmejo::Metric metric11(MockWDSMetric{}, bevarmejo::RVMode::Include{});

    bevarmejo::Metric metric12([](const bevarmejo::WDS& a_wds) -> bevarmejo::wds::aux::QuantitySeries<double>
    {
        return bevarmejo::wds::aux::QuantitySeries<double>(a_wds.result_time_series());
    }, bevarmejo::RVMode::Include{});

    bevarmejo::Metric metric13(MockFunction, bevarmejo::RVMode::Include{});

    bevarmejo::Metric metric21(MockWDSMetric{}, bevarmejo::RVMode::OrderedInclude{}, "city_pipes");

    bevarmejo::Metric metric22([](const bevarmejo::WDS& a_wds) -> bevarmejo::wds::aux::QuantitySeries<double>
    {
        return bevarmejo::wds::aux::QuantitySeries<double>(a_wds.result_time_series());
    }, bevarmejo::RVMode::OrderedInclude{}, "city_pipes");

    bevarmejo::Metric metric23(MockFunction, bevarmejo::RVMode::OrderedInclude{}, "city_pipes");


    

}    

} // namespace __bevarmejo