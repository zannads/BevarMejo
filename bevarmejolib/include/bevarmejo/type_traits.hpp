#ifndef BEVARMEJOLIB__TYPE_TRAITS_HPP
#define BEVARMEJOLIB__TYPE_TRAITS_HPP

#include <type_traits>
#include <vector>

namespace bevarmejo {

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

} // namespace bevarmejo

#endif // BEVARMEJOLIB__TYPE_TRAITS_HPP
