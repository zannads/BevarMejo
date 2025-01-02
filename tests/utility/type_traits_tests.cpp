#include <gtest/gtest.h>

#include "bevarmejo/utility/type_traits.hpp"

namespace __bevarmejo
{

// Tests for is_vector
TEST(TypeTraitsTests, IsVector)
{
    static_assert(bevarmejo::is_vector<std::vector<int>>::value, "is_vector failed for std::vector<int>");
    static_assert(!bevarmejo::is_vector<int>::value, "is_vector failed for int");
}

// Tests for is_vector_of_numeric
TEST(TypeTraitsTests, IsVectorOfNumeric)
{
    static_assert(bevarmejo::is_vector_of_numeric<std::vector<double>>::value, "is_vector_of_numeric failed for std::vector<double>");
    static_assert(!bevarmejo::is_vector_of_numeric<std::vector<std::string>>::value, "is_vector_of_numeric failed for std::vector<std::string>");
    static_assert(!bevarmejo::is_vector_of_numeric<int>::value, "is_vector_of_numeric failed for int");
}

// Tests for contains_type
TEST(TypeTraitsTests, ContainsType)
{
    using TestTuple = std::tuple<int, double, std::string>;
    static_assert(bevarmejo::contains_type<int, TestTuple>::value, "contains_type failed for int");
    static_assert(bevarmejo::contains_type<double, TestTuple>::value, "contains_type failed for double");
    static_assert(bevarmejo::contains_type<std::string, TestTuple>::value, "contains_type failed for std::string");
    static_assert(!bevarmejo::contains_type<float, TestTuple>::value, "contains_type failed for float");
}

// Tests for index_of_type
TEST(TypeTraitsTests, IndexOfType)
{
    using TestTuple = std::tuple<int, double, std::string>;
    static_assert(bevarmejo::index_of_type<int, TestTuple>::value == 0, "index_of_type failed for int");
    static_assert(bevarmejo::index_of_type<double, TestTuple>::value == 1, "index_of_type failed for double");
    static_assert(bevarmejo::index_of_type<std::string, TestTuple>::value == 2, "index_of_type failed for std::string");
    static_assert(bevarmejo::index_of_type<float, TestTuple>::value == 3, "index_of_type failed for float");
    static_assert(bevarmejo::index_of_type<char, TestTuple>::value >= 3, "index_of_type failed for char");
}

// Tests for EnumClassEqCore and EnumClassEq through make_selector
TEST(TypeTraitsTests, EnumClassEq)
{
    class MockTag
    {
    };

    using TestTuple = std::tuple<int, double, std::string, MockTag>;

    bevarmejo::TagsSelector<TestTuple> selector = bevarmejo::make_selector<TestTuple, MockTag>();
    EXPECT_EQ(selector->value(), 3);
    EXPECT_TRUE(selector->is<MockTag>());
    EXPECT_FALSE(selector->is<int>());
    EXPECT_FALSE(selector->is<double>());
    EXPECT_FALSE(selector->is<std::string>());

    selector = bevarmejo::make_selector<TestTuple, int>();
    EXPECT_EQ(selector->value(), 0);

    // Test copy constructor (should not work because of unique_ptr)
    // auto selector2 = selector;
    
    // Test move constructor (should work)
    auto selector2 = std::move(selector);
    EXPECT_EQ(selector2->value(), 0);
    EXPECT_TRUE(selector2->is<int>());

    // Test clone method instead of copy constructor
    auto selector3 = selector2->clone();
    EXPECT_EQ(selector3->value(), 0);
    EXPECT_TRUE(selector3->is<int>());
    EXPECT_FALSE(selector3->is<double>());

    // Uncomment to test tag non existent in the tuple
    // selector = bevarmejo::make_selector<TestTuple, float>();

}

} // namespace __bevarmejo
