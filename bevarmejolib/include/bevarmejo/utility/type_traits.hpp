#ifndef BEVARMEJOLIB__TYPE_TRAITS_HPP
#define BEVARMEJOLIB__TYPE_TRAITS_HPP

#include <tuple>
#include <type_traits>
#include <vector>

namespace bevarmejo
{
// Type traits to check vector of numeric objects
template <typename T>
struct is_vector : std::false_type {};

template <typename T, typename Alloc>
struct is_vector<std::vector<T, Alloc>> : std::true_type {};

template <typename T>
struct is_vector_of_numeric : std::false_type {};

template <typename T, typename Alloc>
struct is_vector_of_numeric<std::vector<T, Alloc>> : std::is_arithmetic<T> {};

// Helper variable template
template <typename T>
inline constexpr bool is_vector_of_numeric_v = is_vector_of_numeric<T>::value;

// Helpers to check if a tuple has a specific type
template <typename T, typename Tuple>
struct contains_type;

template <typename T, typename... Us>
struct contains_type<T, std::tuple<Us...>> : std::disjunction<std::is_same<T, Us>...> {};

// Helper to get the index of a type in a tuple
template <typename T, typename Tuple>
struct index_of_type;

template <typename T, typename... Us>
struct index_of_type<T, std::tuple<Us...>>
{
    // Helper to recursively get the index
    template <std::size_t Index, typename... Remaining>
    struct index_of_type_impl;

    template <std::size_t Index, typename First, typename... Remaining>
    struct index_of_type_impl<Index, First, Remaining...>
    {
        static constexpr std::size_t value = std::is_same_v<T, First> ? Index : index_of_type_impl<Index + 1, Remaining...>::value;
    };

    template <std::size_t Index>
    struct index_of_type_impl<Index>
    {
        static_assert(std::true_type::value, "Type not found in the tuple.");
    };

    static constexpr std::size_t value = index_of_type_impl<0, Us...>::value;
};

namespace detail
{

template <typename Tuple>
class EnumClassEqCore
{
public:
    virtual ~EnumClassEqCore() = default;

// Templated "is" method
public:
    template <typename T>
    bool is() const
    {
        static_assert(bevarmejo::contains_type<T, Tuple>, "Type is not allowed because it is not in the tuple.");
        return __is(typeid(T));
    }

// Pure virtual method to get the value
public:
    // Get the object value
    virtual std::size_t value() const = 0;

// Hidden virtual method for the "is" method
protected:
    virtual bool __is(const std::type_info& type) const = 0;

}; // class EnumClassEqCore

} // namespace detail

template <typename Tuple, typename Tag>
class EnumClassEq final : public detail::EnumClassEqCore<Tuple>
{
protected:
    bool __is(const std::type_info& type) const override
    {
        return typeid(Tag) == type;
    }

public:
    std::size_t value() const override
    {
        return bevarmejo::index_of_type<Tag, Tuple>::value;
    }

}; // class EnumClassEq

template <typename Tuple>
using TagsSelector = std::unique_ptr<detail::EnumClassEqCore<Tuple>>;

template <typename Tuple, typename Tag>
TagsSelector<Tuple> make_selector()
{
    return std::make_unique<EnumClassEq<Tuple, Tag>>();
}

} // namespace bevarmejo

#endif // BEVARMEJOLIB__TYPE_TRAITS_HPP
