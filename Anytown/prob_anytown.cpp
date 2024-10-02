// 
//  model_anytown.cpp
//	anytown original test
//
//  Created by Dennis Zanutto on 11/07/23.

#include <iostream>
#include <filesystem>
namespace fsys = std::filesystem;
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "bevarmejo/io.hpp"
namespace bemeio = bevarmejo::io;
#include "bevarmejo/bemexcept.hpp"
#include "bevarmejo/constants.hpp"
#include "bevarmejo/econometric_functions.hpp"
#include "bevarmejo/hydraulic_functions.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "prob_anytown.hpp"

namespace bevarmejo {

namespace io {
namespace log {
namespace cname {
static const std::string anytown_problem = "anytown::Problem";
} // namespace cname
} // namespace log
} // namespace io

namespace anytown {

std::istream& operator>>(std::istream& is, bevarmejo::anytown::tanks_costs& tc)
{
	bemeio::stream_in(is, tc.volume_gal); is.ignore(1000, ';');
	bemeio::stream_in(is, tc.cost);

	return is;
}

std::istream& operator>>(std::istream& is, bevarmejo::anytown::pipes_alt_costs& pac) {
	bemeio::stream_in(is, pac.diameter_in); is.ignore(1000, ';');

	bemeio::stream_in(is, pac.new_cost); is.ignore(1000, ';');
	bemeio::stream_in(is, pac.dup_city); is.ignore(1000, ';');
	bemeio::stream_in(is, pac.dup_residential); is.ignore(1000, ';');
	bemeio::stream_in(is, pac.clean_city); is.ignore(1000, ';');
	bemeio::stream_in(is, pac.clean_residential);

	return is;
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

Problem::Problem(Formulation a_formulation, json settings, const std::vector<std::filesystem::path>& lookup_paths) :
	m__formulation{a_formulation}
{
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

	load_subnets(settings, lookup_paths);

	if (m__formulation != Formulation::opertns_f1) {
		// Custom made subnetworks for the temporary elements 
		m__anytown->add_subnetwork(label::__temp_elems, wds::Subnetwork{});

		load_other_data(settings, lookup_paths);
	}

	if (m__formulation == Formulation::rehab_f1 || m__formulation == Formulation::rehab_f2) {
		// Need to se the operations for the rehabilitation problems
		std::vector<double> operations = settings["Operations"].get<std::vector<double>>();
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

void Problem::load_network(json settings, std::vector<fsys::path> lookup_paths, std::function<void (EN_Project)> preprocessf) {
	assert(settings != nullptr);

	// Check the existence of the inp_filename in any of the lookup paths and its extension
	auto file = bemeio::locate_file(fsys::path{settings["WDS"]["inp"]}, lookup_paths);
	if (!file.has_value()) {
		throw std::runtime_error("The provided inp file does not exist in the lookup paths. Check the settings file.\n");
	}
	auto inp_filename = file.value();

	m__anytown = std::make_shared<WDS>(inp_filename, preprocessf);
}

void Problem::load_subnets(json settings, std::vector<fsys::path> lookup_paths) {
	for (const auto& udeg : settings["WDS"]["UDEGs"]) {
		// Locate the file in the lookup paths
		auto file = bemeio::locate_file(fsys::path{udeg}, lookup_paths);
		if (file.has_value()) {
			try{
				m__anytown->add_subnetwork(file.value());
			}
			catch (const std::exception& ex) {
				std::cerr << ex.what();
			}
		}
		// else skip but log the error 
		// TODO: log the error in case it fails at later stages
	}
	// TODO: assert that all the required subnetworks are present
}

void Problem::load_other_data(json settings, std::vector<fsys::path> lookup_paths) {
	// Load Pipe rehabilitation alternative costs 
	auto file = bemeio::locate_file(fsys::path{settings["Available diameters"]}, lookup_paths);
	if (!file.has_value()) {
		throw std::runtime_error("The provided available diameters file does not exist in the lookup paths. Check the settings file.\n");
	}
	auto prac_filename = file.value();

	// TODO: move also this to json?
	std::ifstream prac_file{prac_filename};
	if (!prac_file.is_open()) {
		throw std::runtime_error("Could not open file " + prac_filename.string());
	}

	std::size_t n_alt_costs = bemeio::load_dimensions(prac_file, "#DATA");
	m__pipes_alt_costs.resize(n_alt_costs);
	bemeio::stream_in(prac_file, m__pipes_alt_costs);
	

	// Load Tank costs 
	file = bemeio::locate_file(fsys::path{settings["Tank costs"]}, lookup_paths);
	if (!file.has_value()) {
		throw std::runtime_error("The provided tank costs file does not exist in the lookup paths. Check the settings file.\n");
	}
	auto tanks_filename = file.value();
	std::ifstream tanks_file{tanks_filename};
	if (!tanks_file.is_open()) {
		throw std::runtime_error("Could not open file " + tanks_filename.string());
	}
	
	std::size_t n_tanks = bemeio::load_dimensions(tanks_file, "#DATA");
	m__tanks_costs.resize(n_tanks);
	bemeio::stream_in(tanks_file, m__tanks_costs);
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

std::string Problem::get_name() const {
	switch (m__formulation)
	{
	case Formulation::rehab_f1:
		return io::value::rehab_f1;
	case Formulation::mixed_f1:
		return io::value::mixed_f1;
	case Formulation::opertns_f1:
		return io::value::opertns_f1;
	case Formulation::twoph_f1:
		return io::value::twoph_f1;
	case Formulation::rehab_f2:
		return io::value::rehab_f2;
	case Formulation::mixed_f2:
		return io::value::mixed_f2;
	default:
		return "Unknown Anytown formulation.";
	}
}

std::string Problem::get_extra_info() const {
	switch (m__formulation)
	{
	case Formulation::rehab_f1:
		return io::other::rehab_f1_exinfo;
	case Formulation::mixed_f1:
		return io::other::mixed_f1_exinfo;
	case Formulation::opertns_f1:
		return io::other::opertns_f1_exinfo;
	case Formulation::twoph_f1:
		return io::other::twoph_f1_exinfo;
	case Formulation::rehab_f2:
		return io::other::rehab_f2_exinfo;
	case Formulation::mixed_f2:
		return io::other::mixed_f2_exinfo;
	default:
		return "No extra information available.\nReason:\n\tUnknown Anytown formulation.";
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
	
	try {
		m__anytown->run_hydraulics();
	} catch (...) {
		bemeio::stream_out( std::cerr, "Error in the hydraulic simulation.\n");
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

void Problem::apply_dv(std::shared_ptr<bevarmejo::wds::WaterDistributionSystem> anytown, const std::vector<double>& dvs) const {
	anytown->cache_indices();

	switch (m__formulation)
	{
	case Formulation::rehab_f1: {
		f1::apply_dv__exis_pipes(*anytown, m__old_HW_coeffs, std::vector(dvs.begin(), dvs.begin()+70), m__pipes_alt_costs);
		apply_dv__new_pipes(*anytown, std::vector(dvs.begin()+70, dvs.begin()+76), m__pipes_alt_costs);
		// No pump apply
		f1::apply_dv__tanks(*anytown, std::vector(dvs.begin()+76, dvs.end()), m__tanks_costs);
		return;
	}
	case Formulation::mixed_f1: {
		f1::apply_dv__exis_pipes(*anytown, m__old_HW_coeffs, std::vector(dvs.begin(), dvs.begin()+70), m__pipes_alt_costs);
		apply_dv__new_pipes(*anytown, std::vector(dvs.begin()+70, dvs.begin()+76), m__pipes_alt_costs);
		apply_dv__pumps(*anytown, std::vector(dvs.begin()+76, dvs.begin()+100));
		f1::apply_dv__tanks(*anytown, std::vector(dvs.begin()+100, dvs.end()), m__tanks_costs);
		return;
	}
	case Formulation::twoph_f1: {
		//TODO: implement the two-phase formulation
	}
	case Formulation::opertns_f1: {
		apply_dv__pumps(*anytown, dvs);
		return;
	}
	case Formulation::rehab_f2: {
		f2::apply_dv__exis_pipes(*anytown, m__old_HW_coeffs, std::vector(dvs.begin(), dvs.begin()+35), m__pipes_alt_costs);
		apply_dv__new_pipes(*anytown, std::vector(dvs.begin()+35, dvs.begin()+41), m__pipes_alt_costs);
		// No pumps apply
		f1::apply_dv__tanks(*anytown, std::vector(dvs.begin()+41, dvs.end()), m__tanks_costs);
		return;
	}
	case Formulation::mixed_f2: {
		f2::apply_dv__exis_pipes(*anytown, m__old_HW_coeffs, std::vector(dvs.begin(), dvs.begin()+35), m__pipes_alt_costs);
		apply_dv__new_pipes(*anytown, std::vector(dvs.begin()+35, dvs.begin()+41), m__pipes_alt_costs);
		apply_dv__pumps(*anytown, std::vector(dvs.begin()+41, dvs.begin()+65));
		f1::apply_dv__tanks(*anytown, std::vector(dvs.begin()+65, dvs.end()), m__tanks_costs);
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
	double design_cost = 0.0;
	
	switch (m__formulation)
	{
	case Formulation::rehab_f1:
		[[fallthrough]];
	case Formulation::twoph_f1: {
		design_cost += f1::cost__exis_pipes(anytown, std::vector(dvs.begin(), dvs.begin()+70), m__pipes_alt_costs);
		design_cost += cost__new_pipes(anytown, std::vector(dvs.begin()+70, dvs.begin()+76), m__pipes_alt_costs);
		design_cost += f1::cost__tanks(anytown, std::vector(dvs.begin()+76, dvs.end()), m__tanks_costs, m__pipes_alt_costs);
		break;
	}
	case Formulation::mixed_f1: {
		design_cost += f1::cost__exis_pipes(anytown, std::vector(dvs.begin(), dvs.begin()+70), m__pipes_alt_costs);
		design_cost += cost__new_pipes(anytown, std::vector(dvs.begin()+70, dvs.begin()+76), m__pipes_alt_costs);
		design_cost += f1::cost__tanks(anytown, std::vector(dvs.begin()+100, dvs.end()), m__tanks_costs, m__pipes_alt_costs);
		break;
	}
	case Formulation::rehab_f2: {
		design_cost += f2::cost__exis_pipes(anytown, std::vector(dvs.begin(), dvs.begin()+35), m__pipes_alt_costs);
		design_cost += cost__new_pipes(anytown, std::vector(dvs.begin()+35, dvs.begin()+41), m__pipes_alt_costs);
		design_cost += f1::cost__tanks(anytown, std::vector(dvs.begin()+41, dvs.end()), m__tanks_costs, m__pipes_alt_costs);
		break;
	}
	case Formulation::mixed_f2: {
		design_cost += f2::cost__exis_pipes(anytown, std::vector(dvs.begin(), dvs.begin()+35), m__pipes_alt_costs);
		design_cost += cost__new_pipes(anytown, std::vector(dvs.begin()+35, dvs.begin()+41), m__pipes_alt_costs);
		design_cost += f1::cost__tanks(anytown, std::vector(dvs.begin()+65, dvs.end()), m__tanks_costs, m__pipes_alt_costs);
		break;
	}
	default:
		break;
	}
		
	double energy_cost_per_day = cost__energy_per_day(anytown);
	double yearly_energy_cost = energy_cost_per_day * bevarmejo::k__days_ina_year;
	
	// since this function is named "cost", I return the opposite of the money I have to pay so it is positive as the word implies
	return -bevarmejo::net_present_value(design_cost, discount_rate, -yearly_energy_cost, amortization_years);
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

void Problem::reset_dv(std::shared_ptr<bevarmejo::wds::WaterDistributionSystem> anytown, const std::vector<double>& dvs) const {
	// Do the opposite operations of apply_dv 
	anytown->cache_indices();

	switch (m__formulation)
	{
	case Formulation::rehab_f1: {
		f1::reset_dv__exis_pipes(*anytown, std::vector(dvs.begin(), dvs.begin()+70), m__old_HW_coeffs);
		reset_dv__new_pipes(*anytown);
		// No pumps reset
		f1::reset_dv__tanks(*anytown, std::vector(dvs.begin()+76, dvs.end()));
		return;
	}
	case Formulation::mixed_f1: {
		f1::reset_dv__exis_pipes(*anytown, std::vector(dvs.begin(), dvs.begin()+70), m__old_HW_coeffs);
		reset_dv__new_pipes(*anytown);
		reset_dv__pumps(*anytown);
		f1::reset_dv__tanks(*anytown, std::vector(dvs.begin()+100, dvs.end()));
		return;
	}
	case Formulation::twoph_f1: {
		// TODO: implement the two-phase formulation
		return;
	}
	case Formulation::opertns_f1: {
		// No need to reset the pumps as they are overwritten
		return;
	}
	case Formulation::rehab_f2: {
		f2::reset_dv__exis_pipes(*anytown, std::vector(dvs.begin(), dvs.begin()+35), m__old_HW_coeffs);
		reset_dv__new_pipes(*anytown);
		// No pumps reset
		f1::reset_dv__tanks(*anytown, std::vector(dvs.begin()+41, dvs.end()));
		return;
	}
	case Formulation::mixed_f2: {
		f2::reset_dv__exis_pipes(*anytown, std::vector(dvs.begin(), dvs.begin()+35), m__old_HW_coeffs);
		reset_dv__new_pipes(*anytown);
		reset_dv__pumps(*anytown);
		f1::reset_dv__tanks(*anytown, std::vector(dvs.begin()+65, dvs.end()));
		return;
	}
	default:
		break;
	}

	return;
}

// ------------------- 3rd level ------------------- //
// ------------------- apply_dv ------------------- //
void f1::apply_dv__exis_pipes(WDS& anyt_wds, std::unordered_map<std::string, double> &old_HW_coeffs, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs) {
	assert(dvs.size() == 2*anyt_wds.subnetwork("existing_pipes").size());
	// Let's assume that indices are already cached

	auto curr_dv = dvs.begin();
	int errorcode = 0;

	// 1. existing pipes
	for (auto& wp_curr_net_ele : anyt_wds.subnetwork("existing_pipes")) {
		// if dvs[i*2] == 0 do nothing
		if (*curr_dv == 0){
			++curr_dv;
			++curr_dv;
#ifdef DEBUGSIM
			std::cout << "No action for pipe " << wp_curr_net_ele.lock()->id() << "\n";
#endif
			continue;
		}

		// something needs to be changed 
		auto curr_net_ele = wp_curr_net_ele.lock();
		// retrieve the link ID and index
		std::shared_ptr<wds::Pipe> curr_pipe = std::dynamic_pointer_cast<wds::Pipe, wds::NetworkElement>(curr_net_ele);
		assert(curr_pipe != nullptr);
		std::string link_id = curr_pipe->id();
		int link_idx = curr_pipe->index();

		if (*curr_dv == 1) { // clean
			// retrieve and save the old HW coefficients
			double old_pipe_roughness = curr_pipe->roughness().value();
			old_HW_coeffs.insert({curr_pipe->id(), old_pipe_roughness});
#ifdef DEBUGSIM
			std::cout << "Cleaning pipe " << link_id << "\n";
#endif
		
			// set the new HW coefficients
			errorcode = EN_setlinkvalue(anyt_wds.ph_, link_idx, EN_ROUGHNESS, bevarmejo::anytown::coeff_HW_cleaned);	
			assert(errorcode <= 100);
			curr_pipe->roughness(bevarmejo::anytown::coeff_HW_cleaned);
		}
		else if (*curr_dv == 2) { // duplicate
			// Ideally I would just need to modify my network object and then
			// this changed would be refelected automatically on the EPANET 
			// project. However, this requires some work, so I will do it the 
			// old way where I manually modify the EN_Project first and then
			// knowing the object is there I will simply fetch the data from it.

			// DUPLICATE on EPANET project
			// new name is Dxx where xx is the original pipe name
			std::string new_link_id = "D"+curr_pipe->id();
			int dup_pipe_idx = 0;

			// retrieve the old property of the already existing pipe
			int out_node1_idx = 0;
			int out_node2_idx = 0;
			errorcode = EN_getlinknodes(anyt_wds.ph_, link_idx, &out_node1_idx, &out_node2_idx);
			assert(errorcode <= 100);

			std::string out_node1_id = epanet::get_node_id(anyt_wds.ph_, out_node1_idx);
			std::string out_node2_id = epanet::get_node_id(anyt_wds.ph_, out_node2_idx);
			
			// create the new pipe
			errorcode = EN_addlink(anyt_wds.ph_, new_link_id.c_str(), EN_PIPE, out_node1_id.c_str(), out_node2_id.c_str(), &dup_pipe_idx);
			assert(errorcode <= 100);
			
			// change the new pipe properties:
			// 1. diameter =  row dvs[i*2+1] column diameter of m__pipes_alt_costs
			// 2. roughness = coeff_HV_new
			// 3. length  = value of link_idx
			double diameter_in = pipes_alt_costs.at(*(curr_dv+1)).diameter_in;
			errorcode = EN_setlinkvalue(anyt_wds.ph_, dup_pipe_idx, EN_DIAMETER, diameter_in);
			assert(errorcode <= 100);
			// errorcode = EN_setlinkvalue(anyt_wds.ph_, dup_pipe_idx, EN_ROUGHNESS, coeff_HW_new);
			// assert(errorcode <= 100);
			double link_length = 0.0;
			errorcode = EN_getlinkvalue(anyt_wds.ph_, link_idx, EN_LENGTH, &link_length);
			assert(errorcode <= 100);
			errorcode = EN_setlinkvalue(anyt_wds.ph_, dup_pipe_idx, EN_LENGTH, link_length);
			assert(errorcode <= 100);


			// DUPLICATE on my network object 
			// ok, the unique_ptr is passed as rvalue reference, so I can't use it anymore
			std::shared_ptr<wds::Pipe> dup_pipe = curr_pipe->duplicate();
			// ADD to the network (should be fine doing it in the loop 
			// as I am not using any iterator of the standard groups
			// this holds as long as I don't add it to existing_pipes or if 
			// I add them after this loop)
			anyt_wds.insert(dup_pipe);
			anyt_wds.cache_indices();
			// add to the set of the "to be removed" elements
			anyt_wds.subnetwork(label::__temp_elems).insert(dup_pipe);
			// Since I duplicated the pipe every property is the same except:
			// the new pipe may have a different diameter and
			// the new pipe MUST have the roughness of a new pipe.
			assert(dup_pipe->index() != 0);
			dup_pipe->diameter(pipes_alt_costs.at(*(curr_dv+1)).diameter_in*MperFT/12*1000);
			dup_pipe->roughness(bevarmejo::anytown::coeff_HW_new);

#ifdef DEBUGSIM
			std::cout << "Duplicated pipe " << link_id << " with diam " << m__pipes_alt_costs.at(*(curr_dv+1)).diameter_in <<"in (" <<dup_pipe->diameter()() << " mm)\n";
#endif
		}

		++curr_dv;
		++curr_dv;
	}

	return;
}
void f2::apply_dv__exis_pipes(WDS& anyt_wds, std::unordered_map<std::string, double> &old_HW_coeffs, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs) {
	assert(dvs.size() == anyt_wds.subnetwork("existing_pipes").size());
	// Let's assume that indices are already cached

	auto it_dv = dvs.begin();
	auto it_net_elem = anyt_wds.subnetwork("existing_pipes").begin();
	int errorcode = 0;

	while (it_dv != dvs.end() && it_net_elem != anyt_wds.subnetwork("existing_pipes").end()) {
		auto dv = *it_dv;
		
		if (dv == 0){
			// Do nothing and skip
#ifdef DEBUGSIM
			std::cout << "No action for pipe " << it_net_elem->lock()->id() << "\n";
#endif
			++it_dv;
			++it_net_elem;
			continue;
		}

		// something needs to be changed, let's get the pipe
		auto net_ele = it_net_elem->lock();
		assert(net_ele != nullptr);
		auto pipe = std::dynamic_pointer_cast<wds::Pipe, wds::NetworkElement>(net_ele);
		assert(pipe != nullptr);

		if (dv == 1) { // clean
#ifdef DEBUGSIM
			std::cout << "Cleaning pipe " << pipe->id() << "\n";
#endif
			// retrieve and save the old HW coefficients
			double old_pipe_roughness = pipe->roughness().value();
			old_HW_coeffs.insert({pipe->id(), old_pipe_roughness});

			// set the new HW coefficients
			errorcode = EN_setlinkvalue(anyt_wds.ph_, pipe->index(), EN_ROUGHNESS, bevarmejo::anytown::coeff_HW_cleaned);	
			assert(errorcode <= 100);
			pipe->roughness(bevarmejo::anytown::coeff_HW_cleaned);

			++it_dv;
			++it_net_elem;
			continue;
		}

		// else, i.e., dv >= 2, we duplicate the pipe 
#ifdef DEBUGSIM
		std::cout << "Duplicated pipe " << pipe->id() << " with diam " << m__pipes_alt_costs.at(dv-2).diameter_in <<"in\n";
#endif

		// DUPLICATE on EPANET project
		// new name is Dxx where xx is the original pipe name
		std::string new_link_id = "D"+pipe->id();
		int dup_pipe_idx = 0;

		// retrieve the old property of the already existing pipe
		int out_node1_idx = 0;
		int out_node2_idx = 0;
		errorcode = EN_getlinknodes(anyt_wds.ph_, pipe->index(), &out_node1_idx, &out_node2_idx);
		assert(errorcode <= 100);

		std::string out_node1_id = epanet::get_node_id(anyt_wds.ph_, out_node1_idx);
		std::string out_node2_id = epanet::get_node_id(anyt_wds.ph_, out_node2_idx);
		
		// create the new pipe
		errorcode = EN_addlink(anyt_wds.ph_, new_link_id.c_str(), EN_PIPE, out_node1_id.c_str(), out_node2_id.c_str(), &dup_pipe_idx);
		assert(errorcode <= 100);
		
		// change the new pipe properties:
		// 1. diameter =  row dvs[i]-2 column diameter of m__pipes_alt_costs
		// 2. roughness = coeff_HV_new
		// 3. length  = value of link_idx
		double diameter_in = pipes_alt_costs.at(dv-2).diameter_in;
		errorcode = EN_setlinkvalue(anyt_wds.ph_, dup_pipe_idx, EN_DIAMETER, diameter_in);
		assert(errorcode <= 100);
		// errorcode = EN_setlinkvalue(anyt_wds.ph_, dup_pipe_idx, EN_ROUGHNESS, coeff_HW_new);
		// assert(errorcode <= 100);
		double link_length = 0.0;
		errorcode = EN_getlinkvalue(anyt_wds.ph_, pipe->index(), EN_LENGTH, &link_length);
		assert(errorcode <= 100);
		errorcode = EN_setlinkvalue(anyt_wds.ph_, dup_pipe_idx, EN_LENGTH, link_length);
		assert(errorcode <= 100);


		// DUPLICATE on my network object 
		// ok, the unique_ptr is passed as rvalue reference, so I can't use it anymore
		std::shared_ptr<wds::Pipe> dup_pipe = pipe->duplicate();
		// ADD to the network (should be fine doing it in the loop 
		// as I am not using any iterator of the standard groups
		// this holds as long as I don't add it to existing_pipes or if 
		// I add them after this loop)
		anyt_wds.insert(dup_pipe);
		anyt_wds.cache_indices();
		// add to the set of the "to be removed" elements
		anyt_wds.subnetwork(label::__temp_elems).insert(dup_pipe);
		// Since I duplicated the pipe every property is the same except:
		// the new pipe may have a different diameter and
		// the new pipe MUST have the roughness of a new pipe.
		assert(dup_pipe->index() != 0);
		dup_pipe->diameter(pipes_alt_costs.at(dv-2).diameter_in*MperFT/12*1000);
		dup_pipe->roughness(bevarmejo::anytown::coeff_HW_new);

		++it_dv;
		++it_net_elem;	

	} // end while

	return;
}

void apply_dv__new_pipes(WDS &anyt_wds, const std::vector<double> &dvs, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs)
{
    assert(dvs.size() == anyt_wds.subnetwork("new_pipes").size());
	// Let's assume that indices are already cached

	auto curr_dv = dvs.begin();
	for (const auto& wp_curr_net_ele : anyt_wds.subnetwork("new_pipes")) {
		auto curr_net_ele = wp_curr_net_ele.lock();	
		// retrieve the link ID from the subnetwork
		std::shared_ptr<wds::Pipe> curr_pipe = std::dynamic_pointer_cast<wds::Pipe, wds::NetworkElement>(curr_net_ele);
		if (curr_pipe == nullptr)
			throw std::runtime_error("Could not cast to Pipe, check the new_pipes subnetwork.");

		std::string link_id = curr_pipe->id();
		int link_idx = curr_pipe->index();

		// change the new pipe properties:
		// diameter =  row dvs[70+i] column diameter of m__pipes_alt_costs
		double diameter_in = pipes_alt_costs.at(*curr_dv).diameter_in;
		int errorcode = EN_setlinkvalue(anyt_wds.ph_, link_idx, EN_DIAMETER, diameter_in);
		assert(errorcode <= 100);
		curr_pipe->diameter(diameter_in*MperFT/12*1000); //save in mm

#ifdef DEBUGSIM
		std::cout << "New pipe with ID " << link_id << " installed with diam of " << diameter_in << " in (" <<curr_pipe->diameter()() <<" mm)\n";
#endif

		++curr_dv;
	}
}

void apply_dv__pumps(WDS& anyt_wds, const std::vector<double>& dvs) {
	assert(dvs.size() == 24); // 24 hours x [npr]
	// Let's assume that indices are already cached

	auto curr_dv = dvs.begin();
	auto patterns = decompose_pumpgroup_pattern(std::vector(curr_dv, curr_dv+24), anyt_wds.pumps().size());
	for (std::size_t i = 0; i < patterns.size(); ++i) {
		// I know pump patterns IDs are from 2, 3, and 4
		int pump_idx = i + 2;
		std::string pump_id = std::to_string(pump_idx);
		int errorcode = EN_getpatternindex(anyt_wds.ph_, pump_id.c_str(), &pump_idx);
		assert(errorcode <= 100);

		// set the pattern
		errorcode = EN_setpattern(anyt_wds.ph_, pump_idx, patterns[i].data(), patterns[i].size());
		assert(errorcode <= 100);
	}
}

void f1::apply_dv__tanks(WDS& anytown, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::tanks_costs> &tanks_costs) {
	assert(dvs.size() == 2*bevarmejo::anytown::max_n_installable_tanks);
	// Let's assume that indices are already cached

	auto curr_dv = dvs.begin();
	for(std::size_t tank_idx = 0; tank_idx < bevarmejo::anytown::max_n_installable_tanks; ++tank_idx) {
		// 0 counts as "don't install" and I can't install two tanks on the same location
		if ((int)*curr_dv == 0 || (tank_idx > 0 && *curr_dv == *(curr_dv-2)) ) {
			// don't install skip the location and the volume
			++curr_dv;
			++curr_dv;
#ifdef DEBUGSIM
			std::cout << "No action for tank " << tank_idx+1 << "\n";
#endif
			continue;
		}
		
		int new_tank_loc_shift = *curr_dv-1; // minus one because of the zero option! This indicates the index of the subnetwork
		assert(new_tank_loc_shift >= 0 && new_tank_loc_shift < anytown.subnetwork("possible_tank_locations").size() );
		auto wp_ne = anytown.subnetwork("possible_tank_locations").begin() + new_tank_loc_shift;
		auto new_tnk_instal_netel = wp_ne->lock(); // as a pointer to network element
		if (new_tnk_instal_netel == nullptr)
			throw std::runtime_error("Could not retrieve the Node, did you delete it?");
		auto new_tank_install_node = std::dynamic_pointer_cast<wds::Node, wds::NetworkElement>(new_tnk_instal_netel); 
		assert(new_tank_install_node != nullptr);
		
		/* Should I create a fake node with a zero demand?
			* No, this is done for the two tanks in the original file only for 
			* the purpose of graphic representation on EPANET.
		*/

		// I should create a new tank at that position and with that volume
		double tank_volume_gal = tanks_costs.at(*(curr_dv+1)).volume_gal;
		double tank_volume_m3 = tank_volume_gal * 0.00378541;
		std::shared_ptr<wds::Tank> new_tank = std::make_shared<wds::Tank>("T"+std::to_string(tank_idx), anytown);
		// elevation , min and max level are the same as in the original tanks
		// Ideally same coordinates of the junction, but I move it slightly in case I want to save the result to file and visualize it
		// diameter from volume divided by the fixed ratio
		auto origin_tank = *(anytown.tanks().begin());
		double elev = origin_tank->elevation();
		double min_lev = origin_tank->min_level().value();
		double min_vol = origin_tank->min_volume().value();
		new_tank->elevation(elev);
		new_tank->initial_level(min_lev);
		new_tank->min_level(min_lev);
		new_tank->min_volume(min_vol);
		new_tank->x_coord(new_tank_install_node->x_coord());
		new_tank->y_coord(new_tank_install_node->y_coord()+bevarmejo::anytown::riser_length_ft); 
		// We assume d = h for a cilindrical tank, thus V = \pi d^2 /4 * h = \pi d^3 / 4
		// given that this is a fixed value we could actually have it as a parameter to reduce computational expenses. 
		double diam_m = std::pow(tank_volume_m3*4/k__pi, 1.0/3); // TODO: fix based on whatever ratio I want
		new_tank->diameter(diam_m);
		double max_lev = diam_m;
		new_tank->max_level(max_lev);
		// do it again in EPANET
		int new_tank_idx = 0; 
		int errco = EN_addnode(anytown.ph_, new_tank->id().c_str(), EN_TANK, &new_tank_idx);
		assert(errco <= 100);
		errco = EN_settankdata(anytown.ph_, new_tank_idx, 
			elev/MperFT, 
			min_lev/MperFT, 
			min_lev/MperFT, 
			max_lev/MperFT, 
			diam_m/MperFT, 
			min_vol/M3perFT3, 
			"");
		assert(errco <= 100);
		anytown.insert(new_tank);

		// The riser has a well defined length, diameter could be a dv, but I fix it to 16 inches for now
		std::shared_ptr<wds::Pipe> riser = std::make_shared<wds::Pipe>("Ris_"+std::to_string(tank_idx), anytown);
		riser->diameter(14.0*MperFT/12*1000);
		riser->length(bevarmejo::anytown::riser_length_ft*MperFT);
		riser->start_node(new_tank.get());
		riser->end_node(new_tank_install_node.get());
		riser->roughness(bevarmejo::anytown::coeff_HW_new);
		anytown.insert(riser);
		// do it again in EPANET
		int riser_idx = 0;
		errco = EN_addlink(anytown.ph_, riser->id().c_str(), EN_PIPE, new_tank->id().c_str(), new_tank_install_node->id().c_str(), &riser_idx);
		assert(errco <= 100);
		errco = EN_setpipedata(anytown.ph_, riser_idx,
			bevarmejo::anytown::riser_length_ft,
			16.0,
			bevarmejo::anytown::coeff_HW_new,
			0.0
		);
		assert(errco <= 100);

		anytown.cache_indices();
		assert(riser->index() != 0 && riser->index() == riser_idx);

		// add them to the "TBR" net
		anytown.subnetwork(label::__temp_elems).insert(new_tank);
		anytown.subnetwork(label::__temp_elems).insert(riser);

		++curr_dv;
		++curr_dv;

#ifdef DEBUGSIM
		bemeio::stream_out(std::cout, "Installed tank at node ", new_tank_install_node->id(), 
		" with volume ", tank_volume_gal, " gal(", tank_volume_m3, " m^3)", 
		" Elev ", elev, " Min level ", min_lev, " Max lev ", max_lev, " Diam ", diam_m, "\n");
#endif
	}
}

// -------------------   cost   ------------------- //
double f1::cost__exis_pipes(const WDS& anytown, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs) {
	assert(dvs.size() == 2*anytown.subnetwork("existing_pipes").size());
	// Let's assume that indices are already cached

	double design_cost = 0.0;
	// 35 pipes x [action, prc]
	auto curr_dv = dvs.begin();

	for(auto& wp_curr_net_ele : anytown.subnetwork("existing_pipes")) {
		if (*curr_dv == 0){
			++curr_dv;
			++curr_dv;
			continue;
		}

		auto curr_net_ele = wp_curr_net_ele.lock();
		std::string link_id = curr_net_ele->id();
		bool city = anytown.subnetwork("city_pipes").contains(link_id);
		// I assume is in the residential as they are mutually exclusive

		// Either duplicate or clean I can use dvs[i*2+1] to get the cost and
		// the length of the pipe from the network object (in case of the 
		// duplicate pipe the length is the same of the original pipe).
		double pipe_cost_per_ft = 0.0;
		std::shared_ptr<wds::Pipe> curr_pipe = std::dynamic_pointer_cast<wds::Pipe, wds::NetworkElement>(curr_net_ele);
		if (curr_pipe == nullptr)
			throw std::runtime_error("Could not cast to Pipe, check the existing_pipes subnetwork.");

		if (*curr_dv == 1) { // clean
			// I can't use dvs[i*2+1] to get the costs, but I have to search 
			// for the diameter in the table.
			auto it = std::find_if(pipes_alt_costs.begin(), pipes_alt_costs.end(), 
				[&curr_pipe](const bevarmejo::anytown::pipes_alt_costs& pac) { 
					return std::abs(pac.diameter_in*MperFT/12*1000 - curr_pipe->diameter().value()) < 0.0001; 
				});

			// Check I actually found it 
			if (it == pipes_alt_costs.end()) 
				throw std::runtime_error("Could not find the diameter of the pipe in the cost table.");

			if (city)
				pipe_cost_per_ft = (*it).clean_city;
			else
				pipe_cost_per_ft = (*it).clean_residential;
		}
		else if (*curr_dv == 2) { // duplicate
			if (city) 
				pipe_cost_per_ft = pipes_alt_costs.at(*(curr_dv+1)).dup_city;
			else
				pipe_cost_per_ft = pipes_alt_costs.at(*(curr_dv+1)).dup_residential;
		} 

		design_cost += pipe_cost_per_ft/MperFT * curr_pipe->length().value(); 
		// again I save the length in mm, but the table is in $/ft

		++curr_dv;
		++curr_dv;
	}

	return design_cost;
}
double f2::cost__exis_pipes(const WDS& anytown, const std::vector<double>& dvs, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs) {
	assert(dvs.size() == 2*anytown.subnetwork("existing_pipes").size());
	// Let's assume that indices are already cached

	double design_cost = 0.0;
	// 35 pipes x [action, prc]
	auto curr_dv = dvs.begin();

	for(auto& wp_curr_net_ele : anytown.subnetwork("existing_pipes")) {
		if (*curr_dv == 0){
			++curr_dv;
			++curr_dv;
			continue;
		}

		auto curr_net_ele = wp_curr_net_ele.lock();
		std::string link_id = curr_net_ele->id();
		bool city = anytown.subnetwork("city_pipes").contains(link_id);
		// I assume is in the residential as they are mutually exclusive

		// Either duplicate or clean I can use dvs[i*2+1] to get the cost and
		// the length of the pipe from the network object (in case of the 
		// duplicate pipe the length is the same of the original pipe).
		double pipe_cost_per_ft = 0.0;
		std::shared_ptr<wds::Pipe> curr_pipe = std::dynamic_pointer_cast<wds::Pipe, wds::NetworkElement>(curr_net_ele);
		if (curr_pipe == nullptr)
			throw std::runtime_error("Could not cast to Pipe, check the existing_pipes subnetwork.");

		if (*curr_dv == 1) { // clean
			// I can't use dvs[i*2+1] to get the costs, but I have to search 
			// for the diameter in the table.
			auto it = std::find_if(pipes_alt_costs.begin(), pipes_alt_costs.end(), 
				[&curr_pipe](const bevarmejo::anytown::pipes_alt_costs& pac) { 
					return std::abs(pac.diameter_in*MperFT/12*1000 - curr_pipe->diameter().value()) < 0.0001; 
				});

			// Check I actually found it 
			if (it == pipes_alt_costs.end()) 
				throw std::runtime_error("Could not find the diameter of the pipe in the cost table.");

			if (city)
				pipe_cost_per_ft = (*it).clean_city;
			else
				pipe_cost_per_ft = (*it).clean_residential;
		}
		else if (*curr_dv == 2) { // duplicate
			if (city) 
				pipe_cost_per_ft = pipes_alt_costs.at(*(curr_dv+1)).dup_city;
			else
				pipe_cost_per_ft = pipes_alt_costs.at(*(curr_dv+1)).dup_residential;
		} 

		design_cost += pipe_cost_per_ft/MperFT * curr_pipe->length().value(); 
		// again I save the length in mm, but the table is in $/ft

		++curr_dv;
		++curr_dv;
	}

	return design_cost;
}

double cost__new_pipes(const WDS &anytown, const std::vector<double> &dvs, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs) {
	assert(dvs.size() == anytown.subnetwork("new_pipes").size());
	// Let's assume that indices are already cached
	
	// 6 pipes x [prc]
	// This must be installed, thus minimum cost will never be 0.
	double design_cost = 0.0;
	auto curr_dv = dvs.begin();
	for (const auto& wp_curr_net_ele : anytown.subnetwork("new_pipes")) {
		auto curr_net_ele = wp_curr_net_ele.lock();
		std::shared_ptr<wds::Pipe> curr_pipe = std::dynamic_pointer_cast<wds::Pipe, wds::NetworkElement>(curr_net_ele);
		if (curr_pipe == nullptr)
			throw std::runtime_error("Could not cast to Pipe, check the new_pipes subnetwork.");
		
		// dvs[i] is the row of the m__pipes_alt_costs table
		double pipe_cost_per_ft = pipes_alt_costs.at(*curr_dv).new_cost;
		design_cost += pipe_cost_per_ft/MperFT * curr_pipe->length().value();

		++curr_dv;
	}

	return design_cost;
}

double cost__energy_per_day(const WDS &anytown)
{
    double total_ene_cost_per_day = 0.0;
	for (const auto& pump : anytown.pumps() ) {
		unsigned long t_prec = 0;
		double power_kW_prec = 0.0;
		// at time t, I should multiply the instant energy at t until t+1, or with this single for loop shift by one all indeces
		for (const auto& [t, power_kW] : pump->instant_energy() ) {
			total_ene_cost_per_day += power_kW_prec * (t - t_prec)/bevarmejo::k__sec_per_hour * bevarmejo::anytown::energy_cost_kWh ; 
			t_prec = t;
			power_kW_prec = power_kW;
		}
	}
	return total_ene_cost_per_day;
}

double f1::cost__tanks(const WDS& anytown, const std::vector<double> &dvs, const std::vector<bevarmejo::anytown::tanks_costs> &tanks_costs, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs) {
	assert(dvs.size() == 2*bevarmejo::anytown::max_n_installable_tanks);
	// Let's assume that indices are already cached

	double design_cost = 0.0;
	auto curr_dv = dvs.begin();
	for(std::size_t tank_idx = 0; tank_idx < bevarmejo::anytown::max_n_installable_tanks; ++tank_idx) {
		// Check if the tanks is going to be installed
		// You can't install two tanks on the same locations so I discard the second one 
		if (*curr_dv == 0. || (tank_idx > 0 && *curr_dv == *(curr_dv-2)) ) {
			// don't install skip the location and the volume
			++curr_dv;
			++curr_dv;
			continue;
		}

		// I don't care where I place it, the cost is always dependent on the volume [dv+1]
		++curr_dv;
		// as of this version I can only choose the specific volume of the table and not intermediate values.
		double tank_cost = tanks_costs.at(*curr_dv).cost;
		design_cost += tank_cost;
		// TODO: decide how to add the cost of the riser (for now I take 16 inches as the standard riser diam)
		design_cost += pipes_alt_costs.at(5).new_cost*bevarmejo::anytown::riser_length_ft; // no need to go back to meters because everything here is in foot
		++curr_dv;
	}
	return design_cost;
}

// ------------------- of__reliability ------------------- //

// ------------------- reset_dv ------------------- //
void f1::reset_dv__exis_pipes(WDS &anytown, const std::vector<double> &dvs, const std::unordered_map<std::string, double> &old_HW_coeffs) {
	assert(dvs.size() == 2*anytown.subnetwork("existing_pipes").size());
	// Let's assume that indices are already cached

	auto curr_dv = dvs.begin();
	int errorcode = 0;

	// 1. existing pipes
	for (auto& wp_curr_net_ele : anytown.subnetwork("existing_pipes")) {
		// if dvs[i*2] == 0 do nothing
		if (*curr_dv == 0){
			++curr_dv;
			++curr_dv;
			continue;
		}

		auto curr_net_ele = wp_curr_net_ele.lock();
		std::shared_ptr<wds::Pipe> curr_pipe = std::dynamic_pointer_cast<wds::Pipe, wds::NetworkElement>(curr_net_ele);
		if (curr_pipe == nullptr)
			throw std::runtime_error("Could not cast to Pipe, check the existing_pipes subnetwork.");
		
		if (*curr_dv == 1) { // reset clean
			// re set the HW coefficients
			errorcode = EN_setlinkvalue(anytown.ph_, curr_pipe->index(), EN_ROUGHNESS, old_HW_coeffs.at(curr_pipe->id()));
			assert(errorcode <= 100);

			curr_pipe->roughness(old_HW_coeffs.at(curr_pipe->id()));
		}
		else if (*curr_dv == 2) { // remove duplicate
			// duplicate pipe has been named Dxx where xx is the original pipe name
			// they are also saved in the subnetwork l__TEMP_ELEMS
			auto it = std::find_if(anytown.subnetwork(label::__temp_elems).begin(), anytown.subnetwork(label::__temp_elems).end(), 
				[&curr_pipe](const std::weak_ptr<wds::NetworkElement>& ne) {
					auto ne_ptr = ne.lock();
					if (ne_ptr == nullptr)
						return false; // it didn't found it, meaning it doesn't exist anymore!
					return ne_ptr->id() == "D"+curr_pipe->id(); 
				});

			std::shared_ptr<wds::Pipe> dup_pipe_to_rem = std::dynamic_pointer_cast<wds::Pipe, wds::NetworkElement>(it->lock());

			// remove the new pipe
			errorcode = EN_deletelink(anytown.ph_, dup_pipe_to_rem->index(), EN_UNCONDITIONAL);
			assert(errorcode <= 100);

			// remove the new pipe from my network object
			anytown.remove(dup_pipe_to_rem);
			anytown.cache_indices();
			// remove the new pipe from the set of the "to be removed" elements
			anytown.subnetwork(label::__temp_elems).remove(dup_pipe_to_rem);
		} 
		
		++curr_dv;
		++curr_dv;
	}
}
void f2::reset_dv__exis_pipes(WDS &anytown, const std::vector<double> &dvs, const std::unordered_map<std::string, double> &old_HW_coeffs) {
	assert(dvs.size() == 2*anytown.subnetwork("existing_pipes").size());
	// Let's assume that indices are already cached

	auto curr_dv = dvs.begin();
	int errorcode = 0;

	// 1. existing pipes
	for (auto& wp_curr_net_ele : anytown.subnetwork("existing_pipes")) {
		// if dvs[i*2] == 0 do nothing
		if (*curr_dv == 0){
			++curr_dv;
			++curr_dv;
			continue;
		}

		auto curr_net_ele = wp_curr_net_ele.lock();
		std::shared_ptr<wds::Pipe> curr_pipe = std::dynamic_pointer_cast<wds::Pipe, wds::NetworkElement>(curr_net_ele);
		if (curr_pipe == nullptr)
			throw std::runtime_error("Could not cast to Pipe, check the existing_pipes subnetwork.");
		
		if (*curr_dv == 1) { // reset clean
			// re set the HW coefficients
			errorcode = EN_setlinkvalue(anytown.ph_, curr_pipe->index(), EN_ROUGHNESS, old_HW_coeffs.at(curr_pipe->id()));
			assert(errorcode <= 100);

			curr_pipe->roughness(old_HW_coeffs.at(curr_pipe->id()));
		}
		else if (*curr_dv == 2) { // remove duplicate
			// duplicate pipe has been named Dxx where xx is the original pipe name
			// they are also saved in the subnetwork l__TEMP_ELEMS
			auto it = std::find_if(anytown.subnetwork(label::__temp_elems).begin(), anytown.subnetwork(label::__temp_elems).end(), 
				[&curr_pipe](const std::weak_ptr<wds::NetworkElement>& ne) {
					auto ne_ptr = ne.lock();
					if (ne_ptr == nullptr)
						return false; // it didn't found it, meaning it doesn't exist anymore!
					return ne_ptr->id() == "D"+curr_pipe->id(); 
				});

			std::shared_ptr<wds::Pipe> dup_pipe_to_rem = std::dynamic_pointer_cast<wds::Pipe, wds::NetworkElement>(it->lock());

			// remove the new pipe
			errorcode = EN_deletelink(anytown.ph_, dup_pipe_to_rem->index(), EN_UNCONDITIONAL);
			assert(errorcode <= 100);

			// remove the new pipe from my network object
			anytown.remove(dup_pipe_to_rem);
			anytown.cache_indices();
			// remove the new pipe from the set of the "to be removed" elements
			anytown.subnetwork(label::__temp_elems).remove(dup_pipe_to_rem);
		} 
		
		++curr_dv;
		++curr_dv;
	}
}

void reset_dv__new_pipes(WDS& anytown) {
	// Let's assume that indices are already cached

	for (auto& wp_curr_net_ele : anytown.subnetwork("new_pipes") ) {
		auto curr_net_ele = wp_curr_net_ele.lock();
		// retrieve the link ID and idx from the subnetwork
		std::shared_ptr<wds::Pipe> curr_pipe = std::dynamic_pointer_cast<wds::Pipe, wds::NetworkElement>(curr_net_ele);
		assert(curr_pipe != nullptr);

		// change the new pipe properties:
		double diameter = bevarmejo::anytown::_nonexisting_pipe_diam_ft;
		int errorcode = EN_setlinkvalue(anytown.ph_, curr_pipe->index(), EN_DIAMETER, diameter);
		assert(errorcode <= 100);

		curr_pipe->diameter(bevarmejo::anytown::_nonexisting_pipe_diam_ft); // it's ok also in ft because its' super small
	}
}

void reset_dv__pumps(WDS& anytown) {
	for (std::size_t i = 0; i < 3; ++i) {
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

void f1::reset_dv__tanks(WDS& anytown, const std::vector<double>& dvs) {
	assert(dvs.size() == 2*bevarmejo::anytown::max_n_installable_tanks);
	// Let's assume that indices are already cached

	auto curr_dv = dvs.begin();
	for(std::size_t tank_idx = 0; tank_idx < bevarmejo::anytown::max_n_installable_tanks; ++tank_idx) {
		// 0 counts as "don't install" and I can't install two tanks on the same location
		if (*curr_dv == 0. || (tank_idx > 0 && *curr_dv == *(curr_dv-2)) ) {
			// don't install skip the location and the volume
			++curr_dv;
			++curr_dv;
			continue;
		}
		
		// remove the new tank and the riser 
		auto itt = anytown.tanks().find("T"+std::to_string(tank_idx));
		if (itt == anytown.tanks().end())
			throw std::runtime_error("Could not find the tank to remove.");
		auto new_tank_to_rem = *itt;
		auto itr = anytown.pipes().find("Ris_"+std::to_string(tank_idx));
		if (itr == anytown.pipes().end())
			throw std::runtime_error("Could not find the riser to remove.");
		auto riser_to_rem = *itr;

		// from my network object I can simply do 
		anytown.remove(new_tank_to_rem);
		anytown.remove(riser_to_rem);

		// remove them from the set of the "to be removed" elements
		anytown.subnetwork(label::__temp_elems).remove(new_tank_to_rem);
		anytown.subnetwork(label::__temp_elems).remove(riser_to_rem);

		// and the objects still exist because I am holding it in the shared_ptr here (new_tank_to_rem, riser_to_rem)
		// remove the new tank and the the riser is automatically deleted 
		riser_to_rem->retrieve_index(anytown.ph_);
		int errorcode = EN_deletelink(anytown.ph_, riser_to_rem->index(), EN_UNCONDITIONAL);
		assert(errorcode <= 100);
		
		new_tank_to_rem->retrieve_index(anytown.ph_);
		
		errorcode = EN_deletenode(anytown.ph_, new_tank_to_rem->index(), EN_UNCONDITIONAL);
		assert(errorcode <= 100);

		anytown.cache_indices();

		++curr_dv;
		++curr_dv;
	}
}

// ------------------- 1st level ------------------- //
// -------------------   Bounds  ------------------- //
std::pair<std::vector<double>, std::vector<double>> Problem::get_bounds() const {
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
			append_bounds(f1::bounds__exis_pipes, m__anytown->subnetwork("existing_pipes"), m__pipes_alt_costs);
			break;
		case Formulation::rehab_f2:
			[[fallthrough]];
		case Formulation::mixed_f2:
			append_bounds(f2::bounds__exis_pipes, m__anytown->subnetwork("existing_pipes"), m__pipes_alt_costs);
			break;
		default:
			break;
	}

	// New pipes (all design problems)
	if (m__formulation != Formulation::opertns_f1)
		append_bounds(bounds__new_pipes, m__anytown->subnetwork("new_pipes"), m__pipes_alt_costs);

	// Pumps (all problems with operations)
	switch (m__formulation)
	{
		case Formulation::mixed_f1:
			[[fallthrough]];
		case Formulation::opertns_f1:
			[[fallthrough]];
		case Formulation::mixed_f2:
			append_bounds(bounds__pumps, m__anytown->pumps());
			break;
		default:
			break;
	}

	// Tanks (all design problems)
	if (m__formulation != Formulation::opertns_f1)
		append_bounds(f1::bounds__tanks, m__anytown->subnetwork("possible_tank_locations"), m__tanks_costs);

	return {std::move(lb), std::move(ub)};
}

// ------------------- 2nd level ------------------- //
std::pair<std::vector<double>, std::vector<double>> f1::bounds__exis_pipes(const wds::Subnetwork &exis_pipes, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs) {
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
	for (std::size_t i = 0; i < n_dvs; i+=2) {
		ub[i] = 2.0;
		ub[i+1] = n_pra-1;
	}

	return std::make_pair(lb, ub);
}
std::pair<std::vector<double>, std::vector<double>> f2::bounds__exis_pipes(const wds::Subnetwork &exis_pipes, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs) {
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

std::pair<std::vector<double>, std::vector<double>> bounds__new_pipes(const wds::Subnetwork &new_pipes, const std::vector<bevarmejo::anytown::pipes_alt_costs> &pipes_alt_costs) {
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

std::pair<std::vector<double>, std::vector<double>> bounds__pumps(const wds::Pumps &pumps) {
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

std::pair<std::vector<double>, std::vector<double>> f1::bounds__tanks(const wds::Subnetwork &tank_locs, const std::vector<bevarmejo::anytown::tanks_costs> &tanks_costs) {
// Structure of the decision variables:
// 2 tanks x [tpl, tvol]
// tpl: tank possible location nodes (plus 0, do nothing) -> 0 - x
// tvol: 5 discrete tank volume possible -> 0 - 4 (to be transformed in continuous between the limits)

	assert(tank_locs.size() == 17);
	assert(tanks_costs.size() == 5);

	auto n_dvs= 2*bevarmejo::anytown::max_n_installable_tanks;
	double n_tpl= tank_locs.size();
	double n_tvol= tanks_costs.size();

	std::vector<double> lb(n_dvs, 0.0);
	std::vector<double> ub(n_dvs, 0.0);
	for (std::size_t i = 0; i < n_dvs; i+=2) {
		ub[i] = n_tpl; // 0 is a valid option, so no minus one
		ub[i+1] = n_tvol-1;
	}

	return std::make_pair(lb, ub);
}


// ------------------- 1st level ------------------- //
// -------------------   Save  ------------------- //
void Problem::save_solution(const std::vector<double>& dv, const fsys::path& out_file) const {
	apply_dv(this->m__anytown, dv);

	int errco = EN_saveinpfile(this->m__anytown->ph_, out_file.c_str());
	assert(errco <= 100);

	reset_dv(this->m__anytown, dv);
}


} // namespace anytown
} // namespace bevarmejo
