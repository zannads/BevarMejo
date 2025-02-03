#pragma once

#include "bevarmejo/io/aliased_key.hpp"

namespace bevarmejo::io::key {

static const AliasedKey beme_version{"Bemelib version"}; // "Bemelib version"

static const AliasedKey problem{"Problem", "UDP"}; // "Problem", "UDP"
static const AliasedKey type{"Type"};  //, "Name"}; // "Type", "Name" (backward compatibility)
static const AliasedKey name{"Name"}; // "Name"
static const AliasedKey params{"Parameters", "Params"}; // "Parameters"

static const AliasedKey lookup_paths{"Lookup paths", "Paths"}; // "Lookup paths", "Paths"

}   // namespace bevarmejo::io::key