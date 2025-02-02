#pragma once

#include <string>

namespace bevarmejo {

std::string now_as_str();

// convert any STL string that can be constructed as a string view as a string 
// with the specific style of the case: camel, snake, kebab, pascal
std::string to_camel_case(const std::string &s);

std::string to_kebab_case(const std::string &s);

std::string to_pascal_case(const std::string &s);

std::string to_snake_case(const std::string &s);

// split a string into a vector of strings using a delimiter
std::vector<std::string> split(const std::string &s, char delimiter);

} // namespace bevarmejo