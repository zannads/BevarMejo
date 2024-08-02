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
    inherited(id, wds),
    m__head(wds.time_series(l__RESULT_TS))
    {
        _add_properties();
        _add_results();
        _update_pointers();
    }

// Copy constructor
Reservoir::Reservoir(const Reservoir& other) : 
    inherited(other),
    m__head(other.m__head)
    {
        _update_pointers();
    }

// Move constructor
Reservoir::Reservoir(Reservoir&& rhs) noexcept : 
    inherited(std::move(rhs)),
    m__head(std::move(rhs.m__head))
    {
        _update_pointers();
    }

Reservoir& Reservoir::operator=(const Reservoir& other) {
    if (this != &other) {
        inherited::operator=(other);
        m__head = other.m__head;
        _update_pointers();
    }
    return *this;
}

Reservoir& Reservoir::operator=(Reservoir&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        m__head = std::move(rhs.m__head);
        _update_pointers();
    }
    return *this;
}

void Reservoir::__retrieve_EN_properties(EN_Project ph) {
    inherited::__retrieve_EN_properties(ph);
}

void Reservoir::retrieve_results(EN_Project ph, long t) {
    inherited::retrieve_results(ph, t);
}

void Reservoir::_add_properties() {
    inherited::_add_properties();
}

void Reservoir::_add_results() {
    inherited::_add_results();
}

void Reservoir::_update_pointers() {
    inherited::_update_pointers();
}

} // namespace wds
} // namespace bevarmejo
