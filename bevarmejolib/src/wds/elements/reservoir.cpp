#include <cassert>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "epanet2_2.h"

#include "bevarmejo/wds/data_structures/temporal.hpp"
#include "bevarmejo/wds/data_structures/variable.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/source.hpp"

#include "reservoir.hpp"

namespace bevarmejo {
namespace wds {

Reservoir::Reservoir(const std::string& id, const WaterDistributionSystem& wds) :
    inherited(id, wds)
    {
        _add_properties();
        _add_results();
        _update_pointers();
    }

// Copy constructor
Reservoir::Reservoir(const Reservoir& other) : 
    inherited(other)
    {
        _update_pointers();
    }

// Move constructor
Reservoir::Reservoir(Reservoir&& rhs) noexcept : 
    inherited(std::move(rhs))
    {
        _update_pointers();
    }

Reservoir& Reservoir::operator=(const Reservoir& other) {
    if (this != &other) {
        inherited::operator=(other);

        _update_pointers();
    }
    return *this;
}

Reservoir& Reservoir::operator=(Reservoir&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));

        _update_pointers();
    }
    return *this;
}

} // namespace wds
} // namespace bevarmejo
