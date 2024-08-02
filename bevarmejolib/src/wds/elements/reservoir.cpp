#include <cassert>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/source.hpp"

#include "reservoir.hpp"

namespace bevarmejo {
namespace wds {

Reservoir::Reservoir(const std::string& id, const WaterDistributionSystem& wds) :
    inherited(id, wds) { }

// Copy constructor
Reservoir::Reservoir(const Reservoir& other) : 
    inherited(other) { }

// Move constructor
Reservoir::Reservoir(Reservoir&& rhs) noexcept : 
    inherited(std::move(rhs)) { }

Reservoir& Reservoir::operator=(const Reservoir& other) {
    if (this != &other) {
        inherited::operator=(other);
    }
    return *this;
}

Reservoir& Reservoir::operator=(Reservoir&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
    }
    return *this;
}

} // namespace wds
} // namespace bevarmejo
