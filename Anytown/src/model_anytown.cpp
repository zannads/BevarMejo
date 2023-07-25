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

#include "pugixml.hpp"

#include "bevarmejo/water_distribution_system.hpp"
#include "bevarmejo/io.hpp"

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



	

} /* namespace bevarmejo */