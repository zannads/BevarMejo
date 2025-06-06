#pragma once

#include "bevarmejo/io/aliased_key.hpp"

namespace bevarmejo::io::key
{

static constexpr bevarmejo::io::AliasedKey beme_version{"Bemelib version"}; // "Bemelib version"

static constexpr bevarmejo::io::AliasedKey problem{"Problem", "UDP"}; // "Problem", "UDP"
#if BEME_VERSION < 240601
static constexpr bevarmejo::io::AliasedKey type{"Type", "Name"}; // "Type", "Name" (backward compatibility)
#else
static constexpr bevarmejo::io::AliasedKey type{ "Type" }; // "Type"
#endif
static constexpr bevarmejo::io::AliasedKey name{"Name"}; // "Name"
static constexpr bevarmejo::io::AliasedKey params{"Parameters", "Params"}; // "Parameters"

static constexpr bevarmejo::io::AliasedKey lookup_paths{"Lookup paths", "Paths"}; // "Lookup paths", "Paths"

}   // namespace bevarmejo::io::key