#pragma once

#include <fstream>

#include <nlohmann/json.hpp>
using Json = nlohmann::json;

#include "bevarmejo/io/fsys.hpp"
#include "bevarmejo/utility/exceptions.hpp"

namespace bevarmejo
{
using name_t = Json::object_t::key_type;
using key_t = Json::object_t::key_type;

namespace io
{

// Expand a json object that is a string representing a valid filepath. If not a string, no-op.
inline std::string expand_if_filepath(const Json& j_in, const Paths& lookup_paths, Json& j_out)
{
    if (!j_in.is_string())
        j_out = j_in;
        return "";
    
    // Else it is a string and we will interpret it as a file path.
    // We will return the file name.
    try
    {
        auto file_path = locate_file(j_in.get<fsys::path>(), lookup_paths);

        // Open, stream in Json and return it.
        std::ifstream file{file_path};

        beme_throw_if(!file.is_open(), std::runtime_error,
            "File not open.",
            "Unkown error opening the file.",
            "File : ", file_path.string());

        file >> j_out;

        return file_path.stem().string();
    }
    catch(const std::exception& e)
    {
        beme_throw(std::runtime_error,
            "Impossible to expand the file path of the Json object.",
            "The provided Json object is a string that should be expanded to a file path.",
            "'Json object content' : ", j_in.dump(),
            e.what());
    }
}

// Expand a json object that is an arrays of strings representing valid filepaths. If not an array of strings, no-op.
inline void expand_if_filepaths(const Json& j_in, const Paths& lpaths, Json& j_out)
{
    if (!j_in.is_array())
    {
        j_out = j_in;
        return;
    }

    for (const auto& j : j_in)
    {
        if (!j.is_string())
        {
            j_out = j_in;
            return;
        }
    }

    // Else it is an array of strings and we will interpret them as file paths.
    // We will return an object where the file names are the keys and the Json objects are the values.
    for (const auto& j : j_in)
    {
        try
        {
            auto file_path = locate_file(j.get<fsys::path>(), lpaths);

            // Open, stream in Json and return it.
            std::ifstream file{file_path};

            beme_throw_if(!file.is_open(), std::runtime_error,
                "File not open.",
                "Unkown error opening the file.",
                "File : ", file_path.string());

            Json j_temp;
            file >> j_temp;

            j_out[file_path.stem().string()] = j_temp;
        }
        catch(const std::exception& e)
        {
            beme_throw(std::runtime_error,
                "Impossible to expand the file paths of the Json object.",
                "The provided Json object is an array of strings that should be expanded to file paths.",
                "'Json object content' : ", j_in.dump(),
                e.what());
        }
    }
}

} // namespace io

} // namespace bevarmejo
