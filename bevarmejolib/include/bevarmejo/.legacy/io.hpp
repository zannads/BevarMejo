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

} // namespace bevarmejo::io

#endif /* BEVARMEJOLIB_IO_HPP */
