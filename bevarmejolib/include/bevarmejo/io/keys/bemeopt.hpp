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
static const Key out_file_format{"Output file format", "Out file format"}; // "Output file format", "Out file format"
static const Key out_key_style{"Output key style", "Out key style"}; // "Output key style", "Out key style"

}   // namespace bevarmejo::io::key
