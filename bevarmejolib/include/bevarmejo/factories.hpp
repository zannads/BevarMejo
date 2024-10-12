#pragma once

#include <filesystem>
namespace fsys = std::filesystem;
#include <vector>

#include <pagmo/problem.hpp>

#include <nlohmann/json.hpp>
using json_o = nlohmann::json;

namespace bevarmejo {

pagmo::problem build_problem(const std::string &name, const json_o &pparams, const std::vector<fsys::path> &lookup_paths);

} // namespace bevarmejo
