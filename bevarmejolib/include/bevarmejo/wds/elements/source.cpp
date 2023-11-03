#include <string>
#include <variant>

#include "bevarmejo/wds/elements/variable.hpp"
#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/element.hpp"

#include "source.hpp"

namespace bevarmejo {
namespace wds {

source::source(const std::string& id) : inherited(id), 
                                        _inflow_(nullptr),
                                        _source_elevation_(nullptr) 
                                        {
                                            _add_results();
                                            _update_pointers();
                                        }

// Copy constructor
source::source(const source& other) : inherited(other), 
                                        _inflow_(nullptr),
                                        _source_elevation_(nullptr) 
                                        {
                                            _update_pointers();
                                        }

// Move constructor
source::source(source&& rhs) noexcept : inherited(std::move(rhs)), 
                                        _inflow_(nullptr),
                                        _source_elevation_(nullptr) 
                                        {
                                            _update_pointers();
                                        }

// Copy assignment operator
source& source::operator=(const source& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
source& source::operator=(source&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        _update_pointers();
    }
    return *this;
}

source::~source() {/* results are cleared when the inherited destructor is called*/ }

void source::_add_results() {
    inherited::_add_results();

    results().temporal_reals().emplace(LINFLOW, vars::L_M3_PER_S);
    results().temporal_reals().emplace(LSOURCE_ELEV, vars::L_METER);
}

void source::_update_pointers() {
    inherited::_update_pointers();

    _inflow_ = &results().temporal_reals().at(LINFLOW);
    _source_elevation_ = &results().temporal_reals().at(LSOURCE_ELEV);
}

} // namespace wds
} // namespace bevarmejo
