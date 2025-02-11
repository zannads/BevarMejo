#pragma once

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
template <std::size_t... Ns>
class AliasedKey final
{
/*----------------------------------------------------------------------------*/
/*---------------------------- Member types ----------------------------------*/
/*----------------------------------------------------------------------------*/
public:
    using styles = bevarmejo::detail::text_case;
    
    // The output style of the AliasedKeys.
    static constexpr bevarmejo::detail::text_case out_style_t = 
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
    
    static constexpr std::size_t out_style_v = static_cast<std::size_t>(out_style_t);

    // Count how many alternatives:
    static constexpr std::size_t n_alternatives = sizeof...(Ns);

private:
    // We store the values in a tuple of Constexpr Strings of different sizes...
    // Each ConstexprString is the original key in SentenceCase transformed in
    // the desired style.
    using data_type = std::tuple<bevarmejo::detail::ConstexprString<Ns>...>;

/*----------------------------------------------------------------------------*/
/*---------------------------- Member objects --------------------------------*/
/*----------------------------------------------------------------------------*/
private:
    data_type m__alternatives;
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
    constexpr AliasedKey(const char (&alt)[N]) :
        m__alternatives{ bevarmejo::detail::ConstexprString<N>(alt) },
        m__values{ bevarmejo::detail::sentence_case_to<out_style_t>(bevarmejo::detail::ConstexprString<N>(alt)) }
    { }

    // Multiple char array constructor
    // Enabble inserted because otherwie the compiler creates a conflict with the
    // deduction guide below. It tries to create AliasedKey<> and use this constructor...
    template <typename std::enable_if_t<(sizeof...(Ns) > 0), int> = 0, std::size_t N, std::size_t... Ms>
    constexpr AliasedKey(const char (&first)[N], const char (&... rest)[Ms]) :
        m__alternatives{ bevarmejo::detail::ConstexprString<N>(first), bevarmejo::detail::ConstexprString<Ms>(rest)... },
        m__values{ bevarmejo::detail::sentence_case_to<out_style_t>(bevarmejo::detail::ConstexprString<N>(first)),
                   bevarmejo::detail::sentence_case_to<out_style_t>(bevarmejo::detail::ConstexprString<Ms>(rest))... }
    { }

    // This works on clang but not on the other compilers... It would merge
    // the two constructors above and require no deduction guide.
    /*
    constexpr AliasedKey(const char (&... alts)[Ns]) :
        m__alternatives{ alts... },
        m__values{ bevarmejo::detail::sentence_case_to<out_style_t>(bevarmejo::detail::ConstexprString(alts))... }
    { }
    */

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
        return std::get<0>(m__values).c_str();
    }

    // Get any alternative value of the AliasedKey in the desired style.
    template <bevarmejo::detail::text_case Style, std::size_t AltIdx=0>
    constexpr auto as() const
    {
        static_assert(AltIdx < n_alternatives, "The alternative index is out of bounds.");

        return bevarmejo::detail::sentence_case_to<Style>(std::get<AltIdx>(m__alternatives));
    }

private:
    template <std::size_t AltIdx>
    constexpr const char* at() const
    {
        static_assert(AltIdx < n_alternatives, "The alternative index is out of bounds.");

        return std::get<AltIdx>(m__values).c_str();
    }

    template <std::size_t AltIdx>
    const char* at_impl_from(std::size_t alt_idx) const
    {
        static_assert(AltIdx < n_alternatives, "The index is out of bounds.");
        beme_throw_if(alt_idx >= n_alternatives, std::out_of_range,
            "Error accessing the AliasedKey values.",
            "The index is out of bounds.",
            "Index : ", alt_idx, " | Size : ", n_alternatives);

        if (alt_idx == AltIdx)
        {
            return at<AltIdx>();
        }
        else if constexpr (AltIdx + 1 < n_alternatives)
        {
            return at_impl_from<AltIdx + 1>(alt_idx);
        }
        else
        {
            return nullptr;
        }
    }

// JSON methods ----------------------------------------------------------------
private:
    template <std::size_t AltIdx = 0>
    std::size_t find_idx_starting_from(const Json &j) const
    {
        if (j.contains(at<AltIdx>()))
        {
            return AltIdx;
        }
        else if constexpr (AltIdx + 1 < n_alternatives)
        {
            return find_idx_starting_from<AltIdx + 1>(j);
        }
        else
        {
            return n_alternatives;
        }
    }

public:
    // Check if the AliasedKey exists in the json object.
    bool exists_in(const Json &j) const
    {
        return find_idx_starting_from<0>(j) < n_alternatives;
    }

    // Extract the value of the AliasedKey from the json object.
    const char* as_in(const Json &j) const
    {
        std::size_t idx = find_idx_starting_from<0>(j);

        if (idx < n_alternatives)
        {
            return at_impl_from<0>(idx);
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
AliasedKey(const char (&alt)[N]) -> AliasedKey<N>;

template <std::size_t N, std::size_t... Ms>
AliasedKey(const char (&first)[N], const char (&... rest)[Ms]) -> AliasedKey<N, Ms...>;

}  // namespace bevarmejo::io

// Helper macro to check for mandatory fields in the settings file.
#define check_mandatory_field(key, j)\
    beme_throw_if(!key.exists_in(j), std::runtime_error, "Error extracting the required data.", "The JSON object does not contain the mandatory field.", "Missing field : ", key())
