#pragma once

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>


namespace bevarmejo {
// convert any STL string to camel case
template <typename T>
T to_camel_case(const T &s) {
    return s;
}

// convert any STL string to snake case
template <typename T>
T to_snake_case(const T &s) {
    return s;
}

// convert any STL string to kebab case
template <typename T>
T to_kebab_case(const T &s) {
    // kebab case means that the string is separated by hyphens '-' and all the letters are lowercase

    T result; 
    result.reserve(s.size());

    bool prev_is_upper = false;
    for (auto c : s) {
        if (std::isupper(c)) {
            if (!prev_is_upper) {
                result.push_back('-');
            }
            result.push_back(std::tolower(c));
            prev_is_upper = true;
        } else {
            result.push_back(c);
            prev_is_upper = false;
        }
    }

    // substitute all spaces with hyphens
    std::replace(result.begin(), result.end(), ' ', '-');

    // remove the beginning hyphen if it exists
    if (!result.empty() && result.front() == '-') {
        result.erase(result.begin());
    }

    return std::move(result);
}

inline std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream token_stream(s);
    while (std::getline(token_stream, token, delimiter)) {
        tokens.push_back(token);
    }
    return std::move(tokens);
}

} // namespace bevarmejo