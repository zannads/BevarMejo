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
#include "bevarmejo/utility/epanet/time.hpp"

#include "bevarmejo/wds/utility/epanet/en_help.hpp"

#include "bevarmejo/utility/exceptions.hpp"
#include "bevarmejo/utility/io.hpp"

#include "bevarmejo/utility/registry.hpp"
#include "bevarmejo/utility/registry_view.hpp"
#include "bevarmejo/utility/unique_string_sequence.hpp"

#include "bevarmejo/wds/utility/time_series.hpp"
#include "bevarmejo/wds/utility/quantity_series.hpp"

#include "bevarmejo/wds/element.hpp"

#include "bevarmejo/wds/elements/curve.hpp"
#include "bevarmejo/wds/elements/curves.hpp"
#include "bevarmejo/wds/elements/pattern.hpp"

#include "bevarmejo/wds/elements/network_element.hpp"

#include "bevarmejo/wds/elements/network_elements/node.hpp"
#include "bevarmejo/wds/elements/network_elements/nodes/junction.hpp"
#include "bevarmejo/wds/elements/network_elements/nodes/source.hpp"
#include "bevarmejo/wds/elements/network_elements/nodes/reservoir.hpp"
#include "bevarmejo/wds/elements/network_elements/nodes/tank.hpp"

#include "bevarmejo/wds/elements/network_elements/link.hpp"
#include "bevarmejo/wds/elements/network_elements/links/dimensioned_link.hpp"
#include "bevarmejo/wds/elements/network_elements/links/pipe.hpp"
#include "bevarmejo/wds/elements/network_elements/links/pump.hpp"
// #include "bevarmejo/wds/elements/valve.hpp"

namespace bevarmejo::label {
static const std::string __EN_PATTERN_TS = "ENPatt";
} // namespace bevarmejo::label

namespace bevarmejo {

static const std::string l__DEMAND_NODES = "Demand Nodes";

class WaterDistributionSystem final
{
/*------- Member types -------*/
public:
    using Component = wds::Element;
    using NetworkElement = wds::NetworkElement;
    
    // System's Network Elements (thus Components):
    // - Nodes (Junctions, Reservoirs, Tanks)
    // - Links (Pipes, Pumps, Valves)
    using Node = wds::Node;
    using Junction = wds::Junction;
    using Reservoir = wds::Reservoir;
    using Tank = wds::Tank;
    using NodeTypes = std::tuple<Node, Junction, Reservoir, Tank>;

    using Link = wds::Link;
    using Pipe = wds::Pipe;
    using Pump = wds::Pump;
    // using Valve = wds::Valve;
    using LinkTypes = std::tuple<Link, Pipe, Pump>;

    using NetworkElementsTypes = std::tuple<Node, Junction, Reservoir, Tank, Link, Pipe, Pump>;

    // System's components: Curves and Patterns
    using Curve = wds::Curve;
    using Pattern = wds::Pattern;
    // Non System's Components:
    using IDSequence = UniqueStringSequence;
    // wds::aux::GlobalTimes
    using TimeSeries = wds::aux::TimeSeries;
    using ResourceTypes = std::tuple<Curve, Pattern, IDSequence, TimeSeries>;

    // System's Network Elements Collections:
    using ID = typename Registry<Component>::key_type;
    using Nodes = Registry<Node>;
    using Junctions = Registry<Junction>;
    using Reservoirs = Registry<Reservoir>;
    using Tanks = Registry<Tank>;

    using Links = Registry<Link>;
    using Pipes = Registry<Pipe>;
    using Pumps = Registry<Pump>;
    // using Valves = Registry<Valve>;

    // System's Components Collections:
    using Curves = Registry<Curve>;
    using Patterns = Registry<Pattern>;

    using IDSequences = Registry<UniqueStringSequence>;

    // Utilities
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

/*------- Member objects -------*/
private:
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
    wds::aux::GlobalTimes m__times;

/*------- Member functions -------*/
// (constructor)
public:
    WaterDistributionSystem() = default;
    WaterDistributionSystem(const WaterDistributionSystem& other) = default;
    WaterDistributionSystem(WaterDistributionSystem&& other) noexcept = default;

// (destructor)
public:
    ~WaterDistributionSystem();

// operator=
public:
    WaterDistributionSystem& operator=(const WaterDistributionSystem& rhs) = default;
    WaterDistributionSystem& operator=(WaterDistributionSystem&& rhs) noexcept = default;

// clone()
public:
    std::unique_ptr<WaterDistributionSystem> clone() const;
    
/*------- Element access -------*/
// See the system's components and their collections.
// The collections can only be accessed as const, meaning that you can only read them.
// If you want to modify them (add or remove components), you should use the modifiers 
// of the WDS class.
// With the Views you can get read access to these subcollections and write access to the
// components of the subcollections.
// Similarly, you can modify the properties of the components by directly accessing them
// with the WDS class methods.
public:
    /*--- System's Network Elements Collections ---*/
    auto nodes() noexcept -> OutputExcludingRegistryView<Node>;
    auto nodes() const noexcept -> InputExcludingRegistryView<Node>;

    auto links() noexcept -> OutputExcludingRegistryView<Link>;
    auto links() const noexcept -> InputExcludingRegistryView<Link>;

    auto junctions() noexcept -> OutputExcludingRegistryView<Junction>;
    auto junctions() const noexcept -> InputExcludingRegistryView<Junction>;

    auto reservoirs() noexcept -> OutputExcludingRegistryView<Reservoir>;
    auto reservoirs() const noexcept -> InputExcludingRegistryView<Reservoir>;

    auto tanks() noexcept -> OutputExcludingRegistryView<Tank>;
    auto tanks() const noexcept -> InputExcludingRegistryView<Tank>;

    auto pipes() noexcept -> OutputExcludingRegistryView<Pipe>;
    auto pipes() const noexcept -> InputExcludingRegistryView<Pipe>;

    auto pumps() noexcept -> OutputExcludingRegistryView<Pump>;
    auto pumps() const noexcept -> InputExcludingRegistryView<Pump>;
    
    /*--- System's Components Collections ---*/
    auto patterns() noexcept -> OutputExcludingRegistryView<Pattern>;
    auto patterns() const noexcept -> InputExcludingRegistryView<Pattern>;

    auto curves() noexcept -> OutputExcludingRegistryView<Curve>;
    auto curves() const noexcept -> InputExcludingRegistryView<Curve>;

    auto id_sequences() noexcept -> OutputExcludingRegistryView<IDSequence>;
    auto id_sequences() const noexcept -> InputExcludingRegistryView<IDSequence>;

    /*--- System's Network Elements Views (Subnetworks) ---*/
    // Expose a subcollection of the system's network elements (subnetwork).
    // Differently from the standard ones, these can not be noexcept because the 
    // user may pass an invalid name for the id_sequence.
    template <typename T>
    auto subnetwork(const std::string& id_sequence_name) -> RegistryView<T, SubsetMode::Include, /*IsMutable = */ true>;
    template <typename T>
    auto subnetwork(const std::string& id_sequence_name) const -> RegistryView<T, SubsetMode::Include, /*IsMutable = */ false>;

    template <typename T>
    auto subnetwork_excluding(const std::string& id_sequence_name) -> RegistryView<T, SubsetMode::Exclude, /*IsMutable = */ true>;
    template <typename T>
    auto subnetwork_excluding(const std::string& id_sequence_name) const -> RegistryView<T, SubsetMode::Exclude, /*IsMutable = */ false>;

    template <typename T>
    auto subnetwork_with_order(const ID& id_sequence_name) -> OutputOrderedRegistryView<T>;
    template <typename T>
    auto subnetwork_with_order(const ID& id_sequence_name) const -> InputOrderedRegistryView<T>;

    template <typename T, SubsetMode M>
    auto network_elements_view(const std::string& id_sequence_name) -> RegistryView<T, M, /*IsMutable = */ true>;
    template <typename T, SubsetMode M>
    auto network_elements_view(const std::string& id_sequence_name) const -> RegistryView<T, M, /*IsMutable = */ false>;

    /*--- System's Network Elements ---*/
    // Common interface for all types of elements:
    template <typename OutputT>
    std::shared_ptr<OutputT> get(const ID& id);
    template <typename OutputT>
    std::shared_ptr<const OutputT> get(const ID& id) const;

    // Network elements
    // Nodes
    template <typename OutputT = Node>
    std::shared_ptr<OutputT> get_node(const ID& id);
    template <typename OutputT = Node>
    std::shared_ptr<const OutputT> get_node(const ID& id) const;

    Junction& junction(const ID& id);
    const Junction& junction(const ID& id) const;

    template <typename OutputT = Junction>
    std::shared_ptr<OutputT> get_junction(const ID& id);
    template <typename OutputT = Junction>
    std::shared_ptr<const OutputT> get_junction(const ID& id) const;

    Reservoir& reservoir(const ID& id);
    const Reservoir& reservoir(const ID& id) const;

    template <typename OutputT = Reservoir>
    std::shared_ptr<OutputT> get_reservoir(const ID& id);
    template <typename OutputT = Reservoir>
    std::shared_ptr<const OutputT> get_reservoir(const ID& id) const;

    Tank& tank(const ID& id);
    const Tank& tank(const ID& id) const;

    template <typename OutputT = Tank>
    std::shared_ptr<OutputT> get_tank(const ID& id);
    template <typename OutputT = Tank>
    std::shared_ptr<const OutputT> get_tank(const ID& id) const;

    // Links
    template <typename OutputT = Link>
    std::shared_ptr<OutputT> get_link(const ID& id);
    template <typename OutputT = Link>
    std::shared_ptr<const OutputT> get_link(const ID& id) const;

    Pipe& pipe(const ID& id);
    const Pipe& pipe(const ID& id) const;

    template <typename OutputT = Pipe>
    std::shared_ptr<OutputT> get_pipe(const ID& id);
    template <typename OutputT = Pipe>
    std::shared_ptr<const OutputT> get_pipe(const ID& id) const;

    Pump& pump(const ID& id);
    const Pump& pump(const ID& id) const;

    template <typename OutputT = Pump>
    std::shared_ptr<OutputT> get_pump(const ID& id);
    template <typename OutputT = Pump>
    std::shared_ptr<const OutputT> get_pump(const ID& id) const;

    /*--- System's Components ---*/
    template <typename OutputT>
    std::shared_ptr<OutputT> get_component(const ID& id);
    template <typename OutputT>
    std::shared_ptr<const OutputT> get_component(const ID& id) const;

    Curve& curve(const std::string& name);
    const Curve& curve(const std::string& name) const;

    template <typename OutputT = Curve>
    std::shared_ptr<OutputT> get_curve(const std::string& name);
    template <typename OutputT = Curve>
    std::shared_ptr<const OutputT> get_curve(const std::string& name) const;

    Pattern& pattern(const std::string& name);
    const Pattern& pattern(const std::string& name) const;

    template <typename OutputT = Pattern>
    std::shared_ptr<OutputT> get_pattern(const std::string& name);
    template <typename OutputT = Pattern>
    std::shared_ptr<const OutputT> get_pattern(const std::string& name) const;

    IDSequence& id_sequence(const std::string& name);
    const IDSequence& id_sequence(const std::string& name) const;

    TimeSeries& time_series(const std::string& name);
    const TimeSeries& time_series(const std::string& name) const;

    TimeSeries& result_time_series();
    const TimeSeries& result_time_series() const;

    time::Instant current_result_time() const;

/*------- Capacity -------*/
public:
    // Check if the system has any component.
    bool empty() const noexcept;

    // Check if the system has a network (contains nodes and links).
    bool empty_network() const noexcept;

    // Check if the network has tanks.
    bool has_tanks() const noexcept;

    // Number of system's components.
    size_t size() const noexcept;

    // Number of network's elements (nodes and links).
    size_t network_size() const noexcept;

    // Number of network's nodes.
    size_t n_nodes() const noexcept;

    // Number of network's links.
    size_t n_links() const noexcept;

    // Number of network's junctions.
    size_t n_junctions() const noexcept;

    // Number of network's reservoirs.
    size_t n_reservoirs() const noexcept;

    // Number of network's tanks.
    size_t n_tanks() const noexcept;

    // Number of network's pipes.
    size_t n_pipes() const noexcept;

    // Number of network's pumps.
    size_t n_pumps() const noexcept;

/*------- Modifiers -------*/
// We can:
// - insert a node, (or insert a copy of...)
// - install a link, (or install a copy of...)
// - register a time series, or a curve, or a pattern, (or register a copy of...)
// - duplicate an already existing link,
// - remove a node,
// - uninstall a link,
// - drop a time series, a curve, a pattern
// More generally, we can:
// - emplace and erase everyone of these components.
// Moreover, we can destroy the network, clear the results, run the hydraulics, cache the indices.
// I should also be able to insert a component from a file.
// Can only emplace objects of final types (no abstract classes): 
// - Junction, Reservoir, Tank, Pipe, Pump, Curve, Pattern, IDSequence, TimeSeries
public:
// Common interface for all types of elements:
    template <typename T, typename... Args>
    auto emplace(const ID& id, Args&&... args) -> T&;

// Network elements
// Nodes
    template <typename N, typename... Args>
    auto insert(const ID& id, Args&&... args) -> N&;

    template <typename... Args>
    auto insert_junction(const ID& id, Args&&... args) -> Junction&;

    template <typename... Args>
    auto insert_reservoir(const ID& id, Args&&... args) -> Reservoir&;

    template <typename... Args>
    auto insert_tank(const ID& id, Args&&... args) -> Tank&;

// Links
    template <typename L, typename... Args>
    auto install(const ID& id, const ID& from_node_id, const ID& to_node_it, Args&&... args) -> L&;

    template <typename... Args>
    auto install_pipe(const ID& id, const ID& from_node_id, const ID& to_node_it, Args&&... args) -> Pipe&;

    template <typename... Args>
    auto install_pump(const ID& id, Args&&... args) -> Pump&;

// Components
    template <typename C, typename... Args>
    auto submit(const ID& id, Args&&... args) -> C&;

    template <typename... Args>
    auto submit_curve(const ID& id, Args&&... args) -> Curve&;

    template <typename... Args>
    auto submit_pattern(const ID& id, Args&&... args) -> Pattern&;

    template <typename... Args>
    auto submit_id_sequence(const ID& id, Args&&... args) -> IDSequence&;
    auto submit_id_sequence(const ID& name, const Json& j) -> IDSequence&;
    auto submit_id_sequence(const fsys::path& file_path) -> IDSequence&;

    template <typename... Args>
    auto submit_time_series(const ID& id, Args&&... args) -> TimeSeries&;

// removal
public:
// Common interface for all types of elements:
    template <typename T>
    auto erase(const ID& id) -> size_type;

// Network elements
// Nodes
    auto remove(const ID& id) -> size_type;

    auto remove_junction(const ID& id) -> size_type;

    auto remove_reservoir(const ID& id) -> size_type;

    auto remove_tank(const ID& id) -> size_type;

// Links
    auto uninstall(const ID& id) -> size_type;

    auto uninstall_pipe(const ID& id) -> size_type;

    auto uninstall_pump(const ID& id) -> size_type;

// Components
    template <typename C>
    auto drop(const ID& id) -> size_type;

    auto drop_curve(const ID& id) -> size_type;

    auto drop_pattern(const ID& id) -> size_type;

    auto drop_ids(const ID& id) -> size_type;

    auto drop_time_series(const ID& id) -> size_type;

// Special modifications
// Duplicate a link (install a copy in parallel)
    template <typename L>
    auto duplicate(const ID& existing_link_id, const ID& new_link_id) -> L&;

public:
    void clear_results();
    
    void run_hydraulics();

///////////////////////////////////////////////////////////////////////////
// EPANET Support starts here.
///////////////////////////////////////////////////////////////////////////
/*------- Member types -------*/

/*------- Member objects -------*/
public:
    // Handler for the project.
    // Public because I may want to modify it (e.g., apply a decision vector).
    // it is just faster than doing insert an interface. I will be careful.
    mutable EN_Project ph_;
protected:
    // Path to the inp file from which the project will be uploaded.
    fsys::path _inp_file_;

/*------- Member functions -------*/
// (constructor)
public:
    WaterDistributionSystem(const fsys::path& inp_file, std::function<void (EN_Project)> preprocessf = [](EN_Project ph){ return;});

/*------- Element access -------*/
public:
    // ph is public, so you can use it to modify the project.
    EN_Project ph() const noexcept;

    const fsys::path& inp_file() const noexcept;

/*------- Capacity -------*/

/*------- Modifiers -------*/
private:
    // Equivalent to constuctor from .inp file
    void load_EN_time_settings();
    void load_EN_analysis_options();
    void load_EN_patterns();
    void load_EN_curves();
    void load_EN_nodes();
    void load_EN_links();
    void load_EN_controls();
    void load_EN_rules();
public:
    // Cache the indices of the elements in the network.
    // This is useful to avoid calling the ENgetnodeindex and ENgetlinkindex functions every time.
    void cache_indices();

}; // class WaterDistributionSystem

/*--- Implementation ---*/
/*------- Element access -------*/
/*--- System's Network Elements Collections ---*/

/*--- System's Network Elements Views (Subnetworks) ---*/
template <typename T>
auto WaterDistributionSystem::subnetwork_with_order(const ID& id_sequence_name) -> OrderedRegistryView<T, true>
{
    if constexpr (std::is_same_v<T, Node>)
        return RegistryView<Node, SubsetMode::OrderedInclude, true>(_nodes_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Junction>)
        return RegistryView<Junction, SubsetMode::OrderedInclude, true>(_junctions_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Reservoir>)
        return RegistryView<Reservoir, SubsetMode::OrderedInclude, true>(_reservoirs_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Tank>)
        return RegistryView<Tank, SubsetMode::OrderedInclude, true>(_tanks_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Link>)
        return RegistryView<Link, SubsetMode::OrderedInclude, true>(_links_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Pipe>)
        return RegistryView<Pipe, SubsetMode::OrderedInclude, true>(_pipes_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Pump>)
        return RegistryView<Pump, SubsetMode::OrderedInclude, true>(_pumps_, m__id_sequences.get(id_sequence_name));

    else
        static_assert(std::true_type::value, 
            "You are trying to get a subnetwork with an invalid type.");
}

template <typename T>
auto WaterDistributionSystem::subnetwork_with_order(const ID& id_sequence_name) const -> InputOrderedRegistryView<T>
{
    if constexpr (std::is_same_v<T, Node>)
        return RegistryView<Node, SubsetMode::OrderedInclude, false>(_nodes_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Junction>)
        return RegistryView<Junction, SubsetMode::OrderedInclude, false>(_junctions_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Reservoir>)
        return RegistryView<Reservoir, SubsetMode::OrderedInclude, false>(_reservoirs_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Tank>)
        return RegistryView<Tank, SubsetMode::OrderedInclude, false>(_tanks_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Link>)
        return RegistryView<Link, SubsetMode::OrderedInclude, false>(_links_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Pipe>)
        return RegistryView<Pipe, SubsetMode::OrderedInclude, false>(_pipes_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Pump>)
        return RegistryView<Pump, SubsetMode::OrderedInclude, false>(_pumps_, m__id_sequences.get(id_sequence_name));

    else
        static_assert(std::true_type::value, 
            "You are trying to get a subnetwork with an invalid type.");
}

template <typename T, SubsetMode M>
auto WaterDistributionSystem::network_elements_view(const std::string& id_sequence_name) -> RegistryView<T, M, true>
{
    if constexpr (std::is_same_v<T, Node>)
        return RegistryView<Node, M, true>(_nodes_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Junction>)
        return RegistryView<Junction, M, true>(_junctions_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Reservoir>)
        return RegistryView<Reservoir, M, true>(_reservoirs_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Tank>)
        return RegistryView<Tank, M, true>(_tanks_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Link>)
        return RegistryView<Link, M, true>(_links_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Pipe>)
        return RegistryView<Pipe, M, true>(_pipes_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Pump>)
        return RegistryView<Pump, M, true>(_pumps_, m__id_sequences.get(id_sequence_name));

    else
        static_assert(std::true_type::value, 
            "You are trying to get a subnetwork with an invalid type.");
}

template <typename T, SubsetMode M>
auto WaterDistributionSystem::network_elements_view(const std::string& id_sequence_name) const -> RegistryView<T, M, false>
{
    if constexpr (std::is_same_v<T, Node>)
        return RegistryView<Node, M, false>(_nodes_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Junction>)
        return RegistryView<Junction, M, false>(_junctions_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Reservoir>)
        return RegistryView<Reservoir, M, false>(_reservoirs_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Tank>)
        return RegistryView<Tank, M, false>(_tanks_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Link>)
        return RegistryView<Link, M, false>(_links_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Pipe>)
        return RegistryView<Pipe, M, false>(_pipes_, m__id_sequences.get(id_sequence_name));

    else if constexpr (std::is_same_v<T, Pump>)
        return RegistryView<Pump, M, false>(_pumps_, m__id_sequences.get(id_sequence_name));

    else
        static_assert(std::true_type::value, 
            "You are trying to get a subnetwork with an invalid type.");
}

/*--- System's Network Elements ---*/
template <typename OutputT>
auto WaterDistributionSystem::get_node(const ID& id) -> std::shared_ptr<OutputT>
{
    return _nodes_.get<OutputT>(id);
}
template <typename OutputT>
auto WaterDistributionSystem::get_node(const ID& id) const -> std::shared_ptr<const OutputT>
{
    return _nodes_.get<OutputT>(id);
}

/*--- System's Components ---*/

template <typename OutputT>
auto WaterDistributionSystem::get_curve(const std::string& name) ->std::shared_ptr<OutputT>
{
    return m__aux_elements_.curves.get<OutputT>(name);
}
template <typename OutputT>
auto WaterDistributionSystem::get_curve(const std::string& name) const -> std::shared_ptr<const OutputT>
{
    return m__aux_elements_.curves.get<OutputT>(name);
}

template <typename OutputT>
auto WaterDistributionSystem::get_pattern(const std::string& name) -> std::shared_ptr<OutputT>
{
    return m__aux_elements_.patterns.get<OutputT>(name);
}
template <typename OutputT>
auto WaterDistributionSystem::get_pattern(const std::string& name) const -> std::shared_ptr<const OutputT>
{
    return m__aux_elements_.patterns.get<OutputT>(name);
}

/*------- Modifiers -------*/
// Emplace whatever you want in the system.

// Nodes
template <typename N, typename... Args>
    auto WaterDistributionSystem::insert(const ID& id, Args&&... args) -> N&
{
    static_assert(std::is_same_v<N, Junction> || std::is_same_v<N, Reservoir> || std::is_same_v<N, Tank>,
        "You are trying to insert a non-node element in the nodes collection.");

    // Have to pass the ID twice because the constructor of the element needs it.
    auto irtn = _nodes_.emplace<N>(id, *this, id, std::forward<Args>(args)...);

    beme_throw_if(!irtn.inserted, std::invalid_argument,
        "Impossible to insert the element.",
        "An element with the same name already exists in the nodes collection.",
        "Name: ", id);
        
    auto insert_in_spec_collection = [this, &id, &irtn](auto& container) -> decltype(auto)
    {
        auto irs = container.insert(id, std::static_pointer_cast<N>(irtn.it.operator->()));
        if (!irs.inserted)
        {
            _nodes_.erase(id);
            beme_throw(std::logic_error,
                "Impossible to insert the element.",
                "The element could not be inserted in the specific collection (either there is no more memory or the containers lost sync).",
                "Element name: ", id);
        }

        return irs.it.operator->();
    };

    if constexpr (std::is_same_v<N, Junction>)
        return *insert_in_spec_collection(_junctions_);

    if constexpr (std::is_same_v<N, Reservoir>)
        return *insert_in_spec_collection(_reservoirs_);

    if constexpr (std::is_same_v<N, Tank>)
        return *insert_in_spec_collection(_tanks_);
}

template <typename... Args>
auto WaterDistributionSystem::insert_junction(const ID& id, Args&&... args) -> Junction&
{
    return insert<Junction>(id, std::forward<Args>(args)...);
}

template <typename... Args>
auto WaterDistributionSystem::insert_reservoir(const ID& id, Args&&... args) -> Reservoir&
{
    return insert<Reservoir>(id, std::forward<Args>(args)...);
}

template <typename... Args>
auto WaterDistributionSystem::insert_tank(const ID& id, Args&&... args) -> Tank&
{
    return insert<Tank>(id, std::forward<Args>(args)...);
}

// Links
template <typename L, typename... Args>
auto WaterDistributionSystem::install(const ID& id, const ID& from_node_id, const ID& to_node_it, Args&&... args) -> L&
{
    static_assert(std::is_same_v<L, Pipe> || std::is_same_v<L, Pump>,
        "You are trying to install a non-link element in the links collection.");

    // Check that the two nodes where you are installing the link exist.
    beme_throw_if(_nodes_.find_index(from_node_id) == -1, std::invalid_argument,
        "Impossible to install the element.",
        "Can not find the node where the link should be installed.",
        "Node name: ", from_node_id);
    beme_throw_if(_nodes_.find_index(to_node_it) == -1, std::invalid_argument,
        "Impossible to install the element.",
        "Can not find the node where the link should be installed.",
        "Node name: ", to_node_it);
    
    // As for the nodes, you have to pass the ID twice because the constructor of the element needs it.
    auto irtn = _links_.emplace<L>(id, *this, id, std::forward<Args>(args)...);

    beme_throw_if(!irtn.inserted, std::invalid_argument,
        "Impossible to insert the element.",
        "An element with the same name already exists in the links collection.",
        "Name: ", id);
        
    // We know the nodes id are correct and exist, so we can connect the link
    // and later insert it in the specific collection.
    irtn.it->from_node(_nodes_.get(from_node_id).get());
    _nodes_.get(from_node_id)->connect_link(irtn.it.operator->().get());

    irtn.it->to_node(_nodes_.get(to_node_it).get());
    _nodes_.get(to_node_it)->connect_link(irtn.it.operator->().get());

    auto insert_in_spec_collection = [this, &id, &from_node_id, &to_node_it, &irtn](auto& container) -> decltype(auto)
    {
        auto irs = container.insert(id, std::static_pointer_cast<L>(irtn.it.operator->()));
        if (!irs.inserted)
        {
            // Reset all insertions and throw an error.
            _nodes_.get(from_node_id)->disconnect_link(irtn.it.operator->().get());
            _nodes_.get(to_node_it)->disconnect_link(irtn.it.operator->().get());
            _links_.erase(id);

            beme_throw(std::logic_error,
                "Impossible to insert the element.",
                "The element could not be inserted in the specific collection (either there is no more memory or the containers lost sync).",
                "Element name: ", id);
        }

        return irs.it.operator->();
    };

    if constexpr (std::is_same_v<L, Pipe>)
        return *insert_in_spec_collection(_pipes_);

    if constexpr (std::is_same_v<L, Pump>)
        return *insert_in_spec_collection(_pumps_);
}

template <typename... Args>
auto WaterDistributionSystem::install_pipe(const ID& id, const ID& from_node_id, const ID& to_node_id, Args&&... args) -> Pipe&
{
    return install<Pipe>(id, from_node_id, to_node_id, std::forward<Args>(args)...);
}

template <typename... Args>
auto WaterDistributionSystem::install_pump(const ID& id, Args&&... args) -> Pump&
{
    return install<Pump>(id, std::forward<Args>(args)...);
}

// Components

template <typename... Args>
auto WaterDistributionSystem::submit_id_sequence(const ID& id, Args&&... args) -> IDSequence&
{
    auto irtn = m__id_sequences.emplace(id, std::forward<Args>(args)...);

    beme_throw_if(!irtn.inserted, std::invalid_argument,
        "Impossible to insert the element.",
        "A sequence with the same name already exists.", "Name: ", id);

    return *irtn.it.operator->();
}

// removal

// Common interface for all types of elements:
template <typename T>
auto WaterDistributionSystem::erase(const ID& id) -> size_type
{
    static_assert(std::is_same_v<T, Junction> || std::is_same_v<T, Reservoir> || std::is_same_v<T, Tank> ||
                  std::is_same_v<T, Pipe> || std::is_same_v<T, Pump> || std::is_same_v<T, Curve> || std::is_same_v<T, Pattern> ||
                  std::is_same_v<T, IDSequence> || std::is_same_v<T, TimeSeries>,
        "You are trying to erase a type of element that you don't really know.\nYou should know what you are doing.");

    // Erase an element that is not in the collection is a no-op.

    if constexpr(std::is_same_v<T, Junction> || std::is_same_v<T, Reservoir> || std::is_same_v<T, Tank>)
        return remove(id);

    if constexpr(std::is_same_v<T, Pipe> || std::is_same_v<T, Pump>)
        return uninstall(id);

    if constexpr(std::is_same_v<T, Curve>)
    {
        static_assert(std::true_type::value, "Not implemented yet.");
        return 0;
    }

    if constexpr(std::is_same_v<T, Pattern>)
    {
        static_assert(std::true_type::value, "Not implemented yet.");
        return 0;
    }

    if constexpr(std::is_same_v<T, IDSequence>)
    {
        static_assert(std::true_type::value, "Not implemented yet.");
        return 0;
    }

    if constexpr(std::is_same_v<T, TimeSeries>)
    {
        static_assert(std::true_type::value, "Not implemented yet.");
        return 0;
    }

    static_assert(std::true_type::value, "Unreachable code.");
}

// Special modifications
// Duplicate a link (install a copy in parallel)
template <typename L>
auto WaterDistributionSystem::duplicate(const ID& existing_link_id, const ID& new_link_id) -> L&
{
    auto existing_link = _links_.get<L>(existing_link_id);
    return install<L>(new_link_id,
        existing_link->from_node()->EN_id(), existing_link->to_node()->EN_id(),
        *existing_link);
}


using WDS = WaterDistributionSystem; // Short for WaterDistributionSystem

} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS__WATER_DISTRIBUTION_SYSTEM_HPP
