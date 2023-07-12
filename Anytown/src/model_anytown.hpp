// 
//  model_anytown.hpp
//	anytown original test
//
//  Created by Dennis Zanutto on 11/07/23.

#ifndef ANYTOWNORIGINALTEST__MODELANYTOWN_HPP
#define ANYTOWNORIGINALTEST__MODELANYTOWN_HPP

#include <iostream>
#include <filesystem>
#include <memory>
#include <utility>
#include <vector>

#include "pugixml.hpp"

#include "bevarmejo/water_distribution_system.hpp"

namespace fsys = std::filesystem;
namespace bevarmejo {

	class ModelAnytown {
	public: 
		ModelAnytown() = default;
		ModelAnytown(const fsys::path& input_directory, pugi::xml_node settings);

		// Try to have copy and move constructor automatically created

		/* PUBLIC functions for Pagmo Algorihtm */
        // Number of objective functions
        std::vector<double>::size_type get_nobj() const;

        // Number of equality constraints
        std::vector<double>::size_type get_nec() const;

        // Number of INequality constraints
        std::vector<double>::size_type get_nic() const;

        // Number of integer decision variables
        std::vector<double>::size_type get_nix() const;

        // Number of continous decision variables is automatically retrieved with get_bounds() and get_nix()

        //
        std::string get_extra_info() const;

        // Mandatory public functions necessary for the optimization algorithm:
        // Implementation of the objective function.
        std::vector<double> fitness(const std::vector<double>& dv) const;

        // Implementation of the box bounds.
        std::pair<std::vector<double>, std::vector<double>> get_bounds() const;

    private: 
        /* Anytonw specific data */
        std::shared_ptr<bevarmejo::WaterDistributionSystem> _anytown_;


        /* Anytown specific functions */


	}; /* class ModelAnytown*/
 
} /* namespace bevarmejo */

#endif // !ANYTOWNORIGINALTEST__MODELANYTOWN_HPP

