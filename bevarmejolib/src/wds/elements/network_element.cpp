#include <string>

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/element.hpp"

#include "network_element.hpp"

namespace bevarmejo {
namespace wds {

NetworkElement::NetworkElement(const std::string& id, const WaterDistributionSystem& wds) :
    inherited(id),
    m__wds(wds),
    m__ud_results()
    { }

// Copy constructor
NetworkElement::NetworkElement(const NetworkElement& other) :
    inherited(other),
    m__wds(other.m__wds),
    m__ud_results()
    {
        for (auto& [key, ptr] : other.m__ud_results) {
            m__ud_results[key] = ptr->clone();
        }
    }

// Move constructor
NetworkElement::NetworkElement(NetworkElement&& other) noexcept :
    inherited(std::move(other)),
    m__wds(other.m__wds),
    m__ud_results(std::move(other.m__ud_results))
    { }

// Copy assignment operator
NetworkElement& NetworkElement::operator=(const NetworkElement& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        // m__wds = rhs.m__wds; // const reference cannot be assigned
        m__ud_results.clear();
        for (auto& [key, ptr] : rhs.m__ud_results) {
            m__ud_results[key] = ptr->clone();
        }
    }
    return *this;
}

// Move assignment operator
NetworkElement& NetworkElement::operator=(NetworkElement&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        // m__wds = std::move(rhs.m__wds); // const reference cannot be assigned
        m__ud_results = std::move(rhs.m__ud_results);
    }
    return *this;
}

void NetworkElement::clear_results() {
    m__ud_results.clear();
}

} // namespace wds
} // namespace bevarmejo
