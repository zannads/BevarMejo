//
//  water_distribution_system.hpp
//  bevarmejo cpp library
//
//  Created by Dennis Zanutto on 04/07/23.
//

#ifndef BEVARMEJOLIB__WDS__WATER_DISTRIBUTION_SYSTEM_HPP
#define BEVARMEJOLIB__WDS__WATER_DISTRIBUTION_SYSTEM_HPP

#include <filesystem>
namespace fsys = std::filesystem;
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

#include "bevarmejo/io/streams.hpp"
#include "bevarmejo/bemexcept.hpp"

#include "bevarmejo/utility/registry.hpp"
#include "bevarmejo/utility/registry_view.hpp"
#include "bevarmejo/utility/unique_string_sequence.hpp"

#include "bevarmejo/wds/auxiliary/time_series.hpp"
#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/element.hpp"

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

namespace bevarmejo::label {
static const std::string __EN_PATTERN_TS = "ENPatt";
} // namespace bevarmejo::label

namespace bevarmejo::wds {

static const std::string l__DEMAND_NODES = "Demand Nodes";

class WaterDistributionSystem {

    /*--- Attributes ---*/
public:
    // Handler for the project.
    // Public because I may want to modify it (e.g., apply a decision vector).
    // it is just faster than doing create an interface. I will be careful.
    mutable EN_Project ph_;
    using Curves = Registry<Curve>;
    using Patterns = Registry<Pattern>;

    using Nodes = Registry<Node>;
    using Junctions = Registry<Junction>;
    using Reservoirs = Registry<Reservoir>;
    using Tanks = Registry<Tank>;

    using Links = Registry<Link>;
    using Pipes = Registry<Pipe>;
    using Pumps = Registry<Pump>;
    // using Valves = Registry<Valve>;

    using IDSequences = Registry<UniqueStringSequence>;

protected:
    // Path to the inp file from which the project will be uploaded.
    fsys::path _inp_file_;
    // Collection of elements of the network
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

    // User defined sequences of IDs (for subnetworks or things of this type)
    IDSequences m__id_sequences;

    // User defined and default TimeSeries for the simulation
    aux::GlobalTimes m__times;
    
    struct ConfigOptions {
        bool save_all_hsteps = true;                // Bool to turn on/off the report behaviour like in EPANET
    } m__config_options;

/*--- Constructors ---*/ 
public:
    // Default constructor
    WaterDistributionSystem();
    
    WaterDistributionSystem(const fsys::path& inp_file, std::function<void (EN_Project)> preprocessf = [](EN_Project ph){ return;});

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
    const fsys::path& inp_file() const {return _inp_file_;}; // you can't change it from outside
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
    // const Valves& valves() const {return _valves_;};
    
    const IDSequences& id_sequences() const { return m__id_sequences; }
    
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
    
    void run_hydraulics();

    /*--- ?? ---*/
    template <typename T>
    std::vector<std::shared_ptr<Element>>::iterator insert(const std::shared_ptr<T>& a_element);

    template <typename T, typename... Args>
    auto insert(const std::string& id, Args&&... args);

    auto insert(const fsys::path& file_path);

    auto insert_ids_from_file(const fsys::path& file_path);
    
    template <typename T>
    std::vector<std::shared_ptr<Element>>::iterator erase(const std::shared_ptr<T>& a_element);


}; // class WaterDistributionSystem

} // namespace wds

using WDS = bevarmejo::wds::WaterDistributionSystem; // short name for WaterDistributionSystem

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
typename std::vector<std::shared_ptr<bevarmejo::wds::Element>>::iterator bevarmejo::wds::WaterDistributionSystem::erase(const std::shared_ptr<T>& a_element) {
    if (a_element == nullptr)
        return _elements_.end();

    // first of all erase it from the specific containers
    if constexpr (std::is_same_v<T, bevarmejo::wds::Node>) {
        _nodes_.erase(a_element->id());
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Junction>) {
        _nodes_.erase(a_element->id());
        _junctions_.erase(a_element->id());
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Tank>) {
        _nodes_.erase(a_element->id());
        _tanks_.erase(a_element->id());
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Reservoir>) {
        _nodes_.erase(a_element->id());
        _reservoirs_.erase(a_element->id());
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Link>) {
        _links_.erase(a_element->id());
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Pipe>) {
        _links_.erase(a_element->id());
        _pipes_.erase(a_element->id());

        // TODO: the same thing but for all types!
        // FIX: when I will connect the network, I will have to erase the links from the nodes.
        //a_element->from_node()->erase_link(a_element.get());
        //a_element->to_node()->erase_link(a_element.get());

    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Pump>) {
        _links_.erase(a_element->id());
        _pumps_.erase(a_element->id());
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Pattern>) {
        m__aux_elements_.patterns.erase(a_element->id());
    } else if constexpr (std::is_same_v<T, bevarmejo::wds::Curve>) {
        m__aux_elements_.patterns.erase(a_element->id());
    } else {
        // Handle other types
        // ...
        // good question, for sure I don't add it to anything else, 
        // but if I'm here I will be here at compile time so maybe warning??
    }

    // now erase it from the general container
    auto it = std::find(_elements_.begin(), _elements_.end(), a_element);
    if (it != _elements_.end())
        return _elements_.erase(it);
}

#endif // BEVARMEJOLIB__WDS__WATER_DISTRIBUTION_SYSTEM_HPP
