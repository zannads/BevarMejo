//
//  subnetwork.hpp
//  bevarmejo cpp library
//
//  Created by Dennis Zanutto on 12/07/23.
//

#ifndef BEVARMEJOLIB__SUBNETWORK_HPP
#define BEVARMEJOLIB__SUBNETWORK_HPP

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "epanet2_2.h"

namespace bevarmejo {
	struct en_gp_object {
		std::string id;
		int index{ 0 };
	};

	class Subnetwork
	{
		/* Constructors and Destructors */
	public:
		Subnetwork() = default;
		Subnetwork(std::filesystem::path subnetwork_filename) {
			load_subnetwork(subnetwork_filename); };
		/*
		Subnetwork(std::string en_object_type, std::vector<int> subnetwork_list);
		Subnetwork(std::string en_object_type, std::vector<std::string> subnetwork_list);
		Subnetwork(std::string en_object_type, std::vector<int> subnetwork_list, EN_Project ph);
		Subnetwork(std::string en_object_type, std::vector<std::string> subnetwork_list, EN_Project ph);
		*/
		//~Subnetwork();

		/* Members */
	private:
		std::string _en_object_type_;
		std::vector<en_gp_object> _subnetwork_list_;
		std::string _comment_;

		/* Methods */
	public: 
		void load_subnetwork(std::filesystem::path subnetwork_filename);
	private: 
		void _load_subnetwork(std::istream& is);

	public:
		void fill_subnetwork_list(EN_Project ph); //TODO: implement

	private:
		bool _is_mapped() const;

	}; /* class Subnetwork */

	

} /* namespace bevarmejo */

#endif /* BEVARMEJOLIB__SUBNETWORK_HPP */