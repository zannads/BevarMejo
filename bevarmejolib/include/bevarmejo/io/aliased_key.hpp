#pragma once

#include <cctype>
#include <tuple>
#include <utility>

#include "bevarmejo/io/json.hpp"

#include "bevarmejo/utility/exceptions.hpp"
#include "bevarmejo/utility/string.hpp"

namespace bevarmejo::io
{

namespace detail
{
// Helper function to create a tuple for all the possible version of a given
// sentence case constexpr string.
template <std::size_t N>
inline constexpr auto make_text_cases(const char (&in)[N])
{
    return std::make_tuple(
        bevarmejo::detail::ConstexprString<N>(in),
        bevarmejo::sentence_case_to_camel_case(in),
        bevarmejo::sentence_case_to_kebab_case(in),
        bevarmejo::sentence_case_to_pascal_case(in),
        bevarmejo::sentence_case_to_snake_case(in)
    );
}

} // namespace detail


// The AliasedKey object is meant to be a static const global object and defined
// in the various translation units where they are needed.
// With the operator() I can access the Sentence value as const references.
// Same with the operator[] method but with the possibility to access the alternatives.
// While with the get method I can access the values in the various formats
// using the template parameter.
template <std::size_t... Ns>
class AliasedKey final
{
/*----------------------------------------------------------------------------*/
/*---------------------------- Member types ----------------------------------*/
/*----------------------------------------------------------------------------*/
private:
    // "Data" is a tuple of: ( tupleOfStylesForAlt1, tupleOfStylesForAlt2, ... )
    // e.g. for 2 alternatives, data_ is:
    //   std::tuple<
    //      std::tuple<ConstexprString<N1>, ConstexprString<N1>, ...>,
    //      std::tuple<ConstexprString<N2>, ConstexprString<N2>, ...>
    //   >
    //
    // Each inner tuple is exactly the result of make_text_cases(...) for that alt.
    using data_type = std::tuple<decltype(detail::make_text_cases(std::declval<const char(&)[Ns]>() ))... >;
public:
    // Count how many alternatives:
    static constexpr std::size_t n_alternatives = sizeof...(Ns);

    // Count how many styles:
    static constexpr std::size_t n_styles = bevarmejo::detail::n_text_cases;

    // Count how many total values:
    static constexpr std::size_t n_versions = n_alternatives * n_styles;
    static constexpr std::size_t size = n_alternatives * n_styles;

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

    using styles = bevarmejo::detail::text_case;

/*----------------------------------------------------------------------------*/
/*---------------------------- Member objects --------------------------------*/
/*----------------------------------------------------------------------------*/
private:
    data_type m__values;

/*----------------------------------------------------------------------------*/
/*--------------------------- Member functions -------------------------------*/
/*----------------------------------------------------------------------------*/
// (constructor)
public:
    AliasedKey() = delete;
    constexpr AliasedKey(AliasedKey&&) = default;
    constexpr AliasedKey(const AliasedKey&) = default;
   // Constructor uses a fold expansion to call make_text_cases(...) for each alt:
    constexpr AliasedKey(const char (&... alts)[Ns]) 
        : m__values{ detail::make_text_cases(alts)... }
    {
        // optional: static_assert all are "sentence-case" if you want a check
    }

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
        return std::get<out_style_v>(std::get<0>(m__values)).c_str();
    }

    // Get any alternative value of the AliasedKey in the desired style.
    template <bevarmejo::detail::text_case Style, std::size_t AltIdx=0>
    constexpr const char* as() const
    {
        static_assert(AltIdx < n_alternatives, "The alternative index is out of bounds.");
        static_assert(static_cast<std::size_t>(Style) < n_styles, "The style index is out of bounds.");

        return std::get<static_cast<std::size_t>(Style)>(std::get<AltIdx>(m__values)).c_str();
    }

private:
    template <std::size_t AbsIdx = out_style_v>
    constexpr const char* at() const
    {
        static_assert(AbsIdx < size, "The index is out of bounds.");
        // Calculate the alternative index and the style index.
        constexpr std::size_t alt_idx = AbsIdx / n_styles;
        constexpr std::size_t style_idx = AbsIdx % n_styles;
        return std::get<style_idx>(std::get<alt_idx>(m__values)).c_str();
    }

    template <std::size_t AbsIdx = 0>
    const char* at_impl_from(std::size_t abs_idx) const
    {
        static_assert(AbsIdx < size, "The index is out of bounds.");
        beme_throw_if(abs_idx >= size, std::out_of_range,
            "Error accessing the AliasedKey values.",
            "The index is out of bounds.",
            "Index : ", abs_idx, " | Size : ", size);

        if (abs_idx == AbsIdx)
        {
            return at<AbsIdx>();
        }
        else if constexpr (AbsIdx + 1 < size)
        {
            return at_impl_from<AbsIdx + 1>(abs_idx);
        }
        else
        {
            return nullptr;
        }
    }

// JSON methods ----------------------------------------------------------------
private:
    template <std::size_t AbsIdx = out_style_v>
    std::size_t find_idx_starting_from(const Json &j) const
    {
        if (j.contains(at<AbsIdx>()))
        {
            return AbsIdx;
        }
        else if constexpr (AbsIdx + 1 < size)
        {
            return find_idx_starting_from<AbsIdx + 1>(j);
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
        return find_idx_starting_from<0>(j) < size;
    }

    // Extract the value of the AliasedKey from the json object.
    const char* as_in(const Json &j) const
    {
        std::size_t idx = find_idx_starting_from<0>(j);

        if (idx < size)
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

}  // namespace bevarmejo::io

// Helper macro to check for mandatory fields in the settings file.
#define check_mandatory_field(key, j)\
    beme_throw_if(!key.exists_in(j), std::runtime_error, "Error extracting the required data.", "The JSON object does not contain the mandatory field.", "Missing field : ", key())