//
//  water_distribution_system.hpp
//  bevarmejo cpp library
//
//  Created by Dennis Zanutto on 04/07/23.
//

#ifndef BEVARMEJOLIB__WDS__WATER_DISTRIBUTION_SYSTEM_HPP
#define BEVARMEJOLIB__WDS__WATER_DISTRIBUTION_SYSTEM_HPP

#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <string>
#include <unordered_set>
#include <vector>

#include "epanet2_2.h"

#include "bevarmejo/io.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/link.hpp"
#include "bevarmejo/wds/elements/junction.hpp"
#include "bevarmejo/wds/elements/source.hpp"
#include "bevarmejo/wds/elements/tank.hpp"
#include "bevarmejo/wds/elements/reservoir.hpp"
#include "bevarmejo/wds/elements/dimensioned_link.hpp"
#include "bevarmejo/wds/elements/pipe.hpp"
#include "bevarmejo/wds/elements/pump.hpp"
#include "bevarmejo/wds/elements/curve.hpp"
#include "bevarmejo/wds/elements/curves.hpp"
#include "bevarmejo/wds/elements/pattern.hpp"

#include "bevarmejo/wds/elements_group.hpp"

namespace bevarmejo {

namespace wds {

static const std::string l__DEMAND_NODES = "Demand Nodes";

class water_distribution_system {

    /*--- Attributes ---*/
public:
    // Handler for the project.
    // Public because I may want to modify it (e.g., apply a decision vector).
    // it is just faster than doing create an interface. I will be careful.
    mutable EN_Project ph_;
    using SubnetworksMap = std::unordered_map<std::string, Subnetwork>;
    using ElemGroupsMap = std::unordered_map<std::string, ElementsGroup<Element>>;

    
protected:
    // Path to the inp file from which the project will be uploaded.
    std::filesystem::path _inp_file_;
    // Collectionf of elements of the network
    std::vector<std::shared_ptr<Element>> _elements_;
    
    // Class-specific collections of elements
    Nodes _nodes_;
    Links _links_;
    Patterns _patterns_;
    Junctions _junctions_;
    // Sources _sources_;
    Tanks _tanks_;
    Reservoirs _reservoirs_;
    Pipes _pipes_;
    Pumps _pumps_;
    Curves _curves_;

    // User defined groups of elements (subnetworks is only for nodes and links)
    // while groups can be defined for any type of element.
    std::unordered_map<std::string, Subnetwork> _subnetworks_;
    std::unordered_map<std::string, ElementsGroup<Element>> _groups_;
    

/*--- Constructors ---*/ 
public:
    // Default constructor
    water_distribution_system();
    
    water_distribution_system(const std::filesystem::path& inp_file);
    
    ~water_distribution_system();
    
    // Equivalent to constuctor from .inp file
    void load_from_inp_file(const std::filesystem::path& inp_file, std::function<void (EN_Project)> preprocessf = [](EN_Project ph){ return;});
    
/*--- Getters and setters ---*/
public:
    const std::filesystem::path& inp_file() const {return _inp_file_;}; // you can't change it from outside
    // you can't modify the inpfile, but you will be able to reset passing a new file
    
    /*--- Object-specific Subnetworks ---*/
    const Nodes& nodes() const {return _nodes_;};
    const Links& links() const {return _links_;};
    const Patterns& patterns() const {return _patterns_;};
    const Junctions& junctions() const {return _junctions_;};
    // const Sources& sources() const {return _sources_;};
    const Tanks& tanks() const {return _tanks_;};
    const Reservoirs& reservoirs() const {return _reservoirs_;};
    const Pipes& pipes() const {return _pipes_;};
    const Pumps& pumps() const {return _pumps_;};
    const Curves& curves() const {return _curves_;};

    /*--- User-defined Subnetworks ---*/
    SubnetworksMap& subnetworks() {return _subnetworks_;};
    const SubnetworksMap& subnetworks() const {return _subnetworks_;};
    void add_subnetwork(const std::string& name, const Subnetwork& subnetwork);
    void add_subnetwork(const std::pair<std::string, Subnetwork>& subnetwork);
    void add_subnetwork(const std::filesystem::path& filename);
    Subnetwork& subnetwork(const std::string& name);
    const Subnetwork& subnetwork(const std::string& name) const;
    void remove_subnetwork(const std::string& name);

    /*--- User-defined Elements Groups ---*/
    
    
/*--- Methods ---*/
public:
    // Cache the indices of the elements in the network.
    // This is useful to avoid calling the ENgetnodeindex and ENgetlinkindex functions every time.
    void cache_indices() const;
    void assign_patterns_EN();
    void assign_demands_EN();
    void assign_curves_EN();
    void connect_network_EN();
    
    void run_hydraulics() const;
private:
    template <typename T>
    std::pair<std::string, ElementsGroup<T>> load_egroup_from_file(const std::filesystem::path& file_path);





public:
    std::string get_node_id(int index) const;

}; // class water_distribution_system

} // namespace wds

using WDS = wds::water_distribution_system; // short name for water_distribution_system

} // namespace bevarmejo


template <typename T>
std::pair<std::string, bevarmejo::wds::ElementsGroup<T>> bevarmejo::wds::water_distribution_system::load_egroup_from_file(const std::filesystem::path& file_path) {
	
	// A group of elements is completely defined by the following attributes:
	// 0. The name (the name of the file)
	// The element itself, whcih is defined by:
	// 1. The type of the elements (find inside the file with the tag #TYPE)
	// 2. The list of elements (find inside the file with the tag #DATA)
	// 3. The comment (find inside the file with the tag #COMMENT)

	int en_object_type = 0;
	std::vector<std::string> ids_list;
	std::string comment;
	std::string name;

	// checks if file exists
	if (!std::filesystem::exists(file_path)) {
		std::ostringstream oss;
		stream_out(oss, "File ", file_path, " does not exist.\n");
		throw std::runtime_error(oss.str());
	}

	// open file
	std::ifstream ifs(file_path);
	if (!ifs.is_open()) {
		std::ostringstream oss;
		stream_out(oss, "File ", file_path, " not opened.\n");
		throw std::runtime_error(oss.str());
	}

	name = file_path.stem().string();

	try {
		// call internal function to load the parameters
		std::tie(en_object_type, ids_list, comment) = __load_egroup_data_from_stream(ifs);
	}
	catch (const std::exception& e) {
		std::ostringstream oss;
		stream_out(oss, "Error while loading subnetwork ", file_path, "\n");
		stream_out(oss, e.what(), "\n");
		throw std::runtime_error(oss.str());
	}

	// If we arrived here it means that the file has been loaded correctly
	// and we can create the group.

	ElementsGroup<T> elements_group;
	
	elements_group.comment(comment);

    for (const auto& id : ids_list ) {

        //TODO: If I change all the containers _nodes_, _links_, etc. to the class ElementsGroup
        // I can simply use the method element(const std::string&) to get the element and add it to the group.
        if (en_object_type == EN_NODE) {
            auto it = std::find_if(_nodes_.begin(), _nodes_.end(), 
                [&id](const std::shared_ptr<Node>& node) { return node->id() == id; });
            if (it != _nodes_.end()) 
                elements_group.insert(*it);
        }
        else if (en_object_type == EN_LINK) {
            auto it = std::find_if(_links_.begin(), _links_.end(), 
                [&id](const std::shared_ptr<Link>& link) { return link->id() == id; });
            if (it != _links_.end()) 
                elements_group.insert(*it);
        }/*
        else if (en_object_type == EN_TIMEPAT) {
            auto it = std::find_if(_patterns_.begin(), _patterns_.end(), 
                [&id](const std::shared_ptr<Pattern>& pattern) { return pattern->id() == id; });
            if (it != _patterns_.end()) 
                elements_group.insert(*it);
        }
        else if (en_object_type == EN_CURVE) {
            auto it = std::find_if(_curves_.begin(), _curves_.end(), 
                [&id](const std::shared_ptr<Curve>& curve) { return curve->id() == id; });
            if (it != _curves_.end()) 
                elements_group.insert(*it);
        }
        else if (en_object_type == EN_CONTROL) {
            auto it = std::find_if(_controls_.begin(), _controls_.end(), 
                [&id](const std::shared_ptr<Control>& control) { return control->id() == id; });
            if (it != _controls_.end()) 
                elements_group.insert(*it);
        }
        else if (en_object_type == EN_RULE) {
            auto it = std::find_if(_rules_.begin(), _rules_.end(), 
                [&id](const std::shared_ptr<Rule>& rule) { return rule->id() == id; });
            if (it != _rules_.end()) 
                elements_group.insert(*it);
        }*/
    }
    
	return std::make_pair(name, elements_group);
}

#endif // BEVARMEJOLIB__WDS__WATER_DISTRIBUTION_SYSTEM_HPP
