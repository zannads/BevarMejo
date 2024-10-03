#include <filesystem>
// #include <sstream>
#include <vector>

#include "bevarmejo/io.hpp"
#include "bevarmejo/bemexcept.hpp"

#include "fsys_helpers.hpp"

// Local static strings for error messages.
namespace bevarmejo::io::log::fname {
static const std::string locate_file = "locate_file";
}

namespace bevarmejo::io::log::mex {
static const std::string file_not_found = "File not located correctly.";
}

namespace bevarmejo::io::other {
static const std::string filename_pre = "Filename : ";
}

std::filesystem::path bevarmejo::io::locate_file(const std::filesystem::path &filename, const std::vector<std::filesystem::path> &lookup_paths, bool log) {
    
    // If file is already absolute, simply test for validity and return it.
    if (filename.is_absolute()) {
        if (std::filesystem::exists(filename) && std::filesystem::is_regular_file(filename)) {
            return filename;
        }

        // Apparently, you passed an absolute path to an invalid file.
        if (!std::filesystem::exists(filename))
            __format_and_throw<std::runtime_error, bevarmejo::FunctionError>(io::log::fname::locate_file, 
                io::log::mex::file_not_found,
                "The requested file is an absolute path pointing to a non existing file.",
                io::other::filename_pre, filename.string()
            );

        if (!std::filesystem::is_regular_file(filename))
            __format_and_throw<std::runtime_error, bevarmejo::FunctionError>(io::log::fname::locate_file, 
                io::log::mex::file_not_found,
                "The requested file has been found (from the absolute path), but it is not a regular file.",
                io::other::filename_pre, filename.string()
            );
    }

    static std::ostringstream oss;
    if (log) oss.clear();

    // To be successful, the file must exist and be a regular file. Otherwise, it is not found.
    for (const auto &path : lookup_paths) {
        std::filesystem::path candidate = path / filename;
        if (std::filesystem::exists(candidate) && std::filesystem::is_regular_file(candidate)) {
            return candidate;
        }

        if (log) {
            io::stream_out(oss, "\t\tCandidate path : ", candidate.string(), "\n");

        }
    }

    __format_and_throw<std::runtime_error, bevarmejo::FunctionError>(io::log::fname::locate_file, 
        io::log::mex::file_not_found,
        "The requested file does not exist or is not a regular file in all the provided lookup paths.",
        io::other::filename_pre, filename.string(), "\n",
        oss.str()
    );

}