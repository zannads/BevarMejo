#include <string>
#include <unordered_map>
#include <variant>

#include "bevarmejo/wds/data_structures/variable.hpp"
#include "bevarmejo/wds/data_structures/temporal.hpp"

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/element.hpp"

#include "network_element.hpp"

namespace bevarmejo {
namespace wds {

NetworkElement::NetworkElement(const std::string& id, const WaterDistributionSystem& wds) :
    inherited(id),
    m__wds(wds),
    _results_(),
    m__ud_results()
    {
        _add_properties();
        _add_results();
        _update_pointers();
    }

// Copy constructor
NetworkElement::NetworkElement(const NetworkElement& other) :
    inherited(other),
    m__wds(other.m__wds),
    _results_(other._results_),
    m__ud_results()
    {
        for (auto& [key, ptr] : other.m__ud_results) {
            m__ud_results[key] = ptr->clone();
        }
        _update_pointers();
    }

// Move constructor
NetworkElement::NetworkElement(NetworkElement&& other) noexcept :
    inherited(std::move(other)),
    m__wds(other.m__wds),
    _results_(std::move(other._results_)),
    m__ud_results(std::move(other.m__ud_results))
    {
        _update_pointers();
    }

// Copy assignment operator
NetworkElement& NetworkElement::operator=(const NetworkElement& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        // m__wds = rhs.m__wds; // const reference cannot be assigned
        _results_ = rhs._results_;
        m__ud_results.clear();
        for (auto& [key, ptr] : rhs.m__ud_results) {
            m__ud_results[key] = ptr->clone();
        }
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
NetworkElement& NetworkElement::operator=(NetworkElement&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        // m__wds = std::move(rhs.m__wds); // const reference cannot be assigned
        _results_ = std::move(rhs._results_);
        m__ud_results = std::move(rhs.m__ud_results);
        _update_pointers();
    }
    return *this;
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
    m__ud_results.clear();
}

void NetworkElement::_update_pointers() { }

} // namespace wds
} // namespace bevarmejo
