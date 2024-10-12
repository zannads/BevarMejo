#pragma once

#include "bevarmejo/io/key.hpp"

namespace bevarmejo::io::key {

static const Key islands{"Islands"}; // "Islands"
static const Key errors{"Errors"}; // "Errors"
static const Key system{"System"}; // "System"
static const Key software{"Software"}; // "Software"
static const Key t0{"Time start"}; // "Time start"
static const Key tend{"Time end"}; // "Time end"

static const Key extras{"Extra info"}; // "Extra info"

static const Key ctime{"Current time"}; // "Current time"
static const Key fevals{"Fitness evaluations", "Fevals"}; // "Fitness evaluations", "Fevals"
static const Key gevals{"Gradient evaluations", "Gevals"}; // "Gradient evaluations", "Gevals"
static const Key hevals{"Hessian evaluations", "Hevals"}; // "Hessian evaluations", "Hevals"
static const Key individuals{"Individuals"}; // "Individuals"

static const Key id{"ID"}; // "ID"
static const Key dv{"Decision vector", "DV"}; // "Decision vector", "DV"
static const Key fv{"Fitness vector", "FV"}; // "Fitness vector", "FV"

}   // namespace bevarmejo::io::key
