#pragma once

#include "bevarmejo/io/aliased_key.hpp"

namespace bevarmejo::io::key {

static constexpr bevarmejo::io::AliasedKey exp_name{"Experiment name", "Exp name", "Name"}; // "Experiment name", "Exp name", "Name" 
static constexpr bevarmejo::io::AliasedKey archi{"Archipelago"}; // "Archipelago"
static constexpr bevarmejo::io::AliasedKey topology{"Topology", "UDT"}; // "Topology"

static constexpr bevarmejo::io::AliasedKey typconfig{"Typical configuration"}; // "Typical configuration"

static constexpr bevarmejo::io::AliasedKey islandname{"Island name", "Isl name", "Name"}; // "Island name", "Isl name", "Name"
static constexpr bevarmejo::io::AliasedKey island{"Island", "UDI"}; // "Island", "UDI"
static constexpr bevarmejo::io::AliasedKey algorithm{"Algorithm", "UDA"}; // "Algorithm", "UDA"
static constexpr bevarmejo::io::AliasedKey r_policy{"Replacement policy", "R policy", "UDRP"}; // "Replacement policy", "R policy", "UDRP"
static constexpr bevarmejo::io::AliasedKey s_policy{"Selection policy", "S policy", "UDSP"}; // "Selection policy", "S policy", "UDSP"

static constexpr bevarmejo::io::AliasedKey population{"Population", "Pop"}; // "Population", "Pop"
static constexpr bevarmejo::io::AliasedKey size{"Size"}; // "Size"
static constexpr bevarmejo::io::AliasedKey generations{"Generations", "Genz"}; // "Generations"
static constexpr bevarmejo::io::AliasedKey repgen{"Report generation", "Report gen"}; // "Report generation", "Report gen"
static constexpr bevarmejo::io::AliasedKey seed{"Population seed", "Seed"}; // "Population seed", "Seed"

static constexpr bevarmejo::io::AliasedKey specs{"Specializations", "Specs"}; // "Specializations", "Specs"
static constexpr bevarmejo::io::AliasedKey rand_starts{"Random starts"}; // "Random starts"

static constexpr bevarmejo::io::AliasedKey settings{"Settings"}; // "Settings"
static constexpr bevarmejo::io::AliasedKey outf_pretty{"Output file enable indent", "of indent"}; // "Output file enable indent", "of indent"

}   // namespace bevarmejo::io::key
