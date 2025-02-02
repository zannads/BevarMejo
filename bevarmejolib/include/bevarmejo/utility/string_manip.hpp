#pragma once

#include <string>

namespace bevarmejo {

std::string now_as_str();

// Convert string in Sentence Case to Camel Case.
std::string sentence_case_to_camel_case(const std::string &s);

std::string sentence_case_to_kebab_case(const std::string &s);

std::string sentence_case_to_pascal_case(const std::string &s);

std::string sentence_case_to_snake_case(const std::string &s);

// split a string into a vector of strings using a delimiter
std::vector<std::string> split(const std::string &s, char delimiter);

} // namespace bevarmejo