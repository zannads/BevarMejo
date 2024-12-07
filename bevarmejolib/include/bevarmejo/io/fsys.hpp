#pragma once
#ifndef BEVARMEJOLIB__IO__FSYS_HPP
#define BEVARMEJOLIB__IO__FSYS_HPP

#include <filesystem>
namespace fsys = std::filesystem;
#include <vector>

namespace bevarmejo::io
{

// Locate a file in a list of paths.
// Handles both absolute and relative paths.
template <bool f__log = false>
fsys::path locate_file(const fsys::path &filename, const std::vector<fsys::path> &lookup_paths)
{
    // If file is already absolute, simply test for validity and return it.
    if (filename.is_absolute())
    {
        beme_throw_if(!fsys::exists(filename), std::runtime_error, 
            "File not located correctly.",
            "The requested file is an absolute path pointing to a non existing file.",
            "Filename : ", filename.string());

        beme_throw_if(!fsys::is_regular_file(filename), std::runtime_error,
            "File not located correctly.",
            "The requested file has been found (from the absolute path), but it is not a regular file.",
            "Filename : ", filename.string());

        return filename;
    }

    if constexpr (f__log)
    {
        std::ostringstream oss;
        for (const auto &path : lookup_paths)
        {
            fsys::path candidate = path / filename;
            if (fsys::exists(candidate) && fsys::is_regular_file(candidate))
            {
                return candidate;
            }
            io::stream_out(oss, "\t\tCandidate path : ", candidate.string(), "\n");
        }

        beme_throw(std::runtime_error,
            "File not located correctly.",
            "The requested file does not exist or is not a regular file in all the provided lookup paths.",
            "Filename : ", filename.string(), "\n",
            oss.str()
        );
    }
    else
    {
        for (const auto &path : lookup_paths)
        {
            fsys::path candidate = path / filename;
            if (fsys::exists(candidate) && fsys::is_regular_file(candidate))
            {
                return candidate;
            }
        }

        beme_throw(std::runtime_error,
            "File not located correctly.",
            "The requested file does not exist or is not a regular file in all the provided lookup paths.",
            "Filename : ", filename.string());
    }
}

} // namespace bevarmejo::io

#endif // BEVARMEJOLIB__IO__FSYS_HPP
