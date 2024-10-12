#include <gtest/gtest.h>

#include "bevarmejo/library_metadata.hpp"

namespace __bevarmejo::detail {

using VM = bevarmejo::VersionManager;

class VersionManagerTest : public ::testing::Test 
{
protected:
    unsigned int v_year{0ul};
    unsigned int v_month{0ul};
    unsigned int v_release{0ul};

    void SetUp() override 
    {
        // Make sure, user and library version are created.
        VM::user();
        VM::library();

        // Set the version year, month and release.
        v_year = VM::library().version().year();
        v_month = VM::library().version().month();
        v_release = VM::library().version().release();
    }
}; // class VersionManagerTest

TEST_F(VersionManagerTest, DefaultConstructors) 
{
    // By default, the user and library version should be the same, and if I 
    // request the default version, it should be the same as the library version.
    EXPECT_EQ(VM::v(), VM::library().version());
    EXPECT_EQ(VM::v(), VM::user().version());
}

class VersionManagerConstructorParametrizedTest : 
    public VersionManagerTest, public ::testing::WithParamInterface<std::tuple<bool, std::initializer_list<unsigned int>, unsigned int, std::string>>
{ 
    void SetUp() override 
    {
        VersionManagerTest::SetUp();
    }
}; // class VersionManagerConstructorParametrizedTest

TEST_P(VersionManagerConstructorParametrizedTest, VersionParametrizedConstructors)
{
    // Try to create contrusctors with invalid values. 
    // Let's test initializer list, string and numeric.
    const auto& [should_succeed, il, n, s] = GetParam();
    
    if (should_succeed)
    {
        ASSERT_NO_THROW(VM::v(il));
        ASSERT_EQ(VM::v(il).year(), il.begin()[0]);
        ASSERT_EQ(VM::v(il).month(), il.begin()[1]);
        ASSERT_EQ(VM::v(il).release(), il.begin()[2]);

        EXPECT_NO_THROW(VM::v(n));
        EXPECT_EQ(VM::v(n).year(), il.begin()[0]);
        EXPECT_EQ(VM::v(n).month(), il.begin()[1]);
        EXPECT_EQ(VM::v(n).release(), il.begin()[2]);

        EXPECT_NO_THROW(VM::v(s));
        EXPECT_EQ(VM::v(s).year(), il.begin()[0]);
        EXPECT_EQ(VM::v(s).month(), il.begin()[1]);
        EXPECT_EQ(VM::v(s).release(), il.begin()[2]);
    }
    else
    {
        EXPECT_THROW(VM::v(il), std::invalid_argument);
        EXPECT_THROW(VM::v(n), std::invalid_argument);
        EXPECT_THROW(VM::v(s), std::invalid_argument);
    }
}

// Invalid version have:
//  - a year before 2000 or after the current year.
//  - an invalid month, i.e., not between 1 and 12, or if the year is the current year, not between 1 and the current month.
//  - an invalid release, i.e., not between 0 and 99, or if the year and month are the current year and month, not between 0 and the current release.
// When parsed from string, they must be in the format "vYY.m.r", 
// where YY is the year, m is the month and r is the release.
INSTANTIATE_TEST_SUITE_P(
    VersionManagerConstructorParametrizedTestCases,
    VersionManagerConstructorParametrizedTest,
    ::testing::Values(
        // Valid versions.
        std::make_tuple(true, {2023, 6, 0}, 230600, std::string("v23.06.0")),
        std::make_tuple(true, {2023, 6, 30}, 230630, std::string("v23.06.30")),
        std::make_tuple(true, {2023, 10, 4}, 231004, std::string("v23.10.04")),
        std::make_tuple(true, {2024, 1, 12}, 240112, std::string("v24.01.12")),

        // Invalid year.
        std::make_tuple(false, {1999, 6, 0}, 990600, std::string("v99.06.0")),
        std::make_tuple(false, {v_year+1, v_month, v_release}, VM::library().version().numeric()+10000, 
                    std::string("v")+std::to_string(v_year+1)+"."+std::to_string(v_month)+"."+std::to_string(v_release)),

        // Invalid month.
        std::make_tuple(false, {2023, 0, 0}, 230000, std::string("v23.00.0")),
        std::make_tuple(false, {2023, 13, 0}, 231300, std::string("v23.13.0")),
        std::make_tuple(false, {v_year, v_month+1, v_release}, VM::library().version().numeric()+100, 
                    std::string("v")+std::to_string(v_year)+"."+std::to_string(v_month+1)+"."+std::to_string(v_release)),

        // Invalid release.
        std::make_tuple(false, {2023, 6, 100}, 231600, std::string("v23.06.100")),
        std::make_tuple(false, {v_year, v_month, v_release+1}, VM::library().version().numeric()+1, 
                    std::string("v")+std::to_string(v_year)+"."+std::to_string(v_month)+"."+std::to_string(v_release+1))
    )
);

// Test the conversion to numeric and string.
class VMAccessParTest : 
    public VersionManagerTest, public ::testing::WithParamInterface<std::tuple<std::initializer_list<unsigned int>, unsigned int, std::string>>
{ 
    void SetUp() override 
    {
        VersionManagerTest::SetUp();
    }
}; // class VMAccessParTest
TEST_P(VMAccessParTest, VersionAccess)
{
    const auto& [il, n, s] = GetParam();
    const auto v = VM::v(il);

    EXPECT_EQ(v.numeric(), n);
    EXPECT_EQ(v.str(), s);
}
INSTANTIATE_TEST_SUITE_P(
    VMAccessParTestCases,
    VMAccessParTest,
    ::testing::Values(
        std::make_tuple(std::initializer_list<unsigned int>{2023, 6, 0}, 230600, std::string("v23.06.0")),
        std::make_tuple(std::initializer_list<unsigned int>{2023, 6, 30}, 230630, std::string("v23.06.30")),
        std::make_tuple(std::initializer_list<unsigned int>{2023, 10, 4}, 231004, std::string("v23.10.04")),
        std::make_tuple(std::initializer_list<unsigned int>{2024, 1, 12}, 240112, std::string("v24.01.12")),
        std::make_tuple(std::initializer_list<unsigned int>{2023, 10, 45}, 231045, std::string("v23.10.45")), 
    )
);

// Test the comparison operators.
// I have to try the 27 combination of year less, equal and greater, then month and release.
// test by version, numeric and string.
TEST_F(VersionManagerTest, VersionComparison)
{
    const auto v = VM::v({2023, 6, 50});
}

// Test parse of the numeric 

// Test parse of the string

// Test the set of the user version.

} // namespace __bevarmejo::detail