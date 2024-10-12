#include <gtest/gtest.h>

#include "bevarmejo/library_metadata.hpp"

namespace __bevarmejo::detail {

// Assuming version_year, version_month, and version_release are globally defined constants.
// Replace them with appropriate values for testing.
constexpr unsigned int version_year = bevarmejo::detail::version_year;
constexpr unsigned int version_month = bevarmejo::detail::version_month;
constexpr unsigned int version_release = bevarmejo::detail::version_release;

// Test default constructor
TEST(VersionTest, DefaultConstructor) {
    bevarmejo::detail::Version default_version;
    EXPECT_EQ(default_version, bevarmejo::detail::Version(version_year, version_month, version_release));
}

// Test parameterized constructor with valid inputs
TEST(VersionTest, ParameterizedConstructorValid) {
    bevarmejo::detail::Version v1(2023, 6, 0);
    EXPECT_EQ(v1.str(), "v23.06.0");

    bevarmejo::detail::Version v2(2024, 10, 1);
    EXPECT_EQ(v2.str(), "v24.10.1");

    bevarmejo::detail::Version v3(2024, 6, 50);
    EXPECT_EQ(v3.str(), "v24.06.50");
}

// Test parameterized constructor with invalid dates (year, month, release)
// A date is invalid if:
// 1. it is out of project versions range (before 2023.06.0 or after the current version)
// 2. the parameters are naturally out of range (1 <= month <= 12, 0 <= release <= 99)
TEST(VersionTest, ParameterizedConstructorInvalidDates) {
    EXPECT_THROW(bevarmejo::detail::Version(2022, 6, 0), std::invalid_argument);
    EXPECT_THROW(bevarmejo::detail::Version(version_year+1, 6, 0), std::invalid_argument);

    EXPECT_THROW(bevarmejo::detail::Version(2023, 5, 0), std::invalid_argument);
    EXPECT_THROW(bevarmejo::detail::Version(2023, 0, 0), std::invalid_argument);
    EXPECT_THROW(bevarmejo::detail::Version(2023, 13, 0), std::invalid_argument);
    EXPECT_THROW(bevarmejo::detail::Version(version_year, version_month+1, 0), std::invalid_argument);

    EXPECT_THROW(bevarmejo::detail::Version(2023, 6, 100), std::invalid_argument);
    EXPECT_THROW(bevarmejo::detail::Version(version_year, version_month, version_release+1), std::invalid_argument);
}

// Test comparison operators
TEST(VersionTest, ComparisonOperators) {
    bevarmejo::detail::Version v1(2023, 6, 0);
    bevarmejo::detail::Version v2(2023, 6, 1);
    bevarmejo::detail::Version v3(2023, 7, 0);
    bevarmejo::detail::Version v4(2024, 6, 0);

    EXPECT_TRUE(v1 < v2);
    EXPECT_TRUE(v2 < v3);
    EXPECT_TRUE(v3 < v4);
    EXPECT_TRUE(v1 < v3);
    EXPECT_TRUE(v1 < v4);
    EXPECT_TRUE(v2 < v4);

    EXPECT_TRUE(v4 > v3);
    EXPECT_TRUE(v4 > v2);
    EXPECT_TRUE(v4 > v1);
    EXPECT_TRUE(v3 > v2);
    EXPECT_TRUE(v3 > v1);
    EXPECT_TRUE(v2 > v1);

    EXPECT_TRUE(v1 <= v2);
    EXPECT_TRUE(v1 <= v3);
    EXPECT_TRUE(v1 <= v4);
    EXPECT_TRUE(v2 <= v3);
    EXPECT_TRUE(v2 <= v4);
    EXPECT_TRUE(v3 <= v4);
    
    EXPECT_TRUE(v4 >= v3);
    EXPECT_TRUE(v4 >= v2);
    EXPECT_TRUE(v4 >= v1);
    EXPECT_TRUE(v3 >= v2);
    EXPECT_TRUE(v3 >= v1);
    EXPECT_TRUE(v2 >= v1);
    
    EXPECT_TRUE(v1 != v2);
    EXPECT_TRUE(v1 != v3);
    EXPECT_TRUE(v1 != v4);
    EXPECT_TRUE(v2 != v3);
    EXPECT_TRUE(v2 != v4);
    EXPECT_TRUE(v3 != v4);

    EXPECT_FALSE(v1 == v2);
    EXPECT_FALSE(v1 == v3);
    EXPECT_FALSE(v1 == v4);
    EXPECT_FALSE(v2 == v3);
    EXPECT_FALSE(v2 == v4);
    EXPECT_FALSE(v3 == v4);

    EXPECT_EQ(v1, v1);
    EXPECT_EQ(v2, v2);
    EXPECT_EQ(v3, v3);
    EXPECT_EQ(v4, v4);
}

// Test parsing from string
TEST(VersionTest, ParseFromString) {
    bevarmejo::detail::Version v = bevarmejo::detail::Version::parse("v23.06.0");
    EXPECT_EQ(v, bevarmejo::detail::Version(2023, 6, 0));
    EXPECT_EQ(v.str(), "v23.06.0");

    v = bevarmejo::detail::Version::parse("v24.1.50");
    EXPECT_EQ(v, bevarmejo::detail::Version(2024, 1, 50));
    EXPECT_EQ(v.str(), "v24.01.50");

    v = bevarmejo::detail::Version::parse("v24.10.1");
    EXPECT_EQ(v, bevarmejo::detail::Version(2024, 10, 1));
    EXPECT_EQ(v.str(), "v24.10.1");
}

// Test parsing from invalid string 
// Invalid strings are those that do not follow the format "vYY.MM.r"
TEST(VersionTest, ParseFromStringInvalid) {
    EXPECT_THROW(bevarmejo::detail::Version::parse("23.06.0"), std::invalid_argument);
    EXPECT_THROW(bevarmejo::detail::Version::parse("v23.06"), std::invalid_argument);

    EXPECT_NO_THROW(bevarmejo::detail::Version::parse("v23.06.0.1"));
    EXPECT_NO_THROW(bevarmejo::detail::Version::parse("v23.6.0"));
    EXPECT_NO_THROW(bevarmejo::detail::Version::parse("blablav23.06.0"));
}

// Test comparison with string versions
TEST(VersionTest, ComparisonWithString) {
    bevarmejo::detail::Version v(2023, 6, 0);

    EXPECT_TRUE(v < "v24.10.1");
    EXPECT_TRUE(v <= "v24.10.1");
    EXPECT_TRUE(v != "v24.10.1");
    EXPECT_FALSE(v == "v24.10.1");
    EXPECT_TRUE(v == "v23.06.0");

    EXPECT_TRUE(v > "v23.05.0");
    EXPECT_TRUE(v >= "v23.05.0");
    EXPECT_FALSE(v < "v23.05.0");
    EXPECT_FALSE(v <= "v23.05.0");
}

// Test static numeric function
TEST(VersionTest, StaticNumericFunction) {
    unsigned int numeric_value = bevarmejo::detail::Version::numeric(2023, 6, 0);
    EXPECT_EQ(numeric_value, 230600);

    numeric_value = bevarmejo::detail::Version::numeric(2024, 10, 1);
    EXPECT_EQ(numeric_value, 241001);
}

// Test static string conversion function
TEST(VersionTest, StaticStringFunction) {
    std::string version_str = bevarmejo::detail::Version::str(2023, 6, 0);
    EXPECT_EQ(version_str, "v23.06.0");

    version_str = bevarmejo::detail::Version::str(2024, 10, 1);
    EXPECT_EQ(version_str, "v24.10.1");
}

} // namespace __bevarmejo::detail
