#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#include "bevarmejo/utility/exceptions.hpp"

namespace bevarmejo
{
// Returns current timestamp as string.
inline std::string now_as_str()
{
    auto currtime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%c", std::localtime(&currtime));
    return std::string(buffer);
}

namespace detail
{
// Bevarmejo library text cases.
enum class text_case : std::size_t
{
    SentenceCase = 0,
    CamelCase = 1,
    KebabCase = 2,
    PascalCase = 3,
    SnakeCase = 4
};
// The number of cases bevarmejo library handles. (Sentence, Camel, Snake, Kebab, Pascal) 
constexpr std::size_t n_text_cases = 5;

// Helper class to hold a compile-time string of length N (including '\0').
// This lets us do transformations in constexpr code without std::string.
template <std::size_t N>
struct ConstexprString
{
    std::array<char, N> data;

    constexpr ConstexprString() : 
        data { }
    {
        data.fill('\0');
    }

    // Construct from a raw string literal (size includes the '\0'):
    constexpr ConstexprString(const char (&str)[N])
    {
        for (std::size_t i = 0; i < N; ++i)
        {
            data[i] = str[i];
        }
    }

    // Subscript operator to access individual characters:
    constexpr char& operator[](std::size_t i)
    {
        return data[i];
    }
    constexpr const char& operator[](std::size_t i) const
    {
        return data[i];
    }

    // Expose the raw data for use in constexpr functions (read-only):
    constexpr const char* c_str() const
    {
        return data.data();
    }

    // Convenient size method:
    constexpr std::size_t size() const
    {
        return N;
    }
};

// Helper function to convert a ConstexprString in Sentence Case to Camel Case.
template <std::size_t N>
inline constexpr ConstexprString<N> sentence_case_to_camel_case(ConstexprString<N> in)
{
    auto out = ConstexprString<N>{}; // Default-initialized to all '\0'.
    bool capitalize_next = false;
    std::size_t write_pos = 0;

    for (std::size_t read_pos = 0; read_pos < N; ++read_pos)
    {
        char c = in[read_pos];
        if (c == '\0')
        {
            out[write_pos] = '\0';
            return out;
        }

        if (c == ' ')
        {
            capitalize_next = true;
            continue;
        }

        if (capitalize_next)
        {
            // See toupper() for why we need the two static_casts.
            out[write_pos++] = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            capitalize_next = false;
        }
        else
        {
            out[write_pos++] = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
    }

    // Ensure the output is null-terminated.
    if (write_pos < N)
    {
        out[write_pos] = '\0';
    }

    return out;
}

// Helper function to convert a ConstexprString in Sentence Case to Kebab Case.
template <std::size_t N>
inline constexpr ConstexprString<N> sentence_case_to_kebab_case(ConstexprString<N> in)
{
    auto out = in;
    
    for (std::size_t read_pos=0, write_pos=0; read_pos < N; ++read_pos)
    {
        char c = in[read_pos];
        if (c == '\0')
        {
            out[write_pos] = '\0';
            break;
        }

        if (c == ' ')
        {
            out[write_pos++] = '-';
        }
        else
        {
            out[write_pos++] = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
    }

    // Length is unchanged, so we are quite sure we got to the end and put the null terminator.
    return out;
}

// Helper function to convert a ConstexprString in Sentence Case to Pascal Case.
template <std::size_t N>
inline constexpr ConstexprString<N> sentence_case_to_pascal_case(ConstexprString<N> in)
{
    auto out = sentence_case_to_camel_case(in);
    out[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(out[0])));
    return out;
}

// Helper function to convert a ConstexprString in Sentence Case to Snake Case.
template <std::size_t N>
inline constexpr ConstexprString<N> sentence_case_to_snake_case(ConstexprString<N> in)
{
    auto out = sentence_case_to_kebab_case(in);
    for (std::size_t i = 0; i < N; ++i)
    {
        if (out[i] == '-')
        {
            out[i] = '_';
        }
    }
    return out;
}

} // namespace detail

// Convert string in Sentence Case to Camel Case.
inline std::string sentence_case_to_camel_case(std::string_view in)
{
    std::string out;
    out.reserve(in.size());

    bool capitalize_next = false;
    
    for (std::size_t read_pos = 0; read_pos < in.size(); ++read_pos)
    {
        char c = in[read_pos];
        if (c == ' ')
        {
            capitalize_next = true;
            continue;
        }

        if (capitalize_next)
        {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            capitalize_next = false;
        }
        else
        {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }

        out.push_back(c);
    }

    return std::move(out);
}

// Convert string in Sentence Case to Kebab Case.
inline std::string sentence_case_to_kebab_case(std::string_view in)
{
    std::string out(in.data(), in.size());
    // Everything to lower case
    std::transform(out.begin(), out.end(), out.begin(), 
        [](unsigned char c) { return std::tolower(c); });

    // Substitute all spaces with hyphens
    std::replace(out.begin(), out.end(), ' ', '-');

    return std::move(out);
}

// Convert string in Sentence Case to Pascal Case.
inline std::string sentence_case_to_pascal_case(std::string_view s)
{
    // Simply convert to camel case and capitalize the first letter.
    std::string camel_case = sentence_case_to_camel_case(std::move(s));
    if (!camel_case.empty())
    {
        camel_case.front() = static_cast<char>(std::toupper(static_cast<unsigned char>(camel_case[0])));
    }
    return std::move(camel_case);
}

// Convert string in Sentence Case to Snake Case.
inline std::string sentence_case_to_snake_case(std::string_view s)
{
    // Simply onvert to kebab case and replace hyphens with underscores.
    std::string kebab_case = sentence_case_to_kebab_case(std::move(s));
    std::replace(kebab_case.begin(), kebab_case.end(), '-', '_');
    return std::move(kebab_case);
}

// Split a string into a vector of strings using a delimiter.
template <typename StringType>
inline std::vector<std::string> split(StringType&& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream token_stream(std::string(std::forward<StringType>(s)));
    while (std::getline(token_stream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return std::move(tokens);
}

} // namespace bevarmejo