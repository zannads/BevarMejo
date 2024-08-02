#include <cassert>
#include <string>
#include <stdexcept>
#include <utility>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/wds/epanet_helpers/en_help.hpp"

#include "bevarmejo/wds/epanet_helpers/en_time_options.hpp"
#include "bevarmejo/wds/auxiliary/time_series.hpp"
#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "link.hpp"

namespace bevarmejo {
namespace wds {

Link::Link(const std::string& id, const WaterDistributionSystem& wds) : 
    inherited(id, wds),
    _node_start_(nullptr),
    _node_end_(nullptr),
    m__initial_status(wds.time_series(l__CONSTANT_TS)),
    m__flow(wds.time_series(l__RESULT_TS)) { }

// Copy constructor
Link::Link(const Link& other) : 
    inherited(other),
    _node_start_(nullptr),
    _node_end_(nullptr),
    m__initial_status(other.m__initial_status),
    m__flow(other.m__flow) { }

// Move constructor
Link::Link(Link&& rhs) noexcept : 
    inherited(std::move(rhs)),
    _node_start_(nullptr),
    _node_end_(nullptr),
    m__initial_status(std::move(rhs.m__initial_status)),
    m__flow(std::move(rhs.m__flow)) { }

// Copy assignment operator
Link& Link::operator=(const Link& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        m__initial_status = rhs.m__initial_status;
        m__flow = rhs.m__flow;
    }
    return *this;
}

// Move assignment operator
Link& Link::operator=(Link&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        m__initial_status = std::move(rhs.m__initial_status);
        m__flow = std::move(rhs.m__flow);
    }
    return *this;
}

void Link::retrieve_index(EN_Project ph) {
    int en_index = 0;
    int errorcode = 0;
    errorcode = EN_getlinkindex(ph, id().c_str(), &en_index);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving index of link " + id() + " from EPANET project.");

    this->index(en_index);
}

void Link::__retrieve_EN_properties(EN_Project ph) {
    assert(index() != 0);
    auto nodes= m__wds.nodes();

    // get the initial status
    double d_initial_status = 0;
    int errorcode = EN_getlinkvalue(ph, index(), EN_INITSTATUS, &d_initial_status);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving initial status of link " + id() + " from EPANET project.");

    m__initial_status= d_initial_status;

    { // Assign Nodes
        int node1_idx= 0;
        int node2_idx= 0;
        int errorcode= EN_getlinknodes(ph, this->index(), &node1_idx, &node2_idx);
        assert(errorcode <= 100);

        std::string node1_id = epanet::get_node_id(ph, node1_idx);
        std::string node2_id = epanet::get_node_id(ph, node2_idx);

        auto it_node1 = nodes.find(node1_id);
        assert(it_node1 != nodes.end());
        auto it_node2 = nodes.find(node2_id);
        assert(it_node2 != nodes.end());

        // Assign the nodes to the links and viceversa
        this->start_node(it_node1->get());
        this->end_node(it_node2->get());

        (*it_node1)->add_link(this);
        (*it_node2)->add_link(this);
    }
}

void Link::retrieve_results(EN_Project ph, long t) {
    assert(index() != 0);
    int errorcode = 0;
    double d_flow = 0;
    errorcode = EN_getlinkvalue(ph, index(), EN_FLOW, &d_flow);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving flow of link " + id() + " from EPANET project.");
    
    if (ph->parser.Flowflag != LPS)
        d_flow = epanet::convert_flow_to_L_per_s(ph, d_flow);

    m__flow.commit(t, d_flow);
}

} // namespace wds
} // namespace bevarmejo
