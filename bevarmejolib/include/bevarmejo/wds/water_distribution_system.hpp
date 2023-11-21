//
//  water_distribution_system.hpp
//  bevarmejo cpp library
//
//  Created by Dennis Zanutto on 04/07/23.
//

#ifndef BEVARMEJOLIB__WDS__WATER_DISTRIBUTION_SYSTEM_HPP
#define BEVARMEJOLIB__WDS__WATER_DISTRIBUTION_SYSTEM_HPP

#include <filesystem>
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
#include "bevarmejo/wds/elements/dimensioned_link.hpp"
#include "bevarmejo/wds/elements/pipe.hpp"

#include "bevarmejo/wds/subnetwork.hpp"

namespace bevarmejo {

namespace wds {

class water_distribution_system{
public:
    // Handler for the project.
    // Public because I may want to modify it (e.g., apply a decision vector).
    // it is just faster than doing create an interface. I will be careful.
    mutable EN_Project ph_;
    
protected:
    // Path to the inp file from which the project will be uploaded.
    std::string _inp_filename_;
    // Collectionf of elements of the network
    std::vector<std::shared_ptr<Element>> _elements_;
    
    // Class-specific collections of elements
    std::vector<std::shared_ptr<node>> _nodes_;
    std::vector<std::shared_ptr<link>> _links_;
    std::vector<std::shared_ptr<pattern>> _patterns_;
    std::vector<std::shared_ptr<junction>> _junctions_;

    // Subnetworks of IDs
    std::unordered_set<subnetwork> _subnetworks_;
    
public:
    
    // Default constructor
    water_distribution_system();
    
    // Constructor from .inp file as a path reference
   // water_distribution_system(const std::filesystem::path& inp_filename);
    
    // Constructor from .inp file
    water_distribution_system(const std::string& inp_filename);
 
    // Copy constructor
    // this is not actually a copy constructor but rather a reinitialization one.
    water_distribution_system(const water_distribution_system &src);
    
    // Move constructor
    water_distribution_system(water_distribution_system &&src) noexcept;
    
    // Copy Assignement operator
    water_distribution_system& operator=(const water_distribution_system& rhs);
    
    // Move Assignement operator
    water_distribution_system& operator=(water_distribution_system&& rhs) noexcept;
    
    ~water_distribution_system();
    
    // Equivalent to constuctor from .inp file
    void init();
    
    // Set/get _inpFilename
    //void set_inpfile(const std::string& inp_filename);
    void set_inpfile(const std::string inp_filename);
    std::string get_inpfile() const;

    // Cache the indices of the elements in the network.
    // This is useful to avoid calling the ENgetnodeindex and ENgetlinkindex functions every time.
    void cache_indices() const;
    void assign_demands();
    void connect_network();
    
    void run_hydraulics() const;

    // add a subnetwork to the list of subnetworks from path to file
    void add_subnetwork(const std::filesystem::path& subnetwork_filename);
    // get back a subnework from the list of subnetworks
    bevarmejo::wds::subnetwork get_subnetwork(const std::string& name) const;
    // tell me if an ID is part of this subnetwork
    bool is_in_subnetork(const std::string& name, const std::string& id) const;

    // Wrappers for linees-of-code consuming EPANET functions
    std::string get_node_id(int index) const;

}; // class water_distribution_system

} // namespace wds

using wds_ = wds::water_distribution_system; // short name for water_distribution_system

} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS__WATER_DISTRIBUTION_SYSTEM_HPP