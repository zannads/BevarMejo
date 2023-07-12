//
//  io.cpp
//  BèvarMéjo
//
//  Created by Dennis Zanutto on 06/07/23.
//

#include <iostream>
#include <sstream>
#include <string_view>

#include "io.hpp"

namespace bevarmejo {

	GenPurTable read_g_p_table(std::istream& is, const std::string_view tag) {
		GenPurTable table;
		std::string line;
		while (std::getline(is, line)) {
			if (line.find(tag) != std::string::npos) {
				std::stringstream ss(line);
				ss >> table.name;
				// I know dimensions follow immediately after the name in the same row
				table.dimensions = get_g_p_table_dimensions(ss);

				// Data are from the next line onwards
				table.data = get_g_p_table_data(is, table.dimensions);

				// TODO: Check if data are consistent with dimensions

				is.clear();
				is.seekg(0, std::ios::beg);
				return table;
			}
		}
	}

	std::vector<unsigned int> get_g_p_table_dimensions(std::istream& is) {
		std::vector<unsigned int> dimensions;

		// If there are no numbers after the name, return only 1 dimension
		if (is.eof()) {
			dimensions.push_back(1u);
			is.clear();
			is.seekg(0, std::ios::beg);
			return dimensions;
		}

		// It means we have at least 1 dimension
		while (!is) {
			unsigned int current_dim;
			is >> current_dim;
			dimensions.push_back(current_dim);
		}
		is.clear();
		is.seekg(0, std::ios::beg);
		return dimensions;
	}

	std::vector<std::string> get_g_p_table_data(
            std::istream& is, const std::vector<unsigned int>& dimensions) {
                if (dimensions.size() == 0) {
                        return std::vector<std::string>();
                }

                if (dimensions.size() == 1) {
                        unsigned int n = dimensions[0];
                        std::vector<std::string> data(n);
                        unsigned int i = 0;
                        std::string line;
                        while (std::getline(is, line) && i < n) {
							line.erase(std::remove(
								line.begin(), line.end(), '\t'), line.end());

							data[i] = line;
							++i;
                        }
                        return data;
                }
                // TODO: implement for more than 1 dimension
        }



	std::stringstream get_data_from_tag(std::istream& is, const std::string_view tag) {
		std::stringstream ss;
		std::string line;

		while (std::getline(is, line)) {
			if (line.find(tag) != std::string::npos) {
				while (std::getline(is, line)) {
					if (line.empty()) {
						is.clear();
						is.seekg(0, std::ios::beg);
						return ss;
					}
					ss << line << std::endl;
				}
			}
		}
		is.clear();
		is.seekg(0, std::ios::beg);
		return ss;
	}

} /* namespace bevarmejo */
