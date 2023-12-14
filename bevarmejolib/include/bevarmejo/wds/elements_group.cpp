#include "elements_group.hpp"

namespace bevarmejo {

int wds::_is_en_object_type_valid(const std::string& en_object_type) {

	int en_object_type_int = 0;
	if (en_object_type == "EN_NODE")
		en_object_type_int = EN_NODE;
	else if (en_object_type == "EN_LINK")
		en_object_type_int = EN_LINK;
	else if (en_object_type == "EN_TIMEPAT")
		en_object_type_int = EN_TIMEPAT;
	else if (en_object_type == "EN_CURVE")
		en_object_type_int = EN_CURVE;
	else if (en_object_type == "EN_CONTROL")
		en_object_type_int = EN_CONTROL;
	else if (en_object_type == "EN_RULE")
		en_object_type_int = EN_RULE;
	else {
		std::ostringstream oss;
		stream_out(oss, "Invalid EN object type: ", en_object_type, "\n");
		throw std::runtime_error(oss.str());
	}

	return en_object_type_int;
}

std::tuple<int, std::vector<std::string>, std::string> wds::__load_egroup_data_from_stream(std::istream& is) {

	int en_object_type = 0;
	std::vector<std::string> ids_list;
	std::string comment;

	// search for the tag #TYPE
	load_dimensions(is, "#TYPE"); 
	// read the type of the elements
	std::string a_obj_type;
	stream_in(is, a_obj_type);
	en_object_type = _is_en_object_type_valid(a_obj_type);

	std::size_t n_elements = load_dimensions(is, "#DATA");
	ids_list = std::vector<std::string>(n_elements);
	stream_in(is, ids_list);

	load_dimensions(is, "#COMMENT");
	stream_in(is, comment);

	return std::make_tuple(en_object_type, ids_list, comment);
}
    
} // namespace bevarmejo 
