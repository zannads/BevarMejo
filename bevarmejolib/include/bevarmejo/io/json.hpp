#pragma once

#include <nlohmann/json.hpp>
using Json = nlohmann::json;

namespace bevarmejo
{
using name_t = Json::object_t::key_type;
using key_t = Json::object_t::key_type;
}