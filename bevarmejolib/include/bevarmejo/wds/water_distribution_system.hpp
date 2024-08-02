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

#include "bevarmejo/wds/epanet_helpers/en_help.hpp"
#include "bevarmejo/wds/epanet_helpers/en_time_options.hpp"

#include "bevarmejo/io.hpp"

#include "bevarmejo/wds/auxiliary/time_series.hpp"
#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/element.hpp"

#include "bevarmejo/wds/elements_group.hpp"
#include "bevarmejo/wds/user_defined_elements_group.hpp"

#include "bevarmejo/wds/auxiliary/curve.hpp"
#include "bevarmejo/wds/auxiliary/curves.hpp"
#include "bevarmejo/wds/auxiliary/pattern.hpp"

#include "bevarmejo/wds/elements/network_element.hpp"

#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/junction.hpp"
#include "bevarmejo/wds/elements/source.hpp"
#include "bevarmejo/wds/elements/reservoir.hpp"
#include "bevarmejo/wds/elements/tank.hpp"

#include "bevarmejo/wds/elements/link.hpp"
#include "bevarmejo/wds/elements/dimensioned_link.hpp"
#include "bevarmejo/wds/elements/pipe.hpp"
#include "bevarmejo/wds/elements/pump.hpp"
// #include "bevarmejo/wds/elements/valve.hpp"



namespace bevarmejo {
namespace wds {

static const std::string l__DEMAND_NODES = "Demand Nodes";
static const std::string l__CONSTANT_TS = "Cst";
static const std::string l__PATTERN_TS = "ENPatt";
static const std::string l__RESULT_TS = "Res";

class NetworkElement;

template <typename T>
class UserDefinedElementsGroup;
using Subnetwork = UserDefinedElementsGroup<NetworkElement>;

class Reservoir;
using Reservoirs = ElementsGroup<Reservoir>;
class Tank;
using Tanks = ElementsGroup<Tank>;
class Node;
using Nodes = ElementsGroup<Node>;
class Junction;
using Junctions = ElementsGroup<Junction>;
class Source;
using Sources = ElementsGroup<Source>;
class Link;
using Links = ElementsGroup<Link>;
class Pipe;
using Pipes = ElementsGroup<Pipe>;
class Pump;
using Pumps = ElementsGroup<Pump>;

class Pattern;
using Patterns = ElementsGroup<Pattern>;

class Curve;
using Curves = ElementsGroup<Curve>;

class WaterDistributionSystem {

    /*--- Attributes ---*/
public:
    // Handler for the project.
    // Public because I may want to modify it (e.g., apply a decision vector).
    // it is just faster than doing create an interface. I will be careful.
    mutable EN_Project ph_;
    using SubnetworksMap = std::unordered_map<std::string, Subnetwork>;
    using ElemGroupsMap = std::unordered_map<std::string, UserDefinedElementsGroup<Element>>;
    using TimeSeriesMap= std::unordered_map<std::string, aux::TimeSeries>; 

protected:
    // Path to the inp file from which the project will be uploaded.
    std::filesystem::path _inp_file_;
    // Collectionf of elements of the network
    std::vector<std::shared_ptr<Element>> _elements_;
    
    // Class-specific collections of elements
    Nodes _nodes_;
    Junctions _junctions_;
    // Sources _sources_;
    Tanks _tanks_;
    Reservoirs _reservoirs_;

    Links _links_;
    Pumps _pumps_;
    // DimensionedLinks _dimensioned_links_;
    Pipes _pipes_;
    // Valves _valves_;

    struct AuxiliaryElements { 
        Patterns patterns;
        Curves curves;
        // Controls controls;
        // Rules rules;

        
    } m__aux_elements_;

    // User defined groups of elements (subnetworks is only for nodes and links)
    // while groups can be defined for any type of element.
    std::unordered_map<std::string, Subnetwork> _subnetworks_;
    std::unordered_map<std::string, UserDefinedElementsGroup<Element>> _groups_;
    
    struct ConfigOptions {
        bool save_all_hsteps = true;                // Bool to turn on/off the report behaviour like in EPANET
        struct TimeOptions {
            epanet::GlobalTimeOptions global;
            epanet::PatternTimeOptions pattern;
        } times;
    } m__config_options;

    // Keep the relevant times here:
    mutable TimeSeriesMap m__time_series_map;

/*--- Constructors ---*/ 
public:
    // Default constructor
    WaterDistributionSystem();
    
    WaterDistributionSystem(const std::filesystem::path& inp_file, std::function<void (EN_Project)> preprocessf = [](EN_Project ph){ return;});

    // Copy constructor
    WaterDistributionSystem(const WaterDistributionSystem& other);

    // Move constructor
    WaterDistributionSystem(WaterDistributionSystem&& other) noexcept;

    // Copy assignment operator
    WaterDistributionSystem& operator=(const WaterDistributionSystem& rhs);

    // Move assignment operator
    WaterDistributionSystem& operator=(WaterDistributionSystem&& rhs) noexcept;

    // Destructor
    ~WaterDistributionSystem();

    std::unique_ptr<WaterDistributionSystem> clone() const;
    
/*--- Getters and setters ---*/
public:
    const std::filesystem::path& inp_file() const {return _inp_file_;}; // you can't change it from outside
    // you can't modify the inpfile, but you will be able to reset passing a new file
    
    /*--- Object-specific Subnetworks ---*/
    const Nodes& nodes() const {return _nodes_;};
    const Links& links() const {return _links_;};
    const Patterns& patterns() const {return m__aux_elements_.patterns;};
    const Junctions& junctions() const {return _junctions_;};
    // const Sources& sources() const {return _sources_;};
    const Tanks& tanks() const {return _tanks_;};
    const Reservoirs& reservoirs() const {return _reservoirs_;};
    const Pipes& pipes() const {return _pipes_;};
    const Pumps& pumps() const {return _pumps_;};
    const Curves& curves() const {return m__aux_elements_.curves;};

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

    const aux::TimeSeries& time_series(const std::string& name) const;
    
    
/*--- Methods ---*/
public:
    // Cache the indices of the elements in the network.
    // This is useful to avoid calling the ENgetnodeindex and ENgetlinkindex functions every time.
    void cache_indices() const;
private:
    // Equivalent to constuctor from .inp file
    void load_EN_time_settings(EN_Project ph);
    void load_EN_analysis_options(EN_Project ph);
    void load_EN_patterns(EN_Project ph);
    void load_EN_curves(EN_Project ph);
    void load_EN_nodes(EN_Project ph);
    void load_EN_links(EN_Project ph);
    void load_EN_controls(EN_Project ph);
    void load_EN_rules(EN_Project ph);
public:
    void clear_results() const;
    
    void run_hydraulics() const;

    /*--- ?? ---*/
    template <typename T>
    std::vector<std::shared_ptr<Element>>::iterator insert(const std::shared_ptr<T>& a_element);

    template <typename T>
    std::vector<std::shared_ptr<Element>>::iterator remove(const std::shared_ptr<T>& a_element);

private:
    template <typename T>
    std::pair<std::string, UserDefinedElementsGroup<T>> load_egroup_from_file(const std::filesystem::path& file_path);

}; // class WaterDistributionSystem

} // namespace wds

using WDS = wds::WaterDistributionSystem; // short name for WaterDistributionSystem

} // namespace bevarmejo


/*--- Implementation ---*/
template <typename T>
typename std::vector<std::shared_ptr<bevarmejo::wds::Element>>::iterator bevarmejo::wds::WaterDistributionSystem::insert(const std::shared_ptr<T>& a_element) {
    if (a_element == nullptr)
        return _elements_.end();

    // TODO: I should in some way, check if the element is already in the network.

    _elements_.push_back(std::static_pointer_cast<bevarmejo::wds::Element, T>(a_element));

    // now, based on the type of T, I should add the element to the specific container.
    // if Element, nothing else to do.
    // If other types, add to the specific container.
    // TODO: links should also register themselves to the nodes.
    if constexpr (std::is_same_v<T, bevarmejo::wds::Node>) {
        _nodes_.insert(a_element);
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Junction>) {
        _nodes_.insert(a_element);
        _junctions_.insert(a_element);
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Tank>) {
        _nodes_.insert(a_element);
        _tanks_.insert(a_element);
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Reservoir>) {
        _nodes_.insert(a_element);
        _reservoirs_.insert(a_element);
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Link>) {
        _links_.insert(a_element);
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Pipe>) {
        _links_.insert(a_element);
        _pipes_.insert(a_element);
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Pump>) {
        _links_.insert(a_element);
        _pumps_.insert(a_element);
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Pattern>) {
        m__aux_elements_.patterns.insert(a_element);
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Curve>) {
        m__aux_elements_.curves.insert(a_element);
    } else {
        // Handle other types
        // ...
        // good question, for sure I don't add it to anything else, 
        // but if I'm here I will be here at compile time so maybe warning??
    }

    // Since I modified the network, I should reset all the results as they were
    // for a previous simulation.

    return _elements_.end() - 1;
}

template <typename T>
typename std::vector<std::shared_ptr<bevarmejo::wds::Element>>::iterator bevarmejo::wds::WaterDistributionSystem::remove(const std::shared_ptr<T>& a_element) {
    if (a_element == nullptr)
        return _elements_.end();

    // first of all remove it from the specific containers
    if constexpr (std::is_same_v<T, bevarmejo::wds::Node>) {
        _nodes_.remove(a_element);
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Junction>) {
        _nodes_.remove(a_element);
        _junctions_.remove(a_element);
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Tank>) {
        _nodes_.remove(a_element);
        _tanks_.remove(a_element);
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Reservoir>) {
        _nodes_.remove(a_element);
        _reservoirs_.remove(a_element);
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Link>) {
        _links_.remove(a_element);
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Pipe>) {
        _links_.remove(a_element);
        _pipes_.remove(a_element);

        // TODO: the same thing but for all types!
        // FIX: when I will connect the network, I will have to remove the links from the nodes.
        //a_element->from_node()->remove_link(a_element.get());
        //a_element->to_node()->remove_link(a_element.get());

    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Pump>) {
        _links_.remove(a_element);
        _pumps_.remove(a_element);
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Pattern>) {
        m__aux_elements_.patterns.remove(a_element);
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Curve>) {
        m__aux_elements_.patterns.remove(a_element);
    } else {
        // Handle other types
        // ...
        // good question, for sure I don't add it to anything else, 
        // but if I'm here I will be here at compile time so maybe warning??
    }

    // now remove it from the general container
    std::vector<std::shared_ptr<bevarmejo::wds::Element>>::iterator next_it;
    auto it = std::find(_elements_.begin(), _elements_.end(), a_element);
    if (it != _elements_.end())
        next_it = _elements_.erase(it);

    
    return next_it;
}

template <typename T>
std::pair<std::string, bevarmejo::wds::UserDefinedElementsGroup<T>> bevarmejo::wds::WaterDistributionSystem::load_egroup_from_file(const std::filesystem::path& file_path) {
	
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
		io::stream_out(oss, "File ", file_path, " does not exist.\n");
		throw std::runtime_error(oss.str());
	}

	// open file
	std::ifstream ifs(file_path);
	if (!ifs.is_open()) {
		std::ostringstream oss;
		io::stream_out(oss, "File ", file_path, " not opened.\n");
		throw std::runtime_error(oss.str());
	}

	name = file_path.stem().string();

	try {
		// call internal function to load the parameters
		std::tie(en_object_type, ids_list, comment) = __load_egroup_data_from_stream(ifs);
	}
	catch (const std::exception& e) {
		std::ostringstream oss;
		io::stream_out(oss, "Error while loading subnetwork ", file_path, "\n");
		io::stream_out(oss, e.what(), "\n");
		throw std::runtime_error(oss.str());
	}

	// If we arrived here it means that the file has been loaded correctly
	// and we can create the group.

	UserDefinedElementsGroup<T> elements_group;

    elements_group.reserve(ids_list.size());
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
