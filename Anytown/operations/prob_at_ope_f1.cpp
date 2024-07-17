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

#include "bevarmejo/constants.hpp"
#include "bevarmejo/econometric_functions.hpp"
#include "bevarmejo/hydraulic_functions.hpp"
#include "bevarmejo/wds/water_distribution_system.hpp"
#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/link.hpp"
#include "bevarmejo/wds/elements/junction.hpp"
#include "bevarmejo/wds/data_structures/demand.hpp"
#include "bevarmejo/wds/elements/source.hpp"
#include "bevarmejo/wds/elements/tank.hpp"
#include "bevarmejo/wds/elements/pipe.hpp"
#include "bevarmejo/wds/elements_group.hpp"

#include "bevarmejo/io.hpp"
#include "bevarmejo/epanet_helpers/en_help.hpp"

#include "Anytown/prob_anytown.hpp"

#include "prob_at_ope_f1.hpp"

namespace fsys = std::filesystem;
namespace bevarmejo {
namespace anytown {
namespace operations {
namespace f1 {

Problem::Problem(json settings, std::vector<fsys::path> lookup_paths) {
    assert(settings != nullptr);

    // Check the existence of the inp_filename in any of the lookup paths and its extension
	auto file = bevarmejo::io::locate_file(fsys::path{settings["WDS"]["inp"].get<std::string>()}, lookup_paths);
	if (!file.has_value()) {
		throw std::runtime_error("The provided inp file does not exist in the lookup paths. Check the settings file.\n");
	}
	auto inp_filename = file.value();

    /* Fix the bug where the curve 2 (i.e., the pump characteristic curve
		* is uploaded as a generic curve and not as a pump curve). 
		* Thus instead of the automatic constructor from inp file: 
		* _anytown_ = std::make_shared<WDS>(inp_filename);
		* I create an empty one first, add the inp file, modify it thorugh the lambda
		* and then use init(). 
	*/
	m_anytown = std::make_shared<WDS>();
	
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

	m_anytown->load_from_inp_file(inp_filename, fix_inp);

    // Load the subnetworks
    for (const auto& udeg : settings["WDS"]["UDEGs"]) {
		// Locate the file in the lookup paths
		file = bevarmejo::io::locate_file(fsys::path{udeg}, lookup_paths);
		if (file.has_value()) {
			try{
				m_anytown->add_subnetwork(file.value());
			}
			catch (const std::exception& ex) {
				std::cerr << ex.what();
			}
		}
		// else skip but log the error 
		// TODO: log the error in case it fails at later stages
	}
	
}

std::vector<double> Problem::fitness(const std::vector<double>& dvs) const {
    assert(dvs.size() == n_dv);

    apply_dv(*m_anytown, dvs);

    // Run the simulation
    try
    {
        m_anytown->run_hydraulics();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        reset_dv(*m_anytown, dvs);
		return std::vector<double>(n_fit, std::numeric_limits<double>::max());
    }

    // Calculate the objective functions 
    std::vector<double> fitv(n_fit, 0.0);

    // Objective function 1: energy cost
    fitv[0] = cost(*m_anytown);
    // Objective function 2: cumulative pressure deficit 
    auto normdeficit_day = bevarmejo::pressure_deficiency(*m_anytown, anytown::min_pressure_psi/PSIperFT*MperFT, /*relative=*/ true);
    for (const auto& [t, deficit] : normdeficit_day) {
        fitv[1] += deficit;
    }

    // Reset the changes and return
    reset_dv(*m_anytown, dvs);
    return fitv;
}
    
std::pair<std::vector<double>, std::vector<double>> Problem::get_bounds() const {
    std::vector<double> lb(n_dv, 0.0);
    std::vector<double> ub(n_dv, 3.0); // number of pumps in parallel in the system
    return std::make_pair(lb, ub);
}

double Problem::cost(const bevarmejo::wds::WaterDistributionSystem& a_wds) const {
    double total_ene_cost_per_day = 0.0;
	for (const auto& pump : a_wds.pumps() ) {
		unsigned long t_prec = 0;
		double power_kW_prec = 0.0;
		// at time t, I should multiply the instant energy at t until t+1, or with this single for loop shift by one all indeces
		for (const auto& [t, power_kW] : pump->instant_energy().value() ) {
			total_ene_cost_per_day += power_kW_prec * (t - t_prec)/bevarmejo::k__sec_per_hour * anytown::energy_cost_kWh ; 
			t_prec = t;
			power_kW_prec = power_kW;
		}
	}

    return total_ene_cost_per_day;
}

void Problem::apply_dv(bevarmejo::wds::WaterDistributionSystem& a_wds, const std::vector<double>& dvs) const {
    a_wds.cache_indices();
    int i = 0;
    for (const auto& pump : a_wds.pumps() ) {

        std::vector<double> pump_pattern(n_dv, 0.0);
        for (int j = 0; j < n_dv; ++j) {
            pump_pattern[j] = dvs[j] > i ? 1.0 : 0.0;
        }
        ++i;

        int errorco = EN_setpattern(a_wds.ph_, pump->speed_pattern()->index(), pump_pattern.data(), n_dv);
        assert(errorco <= 100);
    }
}

} // namespace f1
} // namespace operations
} // namespace anytown
} // namespace bevarmejo