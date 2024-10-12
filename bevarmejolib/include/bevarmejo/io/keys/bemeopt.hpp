#pragma once

#include "bevarmejo/io/key.hpp"

namespace bevarmejo::io::key {

static const Key exp_name{"Experiment name", "Exp name", "Name"}; // "Experiment name", "Exp name", "Name" 
static const Key archi{"Archipelago"}; // "Archipelago"
static const Key topology{"Topology", "UDT"}; // "Topology"

static const Key typconfig{"Typical configuration"}; // "Typical configuration"

static const Key islandname{"Island name", "Isl name", "Name"}; // "Island name", "Isl name", "Name"
static const Key island{"Island", "UDI"}; // "Island", "UDI"
static const Key algorithm{"Algorithm", "UDA"}; // "Algorithm", "UDA"
static const Key r_policy{"Replacement policy", "R policy", "UDRP"}; // "Replacement policy", "R policy", "UDRP"
static const Key s_policy{"Selection policy", "S policy", "UDSP"}; // "Selection policy", "S policy", "UDSP"

static const Key population{"Population", "Pop"}; // "Population", "Pop"
static const Key size{"Size"}; // "Size"
static const Key generations{"Generations", "Genz"}; // "Generations"
static const Key repgen{"Report generation", "Report gen"}; // "Report generation", "Report gen"
static const Key seed{"Population seed", "Seed"}; // "Population seed", "Seed"

static const Key specs{"Specializations", "Specs"}; // "Specializations", "Specs"
static const Key rand_starts{"Random starts"}; // "Random starts"

static const Key settings{"Settings"}; // "Settings"
static const Key outf_format{"Output file format", "of format"}; // "Output file format", "of format"
static const Key outf_key_style{"Output file key style", "of key style"}; // "Output file key style", "of key style"
static const Key outf_pretty{"Output file enable indent", "of indent"}; // "Output file enable indent", "of indent"
static const Key outf_pretty_json_indent{"Output file indent value", "of indent v"}; // "Output file pretty JSON indent value", "of pretty JSON indent v"

}   // namespace bevarmejo::io::key
