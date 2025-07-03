//
//  water_distribution_system.cpp
//  hanoiOptimization
//
//  Created by Dennis Zanutto on 04/07/23.
//

#include <assert.h>
#include <exception>
#include <filesystem>
namespace fsys = std::filesystem;
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "epanet2_2.h"

#include "bevarmejo/io/streams.hpp"
#include "bevarmejo/io/json.hpp"

#include "water_distribution_system.hpp"

namespace bevarmejo {

WaterDistributionSystem::~WaterDistributionSystem(){
    // The EPANET Project handler is destroyed automatically when this class dies
    // because its lifetime is managed by the shared pointer.

    // First clear all the elements, then the time series and finally the config options
    
    _pipes_.clear();
    _pumps_.clear();
    _links_.clear();
    
    _junctions_.clear();
    _tanks_.clear();
    _reservoirs_.clear();
    _nodes_.clear();
    
    m__aux_elements_.patterns.clear();
    m__aux_elements_.curves.clear();

    // Times and config options can die in peace now as noone is referencing them.
}

/*
std::unique_ptr<WaterDistributionSystem> WaterDistributionSystem::clone() const
{
    std::unique_ptr<WaterDistributionSystem> wds_clone = std::make_unique<WaterDistributionSystem>();

    // Clone the elements
    // I start from curves and patterns since the other depende on them

    // The nodes can be complitely defined thanks to curves and patterns, so it's
    // their moment.

    // Finally once everything is in place I can copy the links and connect the
    // the network.

    return wds_clone;
}
*/

/*------- Element access -------*/

/*--- System's Network Elements Collections ---*/
auto WaterDistributionSystem::nodes() noexcept -> OutputExcludingRegistryView<Node>
{
    return RegistryView<Node, SubsetMode::Exclude, true>(_nodes_);
}
auto WaterDistributionSystem::nodes() const noexcept -> InputExcludingRegistryView<Node>
{
    return RegistryView<Node, SubsetMode::Exclude, false>(_nodes_);
}

auto WaterDistributionSystem::links() noexcept -> OutputExcludingRegistryView<Link>
{
    return RegistryView<Link, SubsetMode::Exclude, true>(_links_);
}
auto WaterDistributionSystem::links() const noexcept -> InputExcludingRegistryView<Link>
{
    return RegistryView<Link, SubsetMode::Exclude, false>(_links_);
}

auto WaterDistributionSystem::junctions() noexcept -> OutputExcludingRegistryView<Junction>
{
    return RegistryView<Junction, SubsetMode::Exclude, true>(_junctions_);
}
auto WaterDistributionSystem::junctions() const noexcept -> InputExcludingRegistryView<Junction>
{
    return RegistryView<Junction, SubsetMode::Exclude, false>(_junctions_);
}

auto WaterDistributionSystem::reservoirs() noexcept -> OutputExcludingRegistryView<Reservoir>
{
    return RegistryView<Reservoir, SubsetMode::Exclude, true>(_reservoirs_);
}
auto WaterDistributionSystem::reservoirs() const noexcept -> InputExcludingRegistryView<Reservoir>
{
    return RegistryView<Reservoir, SubsetMode::Exclude, false>(_reservoirs_);
}

auto WaterDistributionSystem::tanks() noexcept -> OutputExcludingRegistryView<Tank>
{
    return RegistryView<Tank, SubsetMode::Exclude, true>(_tanks_);
}
auto WaterDistributionSystem::tanks() const noexcept -> InputExcludingRegistryView<Tank>
{
    return RegistryView<Tank, SubsetMode::Exclude, false>(_tanks_);
}

auto WaterDistributionSystem::pipes() noexcept -> OutputExcludingRegistryView<Pipe>
{
    return RegistryView<Pipe, SubsetMode::Exclude, true>(_pipes_);
}
auto WaterDistributionSystem::pipes() const noexcept -> InputExcludingRegistryView<Pipe>
{
    return RegistryView<Pipe, SubsetMode::Exclude, false>(_pipes_);
}

auto WaterDistributionSystem::pumps() noexcept -> OutputExcludingRegistryView<Pump>
{
    return RegistryView<Pump, SubsetMode::Exclude, true>(_pumps_);
}
auto WaterDistributionSystem::pumps() const noexcept -> InputExcludingRegistryView<Pump>
{
    return RegistryView<Pump, SubsetMode::Exclude, false>(_pumps_);
}

/*--- System's Components Collections ---*/

auto WaterDistributionSystem::patterns() noexcept -> OutputExcludingRegistryView<Pattern>
{
    return RegistryView<Pattern, SubsetMode::Exclude, true>(m__aux_elements_.patterns);
}
auto WaterDistributionSystem::patterns() const noexcept -> InputExcludingRegistryView<Pattern>
{
    return RegistryView<Pattern, SubsetMode::Exclude, false>(m__aux_elements_.patterns);
}

auto WaterDistributionSystem::curves() noexcept -> OutputExcludingRegistryView<Curve>
{
    return RegistryView<Curve, SubsetMode::Exclude, true>(m__aux_elements_.curves);
}
auto WaterDistributionSystem::curves() const noexcept -> InputExcludingRegistryView<Curve>
{
    return RegistryView<Curve, SubsetMode::Exclude, false>(m__aux_elements_.curves);
}

auto WaterDistributionSystem::id_sequences() noexcept -> OutputExcludingRegistryView<IDSequence>
{
    return RegistryView<IDSequence, SubsetMode::Exclude, true>(m__id_sequences);
}
auto WaterDistributionSystem::id_sequences() const noexcept -> InputExcludingRegistryView<IDSequence>
{
    return RegistryView<IDSequence, SubsetMode::Exclude, false>(m__id_sequences);
}

/*--- System's Network Elements Views (Subnetworks) ---*/
// Everything is templatized here. See hpp file.

/*--- System's Network Elements ---*/
auto WaterDistributionSystem::junction(const ID& id) -> Junction&
{
    return _junctions_.at(id);
}
auto WaterDistributionSystem::junction(const ID& id) const -> const Junction&
{
    return _junctions_.at(id);
}

auto WaterDistributionSystem::reservoir(const ID& id) -> Reservoir&
{
    return _reservoirs_.at(id);
}
auto WaterDistributionSystem::reservoir(const ID& id) const -> const Reservoir&
{
    return _reservoirs_.at(id);
}

auto WaterDistributionSystem::tank(const ID& id) -> Tank&
{
    return _tanks_.at(id);
}
auto WaterDistributionSystem::tank(const ID& id) const -> const Tank&
{
    return _tanks_.at(id);
}

auto WaterDistributionSystem::pipe(const ID& id) -> Pipe&
{
    return _pipes_.at(id);
}
auto WaterDistributionSystem::pipe(const ID& id) const -> const Pipe&
{
    return _pipes_.at(id);
}

auto WaterDistributionSystem::pump(const ID& id) -> Pump&
{
    return _pumps_.at(id);
}
auto WaterDistributionSystem::pump(const ID& id) const -> const Pump&
{
    return _pumps_.at(id);
}

/*--- System's Components ---*/
auto WaterDistributionSystem::curve(const std::string& name) -> Curve&
{
    return m__aux_elements_.curves.at(name);
}
auto WaterDistributionSystem::curve(const std::string& name) const -> const Curve&
{
    return m__aux_elements_.curves.at(name);
}

auto WaterDistributionSystem::pattern(const std::string& name) -> Pattern&
{
    return m__aux_elements_.patterns.at(name);
}
auto WaterDistributionSystem::pattern(const std::string& name) const -> const Pattern&
{
    return m__aux_elements_.patterns.at(name);
}

auto WaterDistributionSystem::id_sequence(const std::string& name) -> IDSequence&
{
    return m__id_sequences.at(name);
}
auto WaterDistributionSystem::id_sequence(const std::string& name) const -> const IDSequence&
{
    return m__id_sequences.at(name);
}

auto WaterDistributionSystem::time_series(const std::string& name) -> TimeSeries&
{
    return m__times.time_series(name);
}
auto WaterDistributionSystem::time_series(const std::string& name) const -> const TimeSeries&
{
    return m__times.time_series(name);
}

auto WaterDistributionSystem::result_time_series() -> TimeSeries&
{
    return m__times.results();
}

auto WaterDistributionSystem::result_time_series() const -> const TimeSeries&
{
    return m__times.results();
}

auto WaterDistributionSystem::current_result_time() const -> time::Instant
{
    return m__times.results().back();
}

/*------- Capacity -------*/
auto WaterDistributionSystem::empty() const noexcept -> bool
{
    return (
        _nodes_.empty() &&
        _links_.empty() &&
        m__aux_elements_.patterns.empty() &&
        m__aux_elements_.curves.empty() &&
        m__id_sequences.empty()
    );
}

auto WaterDistributionSystem::empty_network() const noexcept -> bool
{
    return _nodes_.empty() && _links_.empty();
}

auto WaterDistributionSystem::has_tanks() const noexcept -> bool
{
    return !_tanks_.empty();
}

auto WaterDistributionSystem::size() const noexcept -> size_t
{
    return (
        _nodes_.size() +
        _links_.size() +
        m__aux_elements_.patterns.size() +
        m__aux_elements_.curves.size() +
        m__id_sequences.size()
    );
}

auto WaterDistributionSystem::network_size() const noexcept -> size_t
{
    return _nodes_.size() + _links_.size();
}

auto WaterDistributionSystem::n_nodes() const noexcept -> size_t
{
    return _nodes_.size();
}

auto WaterDistributionSystem::n_links() const noexcept -> size_t
{
    return _links_.size();
}

auto WaterDistributionSystem::n_junctions() const noexcept -> size_t
{
    return _junctions_.size();
}

auto WaterDistributionSystem::n_reservoirs() const noexcept -> size_t
{
    return _reservoirs_.size();
}

auto WaterDistributionSystem::n_tanks() const noexcept -> size_t
{
    return _tanks_.size();
}

auto WaterDistributionSystem::n_pipes() const noexcept -> size_t
{
    return _pipes_.size();
}

auto WaterDistributionSystem::n_pumps() const noexcept -> size_t
{
    return _pumps_.size();
}

/*------- Modifiers -------*/
// Most of them are templated, the others not implemented yet. See hpp file.
// Additive modifiers
// Common interface for all types of elements:

// Network elements
// Nodes

// Links

// Components

auto WaterDistributionSystem::submit_id_sequence(const ID& name, const Json& j) -> IDSequence&
{
    auto names = j.get<std::vector<ID>>();
    auto ires = m__id_sequences.emplace(name, names);
    
    beme_throw_if(!ires.inserted, std::invalid_argument,
        "Impossible to submit the sequence of names.",
        "A sequence with the same name already exists.",
        "Name: ", name);

    return *ires.it.operator->();
}

// Subtractive modifiers
// Common interface for all types of elements is templated (see hpp file).

// Network elements
// Nodes
auto WaterDistributionSystem::remove(const ID& id) -> size_type
{
    size_type n_removed = 0;
    
    auto it = _nodes_.find(id);
    if (it == _nodes_.end())
        return n_removed;

    // All the links connected to the node should be destroyed...
    // I have to be careful, because if I call WDS::erase(link) it will change the node's links collection
    // and the iterator will be invalidated.
    // Also I can' use erase<L>(id) because I don't know the exact type of the link.
    // Therefore I create a copy of it and use it to erase the links.
    auto links_to_erase = it->connected_links();

    for (auto& p_link : links_to_erase)
        n_removed += uninstall(p_link->EN_id());

    assert(it->connected_links().empty());

    n_removed += _nodes_.erase(id);

    _junctions_.erase(id);
    _reservoirs_.erase(id);
    _tanks_.erase(id);

    return n_removed;
}

auto WaterDistributionSystem::remove_junction(const ID& id) -> size_type
{
    size_type n_removed = 0;

    auto it = _junctions_.find(id);
    if (it == _junctions_.end())
        return n_removed;

    // I confirmed that the junction is a node, so I can call remove.
    return remove(id);
}

auto WaterDistributionSystem::remove_reservoir(const ID& id) -> size_type
{
    size_type n_removed = 0;

    auto it = _reservoirs_.find(id);
    if (it == _reservoirs_.end())
        return n_removed;

    // I confirmed that the reservoir is a node, so I can call remove.
    return remove(id);
}

auto WaterDistributionSystem::remove_tank(const ID& id) -> size_type
{
    size_type n_removed = 0;

    auto it = _tanks_.find(id);
    if (it == _tanks_.end())
        return n_removed;

    // I confirmed that the tank is a node, so I can call remove.
    return remove(id);
}

// Links
auto WaterDistributionSystem::uninstall(const ID& id) -> size_type
{
    size_type n_removed = 0;

    auto it = _links_.find(id);
    if (it == _links_.end())
        return n_removed;

    // The nodes should forget this link. Then simply erase it from all collections.
    auto start_node_id = it->from_node()->EN_id();
    auto end_node_id = it->to_node()->EN_id();

    _nodes_.at(start_node_id).disconnect_link(it.operator->().get());
    _nodes_.at(end_node_id).disconnect_link(it.operator->().get());
    
    n_removed = _links_.erase(id);

    // Erase from all collections because erase a non existing element should be
    //  a no-op (still you iterate over all the elements again).
    _pipes_.erase(id);
    _pumps_.erase(id);

    return n_removed;
}

auto WaterDistributionSystem::uninstall_pipe(const ID& id) -> size_type
{
    size_type n_removed = 0;

    auto it = _pipes_.find(id);
    if (it == _pipes_.end())
        return n_removed;

    // We confirmed it's a pipe, so we can call uninstall.
    return uninstall(id);
}

auto WaterDistributionSystem::uninstall_pump(const ID& id) -> size_type
{
    size_type n_removed = 0;

    auto it = _pumps_.find(id);
    if (it == _pumps_.end())
        return n_removed;

    // We confirmed it's a pump, so we can call uninstall.
    return uninstall(id);
}

// Components


void WaterDistributionSystem::clear_results()
{
    for (const auto& [name, node] : _nodes_)
        node.clear_results();
    
    for (const auto& [name, link] : _links_)
        link.clear_results();
}

} // namespace bevarmejo
