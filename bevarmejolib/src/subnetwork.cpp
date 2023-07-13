//
//  subnetwork.cpp
//  bevarmejo cpp library
//
//  Created by Dennis Zanutto on 12/07/23.
//
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "epanet2_2.h"

#include "io.hpp"

#include "subnetwork.hpp"

namespace bevarmejo {

	void Subnetwork::load_subnetwork(std::filesystem::path subnetwork_filename)
	{
		// checks if file exists
		if (!std::filesystem::exists(subnetwork_filename)) {
			std::ostringstream oss;
			oss << "File " << subnetwork_filename << " does not exist." << std::endl;
			throw std::runtime_error(oss.str());
		}

		// open file
		std::ifstream ifs(subnetwork_filename);
		if (!ifs.is_open()) {
			std::ostringstream oss;
			oss << "File " << subnetwork_filename << " not opened." << std::endl;
			throw std::runtime_error(oss.str());
		}

		_name_ = subnetwork_filename.stem().string();

		// call internal function to load subnetwork
		_load_subnetwork(ifs);
	}

	void Subnetwork::_load_subnetwork(std::istream& is)
	{
		auto info = read_g_p_table(is, "#TYPE");
		_en_object_type_ = info.data[0]; // TODO: should check if it is a valid type


		info = read_g_p_table(is, "#DATA");
		for (auto& row : info.data) {
			en_couple en_object;
			en_object.id = row[0];
			_subnetwork_list_.push_back(en_object);
		}

		info = read_g_p_table(is, "#COMMENT");
		_comment_ = info.data[0]; 

		return;	
	}

	bool Subnetwork::_is_mapped() const
	{
		// if at least one element is zero, the subnetwork is not mapped
		bool is_mapped = true;
		for (auto& en_object : _subnetwork_list_) {
			if (en_object.index == 0) {
				is_mapped = false;
				return is_mapped;
			}
		}

		// TODO: if two elements have the same index, I should not trust the mapping

		return is_mapped;
	}

} /* namespace bevarmejo */