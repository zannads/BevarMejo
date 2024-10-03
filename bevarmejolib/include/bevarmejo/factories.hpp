#pragma once

#include <filesystem>
#include <vector>

#include <pagmo/problem.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace bevarmejo {

pagmo::problem build_problem(json jinput, std::vector<std::filesystem::path> lookup_paths);

} // namespace bevarmejo
