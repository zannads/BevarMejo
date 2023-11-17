#include <cassert>
#include <string>
#include <unordered_map>
#include <variant>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/variable.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/link.hpp"

#include "pipe.hpp"

namespace bevarmejo {
namespace wds {

pipe::pipe(const std::string& id) : 
    inherited(id),
    _length_(nullptr)
    {
        _add_properties();
        _add_results();
        _update_pointers();
    }

// Copy constructor
pipe::pipe(const pipe& other) : 
    inherited(other),
    _length_(nullptr)
    {
        _update_pointers();
    }

// Move constructor
pipe::pipe(pipe&& rhs) noexcept : 
    inherited(std::move(rhs)),
    _length_(nullptr)
    {
        _update_pointers();
    }

// Copy assignment operator
pipe& pipe::operator=(const pipe& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
pipe& pipe::operator=(pipe&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        _update_pointers();
    }
    return *this;
}

// Destructor
pipe::~pipe() {
    // delete _length_;
}

void pipe::retrieve_properties(EN_Project ph) {
    inherited::retrieve_properties(ph);
    assert(index()!= 0);

    int errorode = 0;
    double length = 0.0;
    errorode = EN_getlinkvalue(ph, index(), EN_LENGTH, &length);
    if (errorode != 0)
        throw std::runtime_error("Error retrieving pipe length");
    _length_->value(length);
}

void pipe::_add_properties() {
    inherited::_add_properties();

    properties().emplace(L_LENGTH, vars::var_real(vars::L_METER, 0.0));
}

void pipe::_update_pointers() {
    inherited::_update_pointers();

    _length_= &std::get<vars::var_real>(properties().at(L_LENGTH));
}

} // namespace wds
} // namespace bevarmejo
