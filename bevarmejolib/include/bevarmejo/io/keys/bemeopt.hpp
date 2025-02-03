#pragma once

#include "bevarmejo/io/aliased_key.hpp"

namespace bevarmejo::io::key {

static const AliasedKey exp_name{"Experiment name", "Exp name", "Name"}; // "Experiment name", "Exp name", "Name" 
static const AliasedKey archi{"Archipelago"}; // "Archipelago"
static const AliasedKey topology{"Topology", "UDT"}; // "Topology"

static const AliasedKey typconfig{"Typical configuration"}; // "Typical configuration"

static const AliasedKey islandname{"Island name", "Isl name", "Name"}; // "Island name", "Isl name", "Name"
static const AliasedKey island{"Island", "UDI"}; // "Island", "UDI"
static const AliasedKey algorithm{"Algorithm", "UDA"}; // "Algorithm", "UDA"
static const AliasedKey r_policy{"Replacement policy", "R policy", "UDRP"}; // "Replacement policy", "R policy", "UDRP"
static const AliasedKey s_policy{"Selection policy", "S policy", "UDSP"}; // "Selection policy", "S policy", "UDSP"

static const AliasedKey population{"Population", "Pop"}; // "Population", "Pop"
static const AliasedKey size{"Size"}; // "Size"
static const AliasedKey generations{"Generations", "Genz"}; // "Generations"
static const AliasedKey repgen{"Report generation", "Report gen"}; // "Report generation", "Report gen"
static const AliasedKey seed{"Population seed", "Seed"}; // "Population seed", "Seed"

static const AliasedKey specs{"Specializations", "Specs"}; // "Specializations", "Specs"
static const AliasedKey rand_starts{"Random starts"}; // "Random starts"

static const AliasedKey settings{"Settings"}; // "Settings"
static const AliasedKey outf_pretty{"Output file enable indent", "of indent"}; // "Output file enable indent", "of indent"

}   // namespace bevarmejo::io::key
