// 
//  model_anytown.cpp
//	anytown original test
//
//  Created by Dennis Zanutto on 11/07/23.

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

#include "problems/anytown.hpp"

namespace bevarmejo {
namespace anytown {

namespace io::key
{
#if BEME_VERSION < 240601
static constexpr bemeio::AliasedKey at_inp {"WDS inp"}; // "AT inp"
static constexpr bemeio::AliasedKey at_subnets {"WDS UDEGs"}; // "AT subnets"
static constexpr bemeio::AliasedKey exi_pipe_opts {"Existing pipe options"}; // "Existing pipe options"
static constexpr bemeio::AliasedKey new_pipe_opts {"New pipe options"}; // "New pipe options"
static constexpr bemeio::AliasedKey tank_opts {"Tank costs"}; // "Tank options"
static constexpr bemeio::AliasedKey opers {"Operations"}; // "Pump group operations"
#else
static constexpr bemeio::AliasedKey at_inp {"AT inp"}; // "AT inp"
static constexpr bemeio::AliasedKey at_subnets {"AT subnets"}; // "AT subnets"
static constexpr bemeio::AliasedKey exi_pipe_opts {"Existing pipe options"}; // "Existing pipe options"
static constexpr bemeio::AliasedKey new_pipe_opts {"New pipe options"}; // "New pipe options"
static constexpr bemeio::AliasedKey tank_opts {"Tank options"}; // "Tank options"
static constexpr bemeio::AliasedKey opers {"Pump group operations"}; // "Pump group operations"
#endif
} // namespace key
// Values for the allowed formulations in the json file.
namespace io::value {
static const std::string rehab_f1 = "rehab::f1";
static const std::string mixed_f1 = "mixed::f1";
static const std::string opertns_f1 = "operations::f1";
static const std::string twoph_f1 = "twophases::f1";
static const std::string rehab_f2 = "rehab::f2";
static const std::string mixed_f2 = "mixed::f2";
} // namespace io::value

// Extra information for the formulations.
namespace io::other {
static const std::string rehab_f1_exinfo =  "Anytown Rehabilitation Formulation 1\nOperations from input, pipes as in Farmani, Tanks as in Vamvakeridou-Lyroudia but discrete)\n";
static const std::string mixed_f1_exinfo =  "Anytown Mixed Formulation 1\nOperations as dv, pipes as in Farmani, Tanks as in Vamvakeridou-Lyroudia but discrete)\n";
static const std::string opertns_f1_exinfo = "Anytown Operations-only problem. Pure 24-h scheduling.\n";
static const std::string twoph_f1_exinfo = "Anytown Rehabilitation Formulation 1\nPipes as in Farmani, Tanks as in Vamvakeridou-Lyroudia but discrete, operations optimized internally)\n";
static const std::string rehab_f2_exinfo =  "Anytown Rehabilitation Formulation 2\nOperations from input, pipes as single dv, Tanks as in Vamvakeridou-Lyroudia (but discrete)\n";
static const std::string mixed_f2_exinfo =  "Anytown Mixed Formulation 2\nOperations as dv, pipes as single dv, Tanks as in Vamvakeridou-Lyroudia (but discrete)\n";
}

std::vector<std::vector<double>> decompose_pumpgroup_pattern(std::vector<double> pg_pattern, const std::size_t n_pumps) {
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
 
Problem::Problem(std::string_view a_formulation_str, const Json& settings, const bemeio::Paths &lookup_paths)
{
	if (a_formulation_str == io::value::rehab_f1)
	{
		m__formulation = Formulation::rehab_f1;
		m__extra_info = io::other::rehab_f1_exinfo;
	}
	else if (a_formulation_str == io::value::mixed_f1)
	{
		m__formulation = Formulation::mixed_f1;
		m__extra_info = io::other::mixed_f1_exinfo;
	}
	else if (a_formulation_str == io::value::opertns_f1)
	{
		m__formulation = Formulation::opertns_f1;
		m__extra_info = io::other::opertns_f1_exinfo;
	}
	else if (a_formulation_str == io::value::twoph_f1)
	{
		beme_throw(std::invalid_argument, "Impossible to construct the anytown Problem.",
			"Formulation 1 of twophase problem is not supported anymore.");
	}
	else if (a_formulation_str == bevarmejo::anytown::io::value::rehab_f2)
	{
		m__formulation = Formulation::rehab_f2;
		m__extra_info = io::other::rehab_f2_exinfo;
	}
	else if (a_formulation_str == bevarmejo::anytown::io::value::mixed_f2)
	{
		m__formulation = Formulation::mixed_f2;
		m__extra_info = io::other::mixed_f2_exinfo;
	}
	else
	{
		beme_throw(std::invalid_argument, "Impossible to construct the anytown Problem.",
			"The provided Anytown formulation is not yet implemented.");
	}
	m__name = bemeio::log::nname::beme_l+nname+std::string(a_formulation_str);

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

	load_network(settings, lookup_paths, fix_inp);

	if (m__formulation != Formulation::opertns_f1) {
		// Custom made subnetworks for the temporary elements 
		m__anytown->submit_id_sequence(label::__temp_elems);

		load_other_data(settings, lookup_paths);
	}

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

void Problem::load_network(const Json& settings, const bemeio::Paths& lookup_paths, std::function<void (EN_Project)> preprocessf)
{
	assert(settings != nullptr &&
		io::key::at_inp.exists_in(settings) &&
		io::key::at_subnets.exists_in(settings)
	);
	const auto inp_filename = settings.at(io::key::at_inp.as_in(settings)).get<fsys::path>();

	// Check the existence of the inp_filename in any of the lookup paths and its extension
	m__anytown = std::make_shared<WDS>(
		bemeio::locate_file</* log = */true>(inp_filename, lookup_paths), 
		preprocessf
	);
	m__anytown_filename = inp_filename.string();

	// Load the sequences of names for the subnetworks
	// If is in array of strings it is the name of the files, otherwise it is a json object with the data.

	auto j_names = Json{}; // Json for the named sequences of names
	bemeio::expand_if_filepaths(settings.at(io::key::at_subnets.as_in(settings)), lookup_paths, 
		j_names);

	for (const auto& [seq_name, names_in_seq] : j_names.items())
	{
		m__anytown->submit_id_sequence(seq_name, names_in_seq.get<std::vector<std::string>>());
	}
	
}

void Problem::load_other_data(const Json& settings, const bemeio::Paths& lookup_paths) {
	assert(
		settings != nullptr && 
		io::key::exi_pipe_opts.exists_in(settings) && 
		io::key::new_pipe_opts.exists_in(settings) &&
		io::key::tank_opts.exists_in(settings)
	);
	if (m__formulation == Formulation::rehab_f1 || m__formulation == Formulation::rehab_f2)
		assert(io::key::opers.exists_in(settings));

	// If the settings is a string it means it is a filename, otherwise it shuold 
	// be a json array with the data.
	auto j_aep = Json{}; // Json for the available existing pipes
	bemeio::expand_if_filepath(settings.at(io::key::exi_pipe_opts.as_in(settings)), lookup_paths, 
		j_aep);
	
	m__exi_pipe_options = j_aep.get<std::vector<anytown::exi_pipe_option>>();
	
	auto j_anp = Json{}; // Json for the available new pipes
	bemeio::expand_if_filepath(settings.at(io::key::new_pipe_opts.as_in(settings)), lookup_paths, 
		j_anp);

	m__new_pipe_options = j_anp.get<std::vector<anytown::new_pipe_option>>();
	
	auto j_tanks = Json{}; // Json for the tanks costs
	bemeio::expand_if_filepath(settings.at(io::key::tank_opts.as_in(settings)), lookup_paths, 
		j_tanks);
		
	m__tank_options = j_tanks.get<std::vector<anytown::tank_option>>();

	if (m__formulation == Formulation::rehab_f1 || m__formulation == Formulation::rehab_f2)
	{
		// Need to se the operations for the rehabilitation problems
		auto j_oper = Json{}; // Json for the operations
		bemeio::expand_if_filepath(settings.at(io::key::opers.as_in(settings)), lookup_paths, 
			j_oper);
		auto operations = j_oper.get<std::vector<double>>();
		assert(operations.size() == 24);

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
}

// ------------------- Pagmo functions ------------------- //
// For now, all formulations have a fixed number of objectives, constraints, but different number of variables
std::vector<double>::size_type Problem::get_nobj() const {
	return 2ul; 
}

std::vector<double>::size_type Problem::get_nec() const {
	return 0ul;
}

std::vector<double>::size_type Problem::get_nic() const {
	return 0ul;
}

std::vector<double>::size_type Problem::get_nix() const {
	switch (m__formulation)
	{
	case Formulation::rehab_f1:
		[[fallthrough]];
	case Formulation::twoph_f1:
		return 80ul;
	case Formulation::mixed_f1:
		return 104ul;
	case Formulation::opertns_f1:
		return 24ul;
	case Formulation::rehab_f2:
		return 45ul;
	case Formulation::mixed_f2:
		return 69ul;
	default:
		return 0ul;
	}
}

// ------------------- 1st level ------------------- //
std::vector<double> Problem::fitness(const std::vector<double>& dvs) const {
	// things to do 
	// 1. EPS 
	//   [x]   apply dvs to the network
	// 	 [x]	run EPS as it is
	// 	 [x]	check energy consumption
	// 	 [x]	check pressure for reliability
	// 	 [x]	check min pressure constraint
	// 	 [ ]	check tanks complete emptying and filling
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


	// For now, instead of creating a copy object, I will just apply the changes to the network and then reset it.
	// This doesn't allow parallelization unless you have different copies of the problem.
	// For duplicate pipes they will have name Dxx where xx is the original pipe name.
	// For cleaned pipes I write on a vector the original HW coefficients and then I reset them.
	// For new pipes and pumps (dvs from 71 onwards) I don't need to reset them as they are always overwritten.

	// 1. EPS
	//auto anytown_temp = m__anytown->clone(); 
	// in the future this will be a perfect copy and I will be able to call 
	// everything in a new thread and then simply discard it.
	
	apply_dv(m__anytown, dvs);

	sim::solvers::epanet::HydSimSettings settings;

	long h_step = 0;
    int errorcode = EN_gettimeparam(m__anytown->ph(), EN_HYDSTEP, &h_step);
    assert(errorcode < 100);
    long r_step = 0;
    errorcode = EN_gettimeparam(m__anytown->ph(), EN_REPORTSTEP, &r_step);
    assert(errorcode < 100);
    long horizon = 0;
    errorcode = EN_gettimeparam(m__anytown->ph(), EN_DURATION, &horizon);
    assert(errorcode < 100);

	settings.resolution(h_step);
	settings.report_resolution(r_step);
	settings.horizon(horizon);

	auto results = sim::solvers::epanet::solve_hydraulics(*m__anytown, settings);

	if (!sim::solvers::epanet::is_successful_with_warnings(results))
	{
		bemeio::stream_out( std::cerr, "Error in the hydraulic simulation. \n");
		reset_dv(m__anytown, dvs);
		return std::vector<double>(get_nobj()+get_nec()+get_nic(), 
					std::numeric_limits<double>::max());
	}

	// Compute OF on res. 
	std::vector<double> fitv= {
		cost(*m__anytown, dvs), // cost is positive when money is going out, like in this case
		of__reliability(*m__anytown) // Special modification of the reliability function
	};
	
	reset_dv(m__anytown, dvs);
	return fitv;
}

// ------------------- 2nd level ------------------- //
void Problem::apply_dv(std::shared_ptr<bevarmejo::WaterDistributionSystem> anytown, const std::vector<double>& dvs) const {
	anytown->cache_indices();

	switch (m__formulation)
	{
	case Formulation::rehab_f1: {
		fep1::apply_dv__exis_pipes(*anytown, __old_HW_coeffs, std::vector(dvs.begin(), dvs.begin()+70), m__exi_pipe_options);
		apply_dv__new_pipes(*anytown, std::vector(dvs.begin()+70, dvs.begin()+76), m__new_pipe_options);
		// No pump apply
		fnt1::apply_dv__tanks(*anytown, std::vector(dvs.begin()+76, dvs.end()), m__tank_options);
		return;
	}
	case Formulation::mixed_f1: {
		fep1::apply_dv__exis_pipes(*anytown, __old_HW_coeffs, std::vector(dvs.begin(), dvs.begin()+70), m__exi_pipe_options);
		apply_dv__new_pipes(*anytown, std::vector(dvs.begin()+70, dvs.begin()+76), m__new_pipe_options);
		apply_dv__pumps(*anytown, std::vector(dvs.begin()+76, dvs.begin()+100));
		fnt1::apply_dv__tanks(*anytown, std::vector(dvs.begin()+100, dvs.end()), m__tank_options);
		return;
	}
	case Formulation::twoph_f1: {
		fep1::apply_dv__exis_pipes(*anytown, __old_HW_coeffs, std::vector(dvs.begin(), dvs.begin()+70), m__exi_pipe_options);
		apply_dv__new_pipes(*anytown, std::vector(dvs.begin()+70, dvs.begin()+76), m__new_pipe_options);
		fnt1::apply_dv__tanks(*anytown, std::vector(dvs.begin()+76, dvs.end()), m__tank_options);

		// Forward these changes also to the internal optimization problem
		auto udp = m_pop.get_problem().extract<bevarmejo::anytown::Problem>();
		assert(udp != nullptr);
		udp->m__anytown = this->m__anytown;

		// I need to force the re-evaluation of the current solutions in the population
		// So I create a "new population", that has the same elements of the previous one,
		// but now, since the problem has changed, it will re-evaluate the solutions.
		// I will then replace the old population with the new one.
		// Super light to copy the internal problem as it only has the internal shared pointer to the network
		pagmo::population new_pop( *udp /*, pop_size = 0, seed = m_pop.get_seed() */); 
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
		apply_dv__pumps(*anytown, m_pop.get_x().at(idx.front()));
		return;
	}
	case Formulation::opertns_f1: {
		apply_dv__pumps(*anytown, dvs);
		return;
	}
	case Formulation::rehab_f2: {
		fep2::apply_dv__exis_pipes(*anytown, __old_HW_coeffs, std::vector(dvs.begin(), dvs.begin()+35), m__exi_pipe_options);
		apply_dv__new_pipes(*anytown, std::vector(dvs.begin()+35, dvs.begin()+41), m__new_pipe_options);
		// No pumps apply
		fnt1::apply_dv__tanks(*anytown, std::vector(dvs.begin()+41, dvs.end()), m__tank_options);
		return;
	}
	case Formulation::mixed_f2: {
		fep2::apply_dv__exis_pipes(*anytown, __old_HW_coeffs, std::vector(dvs.begin(), dvs.begin()+35), m__exi_pipe_options);
		apply_dv__new_pipes(*anytown, std::vector(dvs.begin()+35, dvs.begin()+41), m__new_pipe_options);
		apply_dv__pumps(*anytown, std::vector(dvs.begin()+41, dvs.begin()+65));
		fnt1::apply_dv__tanks(*anytown, std::vector(dvs.begin()+65, dvs.end()), m__tank_options);
	}
	default:
		break;
	}

	return;
}

double Problem::cost(const WDS &anytown,  const std::vector<double> &dvs) const {

	if (m__formulation == Formulation::opertns_f1) {
		// Just the operational cost!
		return cost__energy_per_day(anytown);
	}

	// If it is a design or integrated problem, we need to consider design cost and net present value of the energy cost.
	double capital_cost = 0.0;
	
	switch (m__formulation)
	{
	case Formulation::rehab_f1:
		[[fallthrough]];
	case Formulation::twoph_f1: {
		capital_cost += fep1::cost__exis_pipes(anytown, std::vector(dvs.begin(), dvs.begin()+70), m__exi_pipe_options);
		capital_cost += cost__new_pipes(anytown, std::vector(dvs.begin()+70, dvs.begin()+76), m__new_pipe_options);
		capital_cost += fnt1::cost__tanks(anytown, std::vector(dvs.begin()+76, dvs.end()), m__tank_options, m__new_pipe_options);
		break;
	}
	case Formulation::mixed_f1: {
		capital_cost += fep1::cost__exis_pipes(anytown, std::vector(dvs.begin(), dvs.begin()+70), m__exi_pipe_options);
		capital_cost += cost__new_pipes(anytown, std::vector(dvs.begin()+70, dvs.begin()+76), m__new_pipe_options);
		capital_cost += fnt1::cost__tanks(anytown, std::vector(dvs.begin()+100, dvs.end()), m__tank_options, m__new_pipe_options);
		break;
	}
	case Formulation::rehab_f2: {
		capital_cost += fep2::cost__exis_pipes(anytown, std::vector(dvs.begin(), dvs.begin()+35), m__exi_pipe_options);
		capital_cost += cost__new_pipes(anytown, std::vector(dvs.begin()+35, dvs.begin()+41), m__new_pipe_options);
		capital_cost += fnt1::cost__tanks(anytown, std::vector(dvs.begin()+41, dvs.end()), m__tank_options, m__new_pipe_options);
		break;
	}
	case Formulation::mixed_f2: {
		capital_cost += fep2::cost__exis_pipes(anytown, std::vector(dvs.begin(), dvs.begin()+35), m__exi_pipe_options);
		capital_cost += cost__new_pipes(anytown, std::vector(dvs.begin()+35, dvs.begin()+41), m__new_pipe_options);
		capital_cost += fnt1::cost__tanks(anytown, std::vector(dvs.begin()+65, dvs.end()), m__tank_options, m__new_pipe_options);
		break;
	}
	default:
		break;
	}
		
	double energy_cost_per_day = cost__energy_per_day(anytown);
	double yearly_energy_cost = energy_cost_per_day * bevarmejo::k__days_ina_year;
	
	// since this function is named "cost", I return the opposite of the money I have to pay so it is positive as the word implies
	return -bevarmejo::net_present_value(capital_cost, discount_rate, -yearly_energy_cost, amortization_years);
}

double of__reliability(const WDS &anytown) {

	double value = 0.0;

	// Resilience index 
	const auto ir_daily = resilience_index_from_min_pressure(anytown, bevarmejo::anytown::min_pressure_psi*MperFT/PSIperFT);
	
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
		const auto normdeficit_daily = pressure_deficiency(anytown, bevarmejo::anytown::min_pressure_psi*MperFT/PSIperFT, /*relative=*/ true);
		// just accumulate through the day, no need to average it out
		for (const auto& [t, deficit] : normdeficit_daily) {
			value += deficit;
		}
	}
	else {
		const auto normdeficit_daily = pressure_deficiency(anytown, bevarmejo::anytown::min_pressure_psi*MperFT/PSIperFT, /*relative=*/ true);
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

void Problem::reset_dv(std::shared_ptr<bevarmejo::WaterDistributionSystem> anytown, const std::vector<double>& dvs) const {
	// Do the opposite operations of apply_dv 
	anytown->cache_indices();

	switch (m__formulation)
	{
	case Formulation::rehab_f1: {
		fep1::reset_dv__exis_pipes(*anytown, std::vector(dvs.begin(), dvs.begin()+70), __old_HW_coeffs);
		reset_dv__new_pipes(*anytown, std::vector(dvs.begin()+70, dvs.begin()+76));
		// No pumps reset
		fnt1::reset_dv__tanks(*anytown, std::vector(dvs.begin()+76, dvs.end()));
		return;
	}
	case Formulation::mixed_f1: {
		fep1::reset_dv__exis_pipes(*anytown, std::vector(dvs.begin(), dvs.begin()+70), __old_HW_coeffs);
		reset_dv__new_pipes(*anytown, std::vector(dvs.begin()+70, dvs.begin()+76));
		reset_dv__pumps(*anytown, std::vector(dvs.begin()+76, dvs.begin()+100));
		fnt1::reset_dv__tanks(*anytown, std::vector(dvs.begin()+100, dvs.end()));
		return;
	}
	case Formulation::twoph_f1: {
		fep1::reset_dv__exis_pipes(*anytown, std::vector(dvs.begin(), dvs.begin()+70), __old_HW_coeffs);
		reset_dv__new_pipes(*anytown, std::vector(dvs.begin()+70, dvs.begin()+76));
		// No pumps reset and no need to do anything to the internal opt problem.
		fnt1::reset_dv__tanks(*anytown, std::vector(dvs.begin()+76, dvs.end()));
		return;
	}
	case Formulation::opertns_f1: {
		// No need to reset the pumps as they are overwritten
		return;
	}
	case Formulation::rehab_f2: {
		fep2::reset_dv__exis_pipes(*anytown, std::vector(dvs.begin(), dvs.begin()+35), __old_HW_coeffs);
		reset_dv__new_pipes(*anytown, std::vector(dvs.begin()+35, dvs.begin()+41));
		// No pumps reset
		fnt1::reset_dv__tanks(*anytown, std::vector(dvs.begin()+41, dvs.end()));
		return;
	}
	case Formulation::mixed_f2: {
		fep2::reset_dv__exis_pipes(*anytown, std::vector(dvs.begin(), dvs.begin()+35), __old_HW_coeffs);
		reset_dv__new_pipes(*anytown, std::vector(dvs.begin()+35, dvs.begin()+41));
		reset_dv__pumps(*anytown, std::vector(dvs.begin()+41, dvs.begin()+65));
		fnt1::reset_dv__tanks(*anytown, std::vector(dvs.begin()+65, dvs.end()));
		return;
	}
	default:
		break;
	}

	return;
}

// ------------------- 3rd level ------------------- //
// ------------------- apply_dv ------------------- //
void fep1::apply_dv__exis_pipes(WDS& anyt_wds, std::unordered_map<std::string, double> &old_HW_coeffs, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::exi_pipe_option> &pipes_alt_costs)
{
	assert(dvs.size() == 2*anyt_wds.subnetwork_with_order<WDS::Pipe>("existing_pipes").size());

	auto curr_dv = dvs.begin();
	for (auto&& [id, pipe] : anyt_wds.subnetwork_with_order<WDS::Pipe>("existing_pipes"))
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
	assert(curr_dv == dvs.end());
	return;
}
void fep2::apply_dv__exis_pipes(WDS& anyt_wds, std::unordered_map<std::string, double> &old_HW_coeffs, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::exi_pipe_option> &pipes_alt_costs)
{
	assert(dvs.size() == anyt_wds.subnetwork_with_order<WDS::Pipe>("existing_pipes").size());

	auto curr_dv = dvs.begin();

	for (auto&& [id, pipe] : anyt_wds.subnetwork_with_order<WDS::Pipe>("existing_pipes"))
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
	assert(curr_dv == dvs.end());
	return;
}

void apply_dv__new_pipes(WDS &anyt_wds, const std::vector<double> &dvs, const std::vector<bevarmejo::anytown::new_pipe_option> &pipes_alt_costs)
{
    assert(dvs.size() == anyt_wds.subnetwork_with_order<WDS::Pipe>("new_pipes").size());

	auto curr_dv = dvs.begin();
	for (auto&& [id, pipe] : anyt_wds.subnetwork_with_order<WDS::Pipe>("new_pipes"))
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
}

void apply_dv__pumps(WDS& anyt_wds, const std::vector<double>& dvs)
{
	assert(dvs.size() == 24); // 24 hours x [npr]

	auto patterns = decompose_pumpgroup_pattern(dvs, anyt_wds.pumps().size());
	std::size_t i = 0;
	for (auto&& [id, pump] : anyt_wds.pumps()) {
		// set the pattern
		int errorcode = EN_setpattern(anyt_wds.ph_, pump.speed_pattern()->EN_index(), patterns[i].data(), patterns[i].size());
		assert(errorcode <= 100);
		++i;
	}
}

void fnt1::apply_dv__tanks(WDS& anytown, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::tank_option> &tank_option)
{
	assert(dvs.size() == 2*bevarmejo::anytown::max_n_installable_tanks);

	// You can't store multiple tanks in the same location, so I need to keep track of the ones I have already installed.
	std::unordered_set<std::size_t> already_installed_tanks;

	auto curr_dv = dvs.begin();
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
		double tank_volume_m3 = tank_volume_gal * 0.00378541;
		
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
		new_tank.y_coord(junction.y_coord()+bevarmejo::anytown::riser_length_ft);
		
		// We assume d = h for a cilindrical tank, thus V = \pi d^2 /4 * h = \pi d^3 / 4
		// given that this is a fixed value we could actually have it as a parameter to reduce computational expenses. 
		double diam_m = std::pow(tank_volume_m3*4/k__pi, 1.0/3); // TODO: fix based on whatever ratio I want
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
		riser.length(bevarmejo::anytown::riser_length_ft*MperFT);
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
			bevarmejo::anytown::riser_length_ft,
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
}

// -------------------   cost   ------------------- //
double fep1::cost__exis_pipes(const WDS& anytown, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::exi_pipe_option> &pipes_alt_costs)
{
	assert(dvs.size() == 2*anytown.subnetwork_with_order<WDS::Pipe>("existing_pipes").size());

	double capital_cost = 0.0;

	auto curr_dv = dvs.begin();
	for (auto&& [id, pipe] : anytown.subnetwork_with_order<WDS::Pipe>("existing_pipes"))
	{
		std::size_t action_type = *curr_dv++;
		std::size_t alt_option = *curr_dv++;

		if (action_type == 0) // no action
			continue;
		
		bool city = anytown.id_sequence("city_pipes").contains(id);
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
	assert(curr_dv == dvs.end());
	return capital_cost;
}
double fep2::cost__exis_pipes(const WDS& anytown, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::exi_pipe_option> &pipes_alt_costs)
{
	assert(dvs.size() == anytown.subnetwork_with_order<WDS::Pipe>("existing_pipes").size());

	double capital_cost = 0.0;
	
	auto curr_dv = dvs.begin();
	for (auto&& [id, pipe] : anytown.subnetwork_with_order<WDS::Pipe>("existing_pipes"))
	{
		std::size_t action_type = *curr_dv++;
		std::size_t alt_option = action_type-2; // -2 because the first two options are no action and clean

		if (action_type == 0) // no action
			continue;

		bool city = anytown.id_sequence("city_pipes").contains(id);
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
	assert(curr_dv == dvs.end());
	return capital_cost;
}

double cost__new_pipes(const WDS &anytown, const std::vector<double> &dvs, const std::vector<bevarmejo::anytown::new_pipe_option> &pipes_alt_costs)
{
	assert(dvs.size() == anytown.subnetwork_with_order<WDS::Pipe>("new_pipes").size());
	
	// 6 pipes x [prc]
	// This must be installed, thus minimum cost will never be 0.
	double capital_cost = 0.0;
	auto curr_dv = dvs.begin();
	for (auto&& [id, pipe] : anytown.subnetwork_with_order<WDS::Pipe>("new_pipes"))
	{
		std::size_t alt_option = *curr_dv++;
		
		double pipe_cost_per_ft = pipes_alt_costs.at(alt_option).cost__per_ft;
		capital_cost += pipe_cost_per_ft/MperFT * pipe.length().value();
	}

	return capital_cost;
}

double cost__energy_per_day(const WDS &anytown)
{
    double total_ene_cost_per_day = 0.0;
	for (const auto& [id, pump] : anytown.pumps() )
	{
		unsigned long t_prec = 0;
		double power_kW_prec = 0.0;
		// at time t, I should multiply the instant energy at t until t+1, or with this single for loop shift by one all indeces
		for (const auto& [t, power_kW] : pump.instant_energy() )
		{
			total_ene_cost_per_day += power_kW_prec * (t - t_prec)/bevarmejo::k__sec_per_hour * bevarmejo::anytown::energy_cost_kWh ; 
			t_prec = t;
			power_kW_prec = power_kW;
		}
	}
	return total_ene_cost_per_day;
}

double fnt1::cost__tanks(const WDS& anytown, const std::vector<double> &dvs, const std::vector<bevarmejo::anytown::tank_option> &tank_option, const std::vector<bevarmejo::anytown::new_pipe_option> &pipes_alt_costs)
{
	assert(dvs.size() == 2*bevarmejo::anytown::max_n_installable_tanks);

	double capital_cost = 0.0;
	std::unordered_set<std::size_t> already_installed_tanks;

	auto curr_dv = dvs.begin();
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
		
		capital_cost += pipes_alt_costs.at(5).cost__per_ft*bevarmejo::anytown::riser_length_ft;

		already_installed_tanks.insert(new_tank_loc_shift);
	}
	return capital_cost;
}

// ------------------- of__reliability ------------------- //

// ------------------- reset_dv ------------------- //
void fep1::reset_dv__exis_pipes(WDS &anytown, const std::vector<double> &dvs, const std::unordered_map<std::string, double> &old_HW_coeffs)
{
	assert(dvs.size() == 2*anytown.subnetwork_with_order<WDS::Pipe>("existing_pipes").size());

	auto curr_dv = dvs.begin();

	// 1. existing pipes
	for (auto&& [id, pipe] : anytown.subnetwork_with_order<WDS::Pipe>("existing_pipes"))
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
	assert(curr_dv == dvs.end());
	return;
}
void fep2::reset_dv__exis_pipes(WDS &anytown, const std::vector<double> &dvs, const std::unordered_map<std::string, double> &old_HW_coeffs)
{
	assert(dvs.size() == anytown.subnetwork_with_order<WDS::Pipe>("existing_pipes").size());

	auto curr_dv = dvs.begin();

	for (auto&& [id, pipe] : anytown.subnetwork_with_order<WDS::Pipe>("existing_pipes"))
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
	assert(curr_dv == dvs.end());
	return;
}

void reset_dv__new_pipes(WDS& anytown, const std::vector<double>& dvs)
{
	assert(dvs.size() == anytown.subnetwork_with_order<WDS::Pipe>("new_pipes").size());

	auto curr_dv = dvs.begin();
	for (auto&& [id, pipe] : anytown.subnetwork_with_order<WDS::Pipe>("new_pipes"))
	{
		int errorcode = EN_setlinkvalue(anytown.ph_, pipe.EN_index(), EN_DIAMETER, bevarmejo::anytown::_nonexisting_pipe_diam_ft);
		assert(errorcode <= 100);

		pipe.diameter(bevarmejo::anytown::_nonexisting_pipe_diam_ft); // it's ok also in ft because its' super small small
	}
}

void reset_dv__pumps(WDS& anytown, const std::vector<double>& dvs)
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

void fnt1::reset_dv__tanks(WDS& anytown, const std::vector<double>& dvs)
{
	assert(dvs.size() == 2*bevarmejo::anytown::max_n_installable_tanks);

	std::unordered_set<std::size_t> already_installed_tanks;

	auto curr_dv = dvs.begin();
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
	assert(curr_dv == dvs.end());
	return;
}

// ------------------- 1st level ------------------- //
// -------------------   Bounds  ------------------- //
std::pair<std::vector<double>, std::vector<double>> Problem::get_bounds() const
{
	std::vector<double> lb;
	std::vector<double> ub;

	const auto append_bounds= [&lb, &ub](auto&& bounds_func, auto&&... args) {
		auto [lower, upper] = bounds_func(std::forward<decltype(args)>(args)...);
		lb.insert(lb.end(), lower.begin(), lower.end());
		ub.insert(ub.end(), upper.begin(), upper.end());
	};

	// Existing pipes (all design problems)
	switch (m__formulation)
	{
		case Formulation::rehab_f1:
			[[fallthrough]];
		case Formulation::mixed_f1:
			[[fallthrough]];
		case Formulation::twoph_f1:
			append_bounds(fep1::bounds__exis_pipes, std::as_const(*m__anytown).subnetwork_with_order<WDS::Pipe>("existing_pipes"), m__exi_pipe_options);
			break;
		case Formulation::rehab_f2:
			[[fallthrough]];
		case Formulation::mixed_f2:
			append_bounds(fep2::bounds__exis_pipes, std::as_const(*m__anytown).subnetwork_with_order<WDS::Pipe>("existing_pipes"), m__exi_pipe_options);
			break;
		default:
			break;
	}

	// New pipes (all design problems)
	if (m__formulation != Formulation::opertns_f1)
		append_bounds(bounds__new_pipes, std::as_const(*m__anytown).subnetwork_with_order<WDS::Pipe>("new_pipes"), m__new_pipe_options);

	// Pumps (all problems with operations)
	switch (m__formulation)
	{
		case Formulation::mixed_f1:
			[[fallthrough]];
		case Formulation::opertns_f1:
			[[fallthrough]];
		case Formulation::mixed_f2:
			append_bounds(bounds__pumps, std::as_const(*m__anytown).pumps());
			break;
		default:
			break;
	}

	// Tanks (all design problems)
	if (m__formulation != Formulation::opertns_f1)
		append_bounds(fnt1::bounds__tanks, std::as_const(*m__anytown).subnetwork_with_order<WDS::Junction>("possible_tank_locations"), m__tank_options);

	return {std::move(lb), std::move(ub)};
}

// ------------------- 2nd level ------------------- //
std::pair<std::vector<double>, std::vector<double>> fep1::bounds__exis_pipes(InputOrderedRegistryView<WDS::Pipe> exis_pipes, const std::vector<bevarmejo::anytown::exi_pipe_option> &pipes_alt_costs)
{
// Structure of the decision variables:
// [35 pipes x [action, pra]
// action: 3 options -> 0 - do nothing, 1 duplicate, 2 - clean 
// pra: 10 alternative in pipe_rehab_cost -> 0 - 9 

	assert(exis_pipes.size() == 35);
	assert(pipes_alt_costs.size() == 10);

	auto n_dvs= 2ul*exis_pipes.size();
	double n_pra= pipes_alt_costs.size();

	std::vector<double> lb(n_dvs, 0.0);
	std::vector<double> ub(n_dvs, 0.0);
	for (std::size_t i = 0; i < n_dvs; i+=2)
	{
		ub[i] = 2.0;
		ub[i+1] = n_pra-1;
	}

	return std::make_pair(lb, ub);
}
std::pair<std::vector<double>, std::vector<double>> fep2::bounds__exis_pipes(InputOrderedRegistryView<WDS::Pipe> exis_pipes, const std::vector<bevarmejo::anytown::exi_pipe_option> &pipes_alt_costs)
{
// Structure of the decision variables:
// [35 pipes x action]
// action: 2+pra options -> 0 - do nothing, 1 - clean, 2- pra
// pra: 10 alternative in pipe_rehab_cost -> 0 - 9 

	assert(exis_pipes.size() == 35);
	assert(pipes_alt_costs.size() == 10);

	auto n_dvs= exis_pipes.size();
	double n_actions= 2+pipes_alt_costs.size();

	return std::make_pair(std::vector<double>(n_dvs, 0), std::vector<double>(n_dvs, n_actions-1));
}

std::pair<std::vector<double>, std::vector<double>> bounds__new_pipes(InputOrderedRegistryView<WDS::Pipe> new_pipes, const std::vector<bevarmejo::anytown::new_pipe_option> &pipes_alt_costs)
{
// Structure of the decision variables:
// 6 pipes x [pra]
// pra: 10 alternative in pipe_rehab_cost -> 0 - 9 
	assert(new_pipes.size() == 6);
	assert(pipes_alt_costs.size() == 10);

	auto n_dvs= new_pipes.size();
	double n_pra= pipes_alt_costs.size();

	std::vector<double> lb(n_dvs, 0.0);
	std::vector<double> ub(n_dvs, n_pra-1);

    return std::make_pair(lb, ub);
}

std::pair<std::vector<double>, std::vector<double>> bounds__pumps(InputExcludingRegistryView<WDS::Pump> pumps)
{
// Structure of the decision variables:
// 24 hours x [npr]
// npr: 4 options indicate the number of pumps running -> 0 - 3
	assert(pumps.size() == 3);

	auto n_dvs= 24;
	double n_npr= 3.0;

	std::vector<double> lb(n_dvs, 0.0);
	std::vector<double> ub(n_dvs, n_npr);

	return std::make_pair(lb, ub);
}

std::pair<std::vector<double>, std::vector<double>> fnt1::bounds__tanks(InputOrderedRegistryView<WDS::Junction> tank_locs, const std::vector<bevarmejo::anytown::tank_option> &tank_option)
{
// Structure of the decision variables:
// 2 tanks x [tpl, tvol]
// tpl: tank possible location nodes (plus 0, do nothing) -> 0 - x
// tvol: 5 discrete tank volume possible -> 0 - 4 (to be transformed in continuous between the limits)

	assert(tank_locs.size() == 17);
	assert(tank_option.size() == 5);

	auto n_dvs= 2*bevarmejo::anytown::max_n_installable_tanks;
	double n_tpl= tank_locs.size();
	double n_tvol= tank_option.size();

	std::vector<double> lb(n_dvs, 0.0);
	std::vector<double> ub(n_dvs, 0.0);
	for (std::size_t i = 0; i < n_dvs; i+=2)
	{
		ub[i] = n_tpl; // 0 is a valid option, so no minus one
		ub[i+1] = n_tvol-1;
	}

	return std::make_pair(lb, ub);
}


// ------------------- 1st level ------------------- //
// -------------------   Save  ------------------- //
void Problem::save_solution(const std::vector<double>& dv, const fsys::path& out_file) const
{
	apply_dv(this->m__anytown, dv);

	int errco = EN_saveinpfile(this->m__anytown->ph_, out_file.string().c_str());
	assert(errco <= 100);

	reset_dv(this->m__anytown, dv);
}

void to_json(Json& j, const bevarmejo::anytown::Problem &prob)
{
	// Reset, just in case.
	j = Json{};

	j[io::key::exi_pipe_opts()] = prob.m__exi_pipe_options;
	j[io::key::new_pipe_opts()] = prob.m__new_pipe_options;
	j[io::key::tank_opts()] = prob.m__tank_options;

	if (prob.m__formulation == Formulation::rehab_f1 || prob.m__formulation == Formulation::rehab_f2) {
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

	j["extra_info"] = prob.get_extra_info();
}

} // namespace anytown
} // namespace bevarmejo
