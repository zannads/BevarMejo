#include <string>
#include <unordered_map>
#include <variant>

#include "epanet2_2.h"

#include "bevarmejo/wds/data_structures/variable.hpp"
#include "bevarmejo/wds/data_structures/temporal.hpp"
#include "bevarmejo/wds/elements/element.hpp"

#include "network_element.hpp"

namespace bevarmejo {
namespace wds {

NetworkElement::NetworkElement(const std::string& id) :
    inherited(id),
    _results_()
    {
        _add_properties();
        _add_results();
        _update_pointers();
    }

// Copy constructor
NetworkElement::NetworkElement(const NetworkElement& other) :
    inherited(other),
    _results_(other._results_)
    {
        _update_pointers();
    }

// Move constructor
NetworkElement::NetworkElement(NetworkElement&& other) noexcept :
    inherited(std::move(other)),
    _results_(std::move(other._results_))
    {
        _update_pointers();
    }

// Copy assignment operator
NetworkElement& NetworkElement::operator=(const NetworkElement& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        _results_ = rhs._results_;
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
NetworkElement& NetworkElement::operator=(NetworkElement&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        _results_ = std::move(rhs._results_);
        _update_pointers();
    }
    return *this;
}

// Destructor
NetworkElement::~NetworkElement() {
    _results_.clear();
}

void NetworkElement::clear_results() {
    for (auto& [key, value] : _results_) {
        std::visit([](auto&& arg) { arg.clear(); }, value);
    }
}

void NetworkElement::_add_properties() {
    inherited::_add_properties();
}

void NetworkElement::_add_results() {
    // If in derived classes you have results, you should override this 
    // function and add them here.
    _results_.clear();
}

void NetworkElement::_update_pointers() { }

} // namespace wds
} // namespace bevarmejo
