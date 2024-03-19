/*--------------------------------
Author: DennisZ
descr: A quick definition of functions to upload the data in the right way.
--------------------------------*/

#ifndef BEVARMEJOLIB__NSGA2_HELPER_HPP
#define BEVARMEJOLIB__NSGA2_HELPER_HPP

#include <iostream>

#include "pagmo/population.hpp"
#include "pagmo/types.hpp"

#include <pagmo/algorithms/nsga2.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "pugixml.hpp"

namespace bevarmejo {
	struct nsga2p {
		unsigned int seed;
		unsigned int nfe = 100u;
		unsigned int report_nfe = 100u;
		pagmo::pop_size_t pop_size = 100u;
		double cr{0.9}, eta_c{15.}, m{1./34.}, eta_m{7.};
	};

	nsga2p quick_settings_upload(pugi::xml_node settings) {
		nsga2p settingsNsga;

		if (settings.attribute("seed")) {
			settingsNsga.seed = settings.attribute("seed").as_uint();
		}
		else {
			settingsNsga.seed = 3u;
		}

		if (settings.attribute("nfe")) {
			settingsNsga.nfe = settings.attribute("nfe").as_uint();
		}
		else {
			settingsNsga.nfe = 100u;
		}
		
		if (settings.attribute("report_nfe")) {
			settingsNsga.report_nfe = settings.attribute("report_nfe").as_uint();
		}
		else {
			settingsNsga.report_nfe = 100u;
		}

		// check I don't do mistakes
		if (settingsNsga.report_nfe == 0) {
			settingsNsga.report_nfe = settingsNsga.nfe;
		}
		
		if (settings.attribute("pop_size")) {
			settingsNsga.pop_size = settings.attribute("pop_size").as_uint();
		}
		else {
			settingsNsga.pop_size = 100u;
		}

		// for the moment I don't care about the rest

		return settingsNsga;
	}

namespace nsga2 {
namespace defaults {

constexpr unsigned int gen = 1u;
constexpr double cr = 0.9;
constexpr double eta_c = 15.;
constexpr double m = 1./34.;
constexpr double eta_m = 7.;
// default for seed is random_device::next()

} // namespace defaults

namespace label {
const std::string cr = "Crossover probability";
const std::string eta_c = "Distribution index for crossover";
const std::string m = "Mutation probability";
const std::string eta_m = "Distribution index for mutation";
}
}

inline pagmo::nsga2 Nsga2(json settings) {
	unsigned int gen = settings.contains("Report gen") ? settings["Report gen"].get<unsigned int>() : nsga2::defaults::gen;
	double cr = settings.contains("cr") ? settings["cr"].get<double>() : nsga2::defaults::cr;
	double eta_c = settings.contains("eta_c") ? settings["eta_c"].get<double>() : nsga2::defaults::eta_c;
	double m = settings.contains("m") ? settings["m"].get<double>() : nsga2::defaults::m;
	double eta_m = settings.contains("eta_m") ? settings["eta_m"].get<double>() : nsga2::defaults::eta_m;

	if (settings.contains("Seed")) 
		return pagmo::nsga2(gen, cr, eta_c, m, eta_m, settings["Seed"].get<unsigned int>());

	// else leave the random deault seed
	return pagmo::nsga2(gen, cr, eta_c, m, eta_m);
}

} /* namespace bevarmejo */

#endif /* BEVARMEJOLIB__NSGA2_HELPER_HPP */