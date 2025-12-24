#pragma once

#include "bevarmejo/io/aliased_key.hpp"

namespace bevarmejo::io::key {

static constexpr bevarmejo::io::AliasedKey islands{"Islands"}; // "Islands"
static constexpr bevarmejo::io::AliasedKey errors{"Errors"}; // "Errors"
static constexpr bevarmejo::io::AliasedKey system{"System"}; // "System"
static constexpr bevarmejo::io::AliasedKey software{"Software"}; // "Software"
static constexpr bevarmejo::io::AliasedKey t0{"Time start"}; // "Time start"
static constexpr bevarmejo::io::AliasedKey tend{"Time end"}; // "Time end"

static constexpr bevarmejo::io::AliasedKey extras{"Extra info"}; // "Extra info"

static constexpr bevarmejo::io::AliasedKey ctime{"Current time"}; // "Current time"
static constexpr bevarmejo::io::AliasedKey fevals{"Fitness evaluations", "Fevals"}; // "Fitness evaluations", "Fevals"
static constexpr bevarmejo::io::AliasedKey gevals{"Gradient evaluations", "Gevals"}; // "Gradient evaluations", "Gevals"
static constexpr bevarmejo::io::AliasedKey hevals{"Hessian evaluations", "Hevals"}; // "Hessian evaluations", "Hevals"
static constexpr bevarmejo::io::AliasedKey individuals{"Individuals"}; // "Individuals"

static constexpr bevarmejo::io::AliasedKey id{"ID"}; // "ID"
static constexpr bevarmejo::io::AliasedKey dv{"Decision vector", "DV"}; // "Decision vector", "DV"
static constexpr bevarmejo::io::AliasedKey fv{"Fitness vector", "FV"}; // "Fitness vector", "FV"

}   // namespace bevarmejo::io::key
