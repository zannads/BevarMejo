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

// Define a fixture class that initialized the GTO with a duration of 0 (Snapshot simulation)
class SnapshotTimeSeries : public ::testing::Test {
protected:
    using GTO= bevarmejo::epanet::GlobalTimeOptions;
    GTO gto;

    void SetUp() override {
        gto.duration__s(0);
    }
};

TEST_F(SnapshotTimeSeries, no_time_steps_constructors) {
    // The default constructor should not be callable (No compile)
    // TimeSeries(); /* Checked */

    // The actual default constructor needs the GTO. 
    // This is the only way to create a TimeSeries object.
    ASSERT_NO_THROW(TimeSeries(this->gto));

    // Since duration is 0, we should have a length of 2 (start and end)
    // with a single 0 value. And if we ask for the duration (back), it should be 0.
    TimeSeries ts(this->gto);
    EXPECT_EQ(ts.length(), 2); // Start and end
    EXPECT_EQ(ts.inner_size(), 1); // Only the start time
    EXPECT_EQ(ts.front(), 0);
    EXPECT_EQ(ts.back(), 0); // The duration
} // no_time_steps_constructors

TEST_F(SnapshotTimeSeries, default_logic_behaviour) {
// THis is to make sure that when the GTO is set to 0, the TimeSeries behaves as a snapshot simulation.
// And I can't play with it in the wrong way : 
//  - add or remove time steps 
// - changing the time steps by means of access functions and iterators 
// - changing the GTO
    ASSERT_EQ(this->gto.duration__s(), 0);
    TimeSeries ts(this->gto);
    ASSERT_EQ(ts.length(), 2); // Start and end 

    // I can't modify neither the gto nor the time_steps directly (this should not compile because the methods are const)
    // ts.gto().duration__s(10); /* Checked */
    // ts.time_steps().push_back(10); /* Checked */
    // ts.time_steps()[0] = 10; /* Checked */

    // Not found should be one more than the size (i.e., like the length = 2)
    // Because with 0 you can access the start and with 1 you can access the end
    EXPECT_EQ(ts.not_found(), 2);

    EXPECT_EQ(ts.at(0), 0);
    EXPECT_EQ(ts.at(1), 0);
    EXPECT_THROW(ts.at(2), std::out_of_range);
    EXPECT_EQ(ts.at(0), ts.front());
    EXPECT_EQ(ts.front(), ts.back()); // Only because it is a snapshot simulation
} 

TEST_F(SnapshotTimeSeries, variadic_constructor) {

    // Empty initializer list, one value different from zero, more than one value
    // They should all throw an exception because the GTO duration is set to 0.
    ASSERT_EQ(this->gto.duration__s(), 0);
    EXPECT_THROW(TimeSeries(this->gto, {}), std::invalid_argument);
    EXPECT_NO_THROW(TimeSeries(this->gto, {0l}));
    EXPECT_THROW(TimeSeries(this->gto, {10l}), std::invalid_argument);
    EXPECT_THROW(TimeSeries(this->gto, {0l, 10l}), std::invalid_argument);
    EXPECT_THROW(TimeSeries(this->gto, {10l, 0l}), std::invalid_argument);
    EXPECT_THROW(TimeSeries(this->gto, {10l, 10l}), std::invalid_argument);

    // EXPECT_THROW(TimeSeries(this->gto, 0l), std::invalid_argument);
    // EXPECT_NO_THROW(TimeSeries(this->gto, 1l)); // Shoudl be one value default initialized to 0
    // EXPECT_EQ(TimeSeries(this->gto, 1l).at(0l), 0l);
    // EXPECT_THROW(TimeSeries(this->gto, 3l), std::invalid_argument);

    // EXPECT_THROW(TimeSeries(this->gto, 0, 10l), std::invalid_argument);
    // EXPECT_NO_THROW(TimeSeries(this->gto, 1, 0l)); // Shoudl be one value default initialized to 0
    // EXPECT_EQ(TimeSeries(this->gto, 1).at(0), 0);
    // EXPECT_THROW(TimeSeries(this->gto, 1, 10l), std::invalid_argument);
    // EXPECT_THROW(TimeSeries(this->gto, 10, 0l), std::invalid_argument);
    // EXPECT_THROW(TimeSeries(this->gto, 10, 10l), std::invalid_argument);

    // std::vector<long> v{};
    // EXPECT_THROW(TimeSeries(this->gto, v), std::invalid_argument);
    // v= {0l};
    // EXPECT_NO_THROW(TimeSeries(this->gto, v));
    // EXPECT_EQ(TimeSeries(this->gto, v).at(0), 0);
    // v= {10l};
    // EXPECT_THROW(TimeSeries(this->gto, v), std::invalid_argument);
    // v= {0l, 10l};
    // EXPECT_THROW(TimeSeries(this->gto, v), std::invalid_argument);
    // v= std::vector<long>{10, 0l};
    // EXPECT_THROW(TimeSeries(this->gto, v), std::invalid_argument);
    // v= {0l, 1l, 10l, 0l};
    // EXPECT_THROW(TimeSeries(this->gto, v), std::invalid_argument);

    // EXPECT_THROW(TimeSeries(this->gto, std::vector<long>{}), std::invalid_argument);
    // EXPECT_NO_THROW(TimeSeries(this->gto, std::vector<long>{0l}));
    // EXPECT_EQ(TimeSeries(this->gto, std::vector<long>{0l}).at(0), 0);
    // EXPECT_THROW(TimeSeries(this->gto, std::vector<long>{10l}), std::invalid_argument);
    // EXPECT_THROW(TimeSeries(this->gto, std::vector<long>{0l, 10l}), std::invalid_argument);
    // EXPECT_THROW(TimeSeries(this->gto, std::vector<long>(10, 0l)), std::invalid_argument);
    // EXPECT_THROW(TimeSeries(this->gto, std::vector<long>({0l, 1l, 10l, 0l})), std::invalid_argument);
}

TEST_F(SnapshotTimeSeries, reset_doesnot_modify) {
    ASSERT_EQ(this->gto.duration__s(), 0);
    TimeSeries ts(this->gto);
    ASSERT_EQ(ts.length(), 2); // Start and end 

    ts.reset();
    EXPECT_EQ(ts.length(), 2ul);
    EXPECT_EQ(ts.at(0), 0l);
    EXPECT_EQ(ts.at(1), 0l);
    EXPECT_THROW(ts.at(2), std::out_of_range);
}

TEST_F(SnapshotTimeSeries, cannot_commit_to_snapshot) {
    ASSERT_EQ(this->gto.duration__s(), 0);
    TimeSeries ts(this->gto);
    ASSERT_EQ(ts.length(), 2); // Start and end 

    // I can't modify the values (this should not compile because I am not returning a lvalue)
    // ts.at(0) = 10; /* Checked */
    // ts.front() = 10; /* Checked */
    // ts.back() = 10; /* Checked */
    EXPECT_THROW(ts.commit(-3l), std::invalid_argument);
    EXPECT_NO_THROW(ts.commit(0l));
    EXPECT_EQ(ts.inner_size(), 1);
    EXPECT_THROW(ts.commit(10l), std::invalid_argument);
}

TEST_F(SnapshotTimeSeries, rollback_does_nothing_to_snapshot) {
    ASSERT_EQ(this->gto.duration__s(), 0);
    TimeSeries ts(this->gto);
    ASSERT_EQ(ts.length(), 2); // Start and end 

    EXPECT_NO_THROW(ts.rollback());
    ASSERT_EQ(ts.inner_size(), 1); 
    EXPECT_EQ(ts.at(0), 0l);
    EXPECT_EQ(ts.at(1), 0l);
    EXPECT_THROW(ts.at(2), std::out_of_range);
}

TEST_F(SnapshotTimeSeries, iterators_access_behaviour) {
    // I need to test 
    // 1. that the iterator are initialized correctly. 
    // 2. dereferencing the end iterator is UB (how ?)
    // 3. I can't modify the values (this should not compile because I am not returning a lvalue) 

    ASSERT_EQ(this->gto.duration__s(), 0);
    TimeSeries ts(this->gto);
    ASSERT_EQ(ts.length(), 2);

    auto it_b= ts.begin();
    EXPECT_EQ(*it_b, 0);
    EXPECT_EQ(it_b[0], 0);
    EXPECT_EQ(it_b[1], 0);

    // Should not be possible to modify it becasue I don't return a lvalue type
    // *it_b= 10l; /* Checked */
    auto it_e= ts.end();
    // Dereferencing end iterator is UB. However I added an assert.
    // ??

    auto it_cb= ts.cbegin();
    EXPECT_EQ(*it_cb, 0);
    //EXPECT_EQ(it_b)
    // How to test *?
    // Should not be possible to modify it becasue I don't return a lvalue type
    // *it_cb= 10l; /* Checked */
    auto it_ce= ts.cend();
    // Dereferencing end iterator is UB. However I added an assert.
    // ??
}

TEST_F(SnapshotTimeSeries, iterators_operators_behaviour_in_bounds) {
    // I need to test 
    // 1. you never go out of bounds (++, --, +, -)

    // With a snapshot simulation, the only values are 0 and 0
    // So I send this test to a non SnapshotTimeSeries test
}

TEST_F(SnapshotTimeSeries, iterators_operators_behaviour_out_of_bounds) {
    ASSERT_EQ(this->gto.duration__s(), 0);
    TimeSeries ts(this->gto);
    ASSERT_EQ(ts.length(), 2);

    auto it_b= ts.begin();
    EXPECT_NO_THROW(++it_b);
    EXPECT_EQ(*it_b, 0);
    EXPECT_NO_THROW(--it_b);
    EXPECT_EQ(*it_b, 0);
    EXPECT_NO_THROW(it_b++);
    EXPECT_EQ(*it_b, 0);
    EXPECT_NO_THROW(it_b--);
    EXPECT_EQ(*it_b, 0);
    EXPECT_NO_THROW(it_b + 1);
    EXPECT_EQ(*(it_b + 1), 0);
    EXPECT_NO_THROW(it_b - 1);
    EXPECT_EQ(*(it_b - 1), 0);
    {
        auto it_b2= it_b + 1;
        EXPECT_EQ(*it_b2, 0);
    }
    {
        auto it_b2= it_b - 1;
        EXPECT_EQ(*it_b2, 0);
    }
}


TEST_F(SnapshotTimeSeries, reverse_iterators_operators_behaviour) {

}

TEST_F(SnapshotTimeSeries, iterators_comparison_behaviour) {
    // I need to test (==, !=, <, >, <=, >=)
    
    ASSERT_EQ(this->gto.duration__s(), 0);
    TimeSeries ts(this->gto);
    ASSERT_EQ(ts.length(), 2);

    auto it_b= ts.begin();
    auto it_e= ts.end();
    auto it_cb= ts.cbegin();
    auto it_ce= ts.cend();

    EXPECT_EQ(it_b == ts.begin(), true);
    EXPECT_EQ(it_b != ts.begin(), false);
    EXPECT_EQ(it_b < ts.begin(), false);
    EXPECT_EQ(it_b > ts.begin(), false);
    EXPECT_EQ(it_b <= ts.begin(), true);
    EXPECT_EQ(it_b >= ts.begin(), true);

    EXPECT_EQ(it_b == it_e, false);
    EXPECT_EQ(it_b != it_e, true);
    EXPECT_EQ(it_b < it_e, true);
    EXPECT_EQ(it_b > it_e, false);
    EXPECT_EQ(it_b <= it_e, true);
    EXPECT_EQ(it_b >= it_e, false);

    EXPECT_EQ(it_b == it_b++, false);
    EXPECT_EQ(it_b == it_b--, false);
    EXPECT_EQ(it_b == ++it_b, true);
    EXPECT_EQ(it_b == --it_b, true);
}

TEST_F(SnapshotTimeSeries, reverse_iterators_comparison_behaviour) {
    // I need to test (==, !=, <, >, <=, >=)
}

TEST_F(SnapshotTimeSeries, iters_shiftaccess_operator) {
    
}

TEST_F(SnapshotTimeSeries, count_correctly) {

}

TEST_F(SnapshotTimeSeries, find_exact_value) {

}

TEST_F(SnapshotTimeSeries, find_exact_bounds) {

}


// Now more or less the same but for 

} // namespace __tests
} // namespace aux
} // namespace wds
} // namespace bevarmejo
