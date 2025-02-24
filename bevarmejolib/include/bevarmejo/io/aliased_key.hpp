#pragma once

#include <array>
#include <cctype>
#include <tuple>
#include <utility>

#include "bevarmejo/io/json.hpp"

#include "bevarmejo/utility/exceptions.hpp"
#include "bevarmejo/utility/string.hpp"

namespace bevarmejo::io
{
/* The AliasedKey object is meant to be a static const global object and defined
 * in the various translation units where they are needed.
 * It is basically a collection of alternative ways in which the user could write
 * a key in a configuration file.
 * The user can call a key in the configuration file in one of these alternative
 * ways.
 * The programmer writes the keys in Sentence case and with a compilation flag
 * (defined in the CMake) the key is transformed in the desired style.
 * The Key is saved in Sentence case and using the magic of constexpr functions
 * the key is transformed in the desired style at compile time.
*/

namespace detail
{
// The output style of the AliasedKeys.
constexpr bevarmejo::detail::text_case out_style_t = 
#if defined(OUT_STYLE_0)
        bevarmejo::detail::text_case::SentenceCase;
#elif defined(OUT_STYLE_1)
        bevarmejo::detail::text_case::CamelCase;
#elif defined(OUT_STYLE_2)
        bevarmejo::detail::text_case::KebabCase;
#elif defined(OUT_STYLE_3)
        bevarmejo::detail::text_case::PascalCase;
#elif defined(OUT_STYLE_4)
        bevarmejo::detail::text_case::SnakeCase;
#else
    #error "The output style selected is invalid."
#endif

// Helper function to create the array of all possible cases for a Key.
template <std::size_t N>
inline constexpr auto make_text_cases(bevarmejo::detail::ConstexprString<N> key_Sc) -> std::array<bevarmejo::detail::ConstexprString<N>, bevarmejo::detail::n_text_cases>
{
    // The cases are written in the following order to minimze search time:
    // 1. The output style.
    // 2. The rest of the styles in order.
    std::array<bevarmejo::detail::ConstexprString<N>, bevarmejo::detail::n_text_cases> cases;

    cases[0] = bevarmejo::detail::sentence_case_to<bevarmejo::io::detail::out_style_t>(key_Sc);

    for (std::size_t i = 1, c=0; c < bevarmejo::detail::n_text_cases; ++c)
    {
        auto tc = static_cast<bevarmejo::detail::text_case>(c);
        if (tc == bevarmejo::io::detail::out_style_t)
        {
            continue;
        }
        else if (tc == bevarmejo::detail::text_case::SentenceCase)
        {
            cases[i] = key_Sc;
            ++i;
        }
        else if (tc == bevarmejo::detail::text_case::CamelCase)
        {
            cases[i] = bevarmejo::detail::sentence_case_to_camel_case(key_Sc);
            ++i;
        }
        else if (tc == bevarmejo::detail::text_case::KebabCase)
        {
            cases[i] = bevarmejo::detail::sentence_case_to_kebab_case(key_Sc);
            ++i;
        }
        else if (tc == bevarmejo::detail::text_case::PascalCase)
        {
            cases[i] = bevarmejo::detail::sentence_case_to_pascal_case(key_Sc);
            ++i;
        }
        else // if (tc == bevarmejo::detail::text_case::SnakeCase)
        {
            cases[i] = bevarmejo::detail::sentence_case_to_snake_case(key_Sc);
            ++i;
        }
    }
    
    return cases;
}
    
} // namespace detail


template <std::size_t... Ns>
class AliasedKey final
{
/*----------------------------------------------------------------------------*/
/*---------------------------- Member types ----------------------------------*/
/*----------------------------------------------------------------------------*/
public:
    using styles = bevarmejo::detail::text_case;
    
    // The output style of the AliasedKeys.
    static constexpr bevarmejo::detail::text_case out_style_t = bevarmejo::io::detail::out_style_t;
private:
    // The output style of the AliasedKeys as a size_t.
    static constexpr std::size_t out_style_v = static_cast<std::size_t>(out_style_t);
public:
    // Count how many alternatives:
    static constexpr std::size_t n_alternatives = sizeof...(Ns);
private:
    // Count the size of the elements:
    static constexpr std::size_t size = n_alternatives * bevarmejo::detail::n_text_cases;

private:
    // We store the values in a tuple of arrays with 5 Constexpr Strings.
    // Each array contains the ConstexprString in all the possible styles.
    using data_type = std::tuple<
        std::array<bevarmejo::detail::ConstexprString<Ns>, bevarmejo::detail::n_text_cases>...>;

/*----------------------------------------------------------------------------*/
/*---------------------------- Member objects --------------------------------*/
/*----------------------------------------------------------------------------*/
private:
    data_type m__values;

/*----------------------------------------------------------------------------*/
/*--------------------------- Member functions -------------------------------*/
/*----------------------------------------------------------------------------*/
// Helper functions ------------------------------------------------------------
private:

// (constructor)
public:
    AliasedKey() = delete;
    constexpr AliasedKey(AliasedKey&&) = default;
    constexpr AliasedKey(const AliasedKey&) = default;
    // Single char array constructor
    template <std::size_t N>
    constexpr AliasedKey(const char (&key)[N]) :
        m__values{bevarmejo::io::detail::make_text_cases(bevarmejo::detail::ConstexprString<N>(key))}
    { }

    // Two char array constructor
    template <typename std::enable_if_t<(sizeof...(Ns) > 0), int> = 0, std::size_t N, std::size_t... Ms>
    constexpr AliasedKey(const char (&first)[N], const char (&... rest)[Ms]) :
        m__values{bevarmejo::io::detail::make_text_cases(bevarmejo::detail::ConstexprString<N>(first)),
                  bevarmejo::io::detail::make_text_cases(bevarmejo::detail::ConstexprString<Ms>(rest))...}
    { }

    // I don't need more than two because no key has more than two alternatives for now.
    // This saves us the complexity of implementing a variadiac constexpr constructor.

// (destructor)
public:
    ~AliasedKey() = default;

// operator=
public:
    constexpr AliasedKey& operator=(AliasedKey&&) = default;
    constexpr AliasedKey& operator=(const AliasedKey&) = default;

// Element access --------------------------------------------------------------
public:
    // Main value of the AliasedKey in the DEFAULT output style.
    constexpr const char* operator()() const
    {
        return std::get<0>(m__values)[out_style_v].c_str();
    }

    // Get any alternative value of the AliasedKey in the desired style.
    template <bevarmejo::detail::text_case Style, std::size_t AltIdx=0>
    constexpr const char* as() const
    {
        static_assert(AltIdx < n_alternatives, "The alternative index is out of bounds.");

        return std::get<AltIdx>(m__values)[static_cast<std::size_t>(Style)].c_str();
    }

private:
    template <std::size_t AltIdx=0>
    constexpr const char* at() const
    {
        static_assert(AltIdx < size, "The index is out of bounds.");

        constexpr std::size_t alt = AltIdx / bevarmejo::detail::n_text_cases;
        constexpr std::size_t style = AltIdx % bevarmejo::detail::n_text_cases;

        return std::get<alt>(m__values)[style].c_str();
    }

    template <std::size_t AltIdx=0>
    const char* at(std::size_t alt_idx) const
    {
        static_assert(AltIdx < size, "The index is out of bounds.");
        beme_throw_if(alt_idx >= size, std::out_of_range,
            "Error accessing the AliasedKey values.",
            "The index is out of bounds.",
            "Index : ", alt_idx, " | Size : ", size);

        if (alt_idx == AltIdx)
        {
            constexpr std::size_t alt = AltIdx / bevarmejo::detail::n_text_cases;
            constexpr std::size_t style = AltIdx % bevarmejo::detail::n_text_cases;

            return std::get<alt>(m__values)[style].c_str();
        }
        else if constexpr (AltIdx + 1 < size)
        {
            return at<AltIdx + 1>(alt_idx);
        }
        else
        {
            return nullptr;
        }
    }

// JSON methods ----------------------------------------------------------------
private:
    template <std::size_t AltIdx = 0>
    std::size_t find_idx(const Json &j) const
    {
        if (j.contains(at<AltIdx>()))
        {
            return AltIdx;
        }
        else if constexpr (AltIdx + 1 < size)
        {
            return find_idx<AltIdx + 1>(j);
        }
        else
        {
            return size;
        }
    }

public:
    // Check if the AliasedKey exists in the json object.
    bool exists_in(const Json &j) const
    {
        return find_idx(j) < size;
    }

    // Extract the value of the AliasedKey from the json object.
    const char* as_in(const Json &j) const
    {
        std::size_t idx = find_idx(j);

        if (idx < size)
        {
            return at(idx);
        }
        else
        {
            // If the key is not found, let's simply return the default value and let
            // the JSON class handle the missing key.
            return operator()();
        }
    }
};

// Deduction guide for the AliasedKey class.
template <std::size_t N>
AliasedKey(const char (&key)[N]) -> AliasedKey<N>;

template <std::size_t N, std::size_t... Ms>
AliasedKey(const char (&first)[N], const char (&... rest)[Ms]) -> AliasedKey<N, Ms...>;

}  // namespace bevarmejo::io

// Helper macro to check for mandatory fields in the settings file.
#define check_mandatory_field(key, j)\
    beme_throw_if(!key.exists_in(j), std::runtime_error, "Error extracting the required data.", "The JSON object does not contain the mandatory field.", "Missing field : ", key())
