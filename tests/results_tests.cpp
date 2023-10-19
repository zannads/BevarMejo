//
// Description: Tests for the results class. All element should work, let's make sure they inherit well all the methods.
//
// Created by: Dennis Zanutto on 19/10/23.

#include <gtest/gtest.h>

// I may want to store either a single variable (e.g., ??)
// or temporal variables ("PRESSURE and FLOW")

#include "bevarmejo/wds/elements/results.hpp"
#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/variable.hpp"
#include "bevarmejo/wds/elements/variables.hpp"

class FakeNodeTest : public testing::Test {
    protected:
        void SetUp() override {
            // create a results object
            _results_ = bevarmejo::wds::results();

            // add some variables to track
            add_properties();
        }

        void TearDown() override { }

        // they will contain the object not point to it...
        bevarmejo::wds::results _results_;

        void add_properties() {
            // add some variables to track
            _results_.clear();
            //_results_.strings().insert("NAME",bevarmejo::wds::vars::variable<std::string>("unit", "test"));
            _results_.integers().emplace("IDp");
            _results_.integers().emplace("IDp1", "dimless");
            _results_.integers().emplace("IDp2", "dimless", 0);
            _results_.integers().emplace("IDo", bevarmejo::wds::vars::variable<int>());
            _results_.integers().emplace("IDo1", bevarmejo::wds::vars::variable<int>("dimless"));
            _results_.integers().emplace("IDo2", bevarmejo::wds::vars::variable<int>("dimless", 0));
            
            _results_.reals().emplace("ELEVATION", "m", 0.0);
            _results_.temporal_reals().emplace("PRESSURE", "Pa");
            _results_.temporal_reals().emplace("FLOW", "wrong unit", 5.0);
            _results_.temporal_reals().emplace("HEAD", "m", 0, 3.0);
        }
};


TEST_F(FakeNodeTest, DefaultConstructor) {
    ASSERT_EQ(10, _results_.size());
    ASSERT_EQ(0, _results_.strings().size());
    ASSERT_EQ(6, _results_.integers().size());
    ASSERT_EQ(_results_.integers().find("IDp")->second.unit(), _results_.integers().at("IDo").unit());
    ASSERT_EQ(_results_.integers()["IDp"].value(), _results_.integers().get("IDo"));
    ASSERT_EQ(3, _results_.temporal_reals().size());
    ASSERT_EQ("PRESSURE", _results_.temporal_reals().find("PRESSURE")->first);
    ASSERT_EQ(0, _results_.temporal_reals().at("PRESSURE").value().size());
    ASSERT_EQ(1, _results_.temporal_reals().get("FLOW").size());
    ASSERT_EQ(0, _results_.temporal_reals().get("HEAD").find(0)->first);
    ASSERT_EQ(3.0, _results_.temporal_reals().at("HEAD").value().when(0));
}