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

	std::size_t Subnetwork::size() const { return _subnetwork_list_.size(); }

	/* Getters */
	std::string Subnetwork::name() const { return _name_; }
	std::vector<std::string> Subnetwork::subnetwork_list() const { return _subnetwork_list_; }
	std::string Subnetwork::at(const int index) const { return _subnetwork_list_.at(index); }


	void Subnetwork::_load_subnetwork(std::istream& is)
	{
		std::size_t dimensions = load_dimensions(is, "#TYPE"); 
		std::string en_object_type;
		stream_in(is, en_object_type);
		_en_object_type_ = _is_en_object_type_valid(en_object_type);

		dimensions = load_dimensions(is, "#DATA");
		std::vector<std::string> subnetwork_list(dimensions);
		stream_in(is, subnetwork_list);
		_subnetwork_list_ = subnetwork_list;

		dimensions = load_dimensions(is, "#COMMENT");
		stream_in(is, _comment_);
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
