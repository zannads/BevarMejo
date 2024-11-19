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

#include "bevarmejo/.legacy/io.hpp"

#include "water_distribution_system.hpp"

namespace bevarmejo::wds {


WaterDistributionSystem::~WaterDistributionSystem(){
    if (ph_!=nullptr){
        EN_close(ph_);
        EN_deleteproject(ph_);
        
        ph_ = nullptr;
        std::cout << "EPANET project deleted\n";
    }

    // First clear all the elements, then the time series and finally the config options
    
    _nodes_.clear();
    _junctions_.clear();
    _tanks_.clear();
    _reservoirs_.clear();
    
    _links_.clear();
    _pipes_.clear();
    _pumps_.clear();

    m__aux_elements_.patterns.clear();
    m__aux_elements_.curves.clear();

    _elements_.clear();
}


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

void WaterDistributionSystem::clear_results()
{
    for (const auto& [name, node] : _nodes_)
        node.clear_results();
    
    for (const auto& [name, link] : _links_)
        link.clear_results();
}

const aux::TimeSeries& WaterDistributionSystem::time_series(const std::string& name) const
{
    return m__times.time_series(name);
}

void WaterDistributionSystem::run_hydraulics()
{
    this->clear_results();
    m__times.results().reset();

    assert(ph_ != nullptr);
    // I assume indices are cached already 
    int errorcode = EN_openH(ph_);
    if (errorcode >= 100)
        throw std::runtime_error("Hydraulic opening failed."); // I don't think I need to close it here

    errorcode = EN_initH(ph_, 10);
    if (errorcode >= 100) {
        int errorcode2 = EN_closeH(ph_);
        throw std::runtime_error("Hydraulic initialization failed.");
    }

    // if the inp file is correct these errors should be always 0
    long h_step;
    errorcode = EN_gettimeparam(ph_, EN_HYDSTEP, &h_step);
    assert(errorcode < 100);
    long r_step;
    errorcode = EN_gettimeparam(ph_, EN_REPORTSTEP, &r_step);
    assert(errorcode < 100);
    long horizon;
    errorcode = EN_gettimeparam(ph_, EN_DURATION, &horizon);
    assert(errorcode < 100);

    long n_reports = horizon / r_step + 1; // +1 because the first report is at time 0
    m__times.results().reserve(n_reports);

    bool solution_has_failed = false;
    bool scheduled; // is the current time a reporting time?
    long t{ 0 }; // current time
    long delta_t{ 0 }; // real hydraulic time step
    
    do
    {
        errorcode = EN_runH(ph_, &t);

        if (errorcode >= 100)
        {
            solution_has_failed = true;
            break;
            // I don'return because I need to close the hydraulics
        }

        // if the current time is a reporting time, I save all the results
        // Use polymorphism to get the results from EPANET.
        scheduled = (t % r_step == 0);
        if (m__config_options.save_all_hsteps || scheduled)
        {
            m__times.results().commit(t);
            
            for (const auto& [id, node] : _nodes_)
                node.retrieve_results(ph_, t);
            
            for (const auto& [id, link] : _links_)
                link.retrieve_results(ph_, t);
        }

        errorcode = EN_nextH(ph_, &delta_t);
        assert(errorcode < 100);

    } while (delta_t > 0);

    errorcode = EN_closeH(ph_);
    assert(errorcode < 100);

    if (solution_has_failed)
        throw std::runtime_error("Hydraulic solution failed.");
}

auto WaterDistributionSystem::insert_ids_from_file(const fsys::path &file_path) -> IDSequences::iterator
{
    if (!fsys::exists(file_path))
        __format_and_throw<std::runtime_error>("WaterDistributionSystem", "insert()", "Impossible to insert the element(s).",
            "File does not exist.", "File: ", file_path);

    std::ifstream ifs(file_path);
    if (!ifs.is_open())
        __format_and_throw<std::runtime_error>("WaterDistributionSystem", "insert()", "Impossible to insert the element(s).",
            "Error opening the file.", "File: ", file_path);

    // Asssume it works form a JSON or my custom type, I will get a vector of strings.

    const auto [en_object_type, ids, comment] = io::get_egroup_data(ifs);

    auto name = file_path.stem().string();

    // Because I know it's only for a list of sequence otherwise, there should be a swith(en_object_type)
    auto ret_type = m__id_sequences.emplace(std::move(name), std::move(ids));

    return ret_type.iterator;
}

} // namespace bevarmejo::wds
