#pragma once

#include <filesystem>
namespace fsys = std::filesystem;
#include <vector>

namespace bevarmejo::io
{
    fsys::path locate_file(const fsys::path &filename, const std::vector<fsys::path> &lookup_paths, bool log = false);

} // namespace bevarmejo::io
