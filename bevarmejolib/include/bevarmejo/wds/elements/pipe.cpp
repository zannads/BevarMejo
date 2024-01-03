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

Pipe::Pipe(const std::string& id) : 
    inherited(id),
    _length_(nullptr)
    {
        _add_properties();
        _add_results();
        _update_pointers();
    }

// Copy constructor
Pipe::Pipe(const Pipe& other) : 
    inherited(other),
    _length_(nullptr)
    {
        _update_pointers();
    }

// Move constructor
Pipe::Pipe(Pipe&& rhs) noexcept : 
    inherited(std::move(rhs)),
    _length_(nullptr)
    {
        _update_pointers();
    }

// Copy assignment operator
Pipe& Pipe::operator=(const Pipe& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
Pipe& Pipe::operator=(Pipe&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        _update_pointers();
    }
    return *this;
}

// Destructor
Pipe::~Pipe() {
    // delete _length_;
}

std::shared_ptr<Pipe> Pipe::duplicate() const {
    std::shared_ptr<Pipe> p_pipe = std::make_shared<Pipe>(*this);
    std::string new_id = "D"+ this->id();
    p_pipe->id(new_id);
    p_pipe->index(0);
    return p_pipe;
}

std::shared_ptr<Pipe> Pipe::duplicate(const std::string& id) const {
    std::shared_ptr<Pipe> p_pipe = std::make_shared<Pipe>(*this);
    p_pipe->id(id);
    p_pipe->index(0);
    return p_pipe;
}

void Pipe::retrieve_properties(EN_Project ph)
{
    inherited::retrieve_properties(ph);
    assert(index()!= 0);

    int errorode = 0;
    double length = 0.0;
    errorode = EN_getlinkvalue(ph, index(), EN_LENGTH, &length);
    if (errorode != 0)
        throw std::runtime_error("Error retrieving pipe length");
    _length_->value(length);
}

void Pipe::_add_properties() {
    inherited::_add_properties();

    properties().emplace(L_LENGTH, vars::var_real(vars::L_METER, 0.0));
}

void Pipe::_update_pointers() {
    inherited::_update_pointers();

    _length_= &std::get<vars::var_real>(properties().at(L_LENGTH));
}

} // namespace wds
} // namespace bevarmejo
