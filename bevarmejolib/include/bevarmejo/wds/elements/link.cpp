// Constructors and destructor from link.hpp
//

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/results.hpp"
#include "bevarmejo/wds/elements/variables.hpp"
#include "bevarmejo/wds/elements/variable.hpp"
#include "bevarmejo/wds/elements/temporal.hpp"

#include "bevarmejo/wds/elements/node.hpp"

#include "link.hpp"

namespace bevarmejo {
namespace wds {

link::link(const std::string& id) : inherited(id),
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
link::link(const link& other) : inherited(other),
                                _node_start_(nullptr),
                                _node_end_(nullptr),
                                _initial_status_(nullptr),
                                _flow_(nullptr)
                                {
                                    _update_pointers();
                                }

// Move constructor
link::link(link&& rhs) noexcept : inherited(std::move(rhs)),
                                    _node_start_(nullptr),
                                    _node_end_(nullptr),
                                    _initial_status_(nullptr),
                                    _flow_(nullptr)
                                    {
                                        _update_pointers();
                                    }

// Copy assignment operator
link& link::operator=(const link& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
link& link::operator=(link&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        _update_pointers();
    }
    return *this;
}

link::~link() {/* results are cleared when the inherited destructor is called*/ }

void link::_add_properties() {
    inherited::_add_properties();

    properties().emplace(L_INITIAL_STATUS, vars::var_int(vars::L_DIMLESS, 0)); 
}

void link::_add_results() {
    inherited::_add_results();

    results().temporal_reals().emplace(L_FLOW, vars::L_M3_PER_S);
}

void link::_update_pointers() {
    inherited::_update_pointers();

    _initial_status_ = &std::get<vars::var_int>(properties().at(L_INITIAL_STATUS));

    _flow_ = &results().temporal_reals().at(L_FLOW);
}

void link::from_node(node* n) {
    if (_node_start_ != nullptr)
        _node_start_->remove_link(this);

    _node_start_ = n;

    if (_node_start_ != nullptr)
        _node_start_->connected_links().insert(this);
}

void link::to_node(node* n) {
    if (_node_end_ != nullptr)
        _node_end_->remove_link(this);

    _node_end_ = n;

    if (_node_end_ != nullptr)
        _node_end_->connected_links().insert(this);
}

} // namespace wds
} // namespace bevarmejo
