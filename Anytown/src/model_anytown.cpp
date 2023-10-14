// 
//  model_anytown.cpp
//	anytown original test
//
//  Created by Dennis Zanutto on 11/07/23.

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#include "pugixml.hpp"

#include "bevarmejo/constants.hpp"
#include "bevarmejo/econometric_functions.hpp"
#include "bevarmejo/hydraulic_functions.hpp"
#include "bevarmejo/water_distribution_system.hpp"
#include "bevarmejo/io.hpp"
#include "bevarmejo/resilience_index.hpp"

#include "model_anytown.hpp"

namespace fsys = std::filesystem;
namespace bevarmejo {

	std::istream& operator>>(std::istream& is, tanks_costs& tc)
	{
		stream_in(is, tc.volume); is.ignore(1000, ';');
		stream_in(is, tc.cost);

		return is;
	}

	std::istream& operator >> (std::istream& is, pipes_alt_costs& pac) {
		stream_in(is, pac.diameter); is.ignore(1000, ';');

		stream_in(is, pac.new_cost); is.ignore(1000, ';');
		stream_in(is, pac.dup_city); is.ignore(1000, ';');
		stream_in(is, pac.dup_residential); is.ignore(1000, ';');
		stream_in(is, pac.clean_city); is.ignore(1000, ';');
		stream_in(is, pac.clean_residential);

		return is;
	}

	ModelAnytown::ModelAnytown(fsys::path input_directory, pugi::xml_node settings) {
		assert(settings != nullptr);

		// Add here load of problem specific data 

		fsys::path inp_filename{settings.child("wds").child_value("inpFile")};
		inp_filename = input_directory / inp_filename;

		_anytown_ = std::make_shared<WaterDistributionSystem>();
		_anytown_->set_inpfile(inp_filename.string());
		_anytown_->init();

		// Load subnetworks 
		for (pugi::xml_node subnet = settings.child("wds").child("subNet"); subnet;
			subnet = subnet.next_sibling("subNet")) {
			fsys::path subnet_filename{subnet.child_value()};
			subnet_filename = input_directory / subnet_filename;

			try
			{
				_anytown_->add_subnetwork(subnet_filename);
			}
			catch (const std::exception& ex)
			{
				std::cout << ex.what();
			}
		}

		// Load Pipe rehabilitation alternative costs 
		fsys::path prac_filename{settings.child_value("avDiams")};
		prac_filename = input_directory / prac_filename;

		std::ifstream prac_file{prac_filename};
		if (!prac_file.is_open()) {
			throw std::runtime_error("Could not open file " + prac_filename.string());
		}

		std::size_t n_alt_costs = load_dimensions(prac_file, "#DATA");
		_pipes_alt_costs_.resize(n_alt_costs);
		stream_in(prac_file, _pipes_alt_costs_);
		

		// Load Tank costs 
		fsys::path tanks_filename{settings.child_value("tankCosts")};
		tanks_filename = input_directory / tanks_filename;

		std::ifstream tanks_file{tanks_filename};
		if (!tanks_file.is_open()) {
			throw std::runtime_error("Could not open file " + tanks_filename.string());
		}
		
		std::size_t n_tanks = load_dimensions(tanks_file, "#DATA");
		_tanks_costs_.resize(n_tanks);
		stream_in(tanks_file, _tanks_costs_);
	}

    std::vector<double>::size_type ModelAnytown::get_nobj() const {
		// Cost and reliability
        return n_obj;
    }

    std::vector<double>::size_type ModelAnytown::get_nec() const {
		// NO equality constraints
        return n_ec;
    }

    std::vector<double>::size_type ModelAnytown::get_nic() const {
		// NO inequality constraints
        return n_ic;
    }

    std::vector<double>::size_type ModelAnytown::get_nix() const {
		// For now they are all integers. //Tanks will be added in the future
        return n_ix;
    }

    std::string ModelAnytown::get_extra_info() const {
        return std::string("\tVersion 1.1");
    }

    std::vector<double> ModelAnytown::fitness(const std::vector<double> &dv) const {

		// things to do 
		// 1. EPS 
		//   [x]   apply dv to the network
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
		// For new pipes and pumps (dv from 71 onwards) I don't need to reset them as they are always overwritten.

		// 1. EPS
		std::vector<double> old_HW_coeffs;
		old_HW_coeffs = apply_dv(_anytown_, dv);

		auto res = _anytown_->run_hydraulics();
		if (res.empty()) {
			// something went wrong
			// TODO: handle this error
		}

		reset_dv( _anytown_, dv, old_HW_coeffs);

		// Compute OF on res. 
		std::vector<double> fitv(n_fit, 0);

		// NPV of the solution (negative as both initial investement and cash flows are negative)
		fitv[0] = -cost(dv, res[2]);
		std::vector<double> hourly_Ir(res[0].size(), 0.0);
		std::vector<bool> min_pressure_constraint(res[0].size(), false);
		for (std::size_t t = 0; t<res[0].size(); ++t) {
			// compute the resilience index for each hour
			// temporary work-around
			// convert from vector of vector of vector to resilience index struct
			auto network_results = this->convert_to_netdata_4_Ir(res[0][t], res[1][t], res[2][t], "possible_tank_locations", "possible_tank_locations");
			//netdata_4_Ir network_results{{1},{1},{0},{2},{1},{0}};
			// TODO: translate min pressure psi in head 
			// TODO: convert units in metric system
			hourly_Ir[t] += bevarmejo::resilience_index(network_results, bevarmejo::min_pressure_psi);

			// TODO: convert units in metric system 
			min_pressure_constraint[t] = bevarmejo::minimum_pressure_satisfied(network_results.head_at_dnodes, bevarmejo::min_pressure_psi);
		}
		// average through the day Index of resilience
		// minuse beacuse we want to maximize
		fitv[1] = -std::accumulate(hourly_Ir.begin(), hourly_Ir.end(), 0.0) / hourly_Ir.size();

        return fitv;
    }

    std::pair<std::vector<double>, std::vector<double>> ModelAnytown::get_bounds() const {
		// Structure of the decision variables
		// [35 pipes x [action, prc], 6 pipes x [prc], 24 hours x [npr] ] 
		// action: 3 options -> 0 - do nothing, 1 duplicate, 2 - clean 
		// prc: 10 options in pipe_rehab_cost -> 0 - 9 
		// npr: 4 options indicate the number of pumps running -> 0 - 3
		//TODO: add tanks and risers.

		std::vector<double> lb(n_dv);
		std::vector<double> ub(n_dv);

		// 35 pipes x [action, prc]
		for (std::size_t i = 0; i < 35; ++i) {
			lb[i*2] = 0;
			ub[i*2] = 2;
			lb[i*2 + 1] = 0;
			ub[i*2 + 1] = 9;
		}

		// 6 pipes x [prc]
		for (std::size_t i = 0; i < 6; ++i) {
			lb[70 + i] = 0;
			ub[70 + i] = 9;
		}

		// 24 hours x [npr]
		for (std::size_t i = 0; i < 24; ++i) {
			lb[76 + i] = 0;
			ub[76 + i] = 3;
		}

		return std::pair<std::vector<double>, std::vector<double>>(lb, ub);
    }

    double ModelAnytown::cost(const std::vector<double> &dv, const std::vector<std::vector<double>> &energy) const {
        double design_cost = 0.0;
		// 35 pipes x [action, prc]
		for (std::size_t i = 0; i < 35; ++i) {
			if (dv[i*2] == 0)
				continue;

			std::string link_id = _anytown_->get_subnetwork("existing_pipes").at(i);
			bool city = _anytown_->is_in_subnetork("city_pipes", link_id);
			// I assume is in the residential as they are mutually exclusive

			if (dv[i*2] == 1) { // duplicate
				auto  pipe_alt_costs = _pipes_alt_costs_.at(dv[i*2+1]);
				if (city) 
					design_cost += pipe_alt_costs.dup_city;
				else
					design_cost += pipe_alt_costs.dup_residential;
			}
			else if (dv[i*2] == 2) { // clean
				// I can't use dv[i*2+1] to get the costs, but Ihave to search for the diameter
				
				// retrieve the link index
				int link_idx = 0;
				int errorcode = EN_getlinkindex(_anytown_->ph_, link_id.c_str(), &link_idx);
				assert(errorcode <= 100);

				// retrieve the link diameter
				double link_diameter = 0.0;
				errorcode = EN_getlinkvalue(_anytown_->ph_, link_idx, EN_DIAMETER, &link_diameter);
				assert(errorcode <= 100);
				
				// found which row of the table _pipe_alt_costs_ starting from the diameter 
				std::size_t row = 0;
				while (row < _pipes_alt_costs_.size()-1 && _pipes_alt_costs_.at(row).diameter != link_diameter) 
					++row;
				
				// IF I haven't found it until the last I put the most expensive one (i.e., the last)
				auto pipe_alt_costs = _pipes_alt_costs_.at(row);
				if (city)
					design_cost += pipe_alt_costs.clean_city;
				else
					design_cost += pipe_alt_costs.clean_residential;
			}
		}
		// 6 pipes x [prc]
		for (std::size_t i = 0; i < 6; ++i) {
			// dv[i] is the row of the _pipes_alt_costs_ table
			auto pipe_alt_costs = _pipes_alt_costs_.at(dv[70+i]);
			design_cost += pipe_alt_costs.new_cost;
		}
		// energy from pumps 
		double total_energy_Wh_perDay = 0.0;
		for (auto& hour : energy) {
			for (auto& pump_energy : hour) {
				total_energy_Wh_perDay += pump_energy;
			}
		}
		double yearly_energy_cost = total_energy_Wh_perDay * energy_cost_kWh / 1000 * days_per_year;
		
		// TODO: tanks costs

		return bevarmejo::net_present_value(design_cost, discount_rate, -yearly_energy_cost, amortization_years);
	
    }

    std::vector<double> ModelAnytown::apply_dv(std::shared_ptr<bevarmejo::WaterDistributionSystem> anytown, const std::vector<double> &dv) const
    {
		std::vector<double> old_HW_coeffs;
		// 1. existing pipes
		for (std::size_t i = 0; i < 35; ++i) {
			// if dv[i*2] == 0 do nothing
			if (dv[i*2] != 0 ){
				// retrieve the link ID
				std::string link_id = anytown->get_subnetwork("existing_pipes").at(i); 
				// now the index associated with the link ID
				int link_idx = 0;
				int errorcode = EN_getlinkindex(anytown->ph_, link_id.c_str(), &link_idx);
				assert(errorcode <= 100);

				if (dv[i*2] == 1) { // duplicate
					// new name is Dxx where xx is the original pipe name
					std::string new_link_id = "D" + link_id;
					int new_link_idx = 0;

					// retrieve the old property of the already existing pipe
					int out_node1_idx = 0;
					int out_node2_idx = 0;
					errorcode = EN_getlinknodes(anytown->ph_, link_idx, &out_node1_idx, &out_node2_idx);
					assert(errorcode <= 100);

					std::string out_node1_id = anytown->get_node_id(out_node1_idx);
					std::string out_node2_id = anytown->get_node_id(out_node2_idx);
					
					// create the new pipe
					errorcode = EN_addlink(anytown->ph_, new_link_id.c_str(), EN_PIPE, out_node1_id.c_str(), out_node2_id.c_str(), &new_link_idx);
					assert(errorcode <= 100);
					
					// change the new pipe properties:
					// 1. diameter =  row dv[i*2+1] column diameter of _pipes_alt_costs_
					// 2. roughness = coeff_HV_new
					// 3. length  = value of link_idx
					double diameter = _pipes_alt_costs_.at(dv[i*2+1]).diameter;
					errorcode = EN_setlinkvalue(anytown->ph_, new_link_idx, EN_DIAMETER, diameter);
					assert(errorcode <= 100);
					// errorcode = EN_setlinkvalue(anytown->ph_, new_link_idx, EN_ROUGHNESS, coeff_HW_new);
					// assert(errorcode <= 100);
					double link_length = 0.0;
					errorcode = EN_getlinkvalue(anytown->ph_, link_idx, EN_LENGTH, &link_length);
					assert(errorcode <= 100);
					errorcode = EN_setlinkvalue(anytown->ph_, new_link_idx, EN_LENGTH, link_length);
					assert(errorcode <= 100);
				}
				else if (dv[i*2] == 2) { // clean
					// retrieve the old HW coefficients
					double link_roughness = 0.0;
					errorcode = EN_getlinkvalue(anytown->ph_, link_idx, EN_ROUGHNESS, &link_roughness);
					old_HW_coeffs.push_back(link_roughness);
				
					// set the new HW coefficients
					errorcode = EN_setlinkvalue(anytown->ph_, link_idx, EN_ROUGHNESS, coeff_HW_cleaned);	
					assert(errorcode <= 100); 
				}
			}
		}

		// 2. new pipes
		for (std::size_t i = 0; i < 6; ++i) {
			// retrieve the link ID from the subnetwork
			std::string link_id = anytown->get_subnetwork("new_pipes").at(i);
			// now the index associated with the link ID
			int link_idx = 0;
			int errorcode = EN_getlinkindex(anytown->ph_, link_id.c_str(), &link_idx);
			assert(errorcode <= 100);

			// change the new pipe properties:
			// diameter =  row dv[70+i] column diameter of _pipes_alt_costs_
			double diameter = _pipes_alt_costs_.at(dv[70+i]).diameter;
			errorcode = EN_setlinkvalue(anytown->ph_, link_idx, EN_DIAMETER, diameter);
			assert(errorcode <= 100);
		}

		// 3. pumps
		auto patterns = decompose_pump_pattern(dv.begin() + 76, dv.end());
		for (std::size_t i = 0; i < patterns.size(); ++i) {
			// I know pump patterns IDs are from 2, 3, and 4
			int pump_idx = i + 2;
			std::string pump_id = std::to_string(pump_idx);
			int errorcode = EN_getpatternindex(anytown->ph_, pump_id.c_str(), &pump_idx);
			assert(errorcode <= 100);

			// set the pattern
			errorcode = EN_setpattern(anytown->ph_, pump_idx, patterns[i].data(), patterns[i].size());
			assert(errorcode <= 100);
		}

        return old_HW_coeffs;
    }

    void ModelAnytown::reset_dv(std::shared_ptr<bevarmejo::WaterDistributionSystem> anytown, const std::vector<double> &dv, const std::vector<double> &old_HW_coeffs) const {
		// Do the opposite operations of apply_dv 
		std::vector<const double>::iterator old_HW_coeffs_iter = old_HW_coeffs.begin();

		// 1. existing pipes
		for (std::size_t i = 0; i <35; ++i ) {
			// if dv[i*2] == 0 do nothing
			if (dv[i*2] != 0) {
				// retrieve the link ID
				std::string link_id = anytown->get_subnetwork("existing_pipes").at(i);
				// now the index associated with the link ID
				int link_idx = 0;
				int errorcode = EN_getlinkindex(anytown->ph_, link_id.c_str(), &link_idx);
				assert(errorcode <= 100);

				if (dv[i*2] == 1) { // remove duplicate
					// duplicate pipe has been named Dxx where xx is the original pipe name
					std::string new_link_id = "D" + link_id;
					int new_link_idx = 0; 
					errorcode = EN_getlinkindex(anytown->ph_, new_link_id.c_str(), &new_link_idx);
					assert(errorcode <= 100);

					// remove the new pipe
					errorcode = EN_deletelink(anytown->ph_, new_link_idx, EN_UNCONDITIONAL);
					assert(errorcode <= 100);
				}
				else if (dv[i*2] == 2) { // reset clean
					// re set the HW coefficients
					errorcode = EN_setlinkvalue(anytown->ph_, link_idx, EN_ROUGHNESS, *old_HW_coeffs_iter);
					assert(errorcode <= 100);

					old_HW_coeffs_iter++;
				}
			}
		}

		// 2. new pipes
		for (std::size_t i = 0; i < 6; ++i) {
			// retrieve the link ID from the subnetwork
			std::string link_id = anytown->get_subnetwork("new_pipes").at(i);
			// now the index associated with the link ID
			int link_idx = 0;
			int errorcode = EN_getlinkindex(anytown->ph_, link_id.c_str(), &link_idx);
			assert(errorcode <= 100);

			// change the new pipe properties:
			double diameter = _nonexisting_pipe_diam_ft;
			errorcode = EN_setlinkvalue(_anytown_->ph_, link_idx, EN_DIAMETER, diameter);
			assert(errorcode <= 100);
		}
	
	}

    std::vector<std::vector<double>> ModelAnytown::decompose_pump_pattern(std::vector<const double>::iterator begin, const std::vector<const double>::iterator end) const {
        // I know it sould be 3 pumps and 24 values but I do it generically as an exercise :)
		auto iter = begin;
		std::size_t n_periods = 0;
		int n_pumps = 0; 
		while (iter != end) {
			// The number of pumps is the max that I find in the vector
			if (*iter > n_pumps) {
				n_pumps = *iter;
			}
			iter++;
			n_periods++;
		}
		
		std::vector<std::vector<double>> patterns (n_pumps, std::vector<double>(n_periods, 0.0));
		for (std::size_t period = 0; period < n_periods; ++period) {
			for (std::size_t pump = 0; pump < *begin; ++pump) {
				patterns[pump][period] = 1.0;
			}
			begin++;
		}

		return patterns;
    }

    bevarmejo::netdata_4_Ir ModelAnytown::convert_to_netdata_4_Ir(const std::vector<double> &heads, const std::vector<double> &flows, const std::vector<double> &energies, 
				const std::string& dnodes_subnet_name, const std::string& res_subnet_name) const
    {
		bevarmejo::netdata_4_Ir network_data{std::vector<double>(_anytown_->get_subnetwork(dnodes_subnet_name).size(),0.0),
								std::vector<double>(_anytown_->get_subnetwork(dnodes_subnet_name).size(),0.0),
								std::vector<double>(_anytown_->get_subnetwork(res_subnet_name).size(),0.0),
								std::vector<double>(_anytown_->get_subnetwork(res_subnet_name).size(),0.0),
								std::vector<double>(energies.size(),0.0)};

		// 1. flows and heads at demand nodes
		for (std::size_t i = 0; i < _anytown_->get_subnetwork(dnodes_subnet_name).size(); ++i) {
			std::string node_id = _anytown_->get_subnetwork(dnodes_subnet_name).at(i);
			int node_idx = 0;
			int errorcode = EN_getnodeindex(_anytown_->ph_, node_id.c_str(), &node_idx);
			assert(errorcode <= 100);

			network_data.flow_at_dnodes.at(i) = flows.at(node_idx);
			network_data.head_at_dnodes.at(i) = heads.at(node_idx);
		}
		// 2. flows and heads at reservoirs
		for (std::size_t i = 0; i < _anytown_->get_subnetwork(res_subnet_name).size(); ++i) {
			std::string node_id = _anytown_->get_subnetwork(res_subnet_name).at(i);
			int node_idx = 0;
			int errorcode = EN_getnodeindex(_anytown_->ph_, node_id.c_str(), &node_idx);
			assert(errorcode <= 100);

			network_data.flow_out_reservoirs.at(i) = flows.at(node_idx);
			network_data.head_at_reservoirs.at(i) = heads.at(node_idx);
		}
		// 3. power at pumps
		// get the step size to move from energy to power
		long delta_t = 0;
		int errorcode = EN_gettimeparam(_anytown_->ph_, EN_HYDSTEP, &delta_t);
		assert(errorcode <= 100);
		for (std::size_t i = 0; i < energies.size(); ++i) {
			network_data.power_at_pumps.at(i) = energies.at(i) / delta_t;
		}

        return network_data;
    }

} /* namespace bevarmejo */