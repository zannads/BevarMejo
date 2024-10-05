#pragma once

#include <filesystem>
#include <vector>

namespace bevarmejo::io
{
    std::filesystem::path locate_file(const std::filesystem::path &filename, const std::vector<std::filesystem::path> &lookup_paths, bool log = false);

} // namespace bevarmejo::io
