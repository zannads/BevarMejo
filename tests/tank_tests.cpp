#include <gtest/gtest.h>

#include "bevarmejo/wds/elements/tank.hpp"
#include "bevarmejo/wds/elements/source.hpp"
#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/element.hpp"

#include "bevarmejo/wds/elements/results.hpp"
#include "bevarmejo/wds/elements/variables.hpp"
#include "bevarmejo/wds/elements/variable.hpp"
#include "bevarmejo/wds/elements/temporal.hpp"

using namespace bevarmejo::wds;

TEST(TankTests, Constructor) {
    
    tank j("T1");
    ASSERT_EQ("T1", j.id());
    EXPECT_EQ("Tank", j.element_name());
    EXPECT_EQ(12, j.element_type());
   
    // inherited from source
    EXPECT_EQ(vars::L_M3_PER_S, j.inflow().unit());
    EXPECT_EQ(0.0, j.inflow().value().size());
    EXPECT_EQ(vars::L_METER, j.source_elevation().unit());
    EXPECT_EQ(0.0, j.source_elevation()().size());
    EXPECT_EQ(&(j.results().temporal_reals().at("Inflow")), &j.inflow());
    EXPECT_EQ(&(j.results().temporal_reals().at("Source Elevation")), &j.source_elevation());

    // inherited from node 
    EXPECT_EQ(0.0, j.x_coord());
    EXPECT_EQ(0.0, j.y_coord());
    EXPECT_EQ(0.0, j.elevation());
    EXPECT_EQ(&(j.results().temporal_reals().at("Pressure")), &j.pressure());
    EXPECT_NE(&(j.results().temporal_reals().at("Pressure")), &j.head());
    
    // inherited from element
    EXPECT_EQ(4, j.results().size());
    EXPECT_EQ(0, j.results().integers().size());
    EXPECT_EQ(0, j.results().temporal_integers().size());
    EXPECT_EQ(0, j.results().reals().size());
    EXPECT_EQ(0, j.results().temporal_reals().get("Pressure").size()); // pressure and the other should be empty

}