#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#include <pagmo/problem.hpp>
#include <pagmo/algorithms/nsga2.hpp>
#include <pagmo/algorithm.hpp>
#include <pagmo/population.hpp>
#include <pagmo/island.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "bevarmejo/constants.hpp"
#include "bevarmejo/econometric_functions.hpp"
#include "bevarmejo/hydraulic_functions.hpp"
#include "bevarmejo/wds/water_distribution_system.hpp"

#include "bevarmejo/labels.hpp"
#include "bevarmejo/io.hpp"
#include "bevarmejo/wds/epanet_helpers/en_help.hpp"

#include "bevarmejo/pagmo_helpers/algorithms/nsga2_help.hpp"

#include "Anytown/prob_anytown.hpp"
#include "Anytown/operations/prob_at_ope_f1.hpp"

#include "prob_at_2ph_f1.hpp"

namespace fsys = std::filesystem;
namespace bemeat= bevarmejo::anytown;
namespace bevarmejo {
namespace anytown {
namespace twophases {
namespace f1 {

Problem::Problem(json settings, std::vector<fsys::path> lookup_paths) {
    // Add here load of problem specific data 

    /* Fix the bug where the curve 2 (i.e., the pump characteristic curve
		* is uploaded as a generic curve and not as a pump curve). 
		* Thus instead of the automatic constructor from inp file: 
		* m__anytown = std::make_shared<WDS>(inp_filename);
		* I create an empty one first, add the inp file, modify it thorugh the lambda
		* and then use init(). 
	*/
	auto fix_inp = [](EN_Project ph) {
		// change curve ID 2 to a pump curve
		assert(ph != nullptr);
		std::string curve_id = "2";
		int curve_idx = 0;
		int errorcode = EN_getcurveindex(ph, curve_id.c_str(), &curve_idx);
		assert(errorcode <= 100);

		errorcode = EN_setcurvetype(ph, curve_idx, EN_PUMP_CURVE);
		assert(errorcode <= 100);
	};
	inherited::load_network(settings, lookup_paths, fix_inp);

	inherited::load_subnets(settings, lookup_paths);
	// Custom made subnetworks for the temporary elements 
	m__anytown->add_subnetwork(label::__temp_elems, wds::Subnetwork{});

	inherited::load_other_data(settings, lookup_paths);	

    // Prepare the internal optimization problem 
    assert(settings.contains("Internal optimization") && settings["Internal optimization"].contains("UDA")
     && settings["Internal optimization"].contains("UDP") && settings["Internal optimization"].contains("Population") );
    auto uda = settings["Internal optimization"]["UDA"];
    auto udp = settings["Internal optimization"]["UDP"];
    auto udpop = settings["Internal optimization"]["Population"];

    // this is nasty but as of now it will work // I am not passing any info for now 
    assert(udpop.contains(label::__report_gen_sh));
    assert(uda.contains(label::__name) && uda[label::__name] == "nsga2");
    m_algo = pagmo::algorithm( bevarmejo::Nsga2( json{ {label::__report_gen_sh, udpop[label::__report_gen_sh] } } ) );

    assert(udp.contains(label::__name) && udp[label::__name] == bemeat::operations::f1::name && udp.contains(label::__params));
    pagmo::problem prob{ bemeat::operations::f1::Problem(udp[label::__params], lookup_paths)};
    
    assert(udpop.contains(label::__size));
    m_pop = pagmo::population( prob, udpop[label::__size].get<unsigned int>()-2u ); // -2 because I will manually add the two extreme solutions (the bounds)
    m_pop.push_back(prob.get_bounds().first); // all zero operations not running 
    m_pop.push_back(prob.get_bounds().second); // all operations running at max
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
    std::vector<double> old_HW_coeffs = apply_dv(m__anytown, dvs);
    
    // Forward these changes also to the internal optimization problem
    auto udp = m_pop.get_problem().extract<bemeat::operations::f1::Problem>();
    assert(udp != nullptr);
    udp->anytown(this->m__anytown);

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
    try {
        m_pop = m_algo.evolve(m_pop);
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
        reset_dv(m__anytown, dvs, old_HW_coeffs);
        return std::vector<double>(n_fit, std::numeric_limits<double>::max());
    }
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

	std::vector<double> best_dv = m_pop.get_x().at(idx.front());

	// Apply this pattern to the network
	{
		int i = 0;
		for (const auto& pump : m__anytown->pumps() ) {

			std::vector<double> pump_pattern(best_dv.size(), 0.0);
			for (int j = 0; j < best_dv.size(); ++j) {
				pump_pattern[j] = dvs[j] > i ? 1.0 : 0.0;
			}
			++i;

			int errorco = EN_setpattern(m__anytown->ph_, pump->speed_pattern()->index(), pump_pattern.data(), n_dv);
			assert(errorco <= 100);
    	}
	}

	// Simulate again and get the results
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

	design_cost += bemeat::f1::cost__tanks(anytown, std::vector(dvs.begin()+100, dvs.end()), m__tanks_costs, m__pipes_alt_costs);

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
} // namespace twophases
} // namespace anytown
} // namespace bevarmejo
