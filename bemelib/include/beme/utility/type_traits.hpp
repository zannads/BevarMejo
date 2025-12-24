#ifndef BEVARMEJOLIB__UTILITY__TYPE_TRAITS_HPP
#define BEVARMEJOLIB__UTILITY__TYPE_TRAITS_HPP

#include <memory>
#include <tuple>
#include <type_traits>
#include <typeinfo>
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

// Convenience alias for contains_type
template <typename T, typename Tuple>
inline constexpr bool contains_type_v = contains_type<T, Tuple>::value;

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
        static constexpr std::size_t value = Index;
    };

    static constexpr std::size_t value = index_of_type_impl<0, Us...>::value;
};

// Convenience alias for index_of_type
template <typename T, typename Tuple>
inline constexpr std::size_t index_of_type_v = index_of_type<T, Tuple>::value;

namespace detail
{

template <typename Tuple>
class EnumClassEqCore
{
/*----------------------------------------------------------------------------*/
/*---------------------------- Member types ----------------------------------*/
/*----------------------------------------------------------------------------*/
protected:
    using self_type = EnumClassEqCore<Tuple>;
    using types = Tuple;

/*----------------------------------------------------------------------------*/
/*---------------------------- Member objects --------------------------------*/
/*----------------------------------------------------------------------------*/
private:

/*----------------------------------------------------------------------------*/
/*--------------------------- Member functions -------------------------------*/
/*----------------------------------------------------------------------------*/
// (constructor)
public:
    EnumClassEqCore() = default;
    EnumClassEqCore(const EnumClassEqCore&) = default;
    EnumClassEqCore(EnumClassEqCore&&) = default;

// (destructor)
public:
    virtual ~EnumClassEqCore() = default;

// operator=
public:
    EnumClassEqCore& operator=(const EnumClassEqCore&) = default;
    EnumClassEqCore& operator=(EnumClassEqCore&&) = default;

// clone
public:
    std::unique_ptr<self_type> clone() const 
    {
        return std::unique_ptr<self_type>(this->__clone());
    }
private:
    virtual self_type* __clone() const = 0;

// Templated "is" method to check the type
public:
    template <typename T>
    bool is() const
    {
        static_assert(bevarmejo::contains_type_v<T, types>, "This type is not a valid type for this selector.");
        return __is(bevarmejo::index_of_type_v<T, types>);
    }

// Pure virtual method to get the value
public:
    // Get the object value
    virtual std::size_t value() const = 0;

// Hidden virtual method for the "is" method
protected:
    virtual bool __is(std::size_t a_value) const = 0;

}; // class EnumClassEqCore

template <typename Tuple, typename Tag>
class EnumClassEq final : public detail::EnumClassEqCore<Tuple>
{
/*----------------------------------------------------------------------------*/
/*---------------------------- Member types ----------------------------------*/
/*----------------------------------------------------------------------------*/
private:
    using self_type = EnumClassEq<Tuple, Tag>;
    using base_type = detail::EnumClassEqCore<Tuple>;
    using types = Tuple;

/*----------------------------------------------------------------------------*/
/*---------------------------- Member objects --------------------------------*/
/*----------------------------------------------------------------------------*/
private:

/*----------------------------------------------------------------------------*/
/*--------------------------- Member functions -------------------------------*/
/*----------------------------------------------------------------------------*/
// (constructor)
public:
    EnumClassEq()
    {
        static_assert(bevarmejo::contains_type_v<Tag, types>, "Tag is not allowed because it is not in the tuple.");
    }
    EnumClassEq(const EnumClassEq&) = default;
    EnumClassEq(EnumClassEq&&) = default;

// (destructor)
public:
    ~EnumClassEq() override = default;

// operator=
public:
    EnumClassEq& operator=(const EnumClassEq&) = default;
    EnumClassEq& operator=(EnumClassEq&&) = default;

// clone
public:
    // No need to override the base clone method 

private:
    self_type* __clone() const override
    {
        return new self_type(*this);
    }

// Hidden virtual method for the "is" method
protected:
    bool __is(std::size_t a_value) const override
    {
        return this->value() == a_value;
    }

// Pure virtual method to get the value
public:
    std::size_t value() const override
    {
        return bevarmejo::index_of_type<Tag, Tuple>::value;
    }

}; // class EnumClassEq

} // namespace detail

template <typename Tuple>
using TagsSelector = std::unique_ptr<detail::EnumClassEqCore<Tuple>>;

template <typename Tuple, typename Tag>
TagsSelector<Tuple> make_selector()
{
    return std::make_unique<detail::EnumClassEq<Tuple, Tag>>();
}

} // namespace bevarmejo

#endif // BEVARMEJOLIB__UTILITY__TYPE_TRAITS_HPP
