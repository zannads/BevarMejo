//
//  io.cpp
//  BèvarMéjo
//
//  Created by Dennis Zanutto on 06/07/23.
//

#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "io.hpp"

namespace bevarmejo {
namespace io {

std::optional<std::filesystem::path> locate_file(const std::filesystem::path &filename, const std::vector<std::filesystem::path> &lookup_paths) {

    // To be successful, the file must exist and be a regular file. Otherwise, it is not found.
    for (const auto &path : lookup_paths) {
        std::filesystem::path candidate = path / filename;
        if (std::filesystem::exists(candidate) && std::filesystem::is_regular_file(candidate)) {
            return candidate;
        }
    }
    // Else not found
    return std::nullopt;
}

} // namespace io
} // namespace bevarmejo
