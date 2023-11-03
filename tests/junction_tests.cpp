#include <gtest/gtest.h>

#include "bevarmejo/wds/elements/junction.hpp"
#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/element.hpp"

#include "bevarmejo/wds/elements/results.hpp"
#include "bevarmejo/wds/elements/variables.hpp"
#include "bevarmejo/wds/elements/variable.hpp"
#include "bevarmejo/wds/elements/temporal.hpp"

using namespace bevarmejo::wds;

TEST(JunctionTests, Constructor) {
    //junction j; // should not build 
    junction j("J1");
    ASSERT_EQ("J1", j.id());
    EXPECT_EQ("Junction", j.element_name());
    EXPECT_EQ(10, j.element_type());
    EXPECT_EQ(vars::L_M3_PER_S, j.demand_constant().unit());
    EXPECT_EQ(vars::L_M3_PER_S, j.demand_delivered().unit());
    EXPECT_EQ(0, j.demand_delivered().value().size());
    ASSERT_EQ(LDEMAND_CONSTANT, j.properties().find("Demand (constant)")->first);
    EXPECT_EQ(vars::L_M3_PER_S, std::get<vars::var_real>(j.properties().at(LDEMAND_CONSTANT)).unit());
    EXPECT_EQ(0.0, std::get<vars::var_real>(j.properties().at(LDEMAND_CONSTANT)).value());
    
    // inherited from node 
    EXPECT_EQ(0.0, j.x_coord());
    EXPECT_EQ(0.0, j.y_coord());
    EXPECT_EQ(0.0, j.elevation());
    EXPECT_EQ(&(j.results().temporal_reals().at("Pressure")), &j.pressure());
    EXPECT_NE(&(j.results().temporal_reals().at("Pressure")), &j.head());
    
    // inherited from element
    EXPECT_EQ(5, j.results().size());
    EXPECT_EQ(0, j.results().integers().size());
    EXPECT_EQ(0, j.results().temporal_integers().size());
    EXPECT_EQ(0, j.results().reals().size());
    EXPECT_EQ(0, j.results().temporal_reals().get("Pressure").size()); // pressure and the other should be empty

}