#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "epanet2_2.h"

#include "bevarmejo/epanet_helpers/en_time_options.hpp"
#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/auxiliary/pattern.hpp"

#include "bevarmejo/quantity_series.hpp"

namespace bevarmejo {
namespace wds {
namespace test {

static const std::string l__test_ts_null = "Test::Null";
static const std::string l__test_ts_empty = "Test::Empty";
static const std::string l__test_ts_one = "Test::One";
static const std::string l__test_ts_constant = "Test::Constant"; // Should have 2 time steps, start and end
static const std::string l__test_ts_regular_input = "Test::RegularInput"; // Should have N (4) time steps, start, step, 2step, 3step=end
static const std::string l__test_ts_results = "Test::Results"; // Should have more than N and regular and non regular time steps

static long k__test_ts_start = 0l;
static long k__test_ts_step = 3600l;
static long k__test_ts_n_steps = 6l;
static long k__test_ts_duration = k__test_ts_step * k__test_ts_n_steps;

// I need the tests to run for integer, doubles and vector of them
using TestTypes = ::testing::Types<int, double, std::vector<int>, std::vector<double>>;

// define a template fixture class for testing
template <typename T>
class OpenQuantitySeriesTest : public ::testing::Test {

protected:
    std::unordered_map<std::string, std::shared_ptr<TimeSteps>> ts_map;

    void SetUp() override {
        ts_map.insert({l__test_ts_null, nullptr});
        ts_map.insert({l__test_ts_empty, std::make_shared<TimeSteps>()});
        ts_map.insert({l__test_ts_one, std::make_shared<TimeSteps>(std::initializer_list<long>{k__test_ts_start})});
        ts_map.insert({l__test_ts_constant, std::make_shared<TimeSteps>(std::initializer_list<long>{k__test_ts_start, k__test_ts_duration})});
        {
            ts_map.insert({l__test_ts_regular_input, std::make_shared<TimeSteps>(k__test_ts_n_steps+1, k__test_ts_start) }); // +1 because of the end time
            time_t curr_time = k__test_ts_start+k__test_ts_step;
            for (auto it = ts_map.at(l__test_ts_regular_input)->begin()+1; it != ts_map.at(l__test_ts_regular_input)->end(); ++it) {
                *it = curr_time;
                curr_time += k__test_ts_step;
            }
        }
        ts_map.insert({l__test_ts_results, std::make_shared<TimeSteps>(
            std::initializer_list<long>{k__test_ts_step*0, k__test_ts_step/2l, k__test_ts_step*3/4, 
                                        k__test_ts_step*1, 2*k__test_ts_step*2, k__test_ts_step*3, k__test_ts_step*7/2,
                                        k__test_ts_step*4, k__test_ts_step*5, k__test_ts_step*k__test_ts_n_steps} ) } );
    }

    void TearDown() override {
        ts_map.clear();
    }

    T CreateValue() {
        if constexpr (std::is_same_v<T, int>) {
            return 1;
        } else if constexpr (std::is_same_v<T, double>) {
            return 1.0;
        } else if constexpr (std::is_same_v<T, std::vector<int>>) {
            return {1, 2, 3};
        } else if constexpr (std::is_same_v<T, std::vector<double>>) {
            return {1.0, 2.0, 3.0};
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

TYPED_TEST_SUITE(OpenQuantitySeriesTest, TestTypes);

TYPED_TEST(OpenQuantitySeriesTest, EmptyConstructorsAndStates) {

    QuantitySeries<TypeParam> qs1;
    ASSERT_EQ(qs1.time_steps(), nullptr);
    EXPECT_EQ(qs1.values().size(), 0);

    EXPECT_THROW(qs1.check_valid(), std::runtime_error);

    EXPECT_EQ(qs1.is_state(QuantitySeries<TypeParam>::State::Invalid), false);
    EXPECT_EQ(qs1.is_state(QuantitySeries<TypeParam>::State::Transitional), true);

    EXPECT_EQ(qs1.is_special_case(QuantitySeries<TypeParam>::Case::Undefined), true);
    EXPECT_EQ(qs1.is_special_case(QuantitySeries<TypeParam>::Case::Constant), false);
    EXPECT_EQ(qs1.is_special_case(QuantitySeries<TypeParam>::Case::Regular), false);
    EXPECT_EQ(qs1.is_special_case(QuantitySeries<TypeParam>::Case::Flexible), false);
}

// Test the constructor passing the vector of time steps only. Independently of the 
// the fact that this is for a constant, regular or whatever, the object should be
// in a transitional state, because it is not valid yet.
TYPED_TEST(OpenQuantitySeriesTest, PointerConstructorAndStates) {

    // Expecting pointer to the right thing and empty object
    // Expecting to not be valid, but in a transitional state (therefore undefined too, always)

    for (auto& [key, value] : this->ts_map) {
        QuantitySeries<TypeParam> qs(value);
        ASSERT_EQ(qs.time_steps(), value);
        EXPECT_EQ(qs.values().size(), 0);

        EXPECT_THROW(qs.check_valid(), std::runtime_error);

        EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Invalid), false);
        EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Transitional), true);

        EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Undefined), true);
        EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Constant), false);
        EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Regular), false);
        EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Flexible), false);
    }

}

// Test the constructor passing the vector of time steps and a single value. 
// Now based on the time steps, the object can be in DIFFERENT states.
TYPED_TEST(OpenQuantitySeriesTest, PointerValueConstructorAndStates) {

    // Normal behaviour: Expecting pointer to the right thing and values of the same size. 
    
    for (auto& [key, value] : this->ts_map) {
        QuantitySeries<TypeParam> qs(value, this->CreateValue());
        ASSERT_EQ(qs.time_steps(), value);
        
        if (key == l__test_ts_null || key == l__test_ts_empty) {
            EXPECT_EQ(qs.values().size(), 0);
        } else {
            EXPECT_EQ(qs.values().size(), value->size()-1);
        }

        if (key == l__test_ts_null || key == l__test_ts_empty || key == l__test_ts_one) {
            EXPECT_THROW(qs.check_valid(), std::runtime_error);
            EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Invalid), false);
            EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Transitional), true);

            EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Undefined), true);
            EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Constant), false);
            EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Regular), false);
            EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Flexible), false);
        } else {
            EXPECT_NO_THROW(qs.check_valid());
            EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Invalid), false);
            EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Transitional), false);

            if (key == l__test_ts_constant) {
                EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Undefined), false);
                EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Constant), true);
                EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Regular), false);
                EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Flexible), false);
            } else if (key == l__test_ts_regular_input) {
                EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Undefined), false);
                EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Constant), false);
                EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Regular), true);
                EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Flexible), false);
            } else {
                // Even if I pass a time series that is for a flexible, this constructor would build a regular
                EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Undefined), false);
                EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Constant), false);
                EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Regular), true);
                EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Flexible), false);
            }
        }
    }

    
    // Test the same constructor but with the case to test the force flexible option
    for (auto& [key, value] : this->ts_map) {
        QuantitySeries<TypeParam> qs(value, this->CreateValue(), /*force_flexible=*/ true);
        ASSERT_EQ(qs.time_steps(), value);
        
        if (key == l__test_ts_null || key == l__test_ts_empty) {
            EXPECT_EQ(qs.values().size(), 0);
        } else {
            EXPECT_EQ(qs.values().size(), value->size());
        }

        if (key == l__test_ts_null || key == l__test_ts_empty || key == l__test_ts_one) {
            EXPECT_THROW(qs.check_valid(), std::runtime_error);
            EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Invalid), false);
            EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Transitional), true);

            EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Undefined), true);
            EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Constant), false);
            EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Regular), false);
            EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Flexible), false);
        } else {
            EXPECT_NO_THROW(qs.check_valid());
            EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Invalid), false);
            EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Transitional), false);

            // In this case it is not a constant anymore, but a flexible because I forced it to be
            EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Undefined), false);
            EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Constant), false);
            EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Regular), false);
            EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Flexible), true);
        }
    } 
}

// Test the constructor passing the vector of time steps and a vector of values (test for different sizes).
// Test for: empty vector, size 1 vector, size 2 vector, NT-x vector, NT-1 vector, NT vector, NT+x vector
template <typename T>
class OpenQuantitySeriesVectorTest : public OpenQuantitySeriesTest<T> {
protected:
    std::vector<std::vector<T>> input_values;

    void SetUp() override {
        OpenQuantitySeriesTest<T>::SetUp();
        
        // N = k__test_ts_n_steps+1 = 7
        input_values.push_back({});                                                                 // empty
        input_values.push_back({this->CreateValue()});                                              // 1
        input_values.push_back({this->CreateValue(), this->CreateValue()});                         // 2
        input_values.push_back({this->CreateValue(), this->CreateValue(), this->CreateValue()});    // N-x (x=4)
        input_values.push_back({this->CreateValue(), this->CreateValue(), this->CreateValue(), 
                                this->CreateValue(), this->CreateValue(), this->CreateValue()});    // N-1
        input_values.push_back({this->CreateValue(), this->CreateValue(), this->CreateValue(),
                                this->CreateValue(), this->CreateValue(), this->CreateValue(),
                                this->CreateValue()});                                              // N
        input_values.push_back({this->CreateValue(), this->CreateValue(), this->CreateValue(),
                                this->CreateValue(), this->CreateValue(), this->CreateValue(),
                                this->CreateValue(), this->CreateValue(), this->CreateValue()});    // N+x (x=2)
    }

    void TearDown() override {
        OpenQuantitySeriesTest<T>::TearDown();
        input_values.clear();
    }

};

TYPED_TEST_SUITE(OpenQuantitySeriesVectorTest, TestTypes);

TYPED_TEST(OpenQuantitySeriesVectorTest, VectorConstructorAndStates) {

    for (auto& [key, value] : this->ts_map) {
        for (auto& input : this->input_values) {
            QuantitySeries<TypeParam> qs(value, input);
            ASSERT_EQ(qs.time_steps(), value);

            if (key == l__test_ts_null || key == l__test_ts_empty) {
                EXPECT_EQ(qs.values().size(), 0);
            } else {
                EXPECT_EQ(qs.values().size(), input.size());
            }

            if (key == l__test_ts_null || key == l__test_ts_empty ) {
                EXPECT_THROW(qs.check_valid(), std::runtime_error);
                EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Invalid), false);
                EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Transitional), true);

                EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Undefined), true);
                EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Constant), false);
                EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Regular), false);
                EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Flexible), false);
            } else if (key == l__test_ts_one) {
                
                if (input.size() <= 1) {
                    EXPECT_THROW(qs.check_valid(), std::runtime_error);
                    EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Invalid), false);
                    EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Transitional), true);

                } else { // > 1, i.e., greater than the time steps
                    EXPECT_THROW(qs.check_valid(), std::runtime_error);
                    EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Invalid), true);
                    EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Transitional), false);
                }

                EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Undefined), true);
                EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Constant), false);
                EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Regular), false);
                EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Flexible), false);
            } else if (key == l__test_ts_constant) {
                if (input.size() == 0) {
                    EXPECT_THROW(qs.check_valid(), std::runtime_error);
                    EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Invalid), false);
                    EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Transitional), true);

                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Undefined), true);
                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Constant), false);
                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Regular), false);
                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Flexible), false);
                } else if (input.size() == 1 || input.size() == 2) {
                    EXPECT_NO_THROW(qs.check_valid());
                    EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Invalid), false);
                    EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Transitional), false);

                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Undefined), false);
                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Regular), false);

                    if (input.size() == 1) {
                        EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Constant), true);
                        EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Flexible), false);
                    } else {
                        EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Constant), false);
                        EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Flexible), true);
                    }
                } else { // > 2, i.e., greater than the time steps
                    EXPECT_THROW(qs.check_valid(), std::runtime_error);
                    EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Invalid), true);
                    EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Transitional), false);

                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Undefined), true);
                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Constant), false);
                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Regular), false);
                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Flexible), false);
                }
            } else { // if (key == l__test_ts_regular_input || key == l__test_ts_results) 
                if (input.size() < value->size()-1) { 
                    // It should always be transitional, because it is not valid yet
                    EXPECT_THROW(qs.check_valid(), std::runtime_error);
                    EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Invalid), false);
                    EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Transitional), true);

                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Undefined), true);
                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Constant), false);
                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Regular), false);
                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Flexible), false);
                
                } else if (input.size() == value->size()-1) {
                    // Special case for the regular input, doesn't matter if it is a regular time or not
                    EXPECT_NO_THROW(qs.check_valid());
                    EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Invalid), false);
                    EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Transitional), false);

                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Undefined), false);
                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Constant), false);
                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Regular), true);
                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Flexible), false);

                } else if (input.size() == value->size()) {
                    // Special case for the flexible input, doesn't matter if it is a regular time or not
                    EXPECT_NO_THROW(qs.check_valid());
                    EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Invalid), false);
                    EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Transitional), false);

                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Undefined), false);
                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Constant), false);
                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Regular), false);
                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Flexible), true);

                } else { // > value->size(), i.e., greater than the time steps
                    EXPECT_THROW(qs.check_valid(), std::runtime_error);
                    EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Invalid), true);
                    EXPECT_EQ(qs.is_state(QuantitySeries<TypeParam>::State::Transitional), false);

                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Undefined), true);
                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Constant), false);
                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Regular), false);
                    EXPECT_EQ(qs.is_special_case(QuantitySeries<TypeParam>::Case::Flexible), false);

                }
            }
        }
    }
}

TYPED_TEST(OpenQuantitySeriesVectorTest, TimeBasedLookup) {

        for (auto& [key, value] : this->ts_map) {
        for (auto& input : this->input_values) {
            QuantitySeries<TypeParam> qs(value, input);
            
            if (qs.is_special_case(QuantitySeries<TypeParam>::Case::Undefined)) {
                EXPECT_THROW(qs.at(0), std::runtime_error);
            }
            else { // Is valid 
                EXPECT_THROW(qs.find_pos(k__test_ts_start-1l), std::out_of_range);
                EXPECT_THROW(qs.find_pos(k__test_ts_start+k__test_ts_duration+100l), std::out_of_range);
                EXPECT_THROW(qs.lower_bound_pos(k__test_ts_start-1l), std::out_of_range);
                EXPECT_THROW(qs.lower_bound_pos(k__test_ts_start+k__test_ts_duration+100l), std::out_of_range);
                EXPECT_THROW(qs.upper_bound_pos(k__test_ts_start-1l), std::out_of_range);
                EXPECT_THROW(qs.upper_bound_pos(k__test_ts_start+k__test_ts_duration+100l), std::out_of_range);

                if (key == l__test_ts_constant) {
                    EXPECT_EQ(0, qs.lower_bound_pos(k__test_ts_start));
                    EXPECT_EQ(0, qs.find_pos(k__test_ts_start));
                    EXPECT_EQ(1, qs.upper_bound_pos(k__test_ts_start));

                    EXPECT_EQ(0, qs.lower_bound_pos(k__test_ts_start+1));
                    EXPECT_EQ(0, qs.find_pos(k__test_ts_start+1));
                    EXPECT_EQ(1, qs.upper_bound_pos(k__test_ts_start+1));

                    EXPECT_EQ(0, qs.lower_bound_pos(k__test_ts_start+k__test_ts_step-1));
                    EXPECT_EQ(0, qs.find_pos(k__test_ts_start+k__test_ts_step-1));
                    EXPECT_EQ(1, qs.upper_bound_pos(k__test_ts_start+k__test_ts_step-1));
                    
                    EXPECT_EQ(0, qs.lower_bound_pos(k__test_ts_start+k__test_ts_step));
                    EXPECT_EQ(0, qs.find_pos(k__test_ts_start+k__test_ts_step));
                    EXPECT_EQ(1, qs.upper_bound_pos(k__test_ts_start+k__test_ts_step));

                    EXPECT_EQ(0, qs.lower_bound_pos(k__test_ts_start+k__test_ts_step+1));
                    EXPECT_EQ(0, qs.find_pos(k__test_ts_start+k__test_ts_step+1));
                    EXPECT_EQ(1, qs.upper_bound_pos(k__test_ts_start+k__test_ts_step+1));

                    EXPECT_EQ(0, qs.lower_bound_pos(k__test_ts_start+k__test_ts_duration-1));
                    EXPECT_EQ(0, qs.find_pos(k__test_ts_start+k__test_ts_duration-1));
                    EXPECT_EQ(1, qs.upper_bound_pos(k__test_ts_start+k__test_ts_duration-1));

                    EXPECT_EQ(0, qs.lower_bound_pos(k__test_ts_start+k__test_ts_duration));
                    EXPECT_EQ(1, qs.find_pos(k__test_ts_start+k__test_ts_duration));
                    EXPECT_EQ(1, qs.upper_bound_pos(k__test_ts_start+k__test_ts_duration));

                }
                else if (key == l__test_ts_regular_input) {
                    EXPECT_EQ(0, qs.lower_bound_pos(k__test_ts_start));
                    EXPECT_EQ(0, qs.find_pos(k__test_ts_start));
                    EXPECT_EQ(1, qs.upper_bound_pos(k__test_ts_start));

                    EXPECT_EQ(0, qs.lower_bound_pos(k__test_ts_start+1));
                    EXPECT_EQ(0, qs.find_pos(k__test_ts_start+1));
                    EXPECT_EQ(1, qs.upper_bound_pos(k__test_ts_start+1));
                    
                    EXPECT_EQ(0, qs.lower_bound_pos(k__test_ts_start+k__test_ts_step-1));
                    EXPECT_EQ(0, qs.find_pos(k__test_ts_start+k__test_ts_step-1));
                    EXPECT_EQ(1, qs.upper_bound_pos(k__test_ts_start+k__test_ts_step-1));

                    EXPECT_EQ(0, qs.lower_bound_pos(k__test_ts_start+k__test_ts_step));
                    EXPECT_EQ(1, qs.find_pos(k__test_ts_start+k__test_ts_step));
                    EXPECT_EQ(2, qs.upper_bound_pos(k__test_ts_start+k__test_ts_step));

                    EXPECT_EQ(1, qs.lower_bound_pos(k__test_ts_start+k__test_ts_step+1));
                    EXPECT_EQ(1, qs.find_pos(k__test_ts_start+k__test_ts_step+1));
                    EXPECT_EQ(2, qs.upper_bound_pos(k__test_ts_start+k__test_ts_step+1));

                    EXPECT_EQ(k__test_ts_n_steps-1, qs.lower_bound_pos(k__test_ts_start+k__test_ts_duration-1));
                    EXPECT_EQ(k__test_ts_n_steps-1, qs.find_pos(k__test_ts_start+k__test_ts_duration-1));
                    EXPECT_EQ(k__test_ts_n_steps-0, qs.upper_bound_pos(k__test_ts_start+k__test_ts_duration-1));

                    EXPECT_EQ(k__test_ts_n_steps-1, qs.lower_bound_pos(k__test_ts_start+k__test_ts_duration));
                    EXPECT_EQ(k__test_ts_n_steps-0, qs.find_pos(k__test_ts_start+k__test_ts_duration));
                    EXPECT_EQ(k__test_ts_n_steps-0, qs.upper_bound_pos(k__test_ts_start+k__test_ts_duration));
                }
                else { // if (key == l__test_ts_results) 
                    // Similar to the regular input, but with different time steps, therefore expected positions
                    EXPECT_EQ(0, qs.lower_bound_pos(k__test_ts_start));
                    EXPECT_EQ(0, qs.find_pos(k__test_ts_start));
                    EXPECT_EQ(1, qs.upper_bound_pos(k__test_ts_start));

                    EXPECT_EQ(0, qs.lower_bound_pos(k__test_ts_start+1));
                    EXPECT_EQ(0, qs.find_pos(k__test_ts_start+1));
                    EXPECT_EQ(1, qs.upper_bound_pos(k__test_ts_start+1));
                    
                    // k__test_ts_start+k__test_ts_step-1 = 3599+start, I have two extra 1800 and 2700, 3600 is the fourth 
                    EXPECT_EQ(2, qs.lower_bound_pos(k__test_ts_start+k__test_ts_step-1));
                    EXPECT_EQ(2, qs.find_pos(k__test_ts_start+k__test_ts_step-1));
                    EXPECT_EQ(3, qs.upper_bound_pos(k__test_ts_start+k__test_ts_step-1));

                    EXPECT_EQ(2, qs.lower_bound_pos(k__test_ts_start+k__test_ts_step));
                    EXPECT_EQ(3, qs.find_pos(k__test_ts_start+k__test_ts_step));
                    EXPECT_EQ(4, qs.upper_bound_pos(k__test_ts_start+k__test_ts_step));

                    EXPECT_EQ(3, qs.lower_bound_pos(k__test_ts_start+k__test_ts_step+1));
                    EXPECT_EQ(3, qs.find_pos(k__test_ts_start+k__test_ts_step+1));
                    EXPECT_EQ(4, qs.upper_bound_pos(k__test_ts_start+k__test_ts_step+1));

                    EXPECT_EQ(9-1, qs.lower_bound_pos(k__test_ts_start+k__test_ts_duration-1));
                    EXPECT_EQ(9-1, qs.find_pos(k__test_ts_start+k__test_ts_duration-1));
                    EXPECT_EQ(9-0, qs.upper_bound_pos(k__test_ts_start+k__test_ts_duration-1));

                    EXPECT_EQ(9-1, qs.lower_bound_pos(k__test_ts_start+k__test_ts_duration));
                    EXPECT_EQ(9-0, qs.find_pos(k__test_ts_start+k__test_ts_duration));
                    EXPECT_EQ(9-0, qs.upper_bound_pos(k__test_ts_start+k__test_ts_duration));
                    
                }
                
            }
        }
        }
}

} // namespace test
} // namespace wds
} // namespace bevarmejo
