// 
//  model_anytown.cpp
//	anytown original test
//
//  Created by Dennis Zanutto on 11/07/23.

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <pagmo/problem.hpp>
#include <pagmo/algorithms/nsga2.hpp>
#include <pagmo/algorithm.hpp>
#include <pagmo/population.hpp>
#include <pagmo/island.hpp>

#include "bevarmejo/utility/exceptions.hpp"
#include "bevarmejo/io/aliased_key.hpp"
#include "bevarmejo/io/fsys.hpp"
#include "bevarmejo/io/json.hpp"
#include "bevarmejo/io/labels.hpp"
#include "bevarmejo/io/streams.hpp"
namespace bemeio = bevarmejo::io;
#include "bevarmejo/constants.hpp"
#include "bevarmejo/econometric_functions.hpp"
#include "bevarmejo/hydraulic_functions.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"
#include "bevarmejo/simulation/solvers/epanet/hydraulic.hpp"
#include "bevarmejo/problem/wds_problem.hpp"

#include "problems/anytown.hpp"

namespace bevarmejo {
namespace anytown {

namespace io::key
{
#if BEME_VERSION < 240601
static constexpr bemeio::AliasedKey at_inp {"WDS inp", "AT inp"}; // "WDS inp", "AT inp"
static constexpr bemeio::AliasedKey at_subnets {"WDS UDEGs", "AT subnets"}; // "WDS UDEGs", "AT subnets"
static constexpr bemeio::AliasedKey exi_pipe_opts {"Existing pipe options"}; // "Existing pipe options"
static constexpr bemeio::AliasedKey new_pipe_opts {"New pipe options"}; // "New pipe options"
static constexpr bemeio::AliasedKey tank_opts {"Tank costs", "Tank options"}; // "Tank costs", "Tank options"
static constexpr bemeio::AliasedKey opers {"Operations", "Pump group operations"}; // "Operations", "Pump group operations"
#else
static constexpr bemeio::AliasedKey at_inp {"AT inp"}; // "AT inp"
static constexpr bemeio::AliasedKey at_subnets {"AT subnets"}; // "AT subnets"
static constexpr bemeio::AliasedKey exi_pipe_opts {"Existing pipe options"}; // "Existing pipe options"
static constexpr bemeio::AliasedKey new_pipe_opts {"New pipe options"}; // "New pipe options"
static constexpr bemeio::AliasedKey tank_opts {"Tank options"}; // "Tank options"
static constexpr bemeio::AliasedKey opers {"Pump group operations"}; // "Pump group operations"
#endif
static constexpr bemeio::AliasedKey max_vel{"Pipe max velocity"}; // "Pipe max velocity"
} // namespace key
// Values for the allowed formulations in the json file.
namespace io::value {
static const std::string rehab_f1 = "rehab::f1";
static const std::string mixed_f1 = "mixed::f1";
static const std::string opertns_f1 = "operations::f1";
static const std::string twoph_f1 = "twophases::f1";
static const std::string rehab_f2 = "rehab::f2";
static const std::string mixed_f2 = "mixed::f2";
static const std::string rehab_f3 = "rehab::f3";
static const std::string mixed_f3 = "mixed::f3";
static const std::string rehab_f4 = "rehab::f4";
static const std::string mixed_f4 = "mixed::f4";
static const std::string rehab_f5 = "rehab::f5";
static const std::string mixed_f5 = "mixed::f5";
static const std::string rehab_f6 = "rehab::f6";
static const std::string mixed_f6 = "mixed::f6";
static const std::string opertns_f2 = "operations::f2";
} // namespace io::value

// Extra information for the formulations.
namespace io::other {
static const std::string rehab_f1_exinfo =  "Anytown Rehabilitation Formulation 1\nOperations from input, pipes as in Farmani, Tanks as in Vamvakeridou-Lyroudia but discrete)\n";
static const std::string mixed_f1_exinfo =  "Anytown Mixed Formulation 1\nOperations as dv, pipes as in Farmani, Tanks as in Vamvakeridou-Lyroudia but discrete)\n";
static const std::string opertns_f1_exinfo = "Anytown Operations-only problem. Pure 24-h scheduling.\n";
static const std::string twoph_f1_exinfo = "Anytown Rehabilitation Formulation 1\nPipes as in Farmani, Tanks as in Vamvakeridou-Lyroudia but discrete, operations optimized internally)\n";
static const std::string rehab_f2_exinfo =  "Anytown Rehabilitation Formulation 2\nOperations from input, pipes as single dv, Tanks as in Vamvakeridou-Lyroudia (but discrete)\n";
static const std::string mixed_f2_exinfo =  "Anytown Mixed Formulation 2\nOperations as dv, pipes as single dv, Tanks as in Vamvakeridou-Lyroudia (but discrete)\n";
static const std::string rehab_f3_exinfo =  "Anytown Rehabilitation Formulation 3\nOperations from input, pipes as single dv, Tanks as before, of reliability formulation 2\n";
static const std::string mixed_f3_exinfo =  "Anytown Mixed Formulation 3\nOperations as dv, pipes as single dv, Tanks as before, of reliability formulation 2\n";
static const std::string rehab_f4_exinfo =  "Anytown Rehabilitation Formulation 4\nOperations from input, pipes as single dv, Tanks as before, of reliability formulation 3 (velocities)\n";
static const std::string mixed_f4_exinfo =  "Anytown Mixed Formulation 4\nOperations as dv, pipes as single dv, Tanks as before, of reliability formulation 3 (velocities)\n";
static const std::string rehab_f5_exinfo =  "Anytown Rehabilitation Formulation 5\nOperations from input, pipes as single dv, Tanks as Farmani, of reliability formulation 3 (velocities)\n";
static const std::string mixed_f5_exinfo =  "Anytown Mixed Formulation 5\nOperations as dv, pipes as single dv, Tanks as Farmani, of reliability formulation 3 (velocities)\n";
static const std::string rehab_f6_exinfo =  "Anytown Rehabilitation Formulation 6\nOperations from input, pipes as single dv, Tanks as LocVolRisDiamH2DRatio, of reliability formulation 3 (velocities)\n";
static const std::string mixed_f6_exinfo =  "Anytown Mixed Formulation 6\nOperations as dv, pipes as single dv, Tanks as LocVolRisDiamH2DRatio, of reliability formulation 3 (velocities)\n";
}

auto decompose_pumpgroup_pattern(
	std::vector<double> pg_pattern,
	const std::size_t n_pumps
) -> std::vector<std::vector<double>>
{
	// I want a copy of the decision variables because every time I put a 
	// pattern to 1 I want to remove it from the vector.
	std::size_t n_periods = pg_pattern.size();
	std::vector<std::vector<double>> patterns (n_pumps, std::vector<double>(n_periods, 0.0));

	for (auto& pump_pattern : patterns) {
		auto it = pg_pattern.begin();
		for (auto& val : pump_pattern){
			if (*it > 0.0) {
				val = 1.0;
				--(*it);
			}
			++it;
		}
	}

	return patterns;
}
 
Problem::Problem(
	std::string_view a_formulation_str,
	const Json& settings,
	const bemeio::Paths &lookup_paths
) :
	inherithed(),
	m__anytown(),
	m__anytown_filename(),
	m__exi_pipe_options(anytown::exi_pipe_options),
	m__new_pipe_options(anytown::new_pipe_options),
	m__tank_options(anytown::tank_options),
	m__formulation(),
	m__exi_pipes_formulation(),
	m__new_tanks_formulation(),
	m__reliability_obj_func_formulation(),
	m__has_design(false),
	m__has_operations(false),
	m__max_velocity__m_per_s(2.0),
	__old_HW_coeffs(),
	m_algo(),
	m_pop()
{
	if (a_formulation_str == io::value::rehab_f1)
	{
		m__formulation = Formulation::rehab_f1;
		m__exi_pipes_formulation = ExistingPipesFormulation::FarmaniEtAl2005;
		m__new_tanks_formulation = NewTanksFormulation::Simple;
		m__reliability_obj_func_formulation = ReliabilityObjectiveFunctionFormulation::Base;
		m__has_design = true;
		m__has_operations = false;
		m__extra_info = io::other::rehab_f1_exinfo;
	}
	else if (a_formulation_str == io::value::mixed_f1)
	{
		m__formulation = Formulation::mixed_f1;
		m__exi_pipes_formulation = ExistingPipesFormulation::FarmaniEtAl2005;
		m__new_tanks_formulation = NewTanksFormulation::Simple;
		m__reliability_obj_func_formulation = ReliabilityObjectiveFunctionFormulation::Base;
		m__has_design = true;
		m__has_operations = true;
		m__extra_info = io::other::mixed_f1_exinfo;
	}
	else if (a_formulation_str == io::value::opertns_f1)
	{
		m__formulation = Formulation::opertns_f1;
		m__exi_pipes_formulation = ExistingPipesFormulation::FarmaniEtAl2005;
		m__new_tanks_formulation = NewTanksFormulation::Simple;
		m__reliability_obj_func_formulation = ReliabilityObjectiveFunctionFormulation::Base;
		m__has_design = false;
		m__has_operations = true;
		m__extra_info = io::other::opertns_f1_exinfo;
	}
	else if (a_formulation_str == io::value::twoph_f1)
	{
		m__formulation = Formulation::twoph_f1;
		m__exi_pipes_formulation = ExistingPipesFormulation::FarmaniEtAl2005;
		m__new_tanks_formulation = NewTanksFormulation::Simple;
		m__reliability_obj_func_formulation = ReliabilityObjectiveFunctionFormulation::Base;
		m__has_design = true;
		m__has_operations = false;
		m__extra_info = io::other::twoph_f1_exinfo;
		beme_throw(std::invalid_argument, "Impossible to construct the anytown Problem.",
			"Formulation 1 of twophase problem is not supported anymore.");
	}
	else if (a_formulation_str == bevarmejo::anytown::io::value::rehab_f2)
	{
		m__formulation = Formulation::rehab_f2;
		m__exi_pipes_formulation = ExistingPipesFormulation::Combined;
		m__new_tanks_formulation = NewTanksFormulation::Simple;
		m__reliability_obj_func_formulation = ReliabilityObjectiveFunctionFormulation::Base;
		m__has_design = true;
		m__has_operations = false;
		m__extra_info = io::other::rehab_f2_exinfo;
	}
	else if (a_formulation_str == bevarmejo::anytown::io::value::mixed_f2)
	{
		m__formulation = Formulation::mixed_f2;
		m__exi_pipes_formulation = ExistingPipesFormulation::Combined;
		m__new_tanks_formulation = NewTanksFormulation::Simple;
		m__reliability_obj_func_formulation = ReliabilityObjectiveFunctionFormulation::Base;
		m__has_design = true;
		m__has_operations = true;
		m__extra_info = io::other::mixed_f2_exinfo;
	}
	else if (a_formulation_str == bevarmejo::anytown::io::value::rehab_f3)
	{
		m__formulation = Formulation::rehab_f3;
		m__exi_pipes_formulation = ExistingPipesFormulation::Combined;
		m__new_tanks_formulation = NewTanksFormulation::Simple;
		m__reliability_obj_func_formulation = ReliabilityObjectiveFunctionFormulation::Hierarchical;
		m__has_design = true;
		m__has_operations = false;
		m__extra_info = io::other::rehab_f3_exinfo;
	}
	else if (a_formulation_str == bevarmejo::anytown::io::value::mixed_f3)
	{
		m__formulation = Formulation::mixed_f3;
		m__exi_pipes_formulation = ExistingPipesFormulation::Combined;
		m__new_tanks_formulation = NewTanksFormulation::Simple;
		m__reliability_obj_func_formulation = ReliabilityObjectiveFunctionFormulation::Hierarchical;
		m__has_design = true;
		m__has_operations = true;
		m__extra_info = io::other::mixed_f3_exinfo;
	}
	else if (a_formulation_str == bevarmejo::anytown::io::value::rehab_f4)
	{
		m__formulation = Formulation::rehab_f4;
		m__exi_pipes_formulation = ExistingPipesFormulation::Combined;
		m__new_tanks_formulation = NewTanksFormulation::Simple;
		m__reliability_obj_func_formulation = ReliabilityObjectiveFunctionFormulation::HierarchicalWithMaxVelocity;
		m__has_design = true;
		m__has_operations = false;
		m__extra_info = io::other::rehab_f4_exinfo;
	}
	else if (a_formulation_str == bevarmejo::anytown::io::value::mixed_f4)
	{
		m__formulation = Formulation::mixed_f4;
		m__exi_pipes_formulation = ExistingPipesFormulation::Combined;
		m__new_tanks_formulation = NewTanksFormulation::Simple;
		m__reliability_obj_func_formulation = ReliabilityObjectiveFunctionFormulation::HierarchicalWithMaxVelocity;
		m__has_design = true;
		m__has_operations = true;
		m__extra_info = io::other::mixed_f4_exinfo;
	}
	else if (a_formulation_str == bevarmejo::anytown::io::value::rehab_f5)
	{
		m__formulation = Formulation::rehab_f5;
		m__exi_pipes_formulation = ExistingPipesFormulation::Combined;
		m__new_tanks_formulation = NewTanksFormulation::FarmaniEtAl2005;
		m__reliability_obj_func_formulation = ReliabilityObjectiveFunctionFormulation::HierarchicalWithMaxVelocity;
		m__has_design = true;
		m__has_operations = false;
		m__extra_info = io::other::rehab_f5_exinfo;
	}
	else if (a_formulation_str == bevarmejo::anytown::io::value::mixed_f5)
	{
		m__formulation = Formulation::mixed_f5;
		m__exi_pipes_formulation = ExistingPipesFormulation::Combined;
		m__new_tanks_formulation = NewTanksFormulation::FarmaniEtAl2005;
		m__reliability_obj_func_formulation = ReliabilityObjectiveFunctionFormulation::HierarchicalWithMaxVelocity;
		m__has_design = true;
		m__has_operations = true;
		m__extra_info = io::other::mixed_f5_exinfo;
	}
	else if (a_formulation_str == bevarmejo::anytown::io::value::rehab_f6)
	{
		m__formulation = Formulation::rehab_f6;
		m__exi_pipes_formulation = ExistingPipesFormulation::Combined;
		m__new_tanks_formulation = NewTanksFormulation::LocVolRisDiamH2DRatio;
		m__reliability_obj_func_formulation = ReliabilityObjectiveFunctionFormulation::HierarchicalWithMaxVelocity;
		m__has_design = true;
		m__has_operations = false;
		m__extra_info = io::other::rehab_f6_exinfo;
	}
	else if (a_formulation_str == bevarmejo::anytown::io::value::mixed_f6)
	{
		m__formulation = Formulation::mixed_f6;
		m__exi_pipes_formulation = ExistingPipesFormulation::Combined;
		m__new_tanks_formulation = NewTanksFormulation::LocVolRisDiamH2DRatio;
		m__reliability_obj_func_formulation = ReliabilityObjectiveFunctionFormulation::HierarchicalWithMaxVelocity;
		m__has_design = true;
		m__has_operations = true;
		m__extra_info = io::other::mixed_f6_exinfo;
	}
	else
	{
		beme_throw(std::invalid_argument, "Impossible to construct the anytown Problem.",
			"The provided Anytown formulation is not yet implemented.");
	}
	m__name = (
        bemeio::other::nsp__beme +
        bemeio::other::sep__namespaces +
        problem_name + 
        bemeio::other::sep__namespaces +
        std::string(a_formulation_str)
    );

	load_network(settings, lookup_paths);

	load_other_data(settings, lookup_paths);
	
	// We have "configured" the formulations for the various parts, we can pass this info to the adapter
	m__dv_adapter.reconfigure(this->get_continuous_dvs_mask());
}

void Problem::load_network(const Json& settings, const bemeio::Paths& lookup_paths)
{
	assert(settings != nullptr && io::key::at_inp.exists_in(settings));

	const auto inp_filename = settings.at(io::key::at_inp.as_in(settings)).get<fsys::path>();

	// Unfortunately, this is always necessary because of the way that the inp file is loaded
	std::function<void (EN_Project)> fix_inp = [](EN_Project ph) {
		// change curve ID 2 to a pump curve
		assert(ph != nullptr);
		std::string curve_id = "2";
		int curve_idx = 0;
		int errorcode = EN_getcurveindex(ph, curve_id.c_str(), &curve_idx);
		assert(errorcode <= 100);

		errorcode = EN_setcurvetype(ph, curve_idx, EN_PUMP_CURVE);
		assert(errorcode <= 100);
	};

	// Check the existence of the inp_filename in any of the lookup paths and its extension
	m__anytown = std::make_shared<WDS>(
		bemeio::locate_file</* log = */true>(inp_filename, lookup_paths), 
		fix_inp
	);
	m__anytown_filename = inp_filename.string();

	// Load the sequences of names for the subnetworks
	if (io::key::at_subnets.exists_in(settings)) {
		auto j_names = Json{}; // Json for the named sequences of names
		// If is in array of strings it is the name of the files, otherwise it is a json object with the data.
		bemeio::expand_if_filepaths(
			settings.at(io::key::at_subnets.as_in(settings)),
			lookup_paths,
			j_names
		);

		for (const auto& [seq_name, names_in_seq] : j_names.items()) {
			m__anytown->submit_id_sequence(seq_name, names_in_seq.get<std::vector<std::string>>());
		}
	}
	
	// Just check that all mandatory networks have been added, otherwise add them from the default.
	// Lambda to check if a mandatory id sequence was added.
	auto has_id_sequence = [this](const auto& subnet_name) -> bool {
		for (auto&& id_seq : m__anytown->id_sequences()) {
			if (id_seq.name == subnet_name) {
				return true;
			}
		}
		return false;
	};

	if (!has_id_sequence(anytown::city_pipes__subnet_name)) {
		m__anytown->submit_id_sequence(anytown::city_pipes__subnet_name, anytown::city_pipes__el_names);
	}
	if (!has_id_sequence(anytown::exis_pipes__subnet_name)) {
		m__anytown->submit_id_sequence(anytown::exis_pipes__subnet_name, anytown::exis_pipes__el_names);
	}
	if (!has_id_sequence(anytown::new_pipes__subnet_name)) {
		m__anytown->submit_id_sequence(anytown::new_pipes__subnet_name, anytown::new_pipes__el_names);
	}
	if (!has_id_sequence(anytown::pos_tank_loc__subnet_name)) {
		m__anytown->submit_id_sequence(anytown::pos_tank_loc__subnet_name, anytown::pos_tank_loc__el_names);
	}

	// Custom made subnetworks for the temporary elements 
	if (m__formulation != Formulation::opertns_f1) {
		m__anytown->submit_id_sequence(label::__temp_elems);
	}

	// Prepare the simulation settings.
	long h_step = 0;
    int errorcode = EN_gettimeparam(m__anytown->ph(), EN_HYDSTEP, &h_step);
    assert(errorcode < 100);
    long horizon = 0;
    errorcode = EN_gettimeparam(m__anytown->ph(), EN_DURATION, &horizon);
    assert(errorcode < 100);

	m__eps_settings.resolution(h_step).horizon(horizon);

    long r_step = 0;
    errorcode = EN_gettimeparam(m__anytown->ph(), EN_REPORTSTEP, &r_step);
    assert(errorcode < 100);

    m__eps_settings.report_resolution(r_step);
}

void Problem::load_other_data(const Json& settings, const bemeio::Paths& lookup_paths) {
	if (m__formulation == Formulation::opertns_f1) { return; }

	assert(settings != nullptr);

	if (io::key::exi_pipe_opts.exists_in(settings)) {
		auto j = Json{};
		bemeio::expand_if_filepath(
			settings.at(io::key::exi_pipe_opts.as_in(settings)),
			lookup_paths,
			j
		);
		
		m__exi_pipe_options = j.get<std::vector<anytown::exi_pipe_option>>();
	}

	if (io::key::new_pipe_opts.exists_in(settings)) {
		auto j = Json{};
		bemeio::expand_if_filepath(
			settings.at(io::key::new_pipe_opts.as_in(settings)),
			lookup_paths,
			j
		);
	
		m__new_pipe_options = j.get<std::vector<anytown::new_pipe_option>>();
	}
	
	if (io::key::tank_opts.exists_in(settings)) {
		auto j = Json{};
		bemeio::expand_if_filepath(
			settings.at(io::key::tank_opts.as_in(settings)),
			lookup_paths,
			j
		);
		
		m__tank_options = j.get<std::vector<anytown::tank_option>>();
	}

	if (!m__has_operations && m__formulation != Formulation::twoph_f1)
	{
		auto operations = std::vector<double> (
			pump_group_operations.begin(),
			pump_group_operations.end()
		);

		if (io::key::opers.exists_in(settings)) {
			auto j = Json{}; // Json for the operations
			bemeio::expand_if_filepath(
				settings.at(io::key::opers.as_in(settings)),
				lookup_paths,
				j
			);

			operations = j.get<std::vector<double>>();
			assert(operations.size() == pgo_dv::size);
		}

		// Fix pumps' patterns
		for (int i = 0; i < 3; ++i) {
			// no need to go through the pumps, I know the pattern ID
			int pattern_idx = 0;
			int errorcode = EN_getpatternindex(m__anytown->ph_, std::to_string(i+2).c_str(), &pattern_idx);
			assert(errorcode <= 100);

			std::vector<double> temp(24, 0.0);
			for (int j = 0; j < 24; ++j) {
				temp[j] = operations[j] > i ? 1.0 : 0.0;
			}

			errorcode = EN_setpattern(m__anytown->ph_, pattern_idx, temp.data(), temp.size());
			assert(errorcode <= 100);

			// Should be the same as
			// m__anytown->pumps().find(std::to_string(i+2))->operator->()->speed_pattern() = temp;
			// but I can avoid as it is not used yet
		}
	}

	// Optional value that is only used in the reliability function from formulations 4:
	m__max_velocity__m_per_s = settings.value(io::key::max_vel.as_in(settings), 2.0); // 2 m/s

	if (m__formulation == Formulation::twoph_f1) {
		/*
		// Prepare the internal optimization problem 
		assert(settings.contains("Internal optimization") && settings["Internal optimization"].contains("UDA")
		&& settings["Internal optimization"].contains("UDP") && settings["Internal optimization"].contains("Population") );
		auto uda = settings["Internal optimization"]["UDA"];
		auto udp = settings["Internal optimization"]["UDP"];
		auto udpop = settings["Internal optimization"]["Population"];

		// this is nasty but as of now it will work // I am not passing any info for now 
		assert(udpop.contains(label::__report_gen_sh));
		assert(uda.contains(label::__name) && uda[label::__name] == "nsga2");
		m_algo = pagmo::algorithm( bevarmejo::Nsga2( Json{ {label::__report_gen_sh, udpop[label::__report_gen_sh] } } ) );

		assert(udp.contains(label::__name) && udp[label::__name] == "bevarmejo::anytown::operations::f1" && udp.contains(label::__params));
		pagmo::problem prob{ Problem(Formulation::opertns_f1, udp[label::__params], lookup_paths)};
		
		assert(udpop.contains(label::__size));
		m_pop = pagmo::population( prob, udpop[label::__size].get<unsigned int>()-2u ); // -2 because I will manually add the two extreme solutions (the bounds)
		m_pop.push_back(prob.get_bounds().first); // all zero operations not running 
		m_pop.push_back(prob.get_bounds().second); // all operations running at max
		*/
	}
}

// ------------------- Pagmo functions ------------------- //
// For now, all formulations have a fixed number of objectives, constraints, but different number of variables
auto Problem::get_nobj() const -> std::vector<double>::size_type
{
	return 2ul; 
}

auto Problem::get_nec() const -> std::vector<double>::size_type
{
	return 0ul;
}

auto Problem::get_nic() const -> std::vector<double>::size_type
{
	return 0ul;
}
	

auto Problem::get_continuous_dvs_mask() const -> std::vector<bool>
{
	std::size_t mask_size = 0;
	if (m__has_design) {
		std::size_t subnet_size = m__anytown->subnetwork_with_order<WDS::Pipe>(exis_pipes__subnet_name).size();
		std::size_t dv_size = [this]() {
			switch (m__exi_pipes_formulation)
			{
				case ExistingPipesFormulation::FarmaniEtAl2005:
					return fep1::dv_size;
				case ExistingPipesFormulation::Combined:
					return fep2::dv_size;
				default:
					return fep1::dv_size;
			}
		}();
		mask_size += subnet_size*dv_size;

		// New pipes
		subnet_size = m__anytown->subnetwork_with_order<WDS::Pipe>(new_pipes__subnet_name).size();
		dv_size = fnp1::dv_size;
		mask_size += dv_size*subnet_size;
	}
	
	if (m__has_operations) {
		mask_size += pgo_dv::size;
	}

	if (m__has_design) {
		std::size_t dv_size = [this]() {
			switch (m__new_tanks_formulation)
			{
				case NewTanksFormulation::Simple:
					return fnt1::dv_size;
				case NewTanksFormulation::FarmaniEtAl2005:
					return fnt2::dv_size;
				case NewTanksFormulation::LocVolRisDiamH2DRatio:
					return fnt3::dv_size;
				default:
					return fnt1::dv_size;
			}
		}();
		mask_size += dv_size*max_n_installable_tanks;
	}

	// Mask indicating which decision variable is continuous and hwihc one is discrete
	// Used by the PagmoDecisionVectorAdapter to rearrange them.
	// we put all true by default because in pagmo by default nix is 0 and they are all considered continuous dvs 
	auto mask = std::vector<bool>();
	mask.reserve(mask_size);

	std::size_t pos = 0;

	if (m__has_design) {

		// Existing pipes
		std::size_t subnet_size = m__anytown->subnetwork_with_order<WDS::Pipe>(exis_pipes__subnet_name).size();
		for (auto i = 0; i < subnet_size; ++i) {
			switch (m__exi_pipes_formulation)
			{
				case ExistingPipesFormulation::FarmaniEtAl2005:
					mask.insert(mask.end(),
						fep1::dv_continous_mask.begin(),
						fep1::dv_continous_mask.end()
					);
					break;
				case ExistingPipesFormulation::Combined:
					mask.insert(mask.end(),
						fep2::dv_continous_mask.begin(),
						fep2::dv_continous_mask.end()
					);
					break;
			}
		}

		// New pipes
		subnet_size = m__anytown->subnetwork_with_order<WDS::Pipe>(new_pipes__subnet_name).size();
		for (auto i = 0; i < subnet_size; ++i) {
			mask.insert(mask.end(),
				fnp1::dv_continous_mask.begin(),
				fnp1::dv_continous_mask.end()
			);
		}
	}

	if (m__has_operations) {
		mask.insert(mask.end(),
			pgo_dv::is_continous_mask.begin(),
			pgo_dv::is_continous_mask.end()
		);
	}

	if (m__has_design) {
		for (auto i = 0; i < max_n_installable_tanks; ++i) {
			switch (m__new_tanks_formulation)
			{
				case NewTanksFormulation::Simple:
					mask.insert(mask.end(),
						fnt1::dv_continous_mask.begin(),
						fnt1::dv_continous_mask.end()
					);
					break;
				case NewTanksFormulation::FarmaniEtAl2005:
					mask.insert(mask.end(),
						fnt2::dv_continous_mask.begin(),
						fnt2::dv_continous_mask.end()
					);
					break;
				case NewTanksFormulation::LocVolRisDiamH2DRatio:
					mask.insert(mask.end(),
						fnt3::dv_continous_mask.begin(),
						fnt3::dv_continous_mask.end()
					);
			}
		}

	}

	return std::move(mask);
}

// ------------------- 1st level ------------------- //
auto Problem::fitness(
	const std::vector<double>& pagmo_dv
) const -> std::vector<double>
{
	// Let's pre-allocate in case something doesn't work out.
	std::vector<double> fitv(get_nobj()+get_nec()+get_nic(), std::numeric_limits<double>::max());

    // First thing first reconvert back from the pagmo ordering to the beme one.
	const auto dvs = m__dv_adapter.from_pagmo_to_beme(pagmo_dv);

	// For now, instead of creating a copy object, I will just apply the changes to the network and then reset it.
	// This doesn't allow parallelization unless you have different copies of the problem.
    apply_dv(dvs);

	// things to do 
	// 1. EPS 
	//   [x]   apply dvs to the network
	// 	 [x]	run EPS as it is
	// 	 [x]	check energy consumption
	// 	 [x]	check pressure for reliability
	// 	 [x]	check min pressure constraint
	// 	 [x]	check tanks complete emptying and filling (implicit)
	// 2. instantenous peak flow
	// 		apply changes wrt EPS
	//      	not running anymore for 24h but instantenous 
	// 			the multiplier is different 
	//		check min pressure constraint 
	// 3. fire flow conditions 
	// 		apply changes wrt IPF
	//			runs for 2h 
	//			the multiplier is different
	//			add the emitters and then remove them and repeat for the next condition
	// 		check min pressure constraint 

	// 1. EPS
	auto results = sim::solvers::epanet::solve_hydraulics(*m__anytown, m__eps_settings);

	if (!sim::solvers::epanet::is_successful_with_warnings(results))
	{
		bemeio::stream_out( std::cerr, "Error in the hydraulic simulation. \n");
		reset_dv(dvs);
		return std::move(fitv);
	}

	// First objective is always cost for all formulations.
	fitv[0] = cost(dvs);
	
	// Second objective is the of__reliability, based on the formulation
	switch (m__reliability_obj_func_formulation)
	{
	case ReliabilityObjectiveFunctionFormulation::Base:
		fitv[1] = fr1::of__reliability(*m__anytown);
		break;
	case ReliabilityObjectiveFunctionFormulation::Hierarchical:
		fitv[1] = fr2::of__reliability(*m__anytown, results);
		break;
	case ReliabilityObjectiveFunctionFormulation::HierarchicalWithMaxVelocity:
		fitv[1] = fr3::of__reliability(*m__anytown, results, m__max_velocity__m_per_s);
		break;
	default:
		break;
	}
	
	reset_dv(dvs);
    return std::move(fitv);
}

// ------------------- 2nd level ------------------- //
auto Problem::apply_dv(
	const std::vector<double>& dvs
) const -> void
{
	m__anytown->cache_indices();

	// Unfortunately I have to follow an order that doesn't make sense, which is the one on which I implemented the dvs.
	// 1. Existing pipes,
	// 2. new pipes
	// 3. operations 
	// 4. tanks
	auto curr_dv = dvs.begin();
	if (m__has_design) {
		// 1. Existing pipes
		std::size_t subnet_size = m__anytown->subnetwork_with_order<WDS::Pipe>(exis_pipes__subnet_name).size();
		std::size_t gene_size;
		switch (m__exi_pipes_formulation)
		{
			case ExistingPipesFormulation::FarmaniEtAl2005:
				gene_size = fep1::dv_size*subnet_size;
				fep1::apply_dv__exis_pipes(
					*m__anytown,
					__old_HW_coeffs,
					curr_dv,
					curr_dv+gene_size,
					m__exi_pipe_options
				);
				break;
			case ExistingPipesFormulation::Combined:
				gene_size = fep2::dv_size*subnet_size;
				fep2::apply_dv__exis_pipes(
					*m__anytown,
					__old_HW_coeffs,
					curr_dv,
					curr_dv+gene_size,
					m__exi_pipe_options
				);
				break;
			default:
				break;
		}
		curr_dv += gene_size;

		// 2. New pipes
		subnet_size = m__anytown->subnetwork_with_order<WDS::Pipe>(new_pipes__subnet_name).size();
		gene_size = fnp1::dv_size*subnet_size;
		fnp1::apply_dv__new_pipes(
			*m__anytown,
			curr_dv,
			curr_dv+gene_size,
			m__new_pipe_options
		);
		curr_dv += gene_size;
	}

	// 3. Operations
	if (m__has_operations) {
		auto gene_size = pgo_dv::size;
		pgo_dv::apply_dv__pumps(
			*m__anytown,
			curr_dv,
			curr_dv+gene_size
		);
		curr_dv += gene_size;
	}

	// 4. Tanks
	if (m__has_design) {
		auto gene_size = 0;
		switch (m__new_tanks_formulation)
		{
			case NewTanksFormulation::Simple:
				gene_size = fnt1::dv_size*max_n_installable_tanks;
				fnt1::apply_dv__tanks(
					*m__anytown,
					curr_dv,
					curr_dv+gene_size,
					m__tank_options
				);
				break;
			case NewTanksFormulation::FarmaniEtAl2005:
				gene_size = fnt2::dv_size *max_n_installable_tanks;
				fnt2::apply_dv__tanks(
					*m__anytown,
					curr_dv,
					curr_dv+gene_size,
					m__new_pipe_options
				);
				break;
			case NewTanksFormulation::LocVolRisDiamH2DRatio:
				gene_size = fnt3::dv_size *max_n_installable_tanks;
				fnt3::apply_dv__tanks(
					*m__anytown,
					curr_dv,
					curr_dv+gene_size,
					m__tank_options,
					m__new_pipe_options
				);
				break;
			default:
				break;
		}
		curr_dv += gene_size;
	}

	/*
	if(m__formulation == Formulation::twoph_f1)
	{
		// Forward these changes also to the internal optimization problem
		auto udp = m_pop.get_problem().extract<bevarmejo::anytown::Problem>();
		assert(udp != nullptr);
		udp->m__anytown = this->m__anytown;

		// I need to force the re-evaluation of the current solutions in the population
		// So I create a "new population", that has the same elements of the previous one,
		// but now, since the problem has changed, it will re-evaluate the solutions.
		// I will then replace the old population with the new one.
		// Super light to copy the internal problem as it only has the internal shared pointer to the network
		pagmo::population new_pop( *udp *//*, pop_size = 0, seed = m_pop.get_seed() */
		/*
		); 
		for (const auto& dv: m_pop.get_x()) {
			new_pop.push_back(dv);
		}
		m_pop = std::move(new_pop);

		// Run the internal optimization problem which evolve n solutions m times
		m_pop = m_algo.evolve(m_pop);

		// The best solution is the one that minimizes the second objective function
		// of the internal optimization problem (Pressure deficit). Ideally, after a
		// while this is like a Single Objective problem trying to minimize cost.
		auto dvs_internal = m_pop.get_x();
		auto fvs_internal = m_pop.get_f();
		std::vector<std::size_t> idx(fvs_internal.size());
		std::iota(idx.begin(), idx.end(), 0u);
		std::sort(idx.begin(), idx.end(), [&fvs_internal] (auto a, auto b) {return fvs_internal[a][1] < fvs_internal[b][1];});
		
		// To calculate the reliability index, since I lost the results in the internal optimization
		// I have to apply the selected pattern to the network and run the simulation again. 
		apply_dv__pumps(*m__anytown, m_pop.get_x().at(idx.front()));
		return;	
	}
	*/
}

auto Problem::cost(
	const std::vector<double> &dvs
) const -> double
{

	if (m__formulation == Formulation::opertns_f1) {
		// Just the operational cost!
		return pgo_dv::cost__energy_per_day(*m__anytown);
	}

	// If we are here is either a design or an integrated problem, so it has design for sure
	// So we will account for the capital cost of interventions
	assertm(m__has_design, "Logic error, this part should be called only for problems with design");
	double capital_cost = 0.0;

	// For each dv group I need to:
	// 1. get the gene size,
	// 2. apply the correct cost function
	// 3. move forwarde the curr_dv
	auto curr_dv = dvs.begin();
	std::size_t gene_size;

	std::size_t subnet_size = m__anytown->subnetwork_with_order<WDS::Pipe>(exis_pipes__subnet_name).size();
	switch (m__exi_pipes_formulation)
	{
		case ExistingPipesFormulation::FarmaniEtAl2005:
			gene_size = fep1::dv_size*subnet_size;
			capital_cost += fep1::cost__exis_pipes(
				*m__anytown,
				curr_dv,
				curr_dv+gene_size,
				m__exi_pipe_options
			);
			break;
		case ExistingPipesFormulation::Combined:
			gene_size = fep2::dv_size*subnet_size;
			capital_cost += fep2::cost__exis_pipes(
				*m__anytown,
				curr_dv,
				curr_dv+gene_size,
				m__exi_pipe_options
			);
			break;
		default:
			break;
	}
	curr_dv += gene_size;

	// 2. New pipes
	subnet_size = m__anytown->subnetwork_with_order<WDS::Pipe>(new_pipes__subnet_name).size();
	gene_size = fnp1::dv_size*subnet_size;
	capital_cost += fnp1::cost__new_pipes(
		*m__anytown,
		curr_dv,
		curr_dv+gene_size,
		m__new_pipe_options
	);
	curr_dv += gene_size;

	// 3. Operations
	if (m__has_operations) {
		gene_size = pgo_dv::size;
		// No cost function here we will take energy later independenlty of the 
		// fact that there is a dv or not
		curr_dv += gene_size;
	}

	// 4. Tanks
	switch (m__new_tanks_formulation)
	{
		case NewTanksFormulation::Simple:
			gene_size = fnt1::dv_size*max_n_installable_tanks;
			capital_cost += fnt1::cost__tanks(
				*m__anytown,
				curr_dv,
				curr_dv+gene_size,
				m__tank_options,
				m__new_pipe_options
			);
			break;
		case NewTanksFormulation::FarmaniEtAl2005:
			gene_size = fnt2::dv_size *max_n_installable_tanks;
			capital_cost += fnt2::cost__tanks(
				*m__anytown,
				curr_dv,
				curr_dv+gene_size,
				m__tank_options,
				m__new_pipe_options
			);
			break;
		case NewTanksFormulation::LocVolRisDiamH2DRatio:
			gene_size = fnt3::dv_size *max_n_installable_tanks;
			capital_cost += fnt3::cost__tanks(
				*m__anytown,
				curr_dv,
				curr_dv+gene_size,
				m__tank_options,
				m__new_pipe_options
			);
			break;
		default:
			break;
	}
	curr_dv += gene_size;
		
	double energy_cost_per_day = pgo_dv::cost__energy_per_day(*m__anytown);
	double yearly_energy_cost = energy_cost_per_day * bevarmejo::k__days_ina_year;
	
	// since this function is named "cost", I return the opposite of the money I have to pay so it is positive as the word implies
	return -bevarmejo::net_present_value(capital_cost, discount_rate, -yearly_energy_cost, amortization_years);
}

auto fr1::of__reliability(
	const WDS &anytown
) -> double
{

	double value = 0.0;

	// Resilience index 
	const auto ir_daily = resilience_index_from_min_pressure(anytown, bevarmejo::anytown::min_pressure__psi*MperFT/PSIperFT);
	
	unsigned long t_prec = 0;
	double ir_prec = 0.0;
	for (const auto& [t, ir] : ir_daily) {
		value += ir_prec*(t - t_prec); // same as energy I need a "forward" weighted sum
		t_prec = t;
		ir_prec = ir;
	}
	auto& t_total = t_prec; // at the end of the loop t_prec is the last time step
	value /= t_total;
	value = -value; // I want to maximize the resilience index

	// ---------------------------------------------------
	// Constraints 
	// Ir goes from 0 to -1, and the algo tries to minimize going towards -1.
	// The constraint are here to reduce this effect. However, if reliability is 0, it means that the pressure constraint
	// is not satisfied and I still want to pass this info to the opt algo in some way. So if the reliability is 0, I 
	// pass the absolute value of the pressure deficit, which is positve, so the worse is the pressure deficit, the worse 
	// the objective. I don't even add the tanks because it is the least of my problems.
	// When there is a little bit of reliability, the pressure constraint can be normalized so I get a value from 0 to inf
	// and that is 1 when the pressure at the node is exactly zero. So if I multiply the reliability by 1 minus the 
	// normalized pressure deficit, I get a value that is the reliability when I satisfy the constraint, reduced by 
	// a little bit when some nodes have a pressurede deficit, and it even changes sign when the pressure deficit is
	// large (pdef > 1 i.e. p < 0). However, if the reliability is bigger than zero, i.e., the solution run correctly
	// also the mean_norm_daily_deficit is between 0 and 1 and it works perfectly as a weight.

	if (value >= 0.0) { // I consider invalid solutions (0.0) and solutions with negative reliability ???
		// reset reliability, forget about this
		value = 0.0;
		const auto normdeficit_daily = pressure_deficiency(anytown, bevarmejo::anytown::min_pressure__psi*MperFT/PSIperFT, /*relative=*/ true);
		// just accumulate through the day, no need to average it out
		for (const auto& [t, deficit] : normdeficit_daily) {
			value += deficit;
		}
	}
	else {
		const auto normdeficit_daily = pressure_deficiency(anytown, bevarmejo::anytown::min_pressure__psi*MperFT/PSIperFT, /*relative=*/ true);
		t_prec = 0;
		double pdef_prec = 0.0;
		double mean_norm_daily_deficit = 0.0;
		for (const auto& [t, deficit] : normdeficit_daily) {
			mean_norm_daily_deficit += pdef_prec*(t - t_prec); // same as energy I need a "forward" weighted sum
			t_prec = t;
			pdef_prec = deficit;
		}
		t_total = t_prec; 
		mean_norm_daily_deficit /= t_total;
		// cum daily deficit is positive when the pressure is below the minimum, so minimize it means getting it to zero 
		value *= (1-mean_norm_daily_deficit); // Ideally this is 0.0 when I satisfy the minimum pressure constraint

		/*if (mean_norm_daily_deficit < 1.0 ) {
			// Tanks constraint
			double tank_cnstr = tanks_operational_levels_use(m__anytown->tanks());
			fitv[1] *= (1-tank_cnstr); // tank constraint is 0.0 when I satisfy the tank constraints, and since it is between 0 and 1 I can use it as a weight.
		}*/
	}

	return value;
}

auto fr2::of__reliability(
	const WDS& anytown,
	const bevarmejo::sim::solvers::epanet::HydSimResults &res
) -> double
{
	double value = 0.0;
	/*
	 * The reliability index will display a hierarchy for the constraints and the objectives.
	 * Since it has to be minimized, we start from the highest priority constraint (highest objective value) and go down.
	 * - from 2 to 1, the main constraint on mathematical solution validity. We are using EPANET that returns a warning
	 *  when the equations are solved but the solution is not valid. This is the first thing to check.
	 * - from 1 to 0, the normalized pressure deficit averaged across the nodes, integrated though time.
	 *   We want to fully satisify this before moving to the reliablity index because the Ir depends on a min pressure constraint.
	 * - from 0 to -1, the reliability index. This can be reduced by multipling by unsatisfied soft constraints.
	 */

	// Get the share of un-solved steps in the hydraulic simulation
	int n_steps = res.size();
	const int n_correct_steps = std::count(res.values().begin(), res.values().end(), 0);

	if ( n_correct_steps <  n_steps )
	{
		return 1.0 + (n_steps - n_correct_steps) / (double)n_steps; 
	}

	// All steps are solved, now check the pressure deficit

	// Get the cumulative deficit of all junctions
	const auto normdeficit_daily = pressure_deficiency(anytown, bevarmejo::anytown::min_pressure__psi*MperFT/PSIperFT, /*relative=*/ true);
	value = normdeficit_daily.integrate_forward();

	if ( value > 0. )
	{
		// Return the average normalized deficit
		return normdeficit_daily.back().first != 0 ? value / normdeficit_daily.back().first : value;
	}
	

	// All constraints are satisfied, now check the reliability index
	const auto ir_daily = resilience_index_from_min_pressure(anytown, bevarmejo::anytown::min_pressure__psi*MperFT/PSIperFT);
	value = ir_daily.integrate_forward();

	value = ir_daily.back().first != 0 ? value / ir_daily.back().first : value;

	// Here you could add other constraint between 0 and 1 and use them ro reduce the reliability index
	// Or wrap this function in a new one that does this for you.

	return -value; // I want to maximize the reliability index
}

auto fr3::of__reliability(
	const WDS& anytown,
	const bevarmejo::sim::solvers::epanet::HydSimResults& res,
	double max_velocity__m_per_s
) -> double
{
	// With this formulation, we have one additional constraint to include with
	// respect to the fr2::of__reliability. This is the maximum velocity constraint.

	// We need to return the average of the constraint violations if there are any.
	// If there are no constraint violations, we return the reliability index.

	// We re use the reliability index from the previous formulation.
	// This is between 2 and 1 for unfeasible solutions.
	// This is between 1 and 0 for solutions that do not satisfy the pressure constraint.
	// This is between 0 and -1 for solutions that satisfy the pressure constraint (the value is the rel index).
	double value = fr2::of__reliability(anytown, res);

	// If value >= 1.0, it's an unfeasible solution, we just forward it. 
	if (value >= 1.0)
	{
		return value;
	}

	// The solution is mathematically feasible, so we compute the maximum velocity constraint violation.

	double observed_max_velocity = 0.0;
	for (const auto& [id, pipe] : anytown.pipes())
	{
		for (const auto& vel : pipe.velocity().values())
		{
			if (vel > observed_max_velocity)
			{
				observed_max_velocity = vel;
			}
		}
	}
	// With the velocity violation written like this we can a violation > 1 or even 2.
	// So the objective function is not really bounded by the 2.
	double velocity_violation = (observed_max_velocity <= max_velocity__m_per_s) ? 0.0 : (observed_max_velocity - max_velocity__m_per_s) / max_velocity__m_per_s;
	
	// We extract the pressure violation and todini's rel index from the value
	double pressure_violation = (value > 0.0) ? value : 0.0;
	double ir = (value <= 0.0) ? value : 0.0; // unneccessary, but here for better readability of return value

	double total_violation = (pressure_violation + velocity_violation) / 2.0;

	return (total_violation > 0.0) ? total_violation : ir;
}

auto Problem::reset_dv(
	const std::vector<double>& dvs
) const -> void
{
	// Do the opposite operations of apply_dv 
	auto curr_dv = dvs.begin();
	if (m__has_design) {
		// 1. Existing pipes
		std::size_t subnet_size = m__anytown->subnetwork_with_order<WDS::Pipe>(exis_pipes__subnet_name).size();
		std::size_t gene_size;
		switch (m__exi_pipes_formulation)
		{
			case ExistingPipesFormulation::FarmaniEtAl2005:
				gene_size = fep1::dv_size*subnet_size;
				fep1::reset_dv__exis_pipes(
					*m__anytown,
					curr_dv,
					curr_dv+gene_size,
					__old_HW_coeffs
				);
				break;
			case ExistingPipesFormulation::Combined:
				gene_size = fep2::dv_size*subnet_size;
				fep2::reset_dv__exis_pipes(
					*m__anytown,
					curr_dv,
					curr_dv+gene_size,
					__old_HW_coeffs
				);
				break;
			default:
				break;
		}
		curr_dv += gene_size;

		// 2. New pipes
		subnet_size = m__anytown->subnetwork_with_order<WDS::Pipe>(new_pipes__subnet_name).size();
		gene_size = fnp1::dv_size*subnet_size;
		fnp1::reset_dv__new_pipes(
			*m__anytown,
			curr_dv,
			curr_dv+gene_size
		);
		curr_dv += gene_size;
	}

	// 3. Operations
	if (m__has_operations) {
		auto gene_size = pgo_dv::size;
		pgo_dv::reset_dv__pumps(
			*m__anytown,
			curr_dv,
			curr_dv+gene_size
		);
		curr_dv += gene_size;
	}

	// 4. Tanks
	if (m__has_design) {
		auto gene_size = 0;
		switch (m__new_tanks_formulation)
		{
			case NewTanksFormulation::Simple:
				gene_size = fnt1::dv_size*max_n_installable_tanks;
				fnt1::reset_dv__tanks(
					*m__anytown,
					curr_dv,
					curr_dv+gene_size
				);
				break;
			case NewTanksFormulation::FarmaniEtAl2005:
				gene_size = fnt2::dv_size *max_n_installable_tanks;
				fnt2::reset_dv__tanks(
					*m__anytown,
					curr_dv,
					curr_dv+gene_size
				);
				break;
			case NewTanksFormulation::LocVolRisDiamH2DRatio:
				gene_size = fnt3::dv_size *max_n_installable_tanks;
				fnt3::reset_dv__tanks(
					*m__anytown,
					curr_dv,
					curr_dv+gene_size
				);
				break;
			default:
				break;
		}
		curr_dv += gene_size;
	}
}

// ------------------- 3rd level ------------------- //
// ------------------- apply_dv ------------------- //
void fep1::apply_dv__exis_pipes(
	WDS& anyt_wds,
	std::unordered_map<std::string, double> &old_HW_coeffs,
	std::vector<double>::const_iterator start_dv,
    std::vector<double>::const_iterator end_dv,
	const std::vector<bevarmejo::anytown::exi_pipe_option> &pipes_alt_costs)
{
	auto curr_dv = start_dv;
	for (auto&& [id, pipe] : anyt_wds.subnetwork_with_order<WDS::Pipe>(exis_pipes__subnet_name))
	{
		std::size_t action_type = *curr_dv++;
		std::size_t alt_option = *curr_dv++;

		if (action_type == 0) // no action
		{
#ifdef DEBUGSIM
			bemeio::stream_out(std::cout, "No action for pipe ", id, "\n");
#endif
		}
		else if (action_type == 1) // clean
		{
#ifdef DEBUGSIM
			bemeio::stream_out(std::cout, "Cleaned pipe ", id, "\n");
#endif
			// retrieve and cache the old HW coefficients, then set the new ones.
			double old_pipe_roughness = pipe.roughness().value();
			old_HW_coeffs.insert({id, old_pipe_roughness});

			int errorcode = EN_setlinkvalue(anyt_wds.ph_, pipe.EN_index(), EN_ROUGHNESS, bevarmejo::anytown::coeff_HW_cleaned);	
			assert(errorcode <= 100);

			pipe.roughness(bevarmejo::anytown::coeff_HW_cleaned);
		}
		else if (action_type == 2) // duplicate
		{
#ifdef DEBUGSIM
			bemeio::stream_out(std::cout, "Duplicated pipe ", id, " with diam ", pipes_alt_costs.at(alt_option).diameter__in, "in (", pipes_alt_costs.at(alt_option).diameter__in*MperFT/12, " mm)\n");
#endif
			// Ideally I would just need to modify my network object and then
			// this changed would be refelected automatically on the EPANET 
			// project. However, this requires some work, so I will do it the 
			// old way where I manually modify the EN_Project first and then
			// knowing the object is there I will simply fetch the data from it.

			// DUPLICATE on EPANET project
			// retrieve the old property of the already existing pipe
			int out_node1_idx = 0;
			int out_node2_idx = 0;
			int errorcode = EN_getlinknodes(anyt_wds.ph_, pipe.EN_index(), &out_node1_idx, &out_node2_idx);
			assert(errorcode <= 100);

			std::string out_node1_id = epanet::get_node_id(anyt_wds.ph_, out_node1_idx);
			std::string out_node2_id = epanet::get_node_id(anyt_wds.ph_, out_node2_idx);
			
			// create the new pipe
			// new name is Dxx where xx is the original pipe name
			auto dup_pipe_id = std::string("D")+id;
			int dup_pipe_idx = 0;
			errorcode = EN_addlink(anyt_wds.ph_, dup_pipe_id.c_str(), EN_PIPE, out_node1_id.c_str(), out_node2_id.c_str(), &dup_pipe_idx);
			assert(errorcode <= 100);
			
			// change the new pipe properties:
			// 1. diameter =  row dvs[i*2+1] column diameter of m__pipes_alt_costs
			// 2. roughness = coeff_HV_new
			// 3. length  = value of link_idx
			double diameter__in = pipes_alt_costs.at(alt_option).diameter__in;
			errorcode = EN_setlinkvalue(anyt_wds.ph_, dup_pipe_idx, EN_DIAMETER, diameter__in);
			assert(errorcode <= 100);
			// errorcode = EN_setlinkvalue(anyt_wds.ph_, dup_pipe_idx, EN_ROUGHNESS, coeff_HW_new);
			// assert(errorcode <= 100);
			double link_length = 0.0;
			errorcode = EN_getlinkvalue(anyt_wds.ph_, pipe.EN_index(), EN_LENGTH, &link_length);
			assert(errorcode <= 100);
			errorcode = EN_setlinkvalue(anyt_wds.ph_, dup_pipe_idx, EN_LENGTH, link_length);
			assert(errorcode <= 100);

			// DUPLICATE on my network object
			auto& new_pipe = anyt_wds.duplicate<WDS::Pipe>(id, dup_pipe_id);
			anyt_wds.cache_indices();
			anyt_wds.id_sequence(label::__temp_elems).push_back(dup_pipe_id);

			new_pipe.diameter(pipes_alt_costs.at(alt_option).diameter__in*MperFT/12*1000);
			new_pipe.roughness(bevarmejo::anytown::coeff_HW_new);
		}
	}
	assert(curr_dv == end_dv);
	return;
}
void fep2::apply_dv__exis_pipes(
	WDS& anyt_wds,
	std::unordered_map<std::string, double> &old_HW_coeffs,
	std::vector<double>::const_iterator start_dv,
    std::vector<double>::const_iterator end_dv,
	const std::vector<bevarmejo::anytown::exi_pipe_option> &pipes_alt_costs)
{
	auto curr_dv = start_dv;

	for (auto&& [id, pipe] : anyt_wds.subnetwork_with_order<WDS::Pipe>(exis_pipes__subnet_name))
	{
		std::size_t dv = *curr_dv++;
		std::size_t alt_option = dv-2; // -2 because the first two options are no action and clean
		
		if (dv == 0)
		{
#ifdef DEBUGSIM
			bemeio::stream_out(std::cout, "No action for pipe ", id, "\n");
#endif
		}
		else if (dv == 1) // clean
		{
#ifdef DEBUGSIM
			bemeio::stream_out(std::cout, "Cleaned pipe ", id, "\n");
#endif
			// retrieve and cache the old HW coefficients, then set the new ones.
			double old_pipe_roughness = pipe.roughness().value();
			old_HW_coeffs.insert({id, old_pipe_roughness});

			int errorcode = EN_setlinkvalue(anyt_wds.ph_, pipe.EN_index(), EN_ROUGHNESS, bevarmejo::anytown::coeff_HW_cleaned);
			assert(errorcode <= 100);

			pipe.roughness(bevarmejo::anytown::coeff_HW_cleaned);
		}
		else //  dv >= 2 // duplicate
		{
#ifdef DEBUGSIM
			bemeio::stream_out(std::cout, "Duplicated pipe ", id, " with diam ", pipes_alt_costs.at(alt_option).diameter__in, "in (", pipes_alt_costs.at(alt_option).diameter__in*MperFT/12, " mm)\n");
#endif
			// DUPLICATE on EPANET project
			// retrieve the old property of the already existing pipe
			int out_node1_idx = 0;
			int out_node2_idx = 0;
			int errorcode = EN_getlinknodes(anyt_wds.ph_, pipe.EN_index(), &out_node1_idx, &out_node2_idx);
			assert(errorcode <= 100);

			std::string out_node1_id = epanet::get_node_id(anyt_wds.ph_, out_node1_idx);
			std::string out_node2_id = epanet::get_node_id(anyt_wds.ph_, out_node2_idx);
			
			// create the new pipe
			// new name is Dxx where xx is the original pipe name
			auto new_link_id = std::string("D")+id;
			int dup_pipe_idx = 0;
			errorcode = EN_addlink(anyt_wds.ph_, new_link_id.c_str(), EN_PIPE, out_node1_id.c_str(), out_node2_id.c_str(), &dup_pipe_idx);
			assert(errorcode <= 100);
			
			// change the new pipe properties:
			// 1. diameter =  row dvs[i]-2 column diameter of m__pipes_alt_costs
			// 2. roughness = coeff_HV_new
			// 3. length  = value of link_idx
			double diameter__in = pipes_alt_costs.at(alt_option).diameter__in;
			errorcode = EN_setlinkvalue(anyt_wds.ph_, dup_pipe_idx, EN_DIAMETER, diameter__in);
			assert(errorcode <= 100);
			// errorcode = EN_setlinkvalue(anyt_wds.ph_, dup_pipe_idx, EN_ROUGHNESS, coeff_HW_new);
			// assert(errorcode <= 100);
			double link_length = 0.0;
			errorcode = EN_getlinkvalue(anyt_wds.ph_, pipe.EN_index(), EN_LENGTH, &link_length);
			assert(errorcode <= 100);
			errorcode = EN_setlinkvalue(anyt_wds.ph_, dup_pipe_idx, EN_LENGTH, link_length);
			assert(errorcode <= 100);


			// DUPLICATE on my network object
			auto& new_pipe = anyt_wds.duplicate<WDS::Pipe>(id, new_link_id);
			anyt_wds.cache_indices();
			anyt_wds.id_sequence(label::__temp_elems).push_back(new_link_id);

			new_pipe.diameter(pipes_alt_costs.at(alt_option).diameter__in*MperFT/12*1000);
			new_pipe.roughness(bevarmejo::anytown::coeff_HW_new);
		}
	}
	assert(curr_dv == end_dv);
	return;
}

void fnp1::apply_dv__new_pipes(
	WDS &anyt_wds,
	std::vector<double>::const_iterator start_dv,
    std::vector<double>::const_iterator end_dv,
	const std::vector<bevarmejo::anytown::new_pipe_option> &pipes_alt_costs)
{
    auto curr_dv = start_dv;

	for (auto&& [id, pipe] : anyt_wds.subnetwork_with_order<WDS::Pipe>(new_pipes__subnet_name))
	{
		std::size_t alt_option = *curr_dv++;	

		double diameter__in = pipes_alt_costs.at(alt_option).diameter__in;
		int errorcode = EN_setlinkvalue(anyt_wds.ph_, pipe.EN_index(), EN_DIAMETER, diameter__in);
		assert(errorcode <= 100);

		pipe.diameter(diameter__in*MperFT/12*1000); //save in mm

#ifdef DEBUGSIM
		std::cout << "New pipe with ID " << id << " installed with diam of " << diameter__in << " in (" <<diameter__in*MperFT/12*1000 <<" mm)\n";
#endif
	}

	assert(curr_dv == end_dv);
	return;
}

void pgo_dv::apply_dv__pumps(
	WDS& anyt_wds,
	std::vector<double>::const_iterator start_dv,
    std::vector<double>::const_iterator end_dv)
{
	// A pump is on only if the dv at that time instant is >= i
	std::size_t i = 0;
	for (auto&& [id, pump] : anyt_wds.pumps()) {
		auto curr_dv = start_dv;
		auto pattern_i = std::vector<double>(pgo_dv::size, 0.0);
		for (auto it_pattern = pattern_i.begin(); it_pattern != pattern_i.end(); ++it_pattern) {
			if (*curr_dv > (double)i) {
				*it_pattern = 1.0;
			}
			++curr_dv;
		}
		assert(curr_dv == end_dv);
		
		// set the pattern
		int errorcode = EN_setpattern(anyt_wds.ph_, pump.speed_pattern()->EN_index(), pattern_i.data(), pattern_i.size());
		assert(errorcode <= 100);
		++i;
	}

	return;
}

auto fnt1::apply_dv__tanks(
	WDS& anytown,
	std::vector<double>::const_iterator start_dv,
    std::vector<double>::const_iterator end_dv,
	const std::vector<bevarmejo::anytown::tank_option> &tank_option
) -> void
{
	// You can't store multiple tanks in the same location, so I need to keep track of the ones I have already installed.
	std::unordered_set<std::size_t> already_installed_tanks;

	auto curr_dv = start_dv;
	for(std::size_t i = 0; i < bevarmejo::anytown::max_n_installable_tanks; ++i)
	{
		std::size_t action_type = *curr_dv++;
		std::size_t tank_vol_option = *curr_dv++;
		std::size_t new_tank_loc_shift = action_type-1; // -1 because the first option is no action.
		
		// Safety check that I don't install the second tank on a location where there is already one.
		if (action_type == 0 || (i > 0 && already_installed_tanks.count(new_tank_loc_shift) != 0))
		{
#ifdef DEBUGSIM
			bemeio::stream_out(std::cout, "No action for tank T"+std::to_string(i)+".\n");
#endif
			continue;
		}
		// else if (action_type > 1) // install
		
		auto&& [junction_id, junction] = *(anytown.subnetwork_with_order<WDS::Junction>("possible_tank_locations").begin() + new_tank_loc_shift);

		// I should create a new tank at that position and with that volume
		double tank_volume_gal = tank_option.at(tank_vol_option).volume__gal;
		double tank_volume_m3 = tank_volume_gal * bevarmejo::k__m3_per_gal;
		
		auto new_tank_id = std::string("T")+std::to_string(i);
		auto& new_tank = anytown.insert_tank(new_tank_id);

		// elevation , min and max level are the same as in the original tanks
		// Ideally same coordinates of the junction, but I move it slightly in case I want to save the result to file and visualize it
		// diameter from volume divided by the fixed ratio
		auto&& [orig_tank_id, orig_tank] = *(anytown.tanks().begin());

		new_tank.elevation(orig_tank.elevation());
		new_tank.initial_level(orig_tank.min_level().value());
		new_tank.min_level(orig_tank.min_level().value());
		new_tank.min_volume(orig_tank.min_volume().value());
		new_tank.x_coord(junction.x_coord());
		new_tank.y_coord(junction.y_coord()+bevarmejo::anytown::riser_length__ft);
		
		// We assume d = h for a cilindrical tank, thus V = \pi d^2 /4 * h = \pi d^3 / 4
		// given that this is a fixed value we could actually have it as a parameter to reduce computational expenses. 
		double diam_m = std::pow(tank_volume_m3*4.0/k__pi, 1.0/3.0); // TODO: fix based on whatever ratio I want
		new_tank.diameter(diam_m);
		double max_lev = diam_m;
		new_tank.max_level(max_lev);

		// do it again in EPANET
		int new_tank_idx = 0; 
		int errco = EN_addnode(anytown.ph_, new_tank_id.c_str(), EN_TANK, &new_tank_idx);
		assert(errco <= 100);

		errco = EN_settankdata(anytown.ph_, new_tank_idx, 
			orig_tank.elevation()/MperFT, 
			orig_tank.min_level().value()/MperFT, 
			orig_tank.min_level().value()/MperFT, 
			max_lev/MperFT, 
			diam_m/MperFT, 
			orig_tank.min_volume().value()/M3perFT3, 
			"");
		assert(errco <= 100);

		// The riser has a well defined length, diameter could be a dv, but I fix it to 16 inches for now
		auto riser_id = std::string("Ris_")+std::to_string(i);

#if BEME_VERSION < 241200
		auto& riser = anytown.install_pipe(riser_id, new_tank_id, junction_id);
#else
		// We changed the direction because the riser RISES from the junction to the tank.
		auto& riser = anytown.install_pipe(riser_id, junction_id, new_tank_id);
#endif

		riser.diameter(16.0*MperFT/12*1000);
		riser.length(bevarmejo::anytown::riser_length__ft*MperFT);
		riser.roughness(bevarmejo::anytown::coeff_HW_new);

		// do it again in EPANET
		int riser_idx = 0;
#if BEME_VERSION < 241200
		errco = EN_addlink(anytown.ph_, riser_id.c_str(), EN_PIPE, new_tank_id.c_str(), junction_id.c_str(), &riser_idx);
#else
		errco = EN_addlink(anytown.ph_, riser_id.c_str(), EN_PIPE, junction_id.c_str(), new_tank_id.c_str(), &riser_idx);
#endif
		assert(errco <= 100);

		errco = EN_setpipedata(anytown.ph_, riser_idx,
			bevarmejo::anytown::riser_length__ft,
			16.0,
			bevarmejo::anytown::coeff_HW_new,
			0.0
		);
		assert(errco <= 100);

		anytown.cache_indices();
		assert(riser.EN_index() != 0 && riser.EN_index() == riser_idx);

		// add them to the "TBR" net and the already installed tanks
		anytown.id_sequence(label::__temp_elems).push_back(new_tank_id);
		anytown.id_sequence(label::__temp_elems).push_back(riser_id);
		already_installed_tanks.insert(new_tank_loc_shift);
#ifdef DEBUGSIM
		bemeio::stream_out(std::cout, "Installed tank at node ", junction_id, 
		" with volume ", tank_volume_gal, " gal(", tank_volume_m3, " m^3)", 
		" Elev ", new_tank.elevation(),
		" Min level ", new_tank.min_level().value(),
		" Max lev ", new_tank.max_level().value(),
		" Diam ", diam_m, "\n");
#endif
	}
	
	assert(curr_dv == end_dv);
	return;
}

auto fnt2::apply_dv__tanks(
	WDS& anytown,
	std::vector<double>::const_iterator start_dv,
    std::vector<double>::const_iterator end_dv,
	const std::vector<bevarmejo::anytown::new_pipe_option> &new_pipes_options
) -> void
{
	// We are assuming a cylindrical tank
	// The decision variable is split like this:
	// 1. (discrete) index for the pipe alternative costs indicating the riser diameter. Also do nothing option to turn off the tank
	// 2. (discrete) index for the tank location with check that one is not installed there yet
	// 3. (continuous) tank diameter [D]
	// 4. (continuous) overflow elevation [Hmax]
	// 5. (continuous) minimum normal day elevation [Hmin]
	// 6. (continuous) bottom of the tank [Elevtion = 5. - 6.] 
	// We do it twice because we can install 2

	// You can't store multiple tanks in the same location, so I need to keep track of the ones I have already installed.
	std::unordered_set<std::size_t> already_installed_tanks;

	auto curr_dv = start_dv;
	for (std::size_t i = 0; i < bevarmejo::anytown::max_n_installable_tanks; ++i)
	{
		std::size_t riser_dv = *curr_dv++;
		std::size_t new_tank_loc_shift = *curr_dv++;
		double tank_diam__m = *curr_dv++;
		double tank_hmax__m = *curr_dv++;
		double tank_hmin__m = *curr_dv++;
		double tank_safetyl__m = *curr_dv++;
		// Hmax, Hmin are in m from the zero of the z axis. So compareable with the head.
		// hsafety is a relative measure in meters.
		// They are connected from this relationship:
		// Elevation = tank_hmin__m - tank_safetyl__m
		// Min level = tank_safetyl__m (because this starts from the elevation)
		// Tank operational height = tank_hmax__m-tank_hmin__m
		// Max level = tank_safetyl__m + tank op height

		// We install a tank if the riser option is not 0 (do nothing) and we haven't installed a tank in that location already (so we can't find it in the container)
		bool install_tank = (riser_dv != 0 && already_installed_tanks.find(new_tank_loc_shift) == already_installed_tanks.end());

		if (!install_tank)
		{
#ifdef DEBUGSIM
			bemeio::stream_out(std::cout, "No action for tank T"+std::to_string(i)+".\n");
#endif
			continue;
		}
		// else we need to install the tank

		auto&& [junction_id, junction] = *(anytown.subnetwork_with_order<WDS::Junction>("possible_tank_locations").begin() + new_tank_loc_shift);

		auto new_tank_id = std::string("T")+std::to_string(i);
		auto& new_tank = anytown.insert_tank(new_tank_id);

		new_tank.elevation(tank_hmin__m-tank_safetyl__m);
		new_tank.initial_level(tank_safetyl__m);
		new_tank.min_level(tank_safetyl__m);
		double min_vol = k__pi*tank_diam__m*tank_diam__m/4.0*new_tank.min_level().value();
		new_tank.min_volume(min_vol);
		new_tank.x_coord(junction.x_coord());
		new_tank.y_coord(junction.y_coord()+bevarmejo::anytown::riser_length__ft);
		new_tank.diameter(tank_diam__m);
		double operational_tank_height = tank_hmax__m-tank_hmin__m;
		// Since tank_hmax__lb < tank_hmin__ub, it could happen that the tank is ill formed.
		if (operational_tank_height < 0)
		{
			operational_tank_height = fnt2::tank_hmax_ub__m-tank_hmin__m;
		}
		new_tank.max_level(operational_tank_height+tank_safetyl__m);

		// do it again in EPANET
		int new_tank_idx = 0; 
		int errco = EN_addnode(anytown.ph_, new_tank_id.c_str(), EN_TANK, &new_tank_idx);
		assert(errco <= 100);

		errco = EN_settankdata(anytown.ph_, new_tank_idx, 
			new_tank.elevation()/MperFT, 
			new_tank.min_level().value()/MperFT, 
			new_tank.min_level().value()/MperFT, 
			new_tank.max_level().value()/MperFT, 
			new_tank.diameter().value()/MperFT, 
			new_tank.min_volume().value()/M3perFT3, 
			"");
		assert(errco <= 100);

		// The riser has a well defined length, diameter could be a dv, but I fix it to 16 inches for now
		auto riser_id = std::string("Ris_")+std::to_string(i);

		auto& riser = anytown.install_pipe(riser_id, junction_id, new_tank_id);
		
		double riser_diam__mm = new_pipes_options.at(riser_dv-1).diameter__in*MperFT/12*1000;
		riser.diameter(riser_diam__mm);
		riser.length(bevarmejo::anytown::riser_length__ft*MperFT);
		riser.roughness(bevarmejo::anytown::coeff_HW_new);

		// do it again in EPANET
		int riser_idx = 0;
		errco = EN_addlink(anytown.ph_, riser_id.c_str(), EN_PIPE, junction_id.c_str(), new_tank_id.c_str(), &riser_idx);
		assert(errco <= 100);

		errco = EN_setpipedata(anytown.ph_, riser_idx,
			bevarmejo::anytown::riser_length__ft,
			new_pipes_options.at(riser_dv-1).diameter__in,
			bevarmejo::anytown::coeff_HW_new,
			0.0
		);
		assert(errco <= 100);

		anytown.cache_indices();
		assert(riser.EN_index() != 0 && riser.EN_index() == riser_idx);

		// add them to the "TBR" net and the already installed tanks
		anytown.id_sequence(label::__temp_elems).push_back(new_tank_id);
		anytown.id_sequence(label::__temp_elems).push_back(riser_id);
		already_installed_tanks.insert(new_tank_loc_shift);
#ifdef DEBUGSIM
		bemeio::stream_out(std::cout, "Installed tank at node ", junction_id, 
		" with: ",
		" Elev ", new_tank.elevation(),
		" Min level ", new_tank.min_level().value(),
		" Max lev ", new_tank.max_level().value(),
		" Diam ", tank_diam__m, "\n");
#endif
	}

	assert(curr_dv == end_dv);
	return;
}

auto anytown::fnt3::apply_dv__tanks(
	WDS& a_anytown_sys,
	std::vector<double>::const_iterator start_dv,
    std::vector<double>::const_iterator end_dv,
	const std::vector<bevarmejo::anytown::tank_option> &tank_options,
	const std::vector<bevarmejo::anytown::new_pipe_option> &new_pipes_options
) -> void
{
	// We assume:
	// a. a cylindrical tank: Area = d^2/4*pi
	// b. operational levels fixed
	// c. riser length fixed -> tank elevation = junction elevation + riser_length
	// d. safety volume = (min oper level - tank elevation ) * Area
	// Therefore the tanks specifications are:
	// 1. (discrete) Location: index of the tank location with check that one is not installed there yet. Also do nothing option to turn off the tank
	// 2. (discrete) Volume: index of the tank volume options
	// 3. (discrete) Riser Diameter: index for the pipe options indicating the riser diameter
	// 4. (discrete) Height-to-Diameter ratio

	// You can't store multiple tanks in the same location, so I need to keep track of the ones I have already installed.
	std::unordered_set<std::size_t> already_installed_tanks;

	auto curr_dv = start_dv;
	for (std::size_t i = 0; i < bevarmejo::anytown::max_n_installable_tanks; ++i)
	{
		std::size_t tank_loc_dv = *curr_dv++;
		std::size_t tank_vol_opt_idx = *curr_dv++;
		std::size_t riser_diam_opt_idx = *curr_dv++;
		std::size_t h2d_ratio_opt_idx = *curr_dv++;

        // This tells you if and where you would like to insall the tank
		bool install_tank = (tank_loc_dv != 0 );
        std::size_t tank_loc_shift = install_tank ? tank_loc_dv-1 : anytown::pos_tank_loc__el_names.size();
        // However, you need to adjust for already installed tanks
        install_tank = install_tank && (already_installed_tanks.find(tank_loc_shift) == already_installed_tanks.end());

        if (!install_tank)
		{
#ifdef DEBUGSIM
			bemeio::stream_out(std::cout, "No action for tank T"+std::to_string(i)+".\n");
#endif
			continue;
		}

		// If we are here, we need to install the tank and now we know all the specifications
        // or at least we could calculate them.
        
        // Elevation is completely defined from the elevation of the node/hunction where it is installed
        // plus the fixed riser length
        auto&& [junction_id, junction] = *(a_anytown_sys.subnetwork_with_order<WDS::Junction>(bevarmejo::anytown::pos_tank_loc__subnet_name).begin() + tank_loc_shift);
        double elev__m = junction.elevation() + bevarmejo::anytown::riser_length__ft*MperFT;

        // The volume is selected from the dvs and together with the ratio we can
        // calculate the diameter and height.
		double vol__m3 = tank_options.at(tank_vol_opt_idx).volume__gal* bevarmejo::k__m3_per_gal;
        // We assume h = a*d for a cilindrical tank, thus V = \pi d^2 /4 * h =  \pi d^3 /4 * a
        // a is the height-to-diameter ratio
        double h2d_ratio = anytown::fnt3::h2d_ratio__min + h2d_ratio_opt_idx*anytown::fnt3::hd2_ratio__step;

		double diam__m = std::pow(vol__m3*4.0/k__pi/h2d_ratio, 1.0/3.0);
        double height__m = h2d_ratio * diam__m;
        double top_lev__m = elev__m+height__m;

        // Operational levels are fixed by the water utility. However, we need to
        // make sure that they are between the physical levels of the tank.
        double min_ope_lev__m = bevarmejo::anytown::min_w_level_tank__ft*MperFT; // Min ope level in absolute m from the ground
        double max_ope_lev__m = bevarmejo::anytown::max_w_level_tank__ft*MperFT; // Max ope level in absolute m from the ground

        // Fix ope levels to make sure that they are both not above the top_level

        // approach 1: tank doesn't match the operational levels, so we discard it.
        // approach 2: that tank operational levels for this tank are adjusted.
        bool strict_ope_levels = true;

        if (strict_ope_levels)
        {
            if ((min_ope_lev__m >= top_lev__m || max_ope_lev__m > top_lev__m))
            {
#ifdef DEBUGSIM
			bemeio::stream_out(std::cout, "No action for tank T"+std::to_string(i)+" because out of operational levels boundaries.\n");
#endif
			continue;
            }
        }
        else
        {
            if (min_ope_lev__m >= top_lev__m)
            {
                min_ope_lev__m = elev__m + height__m*0.2857142857; // Where the 0.28... is the ratio in the original tanks...
                max_ope_lev__m = top_lev__m;
            }
            else if (max_ope_lev__m > top_lev__m )
            {
                max_ope_lev__m = top_lev__m;
            }
        }
        
		auto new_tank_id = std::string("T")+std::to_string(i);
		auto& new_tank = a_anytown_sys.insert_tank(new_tank_id);

        new_tank.elevation(elev__m);
        new_tank.diameter(diam__m);
        new_tank.min_volume(k__pi*diam__m*diam__m/4.0*(min_ope_lev__m-elev__m));

        new_tank.min_level(min_ope_lev__m-elev__m);
        new_tank.max_level(max_ope_lev__m-elev__m);
        new_tank.initial_level(min_ope_lev__m-elev__m);
        
        new_tank.x_coord(junction.x_coord());
		new_tank.y_coord(junction.y_coord()+bevarmejo::anytown::riser_length__ft);

        // do it again in EPANET
        int new_tank_idx = 0; 
		int errco = EN_addnode(a_anytown_sys.ph_, new_tank_id.c_str(), EN_TANK, &new_tank_idx);
		assert(errco <= 100);

		errco = EN_settankdata(a_anytown_sys.ph_, new_tank_idx, 
			new_tank.elevation()/MperFT, 
			new_tank.min_level().value()/MperFT, 
			new_tank.min_level().value()/MperFT, 
			new_tank.max_level().value()/MperFT, 
			new_tank.diameter().value()/MperFT, 
			new_tank.min_volume().value()/M3perFT3, 
			"");
		assert(errco <= 100);
        
        // The riser has a well defined length, but the diameter is a dv.
		auto riser_id = std::string("Ris_")+std::to_string(i);

		auto& riser = a_anytown_sys.install_pipe(riser_id, junction_id, new_tank_id);
		
		double riser_diam__mm = new_pipes_options.at(riser_diam_opt_idx).diameter__in*MperFT/12*1000;
		riser.diameter(riser_diam__mm);
		riser.length(bevarmejo::anytown::riser_length__ft*MperFT);
		riser.roughness(bevarmejo::anytown::coeff_HW_new);

		// do it again in EPANET
		int riser_idx = 0;
		errco = EN_addlink(a_anytown_sys.ph_, riser_id.c_str(), EN_PIPE, junction_id.c_str(), new_tank_id.c_str(), &riser_idx);
		assert(errco <= 100);

		errco = EN_setpipedata(a_anytown_sys.ph_, riser_idx,
			bevarmejo::anytown::riser_length__ft,
			new_pipes_options.at(riser_diam_opt_idx).diameter__in,
			bevarmejo::anytown::coeff_HW_new,
			0.0
		);
		assert(errco <= 100);

		a_anytown_sys.cache_indices();
		assert(riser.EN_index() != 0 && riser.EN_index() == riser_idx);

		// add them to the "TBR" net and the already installed tanks
		a_anytown_sys.id_sequence(label::__temp_elems).push_back(new_tank_id);
		a_anytown_sys.id_sequence(label::__temp_elems).push_back(riser_id);
		already_installed_tanks.insert(tank_loc_shift);
#ifdef DEBUGSIM
		bemeio::stream_out(std::cout, "Installed tank at node ", junction_id, 
		" with volume ", tank_options.at(tank_vol_opt_idx).volume__gal, " gal(", vol__m3, " m^3)", 
		" Elev ", new_tank.elevation(),
		" Min level ", new_tank.min_level().value(),
		" Max lev ", new_tank.max_level().value(),
		" Diam ", diam__m, "\n");
#endif
    }

	assert(curr_dv == end_dv);
	return;
}


// -------------------   cost   ------------------- //
auto fep1::cost__exis_pipes(
	const WDS& anytown,
	std::vector<double>::const_iterator start_dv,
    std::vector<double>::const_iterator end_dv,
	const std::vector<bevarmejo::anytown::exi_pipe_option> &pipes_alt_costs
) -> double
{
	double capital_cost = 0.0;

	auto curr_dv = start_dv;
	for (auto&& [id, pipe] : anytown.subnetwork_with_order<WDS::Pipe>(exis_pipes__subnet_name))
	{
		std::size_t action_type = *curr_dv++;
		std::size_t alt_option = *curr_dv++;

		if (action_type == 0) // no action
			continue;
		
		bool city = anytown.id_sequence(city_pipes__subnet_name).contains(id);
		double pipe_cost_per_ft = 0.0;

		if (action_type == 1) // clean
		{
			// The cost per unit of length is a function of the diameter of the pipe.
			// Therefore, I have to search for the diameter in the table.
			double pipe_diam = pipe.diameter().value();
			auto it = std::find_if(pipes_alt_costs.begin(), pipes_alt_costs.end(), 
				[&pipe_diam](const auto& pac) { 
					return std::abs(pac.diameter__in*MperFT/12*1000 - pipe_diam) < 0.0001; 
				});

			// Check I actually found it 
			assert(it != pipes_alt_costs.end());

			if (city)
				pipe_cost_per_ft = it->cost_clean_city__per_ft;
			else
				pipe_cost_per_ft = it->cost_clean_resi__per_ft;
		}
		else // if (action_type == 2) // duplicate
		{
			if (city) 
				pipe_cost_per_ft = pipes_alt_costs.at(alt_option).cost_dup_city__per_ft;
			else
				pipe_cost_per_ft = pipes_alt_costs.at(alt_option).cost_dup_resi__per_ft;
		} 

		capital_cost += pipe_cost_per_ft/MperFT * pipe.length().value(); 
		// again I save the length in mm, but the table is in $/ft
	}

	assert(curr_dv == end_dv);
	return capital_cost;
}
auto fep2::cost__exis_pipes(
	const WDS& anytown,
	std::vector<double>::const_iterator start_dv,
    std::vector<double>::const_iterator end_dv,
	const std::vector<bevarmejo::anytown::exi_pipe_option> &pipes_alt_costs
) -> double
{
	double capital_cost = 0.0;
	
	auto curr_dv = start_dv;
	for (auto&& [id, pipe] : anytown.subnetwork_with_order<WDS::Pipe>(exis_pipes__subnet_name))
	{
		std::size_t action_type = *curr_dv++;
		std::size_t alt_option = action_type-2; // -2 because the first two options are no action and clean

		if (action_type == 0) // no action
			continue;

		bool city = anytown.id_sequence(city_pipes__subnet_name).contains(id);
		double pipe_cost_per_ft = 0.0;

		if (action_type == 1) // clean
		{
			double pipe_diam = pipe.diameter().value();
			auto it = std::find_if(pipes_alt_costs.begin(), pipes_alt_costs.end(), 
				[&pipe_diam](const auto& pac) { 
					return std::abs(pac.diameter__in*MperFT/12*1000 - pipe_diam) < 0.0001; 
				});

			assert(it != pipes_alt_costs.end());
				
			if (city)
				pipe_cost_per_ft = it->cost_clean_city__per_ft;
			else
				pipe_cost_per_ft = it->cost_clean_resi__per_ft;
		}
		else // if (dv >= 2) duplicate
		{
			if (city) 
				pipe_cost_per_ft = pipes_alt_costs.at(alt_option).cost_dup_city__per_ft;
			else
				pipe_cost_per_ft = pipes_alt_costs.at(alt_option).cost_dup_resi__per_ft;
		} 

		capital_cost += pipe_cost_per_ft/MperFT * pipe.length().value(); 
	}

	assert(curr_dv == end_dv);
	return capital_cost;
}

auto fnp1::cost__new_pipes(
	const WDS &anytown,
	std::vector<double>::const_iterator start_dv,
    std::vector<double>::const_iterator end_dv,
	const std::vector<bevarmejo::anytown::new_pipe_option> &pipes_alt_costs
) -> double
{
	// 6 pipes x [prc]
	// This must be installed, thus minimum cost will never be 0.
	double capital_cost = 0.0;
	
	auto curr_dv = start_dv;
	for (auto&& [id, pipe] : anytown.subnetwork_with_order<WDS::Pipe>(new_pipes__subnet_name))
	{
		std::size_t alt_option = *curr_dv++;
		
		double pipe_cost_per_ft = pipes_alt_costs.at(alt_option).cost__per_ft;
		capital_cost += pipe_cost_per_ft/MperFT * pipe.length().value();
	}

	assert(curr_dv == end_dv);
	return capital_cost;
}

auto pgo_dv::cost__energy_per_day(
	const WDS &anytown
) -> double
{
    double total_ene_cost_per_day = 0.0;
	for (const auto& [id, pump] : anytown.pumps() )
	{
		unsigned long t_prec = 0;
		double power_kW_prec = 0.0;
		// at time t, I should multiply the instant energy at t until t+1, or with this single for loop shift by one all indeces
		for (const auto& [t, power_kW] : pump.instant_energy() )
		{
			total_ene_cost_per_day += power_kW_prec * (t - t_prec)/bevarmejo::k__sec_per_hour * bevarmejo::anytown::energy_cost__kWh ; 
			t_prec = t;
			power_kW_prec = power_kW;
		}
	}
	return total_ene_cost_per_day;
}

auto fnt1::cost__tanks(
	const WDS& anytown,
	std::vector<double>::const_iterator start_dv,
    std::vector<double>::const_iterator end_dv,
	const std::vector<bevarmejo::anytown::tank_option> &tank_option,
	const std::vector<bevarmejo::anytown::new_pipe_option> &pipes_alt_costs
) -> double
{
	double capital_cost = 0.0;
	std::unordered_set<std::size_t> already_installed_tanks;

	auto curr_dv = start_dv;
	for(std::size_t i = 0; i < bevarmejo::anytown::max_n_installable_tanks; ++i)
	{
		std::size_t action_type = *curr_dv++;
		std::size_t tank_vol_option = *curr_dv++;
		std::size_t new_tank_loc_shift = action_type-1;

		if (action_type == 0 || (i > 0 && already_installed_tanks.count(new_tank_loc_shift) != 0))
		{
			continue;
		}

		// I don't care where I place it, the cost is always dependent on the volume [dv+1]
		// In this version I can only choose the specific volume from the table and not intermediate values.
		// I also need to account for the cost of the riser pipe (in this case 16 ft so option 5)
		double tank_cost = tank_option.at(tank_vol_option).cost;
		capital_cost += tank_cost;
		
		capital_cost += pipes_alt_costs.at(5).cost__per_ft*bevarmejo::anytown::riser_length__ft;

		already_installed_tanks.insert(new_tank_loc_shift);
	}

	assert(curr_dv == end_dv);
	return capital_cost;
}

auto fnt2::cost__tanks(
	const WDS& anytown,
	std::vector<double>::const_iterator start_dv,
    std::vector<double>::const_iterator end_dv,
	const std::vector<bevarmejo::anytown::tank_option> &tank_options,
	const std::vector<bevarmejo::anytown::new_pipe_option> &new_pipes_options
) -> double
{
	assert(tank_options.size()>2); // otherwise I can't make the interpolation
	assert(new_pipes_options.size() > 1);

	// Simply look for the installed tanks and figure out how much they cost

	double capital_cost = 0.0;
	auto& temp_elems = anytown.id_sequence(label::__temp_elems);

	for (std::size_t i = bevarmejo::anytown::max_n_installable_tanks; i; --i)
	{
		auto t = i-1;
		auto new_tank_id = std::string("T")+std::to_string(t);
		auto riser_id = std::string("Ris_")+std::to_string(t);

		if (temp_elems.contains(new_tank_id)) //  we can assume it also correctly contains riser_id
		{
			// To compute the cost I need the volume of the tank and the diameter of the riser.
			// Then, I use this to get the cost out of tank options and new_pipe_options
			double vol__gal = anytown.tank(new_tank_id).max_volume().value() / bevarmejo::k__m3_per_gal;

			// I need to interpolate it on the tank_options...
			// I should make sure it is in crescent order by volume
			// Extrapolate using boundary slopes for values outside the range.
			// Use linear interpolation between nearest two points.

			auto interp = [](const tank_option& a, const tank_option& b, double vol__gal) {
				double delta_vol = b.volume__gal - a.volume__gal;
				if (delta_vol == 0.0) return (a.cost + b.cost) / 2.0; // avoid div-by-zero
				double t = (vol__gal - a.volume__gal) / delta_vol;
				return a.cost + t * (b.cost - a.cost);
			};

			if (vol__gal < tank_options.front().volume__gal)
			{
				capital_cost += interp(tank_options[0], tank_options[1], vol__gal);
			}
			else if (vol__gal > tank_options.back().volume__gal)
			{
				auto last = tank_options.size() - 1;
				capital_cost += interp(tank_options[last-1], tank_options[last], vol__gal);
			}
			else
			{
				auto it = std::lower_bound(
					tank_options.begin(), tank_options.end(), vol__gal,
					[](const tank_option& t, double v) {
						return t.volume__gal < v;
					});
			
				capital_cost += interp(*(it - 1), *it, vol__gal);
			}
			
			// The riser diameter is defined by the first decison variable (once you removed the do-nothing option)
			std::size_t pos = 0 + t*fnt2::dv_size;
			std::size_t riser_diam_option_idx = *(start_dv+pos)-1.0;

			capital_cost += new_pipes_options.at(riser_diam_option_idx).cost__per_ft*bevarmejo::anytown::riser_length__ft;
		}
	}

	return capital_cost;
}

auto anytown::fnt3::cost__tanks(
    const WDS& anytown,
    std::vector<double>::const_iterator start_dv,
    std::vector<double>::const_iterator end_dv,
    const std::vector<bevarmejo::anytown::tank_option> &tank_options,
    const std::vector<bevarmejo::anytown::new_pipe_option> &new_pipes_options
) -> double
{
    // The cost of installing a tank is independent of the location and needs to
    // take into account only two aspects: its volume and the riser cost.
    // I count the cost of the tank even if it was not installed, so that solutions
    // with a double tank or a ill tank (which don't install the tank in apply_dv
    // for hydraulic motivation) are more expensive than their same counterpart 
    // where the tank is not installed because the OA doesn't want to.
    
    double capital_cost = 0.0;
	
	auto curr_dv = start_dv;
	for(std::size_t i = 0; i < bevarmejo::anytown::max_n_installable_tanks; ++i)
	{
		std::size_t tank_loc_dv = *curr_dv++;
		std::size_t tank_vol_opt_idx = *curr_dv++;
		std::size_t riser_diam_opt_idx = *curr_dv++;
		auto h2d_ratio = *curr_dv++;

		if (tank_loc_dv == 0) { continue; }

		capital_cost += tank_options.at(tank_vol_opt_idx).cost;
		
		capital_cost += new_pipes_options.at(riser_diam_opt_idx).cost__per_ft*bevarmejo::anytown::riser_length__ft;
	}

	assert(curr_dv == end_dv);
	return capital_cost;
}

// ------------------- of__reliability ------------------- //

// ------------------- reset_dv ------------------- //
void fep1::reset_dv__exis_pipes(
	WDS &anytown,
	std::vector<double>::const_iterator start_dv,
    std::vector<double>::const_iterator end_dv,
	const std::unordered_map<std::string, double> &old_HW_coeffs)
{
	auto curr_dv = start_dv;
	for (auto&& [id, pipe] : anytown.subnetwork_with_order<WDS::Pipe>(exis_pipes__subnet_name))
	{	
		std::size_t action_type = *curr_dv++;
		std::size_t alt_option = *curr_dv++;

		if (action_type == 0) // no action
		{
			continue;
		}

		else if (action_type == 1) // clean
		{
			// reset the HW coefficients
			int errorcode = EN_setlinkvalue(anytown.ph_, pipe.EN_index(), EN_ROUGHNESS, old_HW_coeffs.at(id));
			assert(errorcode <= 100);

			pipe.roughness(old_HW_coeffs.at(id));
		}

		else // if (action_type == 2) // duplicate
		{
			// duplicate pipe has been named Dxx where xx is the original pipe name
			auto dup_pipe_id = std::string("D")+id;

			int errorcode = EN_deletelink(anytown.ph_, anytown.pipe(dup_pipe_id).EN_index(), EN_UNCONDITIONAL);
			assert(errorcode <= 100);

			anytown.id_sequence(label::__temp_elems).erase(dup_pipe_id);
			anytown.uninstall_pipe(dup_pipe_id);
			anytown.cache_indices();
		}
	}
	
	assert(curr_dv == end_dv);
	return;
}
void fep2::reset_dv__exis_pipes(
	WDS &anytown,
	std::vector<double>::const_iterator start_dv,
    std::vector<double>::const_iterator end_dv,
	const std::unordered_map<std::string, double> &old_HW_coeffs)
{
	auto curr_dv = start_dv;
	for (auto&& [id, pipe] : anytown.subnetwork_with_order<WDS::Pipe>(exis_pipes__subnet_name))
	{
		std::size_t dv = *curr_dv++;
		std::size_t alt_option = dv-2; // -2 because the first two options are no action and clean

		if (dv == 0)
		{
			continue;
		}

		else if (dv == 1) // clean
		{
			// reset the HW coefficients
			int errorcode = EN_setlinkvalue(anytown.ph_, pipe.EN_index(), EN_ROUGHNESS, old_HW_coeffs.at(id));
			assert(errorcode <= 100);

			pipe.roughness(old_HW_coeffs.at(id));
		}

		else // if (dv >= 2) // duplicate
		{
			// duplicate pipe has been named Dxx where xx is the original pipe name
			auto dup_pipe_id = std::string("D")+id;

			int errorcode = EN_deletelink(anytown.ph_, anytown.pipe(dup_pipe_id).EN_index(), EN_UNCONDITIONAL);
			assert(errorcode <= 100);

			anytown.id_sequence(label::__temp_elems).erase(dup_pipe_id);
			anytown.uninstall_pipe(dup_pipe_id);
			anytown.cache_indices();
		}
	}
	
	assert(curr_dv == end_dv);
	return;
}

void fnp1::reset_dv__new_pipes(
	WDS& anytown,
	std::vector<double>::const_iterator start_dv,
    std::vector<double>::const_iterator end_dv)
{
	auto curr_dv = start_dv;
	for (auto&& [id, pipe] : anytown.subnetwork_with_order<WDS::Pipe>(new_pipes__subnet_name))
	{
		int errorcode = EN_setlinkvalue(anytown.ph_, pipe.EN_index(), EN_DIAMETER, bevarmejo::anytown::nonexisting_pipe_diam__ft);
		assert(errorcode <= 100);

		pipe.diameter(bevarmejo::anytown::nonexisting_pipe_diam__ft); // it's ok also in ft because its' super small small
	}
}

void pgo_dv::reset_dv__pumps(
	WDS& anytown,
	std::vector<double>::const_iterator start_dv,
    std::vector<double>::const_iterator end_dv)
{
	for (std::size_t i = 0; i < 3; ++i)
	{
		// I know pump patterns IDs are from 2, 3, and 4
		int pump_idx = i + 2;
		std::string pump_id = std::to_string(pump_idx);
		int errorcode = EN_getpatternindex(anytown.ph_, pump_id.c_str(), &pump_idx);
		assert(errorcode <= 100);

		// set the pattern, use empty array of 24 double to zero
		errorcode = EN_setpattern(anytown.ph_, pump_idx, std::vector<double>(24, .0).data(), 24);
		assert(errorcode <= 100);
	}
}

void fnt1::reset_dv__tanks(
	WDS& anytown,
	std::vector<double>::const_iterator start_dv,
    std::vector<double>::const_iterator end_dv)
{
	std::unordered_set<std::size_t> already_installed_tanks;

	auto curr_dv = start_dv;
	for(std::size_t i = 0; i < bevarmejo::anytown::max_n_installable_tanks; ++i)
	{
		std::size_t action_type = *curr_dv++;
		std::size_t tank_vol_option = *curr_dv++;
		std::size_t new_tank_loc_shift = action_type-1;
		
		if (action_type == 0 || (i > 0 && already_installed_tanks.count(new_tank_loc_shift) != 0))
		{
			continue;
		}
		// else if (action_type > 1) // install
		auto new_tank_id = std::string("T")+std::to_string(i);
		auto riser_id = std::string("Ris_")+std::to_string(i);

		// remove the new tank and the the riser is automatically deleted 
		int errorcode = EN_deletenode(anytown.ph_, anytown.tank(new_tank_id).EN_index(), EN_UNCONDITIONAL);
		assert(errorcode <= 100);

		anytown.id_sequence(label::__temp_elems).erase(new_tank_id);
		anytown.id_sequence(label::__temp_elems).erase(riser_id);
		anytown.remove_tank(new_tank_id);
		anytown.cache_indices();

		already_installed_tanks.insert(new_tank_loc_shift);
	}

	assert(curr_dv == end_dv);
	return;
}

auto fnt2::reset_dv__tanks(
	WDS& anytown,
	std::vector<double>::const_iterator start_dv,
    std::vector<double>::const_iterator end_dv
) -> void
{
	auto& temp_elems = anytown.id_sequence(label::__temp_elems);
	for (std::size_t i = max_n_installable_tanks; i; --i)
	{
		auto new_tank_id = std::string("T")+std::to_string(i-1);
		auto riser_id = std::string("Ris_")+std::to_string(i-1);

		if (temp_elems.contains(new_tank_id)) //  we can assume it also correctly contains riser_id
		{
			// remove the new tank and the the riser is automatically deleted 
			int errorcode = EN_deletenode(anytown.ph_, anytown.tank(new_tank_id).EN_index(), EN_UNCONDITIONAL);
			assert(errorcode <= 100);

			temp_elems.erase(new_tank_id);
			temp_elems.erase(riser_id);
			anytown.remove_tank(new_tank_id); // also removing the links too
			anytown.cache_indices();
		}
	}
}

auto anytown::fnt3::reset_dv__tanks(
    WDS& a_anytown_sys,
    std::vector<double>::const_iterator start_dv,
    std::vector<double>::const_iterator end_dv
) -> void
{
    auto& temp_elems = a_anytown_sys.id_sequence(label::__temp_elems);
	for (std::size_t i = max_n_installable_tanks; i; --i)
	{
		auto new_tank_id = std::string("T")+std::to_string(i-1);
		auto riser_id = std::string("Ris_")+std::to_string(i-1);

		if (temp_elems.contains(new_tank_id)) //  we can assume it also correctly contains riser_id
		{
			// remove the new tank and the the riser is automatically deleted 
			int errorcode = EN_deletenode(a_anytown_sys.ph_, a_anytown_sys.tank(new_tank_id).EN_index(), EN_UNCONDITIONAL);
			assert(errorcode <= 100);

			temp_elems.erase(new_tank_id);
			temp_elems.erase(riser_id);
			a_anytown_sys.remove_tank(new_tank_id); // also removing the links too
			a_anytown_sys.cache_indices();
		}
	}
}

// ------------------- 1st level ------------------- //
// -------------------   Bounds  ------------------- //
auto Problem::get_bounds() const -> std::pair<std::vector<double>, std::vector<double>>
{
	std::vector<double> lb;
	std::vector<double> ub;

	const auto append_bounds= [&lb, &ub](auto&& bounds_func, auto&&... args) {
		auto [lower, upper] = bounds_func(std::forward<decltype(args)>(args)...);
		lb.insert(lb.end(), lower.begin(), lower.end());
		ub.insert(ub.end(), upper.begin(), upper.end());
	};

	if (m__has_design && m__exi_pipes_formulation == ExistingPipesFormulation::FarmaniEtAl2005)
	{
		append_bounds(fep1::bounds__exis_pipes,
			std::as_const(*m__anytown).subnetwork_with_order<WDS::Pipe>(exis_pipes__subnet_name),
			m__exi_pipe_options
		);
	}
	if (m__has_design && m__exi_pipes_formulation == ExistingPipesFormulation::Combined)
	{
		append_bounds(fep2::bounds__exis_pipes,
			std::as_const(*m__anytown).subnetwork_with_order<WDS::Pipe>(exis_pipes__subnet_name),
			m__exi_pipe_options
		);
	}

	if (m__has_design)
	{
		append_bounds(fnp1::bounds__new_pipes,
			std::as_const(*m__anytown).subnetwork_with_order<WDS::Pipe>(new_pipes__subnet_name),
			m__new_pipe_options
		);
	}

	if (m__has_operations)
	{
		append_bounds(pgo_dv::bounds__pumps,
			std::as_const(*m__anytown).pumps()
		);
	}

	if (m__has_design && m__new_tanks_formulation == NewTanksFormulation::Simple)
	{
		append_bounds(fnt1::bounds__tanks,
			std::as_const(*m__anytown).subnetwork_with_order<WDS::Junction>(pos_tank_loc__subnet_name),
			m__tank_options
		);
	}

	if (m__has_design && m__new_tanks_formulation == NewTanksFormulation::FarmaniEtAl2005)
	{
		append_bounds(fnt2::bounds__tanks,
			std::as_const(*m__anytown).subnetwork_with_order<WDS::Junction>(pos_tank_loc__subnet_name),
			m__tank_options,
			m__new_pipe_options
		);
	}

	if (m__has_design && m__new_tanks_formulation == NewTanksFormulation::LocVolRisDiamH2DRatio)
	{
		append_bounds(fnt3::bounds__tanks,
			std::as_const(*m__anytown).subnetwork_with_order<WDS::Junction>(pos_tank_loc__subnet_name),
			m__tank_options,
			m__new_pipe_options
		);
	}

	// We added the bound in the beme order, so now we use the adpater to map them to the pagmo order.
	return {m__dv_adapter.from_beme_to_pagmo(lb), m__dv_adapter.from_beme_to_pagmo(ub)};
}

// ------------------- 2nd level ------------------- //
auto fep1::bounds__exis_pipes(
	InputOrderedRegistryView<WDS::Pipe> exis_pipes,
	const std::vector<bevarmejo::anytown::exi_pipe_option> &pipes_alt_costs
) -> std::pair<std::vector<double>, std::vector<double>>
{
// Structure of the decision variables:
// [35 pipes x [action, pra]
// action: 3 options -> 0 - do nothing, 1 duplicate, 2 - clean 
// pra: 10 alternative in pipe_rehab_cost -> 0 - 9 

	assert(exis_pipes.size() == 35);
	assert(pipes_alt_costs.size() == 10);

	auto dv_size= 2ul*exis_pipes.size();
	double n_pra= pipes_alt_costs.size();

	std::vector<double> lb(dv_size, 0.0);
	std::vector<double> ub(dv_size, 0.0);
	for (std::size_t i = 0; i < dv_size; i+=2)
	{
		ub[i] = 2.0;
		ub[i+1] = n_pra-1;
	}

	return std::make_pair(lb, ub);
}
auto fep2::bounds__exis_pipes(
	InputOrderedRegistryView<WDS::Pipe> exis_pipes,
	const std::vector<bevarmejo::anytown::exi_pipe_option> &pipes_alt_costs
) -> std::pair<std::vector<double>, std::vector<double>>
{
// Structure of the decision variables:
// [35 pipes x action]
// action: 2+pra options -> 0 - do nothing, 1 - clean, 2- pra
// pra: 10 alternative in pipe_rehab_cost -> 0 - 9 

	assert(exis_pipes.size() == 35);
	assert(pipes_alt_costs.size() == 10);

	auto dv_size= exis_pipes.size();
	double n_actions= 2+pipes_alt_costs.size();

	return std::make_pair(std::vector<double>(dv_size, 0), std::vector<double>(dv_size, n_actions-1));
}

auto fnp1::bounds__new_pipes(
	InputOrderedRegistryView<WDS::Pipe> new_pipes,
	const std::vector<bevarmejo::anytown::new_pipe_option> &pipes_alt_costs
) -> std::pair<std::vector<double>, std::vector<double>>
{
// Structure of the decision variables:
// 6 pipes x [pra]
// pra: 10 alternative in pipe_rehab_cost -> 0 - 9 
	assert(new_pipes.size() == 6);
	assert(pipes_alt_costs.size() == 10);

	auto dv_size= new_pipes.size();
	double n_pra= pipes_alt_costs.size();

	std::vector<double> lb(dv_size, 0.0);
	std::vector<double> ub(dv_size, n_pra-1);

    return std::make_pair(lb, ub);
}

auto pgo_dv::bounds__pumps(
	InputExcludingRegistryView<WDS::Pump> pumps
) -> std::pair<std::vector<double>, std::vector<double>>
{
// Structure of the decision variables:
// 24 hours x [npr]
// npr: 4 options indicate the number of pumps running -> 0 - 3
	assert(pumps.size() == 3);

	auto dv_size= 24;
	double n_npr= 3.0;

	std::vector<double> lb(dv_size, 0.0);
	std::vector<double> ub(dv_size, n_npr);

	return std::make_pair(lb, ub);
}

auto fnt1::bounds__tanks(
	InputOrderedRegistryView<WDS::Junction> tank_locs,
	const std::vector<bevarmejo::anytown::tank_option> &tank_option
) -> std::pair<std::vector<double>, std::vector<double>>
{
// Structure of the decision variables:
// 2 tanks x [tpl, tvol]
// tpl: tank possible location nodes (plus 0, do nothing) -> 0 - x
// tvol: 5 discrete tank volume possible -> 0 - 4 (to be transformed in continuous between the limits)

	assert(tank_locs.size() == 17);
	assert(tank_option.size() == 5);

	auto dv_size= 2*bevarmejo::anytown::max_n_installable_tanks;
	double n_tpl= tank_locs.size();
	double n_tvol= tank_option.size();

	std::vector<double> lb(dv_size, 0.0);
	std::vector<double> ub(dv_size, 0.0);
	for (std::size_t i = 0; i < dv_size; i+=2)
	{
		ub[i] = n_tpl; // 0 is a valid option, so no minus one
		ub[i+1] = n_tvol-1;
	}

	return std::make_pair(lb, ub);
}

auto fnt2::bounds__tanks(
	InputOrderedRegistryView<WDS::Junction> tank_locs,
	const std::vector<bevarmejo::anytown::tank_option> &tank_options,
	const std::vector<bevarmejo::anytown::new_pipe_option> &new_pipe_options
) -> std::pair<std::vector<double>, std::vector<double>>
{
	assert(tank_locs.size() == 17);
	assert(tank_options.size() == 5);
	assert(new_pipe_options.size() == 10);

	auto dv_size = fnt2::dv_size*bevarmejo::anytown::max_n_installable_tanks;
	double n_tpl = tank_locs.size(); // number of tanks possible locations
	double n_rpd = new_pipe_options.size(); // number of risers possible diameters
	
	std::vector<double> lb(dv_size, 0.0);
	std::vector<double> ub(dv_size, 0.0);
	
	// The order is (see apply_dv) 
	for (auto i = 0; i < max_n_installable_tanks; ++i)
	{
		auto j = i*fnt2::dv_size;
		// riser possible diameters + do nothing option
		lb[0+j] = 0.0;
		ub[0+j] = n_rpd;
		// Tanks location index
		lb[1+j] = 0.0;
		ub[1+j] = n_tpl-1;
		// diameter
		lb[2+j] = fnt2::tank_diam_lb__m;
		ub[2+j] = fnt2::tank_diam_ub__m;
		// overflow elev
		lb[3+j] = fnt2::tank_hmax_lb__m;
		ub[3+j] = fnt2::tank_hmax_ub__m;
		// min elev 
		lb[4+j] = fnt2::tank_hmin_lb__m;
		ub[4+j] = fnt2::tank_hmin_ub__m;
		// safety level
		lb[5+j] = fnt2::tank_safetyl_lb__m;
		ub[5+j] = fnt2::tank_safetyl_ub__m;
	}

	return std::make_pair(std::move(lb), std::move(ub));
}

auto anytown::fnt3::bounds__tanks(
    InputOrderedRegistryView<WDS::Junction> tank_locs,
    const std::vector<bevarmejo::anytown::tank_option> &tank_options,
    const std::vector<bevarmejo::anytown::new_pipe_option> &new_pipe_options
) -> std::pair<std::vector<double>, std::vector<double>>
{
    std::size_t dv_size = anytown::fnt3::dv_size*bevarmejo::anytown::max_n_installable_tanks;
    std::vector<double> lb(dv_size, 0.0);
	std::vector<double> ub(dv_size, 0.0);

    // The order is (see apply_dv) 
	for (auto i = 0; i < bevarmejo::anytown::max_n_installable_tanks; ++i)
	{
		auto j = i*anytown::fnt3::dv_size;
		// Tanks location index + do nothing option
		lb[0+j] = 0.0;
		ub[0+j] = anytown::pos_tank_loc__el_names.size();
		// Tanks volume index
		lb[1+j] = 0.0;
		ub[1+j] = tank_options.size()-1;
		// Riser diameter index
		lb[2+j] = 0;
		ub[2+j] = new_pipe_options.size()-1;
		// Heigh-to-diameter ratio index
		lb[3+j] = 0;
		ub[3+j] = anytown::fnt3::h2d_ratio__steps;
	}

	return std::make_pair(std::move(lb), std::move(ub));
}

// ------------------- 1st level ------------------- //
// -------------------   Save  ------------------- //
void Problem::save_solution(const std::vector<double>& pagmo_dv, const fsys::path& out_file) const
{
	auto dvs = m__dv_adapter.from_pagmo_to_beme(pagmo_dv);

	apply_dv(dvs);

	int errco = EN_saveinpfile(this->m__anytown->ph_, out_file.string().c_str());
	assert(errco <= 100);

	reset_dv(dvs);
}

void to_json(Json& j, const bevarmejo::anytown::Problem &prob)
{
	// Reset, just in case.
	j = Json{};

	j[io::key::exi_pipe_opts()] = prob.m__exi_pipe_options;
	j[io::key::new_pipe_opts()] = prob.m__new_pipe_options;
	j[io::key::tank_opts()] = prob.m__tank_options;

	if (!prob.m__has_operations && prob.m__formulation != Formulation::twoph_f1)
	{
		// I need to merge the pumping patterns
		std::vector<double> pumpgroup_pattern(24, 0.0);
		for (const auto& [id, pump] : prob.m__anytown->pumps()) {
			auto pump_pattern_idx = pump.speed_pattern()->EN_index();
			for (std::size_t i = 1; i <= 24; ++i) {
				double val = 0.0;
				int errorcode = EN_getpatternvalue(prob.m__anytown->ph_, pump_pattern_idx, i, &val);
				pumpgroup_pattern[i-1] += val;
			}
		}
		j[io::key::opers()] = pumpgroup_pattern;
	}
		
	j[io::key::at_inp()] = prob.m__anytown_filename;
	j[io::key::at_subnets()] = Json{};
	for (const auto& [seq_name, names_in_seq] : prob.m__anytown->id_sequences())
	{
		j[io::key::at_subnets()][seq_name] = names_in_seq;
	}

	// Remove the temporary elements
	j[io::key::at_subnets()].erase(label::__temp_elems);

	j["extra_info"] = prob.get_extra_info();

	if (prob.m__reliability_obj_func_formulation == ReliabilityObjectiveFunctionFormulation::HierarchicalWithMaxVelocity)
	{
		j[io::key::max_vel()] = prob.m__max_velocity__m_per_s;
	}
}

} // namespace anytown
} // namespace bevarmejo
