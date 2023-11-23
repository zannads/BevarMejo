#include <gtest/gtest.h>

# include <string>
# include <variant>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/variable.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"

#include "bevarmejo/wds/elements/link.hpp"
#include "bevarmejo/wds/elements/pipe.hpp"

#include "bevarmejo/wds/elements/pattern.hpp"
#include "bevarmejo/wds/elements/demand.hpp"

#include "bevarmejo/wds/elements/junction.hpp"

using namespace bevarmejo::wds;

TEST(JunctionT, Constructor) {
    //Element e; // should not build
    //NetworkElement ne; // should not build
    //NetworkElement ne1("NE1"); // should not build
    //Node n; // should not build
    //Node n1("N1"); // should not build
    //Junction j; // should not build 
    Junction j("J1");
    //--------------------- Inherited from element
    ASSERT_EQ( "J1", j.id() );
    ASSERT_EQ( 0, j.index() );
    EXPECT_EQ( "Junction", j.element_name() );
    EXPECT_EQ( 10, j.element_type());

    // Properties :
    //  J: Demand constant 
    //  N: --
    //  NE: -
    //  E: --

    EXPECT_EQ(1, j.properties().size());
    EXPECT_EQ("Demand (constant)", j.properties().begin()->first);
    EXPECT_EQ(&j.demand_constant(), 
        &std::get<vars::var_real>(j.properties().at(LDEMAND_CONSTANT)));

    EXPECT_EQ("L/s", j.demand_constant().unit());
    EXPECT_EQ(0.0, j.demand_constant().value());

    //--------------------- Inherited from network element
    // Results :
    //  J: Demand requested, Demand delivered, Demand undelivered
    //  N: Head, Pressure
    //  NE: -

    EXPECT_EQ(5, j.results().size());
    EXPECT_NO_THROW( std::get<vars::var_tseries_real>(j.results().at(LDEMAND_REQUESTED)) );
    EXPECT_NO_THROW( std::get<vars::var_tseries_real>(j.results().at(LDEMAND_DELIVERED)) );
    EXPECT_NO_THROW( std::get<vars::var_tseries_real>(j.results().at(LDEMAND_UNDELIVERED)) );
    EXPECT_NO_THROW( std::get<vars::var_tseries_real>(j.results().at(LABEL_HEAD)) );
    EXPECT_NO_THROW( std::get<vars::var_tseries_real>(j.results().at(LABEL_PRESSURE)) );
    EXPECT_THROW( std::get<vars::var_tseries_int>(j.results().at(LDEMAND_REQUESTED)), std::bad_variant_access );
    
    EXPECT_EQ(&j.demand_requested(), 
        &std::get<vars::var_tseries_real>(j.results().at(LDEMAND_REQUESTED)));
    EXPECT_EQ(&j.demand_delivered(),
        &std::get<vars::var_tseries_real>(j.results().at(LDEMAND_DELIVERED)));
    EXPECT_EQ(&j.demand_undelivered(),
        &std::get<vars::var_tseries_real>(j.results().at(LDEMAND_UNDELIVERED)));
    EXPECT_EQ(&j.head(),
        &std::get<vars::var_tseries_real>(j.results().at(LABEL_HEAD)));
    EXPECT_EQ(&j.pressure(),
        &std::get<vars::var_tseries_real>(j.results().at(LABEL_PRESSURE)));

    
    EXPECT_EQ("L/s", j.demand_requested().unit());
    EXPECT_EQ(0, j.demand_requested().value().size());
    EXPECT_EQ("L/s", j.demand_delivered().unit());
    EXPECT_EQ(0, j.demand_delivered().value().size());
    EXPECT_EQ("L/s", j.demand_undelivered().unit());
    EXPECT_EQ(0, j.demand_undelivered().value().size());
    EXPECT_EQ("m", j.head().unit());
    EXPECT_EQ(0, j.head().value().size());

    //--------------------- Inherited from node
    EXPECT_EQ(0.0, j.x_coord());
    EXPECT_EQ(0.0, j.y_coord());
    EXPECT_EQ(0.0, j.elevation());
    EXPECT_EQ(0, j.connected_links().size());

    //--------------------- Specific to junction
    EXPECT_EQ(0, j.demands().size());

}

TEST(JunctionT, Methods) {
    Junction j("J1");

    EXPECT_EQ(false, j.has_demand());

    j.demand_constant().value(10.0);
    EXPECT_EQ(true, j.has_demand());
    j.demand_constant()() = 0.0;

    // To not have it trivial, add a demand (no pattern) and a Link*

    j.add_demand("Domestic", 10.0, nullptr);
    Demand d("Hospital", 20.0, nullptr);
    j.add_demand(d);
    EXPECT_EQ(2, j.demands().size());
    EXPECT_EQ(true, j.has_demand());

    auto& d2 = j.demand("Hospital");
    EXPECT_EQ("Hospital", d2.category());

    EXPECT_THROW( j.demand("School"), std::out_of_range );

    j.remove_demand("Hospital");
    EXPECT_EQ(1, j.demands().size());
    EXPECT_THROW( j.demand("Hospital"), std::out_of_range );
    // stay with one demands

    j.add_link(nullptr);
    EXPECT_EQ(0, j.connected_links().size());

    Pipe p("L1");
    j.add_link(&p);
    EXPECT_EQ(1, j.connected_links().size());
    EXPECT_EQ(&p, *j.connected_links().begin());

    j.remove_link(&p);
    EXPECT_EQ(0, j.connected_links().size());
    j.remove_link(nullptr);
    EXPECT_EQ(0, j.connected_links().size());
    EXPECT_NO_THROW( j.remove_link(&p) );

}

TEST(JunctionT, MethodsEN) {
    // TODO
}

TEST(JunctionT, CopyConstructors) {
    Junction j1("J1");
    j1.index(10);

    Demand d("Hospital", 20.0, nullptr);
    j1.add_demand(d);
    Pipe p("L1");
    j1.add_link(&p);
    j1.x_coord(10.0);
    j1.y_coord(20.0);
    j1.elevation(30.0);


    Junction j2(j1);
    Junction j3 = j1;

    EXPECT_EQ( "J1", j2.id() );
    EXPECT_EQ( "J1", j3.id() );

    EXPECT_EQ( 10, j2.index() );
    EXPECT_EQ( 10, j3.index() );

    EXPECT_NO_THROW( std::get<vars::var_tseries_real>(j2.results().at(LDEMAND_REQUESTED)) );
    EXPECT_NO_THROW( std::get<vars::var_tseries_real>(j2.results().at(LDEMAND_DELIVERED)) );
    EXPECT_NO_THROW( std::get<vars::var_tseries_real>(j2.results().at(LDEMAND_UNDELIVERED)) );
    EXPECT_NO_THROW( std::get<vars::var_tseries_real>(j2.results().at(LABEL_HEAD)) );
    EXPECT_NO_THROW( std::get<vars::var_tseries_real>(j2.results().at(LABEL_PRESSURE)) );
    
    EXPECT_NO_THROW( std::get<vars::var_tseries_real>(j3.results().at(LDEMAND_REQUESTED)) );
    EXPECT_NO_THROW( std::get<vars::var_tseries_real>(j3.results().at(LDEMAND_DELIVERED)) );
    EXPECT_NO_THROW( std::get<vars::var_tseries_real>(j3.results().at(LDEMAND_UNDELIVERED)) );
    EXPECT_NO_THROW( std::get<vars::var_tseries_real>(j3.results().at(LABEL_HEAD)) );
    EXPECT_NO_THROW( std::get<vars::var_tseries_real>(j3.results().at(LABEL_PRESSURE)) );
    
    EXPECT_EQ(1, j2.properties().size());
    EXPECT_NO_THROW( std::get<vars::var_real>(j2.properties().at(LDEMAND_CONSTANT)) );
    EXPECT_EQ(1, j3.properties().size());
    EXPECT_NO_THROW( std::get<vars::var_real>(j3.properties().at(LDEMAND_CONSTANT)) );
    
    EXPECT_EQ(&j2.demand_requested(), 
        &std::get<vars::var_tseries_real>(j2.results().at(LDEMAND_REQUESTED)));
    EXPECT_EQ(&j2.demand_delivered(),
        &std::get<vars::var_tseries_real>(j2.results().at(LDEMAND_DELIVERED)));
    EXPECT_EQ(&j2.demand_undelivered(),
        &std::get<vars::var_tseries_real>(j2.results().at(LDEMAND_UNDELIVERED)));
    EXPECT_EQ(&j2.head(),
        &std::get<vars::var_tseries_real>(j2.results().at(LABEL_HEAD)));
    EXPECT_EQ(&j2.pressure(),
        &std::get<vars::var_tseries_real>(j2.results().at(LABEL_PRESSURE)));

    EXPECT_EQ(&j3.demand_requested(), 
        &std::get<vars::var_tseries_real>(j3.results().at(LDEMAND_REQUESTED)));
    EXPECT_EQ(&j3.demand_delivered(),
        &std::get<vars::var_tseries_real>(j3.results().at(LDEMAND_DELIVERED)));
    EXPECT_EQ(&j3.demand_undelivered(),
        &std::get<vars::var_tseries_real>(j3.results().at(LDEMAND_UNDELIVERED)));
    EXPECT_EQ(&j3.head(),
        &std::get<vars::var_tseries_real>(j3.results().at(LABEL_HEAD)));
    EXPECT_EQ(&j3.pressure(),
        &std::get<vars::var_tseries_real>(j3.results().at(LABEL_PRESSURE)));

    EXPECT_EQ(1, j2.demands().size());
    EXPECT_EQ(true, j2.has_demand());

    EXPECT_EQ(1, j3.demands().size());
    EXPECT_EQ(true, j3.has_demand());

    EXPECT_EQ(1, j2.connected_links().size());
    EXPECT_EQ(1, j3.connected_links().size());
    EXPECT_EQ(&p, *j2.connected_links().begin());
    EXPECT_EQ(*j1.connected_links().begin(), *j3.connected_links().begin());

    EXPECT_EQ(10.0, j2.x_coord());
    EXPECT_EQ(10.0, j3.x_coord());
    EXPECT_EQ(20.0, j2.y_coord());
    EXPECT_EQ(20.0, j3.y_coord());
    EXPECT_EQ(30.0, j2.elevation());
    EXPECT_EQ(30.0, j3.elevation());
}

TEST(JunctionT, MoveConstructors) {
    Junction j1("J1");
    j1.index(10);

    Demand d("Hospital", 20.0, nullptr);
    j1.add_demand(d);
    Pipe p("L1");
    j1.add_link(&p);
    j1.x_coord(10.0);
    j1.y_coord(20.0);
    j1.elevation(30.0);


    Junction j2(j1);
    Junction j3 = j1;

    Junction j4(std::move(j2));
    Junction j5 = std::move(j3);

    EXPECT_EQ( "J1", j4.id() );
    EXPECT_NE( "J1", j2.id() );
    EXPECT_EQ( "J1", j5.id() );
    EXPECT_NE( "J1", j3.id() );

    EXPECT_EQ( 10, j2.index() );
    EXPECT_EQ( 10, j4.index() ); // Question is what do I want to do with old objects properties after the move???
    EXPECT_EQ( 10, j3.index() );
    EXPECT_EQ( 10, j5.index() );

    ASSERT_NO_THROW( std::get<vars::var_tseries_real>(j4.results().at(LDEMAND_REQUESTED)) );
    ASSERT_NO_THROW( std::get<vars::var_tseries_real>(j4.results().at(LDEMAND_DELIVERED)) );
    ASSERT_NO_THROW( std::get<vars::var_tseries_real>(j4.results().at(LDEMAND_UNDELIVERED)) );
    ASSERT_NO_THROW( std::get<vars::var_tseries_real>(j4.results().at(LABEL_HEAD)) );
    ASSERT_NO_THROW( std::get<vars::var_tseries_real>(j4.results().at(LABEL_PRESSURE)) );
    EXPECT_EQ(0, j2.results().size());

    ASSERT_NO_THROW( std::get<vars::var_tseries_real>(j5.results().at(LDEMAND_REQUESTED)) );
    ASSERT_NO_THROW( std::get<vars::var_tseries_real>(j5.results().at(LDEMAND_DELIVERED)) );
    ASSERT_NO_THROW( std::get<vars::var_tseries_real>(j5.results().at(LDEMAND_UNDELIVERED)) );
    ASSERT_NO_THROW( std::get<vars::var_tseries_real>(j5.results().at(LABEL_HEAD)) );
    ASSERT_NO_THROW( std::get<vars::var_tseries_real>(j5.results().at(LABEL_PRESSURE)) );
    EXPECT_EQ(0, j3.results().size());
    
    EXPECT_EQ(1, j4.properties().size());
    ASSERT_NO_THROW( std::get<vars::var_real>(j4.properties().at(LDEMAND_CONSTANT)) );
    EXPECT_EQ(0, j2.properties().size());
    
    EXPECT_EQ(1, j5.properties().size());
    ASSERT_NO_THROW( std::get<vars::var_real>(j5.properties().at(LDEMAND_CONSTANT)) );
    EXPECT_EQ(0, j3.properties().size());
    
    EXPECT_EQ(&j4.demand_requested(), 
        &std::get<vars::var_tseries_real>(j4.results().at(LDEMAND_REQUESTED)));
    EXPECT_EQ(&j4.demand_delivered(),
        &std::get<vars::var_tseries_real>(j4.results().at(LDEMAND_DELIVERED)));
    EXPECT_EQ(&j4.demand_undelivered(),
        &std::get<vars::var_tseries_real>(j4.results().at(LDEMAND_UNDELIVERED)));
    EXPECT_EQ(&j4.head(),
        &std::get<vars::var_tseries_real>(j4.results().at(LABEL_HEAD)));
    EXPECT_EQ(&j4.pressure(),
        &std::get<vars::var_tseries_real>(j4.results().at(LABEL_PRESSURE)));

    EXPECT_EQ(&j5.demand_requested(), 
        &std::get<vars::var_tseries_real>(j5.results().at(LDEMAND_REQUESTED)));
    EXPECT_EQ(&j5.demand_delivered(),
        &std::get<vars::var_tseries_real>(j5.results().at(LDEMAND_DELIVERED)));
    EXPECT_EQ(&j5.demand_undelivered(),
        &std::get<vars::var_tseries_real>(j5.results().at(LDEMAND_UNDELIVERED)));
    EXPECT_EQ(&j5.head(),
        &std::get<vars::var_tseries_real>(j5.results().at(LABEL_HEAD)));
    EXPECT_EQ(&j5.pressure(),
        &std::get<vars::var_tseries_real>(j5.results().at(LABEL_PRESSURE)));

    EXPECT_EQ(1, j4.demands().size());
    EXPECT_EQ(true, j4.has_demand());
    EXPECT_EQ(0, j2.demands().size());
    EXPECT_EQ(false, j2.has_demand());

    EXPECT_EQ(1, j5.demands().size());
    EXPECT_EQ(true, j5.has_demand());
    EXPECT_EQ(0, j3.demands().size());
    EXPECT_EQ(false, j3.has_demand());

    EXPECT_EQ(1, j4.connected_links().size());
    EXPECT_EQ(1, j5.connected_links().size());
    EXPECT_EQ(&p, *j4.connected_links().begin());
    EXPECT_EQ(*j1.connected_links().begin(), *j5.connected_links().begin());

    EXPECT_EQ(10.0, j4.x_coord());
    EXPECT_EQ(10.0, j5.x_coord());
    EXPECT_EQ(20.0, j4.y_coord());
    EXPECT_EQ(20.0, j5.y_coord());
    EXPECT_EQ(30.0, j4.elevation());
    EXPECT_EQ(30.0, j5.elevation());

}