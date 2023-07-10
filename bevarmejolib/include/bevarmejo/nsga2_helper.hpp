/*--------------------------------
Author: DennisZ
descr: A quick definition of functions to upload the data in the right way.
--------------------------------*/

#ifndef BEVARMEJOLIB__NSGA2_HELPER_HPP
#define BEVARMEJOLIB__NSGA2_HELPER_HPP

#include <iostream>

#include "pagmo/population.hpp"
#include "pagmo/types.hpp"

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

} /* namespace bevarmejo */

#endif /* BEVARMEJOLIB__NSGA2_HELPER_HPP */