#pragma once

#include "bevarmejo/io/aliased_key.hpp"

namespace bevarmejo::io::key {

static const AliasedKey islands{"Islands"}; // "Islands"
static const AliasedKey errors{"Errors"}; // "Errors"
static const AliasedKey system{"System"}; // "System"
static const AliasedKey software{"Software"}; // "Software"
static const AliasedKey t0{"Time start"}; // "Time start"
static const AliasedKey tend{"Time end"}; // "Time end"

static const AliasedKey extras{"Extra info"}; // "Extra info"

static const AliasedKey ctime{"Current time"}; // "Current time"
static const AliasedKey fevals{"Fitness evaluations", "Fevals"}; // "Fitness evaluations", "Fevals"
static const AliasedKey gevals{"Gradient evaluations", "Gevals"}; // "Gradient evaluations", "Gevals"
static const AliasedKey hevals{"Hessian evaluations", "Hevals"}; // "Hessian evaluations", "Hevals"
static const AliasedKey individuals{"Individuals"}; // "Individuals"

static const AliasedKey id{"ID"}; // "ID"
static const AliasedKey dv{"Decision vector", "DV"}; // "Decision vector", "DV"
static const AliasedKey fv{"Fitness vector", "FV"}; // "Fitness vector", "FV"

}   // namespace bevarmejo::io::key
