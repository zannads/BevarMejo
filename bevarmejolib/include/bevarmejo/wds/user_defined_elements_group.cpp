#include <memory>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <utility>
#include <vector>

#include "bevarmejo/io.hpp"

#include "bevarmejo/epanet_helpers/en_help.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/link.hpp"
#include "bevarmejo/wds/elements/junction.hpp"
#include "bevarmejo/wds/elements/source.hpp"
#include "bevarmejo/wds/elements/tank.hpp"
#include "bevarmejo/wds/elements/reservoir.hpp"
#include "bevarmejo/wds/elements/dimensioned_link.hpp"
#include "bevarmejo/wds/elements/pipe.hpp"
#include "bevarmejo/wds/elements/pump.hpp"
//#include "bevarmejo/wds/elements/valve.hpp"
#include "bevarmejo/wds/auxiliary/pattern.hpp"
#include "bevarmejo/wds/auxiliary/curve.hpp"
#include "bevarmejo/wds/elements_group.hpp"

#include "user_defined_elements_group.hpp"

namespace bevarmejo {

std::tuple<int, std::vector<std::string>, std::string> wds::__load_egroup_data_from_stream(std::istream& is) {

	int en_object_type = 0;
	std::vector<std::string> ids_list;
	std::string comment;

	// search for the tag #TYPE
	io::load_dimensions(is, "#TYPE"); 
	// read the type of the elements
	std::string a_obj_type;
	io::stream_in(is, a_obj_type);
	en_object_type = epanet::is_string_en_object_type(a_obj_type);

	std::size_t n_elements = io::load_dimensions(is, "#DATA");
	ids_list = std::vector<std::string>(n_elements);
	io::stream_in(is, ids_list);

	io::load_dimensions(is, "#COMMENT");
	io::stream_in(is, comment);

	return std::make_tuple(en_object_type, ids_list, comment);
}
    
} // namespace bevarmejo 
