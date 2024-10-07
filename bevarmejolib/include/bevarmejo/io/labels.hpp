#pragma once

#include <string>

namespace bevarmejo::io::log::nname {
static const std::string beme_l = "bevarmejo::"; // "bevarmejo::"
}

namespace bevarmejo::io::other {

static const std::string beme_filenames_separator = "__"; // "__"

static const std::string bemeexp_prefix = "bemeexp"; // "bemeexp"
static const std::string bemeexp_exp_suffix = ".exp"; // ".exp"
static const std::string bemeexp_isl_suffix = ".isl"; // ".isl"
static const std::string bemeexp_rnt_suffix = ".rnt"; // ".rnt"
static const std::string bemeexp_log_suffix = ".log"; // ".log"
static const std::string bemeexp_out_folder = "output"; // "output"

static const std::string optim_file_prefix = "bemeopt"; // "bemeopt"
// no suffix

static const std::string sim_file_prefix = "bemesim"; // "bemesim"
// no suffix

} // namespace bevarmejo::io::other
