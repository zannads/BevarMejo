// 
//  model_anytown.cpp
//	anytown original test
//
//  Created by Dennis Zanutto on 11/07/23.

#include <iostream>
#include <string>
#include <vector>

#include "bevarmejo/io.hpp"

#include "prob_anytown.hpp"

namespace bevarmejo {

std::istream& anytown::operator>>(std::istream& is, anytown::tanks_costs& tc)
{
	io::stream_in(is, tc.volume_gal); is.ignore(1000, ';');
	io::stream_in(is, tc.cost);

	return is;
}

std::istream& anytown::operator>>(std::istream& is, anytown::pipes_alt_costs& pac) {
	io::stream_in(is, pac.diameter_in); is.ignore(1000, ';');

	io::stream_in(is, pac.new_cost); is.ignore(1000, ';');
	io::stream_in(is, pac.dup_city); is.ignore(1000, ';');
	io::stream_in(is, pac.dup_residential); is.ignore(1000, ';');
	io::stream_in(is, pac.clean_city); is.ignore(1000, ';');
	io::stream_in(is, pac.clean_residential);

	return is;
}

std::vector<std::vector<double>> anytown::decompose_pumpgroup_pattern(std::vector<double> pg_pattern, const std::size_t n_pumps) {
	// I want a copy of the decision variables because every time I put a 
	// pattern to 1 I want to remove it from the vector.
	std::size_t n_periods = pg_pattern.size();
	std::vector<std::vector<double>> patterns (n_pumps, std::vector<double>(n_periods, 0.0));

	for (auto& pump_pattern : patterns) {
		auto it = pg_pattern.begin();
		for (auto& val : pump_pattern){
			if (*it > 0.0) {
				val = 1.0;
				--(*it);
			}
			++it;
		}
	}

	return patterns;
}

} /* namespace bevarmejo */