//
//  io/streams.hpp
//  BèvarMéjo
//
//  Created by Dennis Zanutto on 06/07/23.
//

#ifndef BEVARMEJOLIB_IO_HPP
#define BEVARMEJOLIB_IO_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "bevarmejo/io/streams.hpp"
#include "bevarmejo/wds/epanet_helpers/en_help.hpp"

namespace bevarmejo::io {
namespace inp::detail {

template <typename P>
inline void temp_net_to_file(const P& p, const std::vector<double>& dv, const std::string& out_file) {
    p.save_solution(dv, out_file);
}

} // namespace inp::detail


/* LOAD dimensions from TAG
* Special type of stream input for tag */
inline std::size_t load_dimensions(std::istream& is, const std::string_view tag) {
    std::size_t dimensions = 0;

    std::string line;
    while( getline(is, line) ) {
        if (line.find(tag) != std::string::npos) {
            std::istringstream iss(line);
            // consume the tag
            stream_in(iss, line);
            // the size is: 
            stream_in(iss, dimensions);

            // When I don't write anything, the dimensions is 0, but I mean it to be 1
            dimensions = dimensions == 0 ? 1 : dimensions;

            return dimensions;
		}
    }

    // If I'm here either it has finished the file
    std::ostringstream oss;
    oss << "Tag " << tag << " not found." << std::endl;
    throw std::runtime_error(oss.str());
}

inline std::tuple<int, std::vector<std::string>, std::string> get_egroup_data(std::istream& is)
{
	// search for the tag #TYPE
	load_dimensions(is, "#TYPE"); 
	// read the type of the elements
	auto a_obj_type = std::string();
	stream_in(is, a_obj_type);
	int en_object_type = bevarmejo::epanet::is_string_en_object_type(a_obj_type);

	std::size_t n_elements = load_dimensions(is, "#DATA");
	auto ids_list = std::vector<std::string>(n_elements);
	stream_in(is, ids_list);

	load_dimensions(is, "#COMMENT");
    auto comment = std::string();
	stream_in(is, comment);

	return std::make_tuple(en_object_type, ids_list, comment);
}

} // namespace bevarmejo::io

#endif /* BEVARMEJOLIB_IO_HPP */
