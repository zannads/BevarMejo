//
//  subnetwork.hpp
//  bevarmejo cpp library
//
//  Created by Dennis Zanutto on 12/07/23.
//

#ifndef BEVARMEJOLIB__WDS__SUBNETWORK_HPP
#define BEVARMEJOLIB__WDS__SUBNETWORK_HPP

#include <filesystem>
#include <functional> // for std::hash
#include <iostream>
#include <string>
#include <vector>

#include "epanet2_2.h"

namespace bevarmejo {
namespace wds {
	class subnetwork
	{
		/* Constructors and Destructors */
	public:
		subnetwork() = default;
		subnetwork(std::filesystem::path subnetwork_filename) {
			load_subnetwork(subnetwork_filename); };
		subnetwork(std::string name) : _name_(name) {};
		/*
		subnetwork(std::string en_object_type, std::vector<int> subnetwork_list);
		subnetwork(std::string en_object_type, std::vector<std::string> subnetwork_list);
		subnetwork(std::string en_object_type, std::vector<int> subnetwork_list, EN_Project ph);
		subnetwork(std::string en_object_type, std::vector<std::string> subnetwork_list, EN_Project ph);
		*/
		//~subnetwork();

		// Equality operator checks only the name for uniqueness 
		// Two networks with the same name are considered equal
		bool operator==(const subnetwork& rhs) const {
			return _name_ == rhs._name_;
		}

		/* Members */
	private:
		std::string _name_;
		int _en_object_type_;
		std::vector<std::string> _subnetwork_list_;
		std::string _comment_;

		/* Methods */
	public: 
		void load_subnetwork(std::filesystem::path subnetwork_filename);
		std::size_t size() const;

		/* Getters */
		std::string name() const;
		std::vector<std::string> subnetwork_list() const; // return full list
		std::string at(const int index) const; // return element at index from the list

	private: 
		void _load_subnetwork(std::istream& is);

	private:
		int _is_en_object_type_valid(const std::string& en_object_type) const;

	}; // class subnetwork

} // namespace wds
} // namespace bevarmejo

// Hash function for subnetwork
namespace std {
    template<>
    struct hash<bevarmejo::wds::subnetwork> {
        size_t operator()(const bevarmejo::wds::subnetwork &s) const {
            return std::hash<std::string>()(s.name()); // use the hash function for std::string
        }
    };
}

#endif // BEVARMEJOLIB__WDS__SUBNETWORK_HPP