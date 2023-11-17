#include <string>
#include <unordered_map>
#include <variant>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/variable.hpp"
#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/element.hpp"

#include "network_element.hpp"

namespace bevarmejo {
namespace wds {

network_element::network_element(const std::string& id) :
    inherited(id),
    _results_()
    {
        _add_properties();
        _add_results();
        _update_pointers();
    }

// Copy constructor
network_element::network_element(const network_element& other) :
    inherited(other),
    _results_(other._results_)
    {
        _update_pointers();
    }

// Move constructor
network_element::network_element(network_element&& other) noexcept :
    inherited(std::move(other)),
    _results_(std::move(other._results_))
    {
        _update_pointers();
    }

// Copy assignment operator
network_element& network_element::operator=(const network_element& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        _results_ = rhs._results_;
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
network_element& network_element::operator=(network_element&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        _results_ = std::move(rhs._results_);
        _update_pointers();
    }
    return *this;
}

// Destructor
network_element::~network_element() {
    _results_.clear();
}

void network_element::_add_properties() {
    inherited::_add_properties();
}

void network_element::_add_results() {
    // If in derived classes you have results, you should override this 
    // function and add them here.
    _results_.clear();
}

void network_element::_update_pointers() { }

} // namespace wds
} // namespace bevarmejo
