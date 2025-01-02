
#include <algorithm>
#include <cctype>
#include <chrono>
#include <sstream>
#include <string>
#include <vector>

#include "bevarmejo/utility/string_manip.hpp"

std::string bevarmejo::now_as_str()
{
    auto currtime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%c", std::localtime(&currtime));
    return std::string(buffer);
}

std::string bevarmejo::to_camel_case(const std::string& s)
{
    // TODO: implement this function
    return std::string(s);
}

std::string bevarmejo::to_kebab_case(const std::string& s)
{
    // kebab case means that the string is separated by hyphens '-' and all the letters are lowercase

    std::string result; 
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

std::string bevarmejo::to_snake_case(const std::string& s)
{
    // TODO: implement this function
    return std::string(s);
}

std::vector<std::string> bevarmejo::split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream token_stream(s);
    while (std::getline(token_stream, token, delimiter)) {
        tokens.push_back(token);
    }
    return std::move(tokens);
}