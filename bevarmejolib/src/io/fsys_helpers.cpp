#include <filesystem>
// #include <sstream>
#include <vector>

#include "bevarmejo/io.hpp"
#include "bevarmejo/bemexcept.hpp"

#include "fsys_helpers.hpp"

std::filesystem::path bevarmejo::io::locate_file(const std::filesystem::path &filename, const std::vector<std::filesystem::path> &lookup_paths, bool log) {

    // static std::ostringstream oss;
    // if (log) oss.clear();

    // To be successful, the file must exist and be a regular file. Otherwise, it is not found.
    for (const auto &path : lookup_paths) {
        std::filesystem::path candidate = path / filename;
        if (std::filesystem::exists(candidate) && std::filesystem::is_regular_file(candidate)) {
            return candidate;
        }

        if (log) {
            // TODO: Log the file not found.

        }
    }

    __format_and_throw<std::runtime_error, bevarmejo::FunctionError>("locate_file", 
        "File not found.",
        "The requested file is not found in the provided lookup paths."
        "\n\tFilename : ", filename.string()
        //oss.str();
        );

}