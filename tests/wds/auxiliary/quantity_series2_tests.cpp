#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "bevarmejo/epanet_helpers/en_time_options.hpp"
#include "bevarmejo/wds/auxiliary/time_series.hpp"

namespace bevarmejo {
namespace wds {
namespace aux {
namespace __tests {

// I need the tests to run for integer, doubles and vector of them
using TestTypes = ::testing::Types<int, double, std::vector<int>, std::vector<double>>;

// Define a template fixture class for setting up the tests with the different types.
template <typename T>
class TimeSeriesTypedTest : public ::testing::Test {

protected:

    T CreateValue(double i=1.0) {
        if constexpr (std::is_same_v<T, int>) {
            return static_cast<int>(1 * i);
        } else if constexpr (std::is_same_v<T, double>) {
            return 1.0*i;
        } else if constexpr (std::is_same_v<T, std::vector<int>>) {
            return {static_cast<int>(1 * i), static_cast<int>(2 * i)};
        } else if constexpr (std::is_same_v<T, std::vector<double>>) {
            return {1.0*i, 2.0*i, 3.0*i};
        }
    }

    T DefaultValue() {
        if constexpr (std::is_same_v<T, int>) {
            return 0;
        } else if constexpr (std::is_same_v<T, double>) {
            return 0.0;
        } else if constexpr (std::is_same_v<T, std::vector<int>>) {
            return {};
        } else if constexpr (std::is_same_v<T, std::vector<double>>) {
            return {};
        }
    }

    bool AreEqual(const T& a, const T& b) {
        if constexpr (std::is_same_v<T, int>) {
            return a == b;
        } else if constexpr (std::is_same_v<T, double>) {
            return a == b;
        } else if constexpr (std::is_same_v<T, std::vector<int>>) {
            return a == b;
        } else if constexpr (std::is_same_v<T, std::vector<double>>) {
            return a == b;
        }
    }
};

template <typename T>
class SnapshotTimeSeries : public TimeSeriesTypedTest<T> {
protected:
    using GTO= bevarmejo::epanet::GlobalTimeOptions;
    GTO gto;

    // GTO shoudl already be SetUp with duration = 0 (Snapshot simulation)
};

//TYPED_TEST_SUITE(TimeSeriesTypedTest, TestTypes);
TYPED_TEST_SUITE(SnapshotTimeSeries, TestTypes);

TYPED_TEST(SnapshotTimeSeries, no_time_steps_constructors) {

    // The default constructor should not be callable
    TimeSeries();

    // The actual default constructor needs the GTO. 
    // This is the only way to create a TimeSeries object.
    ASSERT_NO_THROW(TimeSeries(this->gto));

    // Since duration is 0, we should have a length of 2 (start and end)
    // with a single 0 value. And if we ask for the duration, it should be 0.
    TimeSeries ts(this->gto);
    EXPECT_EQ(ts.length(), 2); // Start and end
    EXPECT_EQ(ts.inner_size(), 1); // Only the start time
    EXPECT_EQ(ts.front(), 0);
    EXPECT_EQ(ts.back(), 0);
} // no_time_steps_constructors

TYPED_TEST(SnapshotTimeSeries, correct_default_logic_behaviour) {
    // THis is to make sure that when the GTO is set to 0, the TimeSeries behaves as a snapshot simulation.
    // And I can't play with it in the wrong way.
    TimeSeries

    // I can't modify neither the gto nor the time_steps directly (this should not compile)
    ts.gto().duration__s(10);
    ts.m__time_steps().push_back(10);
    ts.m__time_steps()[0] = 10;

    // Iterators
    EXPECT_EQ(*ts.begin(), 0);
    EXPECT_EQ(*ts.cbegin(), 0);
    EXPECT_EQ(*ts.end(), 0);
    EXPECT_EQ(*ts.cend(), 0);
    EXPECT_EQ(*ts.rbegin(), 0);
    EXPECT_EQ(*ts.crbegin(), 0);
    EXPECT_EQ(*ts.rend(), 0);
    EXPECT_EQ(*ts.crend(), 0);

    EXPECT_LT(ts.begin(), ts.end());
    EXPECT_LT(ts.rbegin(), ts.rend());
    EXPECT_EQ(ts.begin()+1, ts.end());
    EXPECT_EQ(ts.rbegin()+1, ts.rend());

    // I can't modify the values
    *ts.begin() = 10;
    *ts.rbegin() = 10;

}