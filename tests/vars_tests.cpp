//
// Description: Tests for the classes contained in the properies/results of the elements.
//
// Notes: You can create a variable<temporal> but the opposite doesn't make much sense.
//
// Created by: Dennis Zanutto on 19/10/23.

#include <gtest/gtest.h>

#include <string>

#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/variable.hpp"

template <typename T>
class CTest {
    public:
        CTest() : _t() {}
        CTest(T t) : _t(t) {}
        T _t;

    bool operator==(const CTest& rhs) const {
        return _t == rhs._t;
    }
};

template <typename T, typename U>
class CTest2 {
    public:
        CTest2() : _t(), _u() {}
        CTest2(T t) : _t(t), _u() {}
        CTest2(T t, U u) : _t(t), _u(u) {}
        T _t;
        U _u;
    
    bool operator==(const CTest2& rhs) const {
        return _t == rhs._t && _u == rhs._u;
    }
};

TEST(TemporalT, Constructors) {
    bevarmejo::wds::vars::temporal<int> t_empty;
    EXPECT_TRUE(t_empty.empty());

    bevarmejo::wds::vars::temporal<int> t_int(42);
    EXPECT_EQ(1, t_int.size());
    EXPECT_EQ(42, t_int[0]); // also if it is not set to 0 this would fail

    bevarmejo::wds::vars::temporal<int> t_int_time(10, 42);
    EXPECT_EQ(1, t_int_time.size() );
    EXPECT_EQ(42, t_int_time.at(10) );

    bevarmejo::wds::vars::temporal<CTest<int>> t_class1(10, 3);
    EXPECT_EQ(1, t_class1.size());
    EXPECT_EQ(CTest<int>(3), t_class1[10]);
    EXPECT_EQ(3, t_class1[10]._t);

    bevarmejo::wds::vars::temporal<CTest<int>> t_class0(10);
    EXPECT_EQ(1, t_class0.size());
    EXPECT_EQ(CTest<int>(), t_class0[10] );
    EXPECT_EQ(0, t_class0[10]._t);

    bevarmejo::wds::vars::temporal<CTest2<double, std::string>> t_class2(
        10, 3, "hello worlds");
    EXPECT_EQ(1, t_class2.size());
    CTest2<double,std::string> m(3.0, "hello worlds");
    EXPECT_EQ(m, t_class2.at(10) );
    EXPECT_EQ(3, t_class2[10]._t);
    EXPECT_EQ("hello worlds", t_class2[10]._u);
}

TEST(TemporalT, Methods) {
    bevarmejo::wds::vars::temporal<int> t(10, 42);
    EXPECT_EQ(42, t.when(10));
    EXPECT_THROW(t.when(20), std::out_of_range);
}

// NEW CLASS UNDER TEST
// variable.hpp

TEST(VariableT, Constructors ) {
    bevarmejo::wds::vars::variable<int> v_empty;
    EXPECT_EQ("", v_empty.unit());
    EXPECT_EQ(0, v_empty.value());

    bevarmejo::wds::vars::variable<int> v_unit("m");
    EXPECT_EQ("m", v_unit.unit());
    EXPECT_EQ(0, v_unit.value());

    bevarmejo::wds::vars::variable<int> v_val(42);
    EXPECT_EQ("", v_val.unit());
    EXPECT_EQ(42, v_val.value());

    bevarmejo::wds::vars::variable<int> v_complete("m", 42);
    EXPECT_EQ("m", v_complete.unit());
    EXPECT_EQ(42, v_complete.value());

    bevarmejo::wds::vars::variable<CTest<int>> v_variadic1("", 10);
    EXPECT_EQ(CTest<int>(10), v_variadic1.value());

    bevarmejo::wds::vars::variable<CTest<int>> v_variadic0;
    EXPECT_EQ(CTest<int>(), v_variadic0.value() );

    bevarmejo::wds::vars::variable<CTest2<double, std::string>> v_variadic2(
        "m", 3, "hello worlds");
    CTest2<double,std::string> m(3.0, "hello worlds");
    EXPECT_EQ(m, v_variadic2.value());
}

TEST(VariableT, Methods) {
    bevarmejo::wds::vars::variable<int> v("m", 42);
    EXPECT_EQ(42, v.value());

    v.value(10);
    EXPECT_EQ(10, v.value());

    EXPECT_EQ(10, v());

    EXPECT_EQ("m", v.unit());

    // no set operator for unit
}

// COMBINATION OF CLASSES UNDER TEST
// variable<temporal>

TEST(VariableTemporalT, All) {
    bevarmejo::wds::vars::variable<bevarmejo::wds::vars::temporal<CTest2<int,std::string>>> var("m", 10, 42, "yeaaah");
    EXPECT_EQ("m", var.unit());
    EXPECT_EQ(1, var.value().size());
    EXPECT_EQ(42, var().when(10)._t);
    EXPECT_EQ("yeaaah", var().when(10)._u);
}