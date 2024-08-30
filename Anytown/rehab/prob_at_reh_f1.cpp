#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "bevarmejo/io.hpp"
#include "bevarmejo/constants.hpp"
#include "bevarmejo/econometric_functions.hpp"
#include "bevarmejo/wds/water_distribution_system.hpp"

#include "Anytown/prob_anytown.hpp"

#include "prob_at_reh_f1.hpp"

namespace fsys = std::filesystem;
namespace bemeat= bevarmejo::anytown;
namespace bevarmejo {
namespace anytown {
namespace rehab {
namespace f1 {

Problem::Problem(json settings, std::vector<std::filesystem::path> lookup_paths) {
	// Add here load of problem specific data 

	/* Fix the bug where the curve 2 (i.e., the pump characteristic curve
		* is uploaded as a generic curve and not as a pump curve). 
		* Thus instead of the automatic constructor from inp file: 
		* m__anytown = std::make_shared<WDS>(inp_filename);
		* I create an empty one first, add the inp file, modify it thorugh the lambda
		* and then use init(). 
	*/

	// Also fix the operations at the beginning loading from the settings file
	std::vector<double> operations = settings["Operations"].get<std::vector<double>>();
	assert(operations.size() == 24);
	
	auto fix_inp = [&operations](EN_Project ph) {
		// change curve ID 2 to a pump curve
		assert(ph != nullptr);
		std::string curve_id = "2";
		int curve_idx = 0;
		int errorcode = EN_getcurveindex(ph, curve_id.c_str(), &curve_idx);
		assert(errorcode <= 100);

		errorcode = EN_setcurvetype(ph, curve_idx, EN_PUMP_CURVE);
		assert(errorcode <= 100);

		// Fix pumps' patterns
		for (int i = 0; i < 3; ++i) {
			// no need to go through the pumps, I know the pattern ID
			int pattern_idx = 0;
			errorcode = EN_getpatternindex(ph, std::to_string(i+2).c_str(), &pattern_idx);
			assert(errorcode <= 100);

			std::vector<double> temp(24, 0.0);
			for (int j = 0; j < 24; ++j) {
				temp[j] = operations[j] > i ? 1.0 : 0.0;
			}

			errorcode = EN_setpattern(ph, pattern_idx, temp.data(), temp.size());
			assert(errorcode <= 100);
		}
	};

	inherited::load_network(settings, lookup_paths, fix_inp);

	inherited::load_subnets(settings, lookup_paths);
	// Custom made subnetworks for the temporary elements 
	m__anytown->add_subnetwork(label::__temp_elems, wds::Subnetwork{});

	inherited::load_other_data(settings, lookup_paths);
}

std::vector<double> Problem::fitness(const std::vector<double> &dvs) const {

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
	// This doesn't allow parallelization and I don't allow for tank insertion. 
	// For duplicate pipes they will have name Dxx where xx is the original pipe name.
	// For cleaned pipes I write on a vector the original HW coefficients and then I reset them.
	// For new pipes and pumps (dvs from 71 onwards) I don't need to reset them as they are always overwritten.

	// 1. EPS
	//auto anytown_temp = m__anytown->clone(); 
	// in the future this will be a perfect copy and I will be able to call 
	// everything in a new thread and then simply discard it.
	
	auto old_HW_coeffs = apply_dv(m__anytown, dvs);
	
	try {
		m__anytown->run_hydraulics();
	} catch (...) {
		io::stream_out( std::cerr, "Error in the hydraulic simulation.\n");
		reset_dv(m__anytown, dvs, old_HW_coeffs);
		return std::vector<double>(n_fit, std::numeric_limits<double>::max());
	}

	// Compute OF on res. 
	std::vector<double> fitv= {
		cost(*m__anytown, dvs), // cost is positive when money is going out, like in this case
		bemeat::of__reliability(*m__anytown)
	};
	
	reset_dv( m__anytown, dvs, old_HW_coeffs);
	return fitv;
}

std::pair<std::vector<double>, std::vector<double>> Problem::get_bounds() const {

	std::vector<double> lb;
	std::vector<double> ub;
	lb.reserve(n_dv);
	ub.reserve(n_dv);

	const auto append_bounds= [this, &lb, &ub](auto&& bounds_func) {
		auto [lower, upper] = bounds_func(*this);
		lb.insert(lb.end(), lower.begin(), lower.end());
		ub.insert(ub.end(), upper.begin(), upper.end());
	};

	append_bounds(bemeat::f1::bounds__exis_pipes);
	append_bounds(bemeat::bounds__new_pipes);
	append_bounds(bemeat::f1::bounds__tanks);

	return {std::move(lb), std::move(ub)};
}

double Problem::cost(const WDS &anytown, const std::vector<double> &dvs) const {
	double design_cost = 0.0;
	
	design_cost += bemeat::f1::cost__exis_pipes(anytown, std::vector(dvs.begin(), dvs.begin()+70), m__pipes_alt_costs);

	design_cost += bemeat::cost__new_pipes(anytown, std::vector(dvs.begin()+70, dvs.begin()+76), m__pipes_alt_costs);

	design_cost += bemeat::f1::cost__tanks(anytown, std::vector(dvs.begin()+76, dvs.end()), m__tanks_costs, m__pipes_alt_costs);

	double energy_cost_per_day = bemeat::cost__energy_per_day(anytown);
	double yearly_energy_cost = energy_cost_per_day * bevarmejo::k__days_ina_year;
	
	// since this function is named "cost", I return the opposite of the money I have to pay so it is positive as the word implies
	return -bevarmejo::net_present_value(design_cost, bemeat::discount_rate, -yearly_energy_cost, bemeat::amortization_years);
}

std::vector<double> Problem::apply_dv(std::shared_ptr<WDS> anytown, const std::vector<double> &dvs) const {
	anytown->cache_indices();

	std::vector<double> old_HW_coeffs= bemeat::f1::apply_dv__exis_pipes(*anytown, std::vector(dvs.begin(), dvs.begin()+70), m__pipes_alt_costs);

	bemeat::apply_dv__new_pipes(*anytown, std::vector(dvs.begin()+70, dvs.begin()+76), m__pipes_alt_costs);

	// No pump apply
	
	bemeat::f1::apply_dv__tanks(*anytown, std::vector(dvs.begin()+76, dvs.end()), m__tanks_costs);

	return old_HW_coeffs;
}

void Problem::reset_dv(std::shared_ptr<WDS> anytown, const std::vector<double> &dvs, const std::vector<double> &old_HW_coeffs) const {
	// Do the opposite operations of apply_dv 
	anytown->cache_indices();

	bemeat::f1::reset_dv__exis_pipes(*anytown, std::vector(dvs.begin(), dvs.begin()+70), old_HW_coeffs);

	bemeat::reset_dv__new_pipes(*anytown);

	// No pump reset

	bemeat::f1::reset_dv__tanks(*anytown, std::vector(dvs.begin()+76, dvs.end()));
}

} // namespace f1
} // namespace rehab
} // namespace anytown
} // namespace bevarmejo
