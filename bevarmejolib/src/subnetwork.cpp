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

		try {
			// call internal function to load subnetwork
			_load_subnetwork(ifs);
		}
		catch (const std::exception& e) {
			std::ostringstream oss;
			oss << "Error while loading subnetwork " << subnetwork_filename << std::endl;
			oss << e.what() << std::endl;
			throw std::runtime_error(oss.str());
		}
	}

	void Subnetwork::_load_subnetwork(std::istream& is)
	{
		auto info = read_g_p_table(is, "#TYPE");
		_en_object_type_ = _is_en_object_type_valid(info.data[0]);


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

	int Subnetwork::_is_en_object_type_valid(const std::string& en_object_type) const
	{
		int en_object_type_int = 0;
		if (en_object_type == "EN_NODE") {
			en_object_type_int = EN_NODE;
		}
		else if (en_object_type == "EN_LINK") {
			en_object_type_int = EN_LINK;
		}
		else if (en_object_type == "EN_TIMEPAT") {
			en_object_type_int = EN_TIMEPAT;
		}
		else if (en_object_type == "EN_CURVE") {
			en_object_type_int = EN_CURVE;
		}
		else if (en_object_type == "EN_CONTROL") {
			en_object_type_int = EN_CONTROL;
		}
		else if (en_object_type == "EN_RULE") {
			en_object_type_int = EN_RULE;
		}
		else {
			std::ostringstream oss;
			oss << "Invalid EN object type: " << en_object_type << std::endl;
			throw std::runtime_error(oss.str());
		}

		return en_object_type_int;
	}

} /* namespace bevarmejo */