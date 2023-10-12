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
    // Data of Anytown: hard coded as they don't change for now.
    
    // Dimensions of the problem.
    constexpr std::size_t n_obj = 2u;
    constexpr std::size_t n_ec = 0u;
    constexpr std::size_t n_ic = 0u;
    constexpr std::size_t n_fit = n_obj + n_ec + n_ic;
    constexpr std::size_t n_dv = 100u;
    constexpr std::size_t n_ix = 100u;
    constexpr std::size_t n_cx = n_dv-n_ix;

    // Other constants always valid.
    constexpr double treatment_plant_head_ft = 10.0;
    constexpr double min_w_level_tank_ft = 225.0;
    constexpr double max_w_level_tank_ft = 250.0;
    constexpr double bottom_height_tank_ft = 215.0; // emergency volume between min_w_level_tank_ft and bottom_height_tank_ft
    constexpr double min_pressure_psi = 40.0;
    constexpr double average_daily_flow_multiplier = 1.0;
    constexpr double peak_flow_multiplier = 1.3;
    constexpr double instantaneous_peak_flow_multiplier = 1.8;    
    // Constants for fire flow conditions.
    constexpr double min_pressure_fireflow_psi = 20.0;
    constexpr double fireflow_multiplier = peak_flow_multiplier;
    constexpr double fireflow_duration_hours = 2.0;
    // Actual fire flow conditions will be loaded from file.

    // Parameters for the fitness function.
    constexpr double coeff_HW_cleaned = 125.0;
    constexpr double coeff_HW_new = 130.0;
    
    constexpr double energy_cost_kWh = 0.12; // dollars per kWh
    constexpr double discount_rate = 0.12; // 12% per year
    constexpr double amortization_years = 20.0; // 20 years
    
    constexpr double riser_length_ft = 101.0;
    // The riser is usually splitted in two parts because ?? 
    constexpr double _riser_length_1_ft = 100.0;
    constexpr double _riser_length_2_ft = 1.0; 

    // Structs for reading data from file.
    struct pipes_alt_costs {
        double diameter;
        double new_cost;
        double dup_city;
        double dup_residential;
        double clean_city;
        double clean_residential;        
    }; 
    std::istream& operator >> (std::istream& is, pipes_alt_costs& pac);

    struct tanks_costs {
        double volume;
        double cost;
    };
    std::istream& operator >> (std::istream& is, tanks_costs& tc);

    constexpr double _nonexisting_pipe_diam_ft = 0.0001;

    // Here the problem is actually construted.
	class ModelAnytown {
	public: 
		ModelAnytown() = default;
		ModelAnytown(fsys::path input_directory, pugi::xml_node settings);

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
        std::vector<pipes_alt_costs> _pipes_alt_costs_;
        std::vector<tanks_costs> _tanks_costs_;

        /* Anytown specific functions */
        double cost(const std::vector<double>& dv, const std::vector<std::vector<double>>& energy) const;
        double reliablity(const std::vector<double>& pressures) const;

        /* Helper functions */
        std::vector<double> apply_dv(std::shared_ptr<bevarmejo::WaterDistributionSystem> anytown, const std::vector<double>& dv) const;
        void reset_dv(std::shared_ptr<bevarmejo::WaterDistributionSystem> anytown, const std::vector<double>& dv, const std::vector<double>& old_HW_coeffs) const;
        std::vector<std::vector<double>> decompose_pump_pattern(std::vector<const double>::iterator begin, const std::vector<const double>::iterator end) const;

	}; /* class ModelAnytown*/
 
} /* namespace bevarmejo */

#endif // !ANYTOWNORIGINALTEST__MODELANYTOWN_HPP

