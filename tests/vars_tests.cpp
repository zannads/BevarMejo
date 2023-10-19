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
#include "bevarmejo/wds/elements/variables.hpp"

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

TEST(TemporalTest, DefaultConstructor) {
    bevarmejo::wds::vars::temporal<int> t;
    EXPECT_TRUE(t.empty());
}

TEST(TemporalTest, ValueConstructor) {
    bevarmejo::wds::vars::temporal<int> t(42);
    EXPECT_EQ(1, t.size());
    EXPECT_EQ(42, t[0]);
}

TEST(TemporalTest, TimeValueConstructor) {
    bevarmejo::wds::vars::temporal<int> t(10, 42);
    EXPECT_EQ(1, t.size());
    EXPECT_EQ(42, t[10]);
}

TEST(TemporalTest, TimeValueConstructorWithParams) {
    bevarmejo::wds::vars::temporal<CTest<int>> t(10, 3);
    EXPECT_EQ(1, t.size());
    EXPECT_EQ(CTest<int>(3), t[10]);
    EXPECT_EQ(3, t[10]._t);
}

TEST(TemporalTest, TimeValueConstructorWithNoParams) {
    bevarmejo::wds::vars::temporal<CTest<int>> t(10);
    EXPECT_EQ(1, t.size());
    EXPECT_EQ(CTest<int>(), t[10] );
    EXPECT_EQ(0, t[10]._t);
}

TEST(TemporalTest, TimeValueConstructorWithMultiParams) {
    bevarmejo::wds::vars::temporal<CTest2<double, std::string>> t(10, 3, "hello worlds");
    EXPECT_EQ(1, t.size());
    CTest2<double,std::string> m(3.0, "hello worlds");
    EXPECT_EQ(m, t[10]);
    EXPECT_EQ(3, t[10]._t);
    EXPECT_EQ("hello worlds", t[10]._u);
}

TEST(TemporalTest, WhenFunction) {
    bevarmejo::wds::vars::temporal<int> t(10, 42);
    EXPECT_EQ(42, t.when(10));
}

TEST(TemporalTest, WhenFunctionThrows) {
    bevarmejo::wds::vars::temporal<int> t(10, 42);
    EXPECT_THROW(t.when(20), std::out_of_range);
}

// NEW CLASS UNDER TEST
// variable.hpp

TEST(VariableTest, Constructor_Default) {
    bevarmejo::wds::vars::variable<int> v;
    EXPECT_EQ("", v.unit());
    EXPECT_EQ(0, v.value());
}

TEST(VariableTest, Constructor_Unit) {
    bevarmejo::wds::vars::variable<int> v("m");
    EXPECT_EQ("m", v.unit());
    EXPECT_EQ(0, v.value());
}

TEST(VariableTest, Constructor_Value) {
    bevarmejo::wds::vars::variable<int> v(42);
    EXPECT_EQ("", v.unit());
    EXPECT_EQ(42, v.value());
}

TEST(VariableTest, Constructor_UnitValue) {
    bevarmejo::wds::vars::variable<int> v("m", 42);
    EXPECT_EQ("m", v.unit());
    EXPECT_EQ(42, v.value());
}

TEST(TemporalTest, UnitValueConstructorWithParams) {
    bevarmejo::wds::vars::variable<CTest<int>> v("", 10);
    EXPECT_EQ(CTest<int>(10), v.value());
}

TEST(TemporalTest, UnitValueConstructorWithNoParams) {
    bevarmejo::wds::vars::variable<CTest<int>> v;
    EXPECT_EQ(CTest<int>(), v.value() );
}

TEST(TemporalTest, UnitValueConstructorWithMultiParams) {
    bevarmejo::wds::vars::variable<CTest2<double, std::string>> v("m", 3, "hello worlds");
    CTest2<double,std::string> m(3.0, "hello worlds");
    EXPECT_EQ(m, v.value());
}

TEST(VariableTest, Function_Value) {
    bevarmejo::wds::vars::variable<int> v("m", 42);
    EXPECT_EQ(42, v.value());
}

TEST(VariableTest, Function_Assignment) {
    bevarmejo::wds::vars::variable<int> v("m", 42);
    v.value(10);
    EXPECT_EQ(10, v.value());
}

TEST(VariableTest, Operator_Parentheses) {
    bevarmejo::wds::vars::variable<int> v("m", 42);
    EXPECT_EQ(42, v());
}

// NEW CLASS UNDER TEST
// variables.hpp

TEST(VariablesTest, DefaultConstructor) {
    bevarmejo::wds::vars::variables<int> vars;
    EXPECT_EQ(0, vars.size());
}

TEST(VariablesTest, NameValueConstructor) {
    bevarmejo::wds::vars::variables<int> vars("x", 42);
    EXPECT_EQ(1, vars.size());
    EXPECT_EQ(42, vars["x"]);
}

TEST(VariablesTest, NameValueConstructorWithParams) {
    bevarmejo::wds::vars::variables<CTest<int>> vars("x", 3);
    EXPECT_EQ(1, vars.size());
    EXPECT_EQ(CTest<int>(3), vars["x"]);
}

TEST(VariableTest, NameValueConstructorWithNoParams) {
    bevarmejo::wds::vars::variables<CTest<int>> vars("x");
    EXPECT_EQ(1, vars.size());
    EXPECT_EQ(CTest<int>(), vars["x"]);
}

TEST(VariableTest, NameValueConstructorWithMultiParams) {
    bevarmejo::wds::vars::variables<CTest2<double, std::string>> vars("x", 3, "hello worlds");
    EXPECT_EQ(1, vars.size());
    CTest2<double,std::string> m(3.0, "hello worlds");
    EXPECT_EQ(m, vars["x"]);
}

TEST(VariablesTest, GetFunction) {
    bevarmejo::wds::vars::variables<int> vars("x", 42);
    EXPECT_EQ(42, vars.at("x"));
}

TEST(VariablesTest, GetFunctionThrows) {
    bevarmejo::wds::vars::variables<int> vars("x", 42);
    EXPECT_THROW(vars.at("y"), std::out_of_range);
}

// COMBINATION OF CLASSES UNDER TEST
// variable<temporal>
// variables<variable>
// variables<variable<temporal>>

TEST(VarsCombinedTest, FinalCombinedConstructor) {
    bevarmejo::wds::vars::variables<bevarmejo::wds::vars::variable<bevarmejo::wds::vars::temporal<CTest2<int,std::string>>>> vars("PRES", "m", 10, 42, "yeaaah");
    EXPECT_EQ(1, vars.size());
    EXPECT_EQ("m", vars["PRES"].unit());
    EXPECT_EQ(1, vars["PRES"].value().size());
    EXPECT_EQ(42, vars["PRES"]().when(10)._t);
    EXPECT_EQ("yeaaah", vars["PRES"]().when(10)._u);
    EXPECT_EQ(vars.get("PRES").when(10)._t, vars["PRES"]().when(10)._t);
}