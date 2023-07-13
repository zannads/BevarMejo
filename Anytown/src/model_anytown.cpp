// 
//  model_anytown.cpp
//	anytown original test
//
//  Created by Dennis Zanutto on 11/07/23.

#include <cassert>
#include <filesystem>
#include <iostream>
#include <memory>

#include "pugixml.hpp"

#include "model_anytown.hpp"

namespace fsys = std::filesystem;
namespace bevarmejo {

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
	}



} /* namespace bevarmejo */