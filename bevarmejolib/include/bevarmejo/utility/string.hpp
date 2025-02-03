#pragma once

#include <algorithm>
#include <cctype>
#include <chrono>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

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

// Convert string in Sentence Case to Camel Case.
inline std::string sentence_case_to_camel_case(std::string_view s)
{
    // TODO: Implement this function
    return std::string(s);
}

// Convert string in Sentence Case to Kebab Case.
inline std::string sentence_case_to_kebab_case(std::string_view s)
{
    std::string result;
    result.reserve(s.size());

    bool prev_is_upper = false;
    for (auto c : s)
    {
        if (std::isupper(c))
        {
            if (!prev_is_upper)
            {
                result.push_back('-');
            }
            result.push_back(std::tolower(c));
            prev_is_upper = true;
        }
        else
        {
            result.push_back(c);
            prev_is_upper = false;
        }
    }

    // Substitute all spaces with hyphens
    std::replace(result.begin(), result.end(), ' ', '-');

    // Remove the beginning hyphen if it exists
    if (!result.empty() && result.front() == '-')
    {
        result.erase(result.begin());
    }

    return std::move(result);
}

// Convert string in Sentence Case to Pascal Case.
inline std::string sentence_case_to_pascal_case(std::string_view s)
{
    std::string camel_case = sentence_case_to_camel_case(std::move(s));
    if (!camel_case.empty())
    {
        camel_case[0] = std::toupper(camel_case[0]);
    }
    return std::move(camel_case);
}

// Convert string in Sentence Case to Snake Case.
inline std::string sentence_case_to_snake_case(std::string_view s)
{
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