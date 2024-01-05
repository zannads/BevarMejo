// Constructors and destructor from link.hpp
//
#include <cassert>
#include <string>
#include <unordered_map>
#include <variant>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/epanet_helpers/en_helpers.hpp"

#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/variable.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"

#include "link.hpp"

namespace bevarmejo {
namespace wds {

Link::Link(const std::string& id) : 
    inherited(id),
    _node_start_(nullptr),
    _node_end_(nullptr),
    _initial_status_(nullptr),
    _flow_(nullptr)
    {
        _add_properties();
        _add_results();
        _update_pointers();
    }

// Copy constructor
Link::Link(const Link& other) : 
    inherited(other),
    _node_start_(nullptr),
    _node_end_(nullptr),
    _initial_status_(nullptr),
    _flow_(nullptr)
    {
        _update_pointers();
    }

// Move constructor
Link::Link(Link&& rhs) noexcept : 
    inherited(std::move(rhs)),
    _node_start_(nullptr),
    _node_end_(nullptr),
    _initial_status_(nullptr),
    _flow_(nullptr)
    {
        _update_pointers();
    }

// Copy assignment operator
Link& Link::operator=(const Link& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
Link& Link::operator=(Link&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        _update_pointers();
    }
    return *this;
}

Link::~Link() {/* results are cleared when the inherited destructor is called*/ }

void Link::_add_properties() {
    inherited::_add_properties();

    properties().emplace(L_INITIAL_STATUS, vars::var_int(vars::l__DIMLESS, 0)); 
}

void Link::_add_results() {
    inherited::_add_results();

    results().emplace(L_FLOW, vars::var_tseries_real(vars::l__L_per_s));
}

void Link::_update_pointers() {
    inherited::_update_pointers();

    _initial_status_ = &std::get<vars::var_int>(properties().at(L_INITIAL_STATUS));

    _flow_ = &std::get<vars::var_tseries_real>(results().at(L_FLOW));
}

void Link::retrieve_index(EN_Project ph) {
    int en_index = 0;
    int errorcode = 0;
    errorcode = EN_getlinkindex(ph, id().c_str(), &en_index);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving index of link " + id() + " from EPANET project.");

    this->index(en_index);
}

void Link::retrieve_properties(EN_Project ph) {
    assert(index() != 0);
    int errorcode = 0;

    // get the initial status
    double d_initial_status = 0;
    errorcode = EN_getlinkvalue(ph, index(), EN_INITSTATUS, &d_initial_status);
    if (errorcode > 100) 
        throw std::runtime_error("Error retrieving initial status of link " + id() + " from EPANET project.");

    _initial_status_->value(d_initial_status);

    // Unfortuntely, I can't retrieve the pointers to the node if I'm not sure the nodes have been created. 
    // So I have to do it in the network class.
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

    this->_flow_->value().insert(std::make_pair(t, d_flow));
}

} // namespace wds
} // namespace bevarmejo
