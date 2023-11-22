#include <cassert>
#include <string>
#include <vector>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/element.hpp"

#include "pattern.hpp"

namespace bevarmejo {
namespace wds {

Pattern::Pattern(const std::string& id) : 
    inherited(id), 
    _multipliers_(),
    _start_time_s_(0),
    _step_s_(3600) { }

Pattern::Pattern(const std::string &id, long a_start_time_s, long a_step_s) :
    inherited(id),
    _multipliers_(),
    _start_time_s_(a_start_time_s),
    _step_s_(a_step_s) { }

// Copy constructor
Pattern::Pattern(const Pattern& other) :
    inherited(other), 
    _multipliers_(other._multipliers_),
    _start_time_s_(other._start_time_s_),
    _step_s_(other._step_s_) { }

// Move constructor
Pattern::Pattern(Pattern&& rhs) noexcept : 
    inherited(std::move(rhs)),
    _multipliers_(std::move(rhs._multipliers_)),
    _start_time_s_(rhs._start_time_s_),
    _step_s_(rhs._step_s_) { }

// Copy assignment operator
Pattern& Pattern::operator=(const Pattern& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        _multipliers_ = rhs._multipliers_;
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
Pattern& Pattern::operator=(Pattern&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        _multipliers_ = std::move(rhs._multipliers_);
        _update_pointers();
    }
    return *this;
}

void Pattern::retrieve_index(EN_Project ph) {
    int en_index = 0;
    int errorcode = EN_getpatternindex(ph, id().c_str(), &en_index);
    if (errorcode > 100) {
        throw std::runtime_error("Error retrieving index of pattern "+id()+" from EPANET project.");
    }
    this->index(en_index);
}

void Pattern::retrieve_properties(EN_Project ph) {
    assert(index()!= 0);

    int en_index = 0;
    int errorcode = 0;
    double val = 0.0;
    int len = 0;
    errorcode = EN_getpatternlen(ph, index(), &len);
    if (errorcode > 100) {
        throw std::runtime_error("Error retrieving length of pattern "+id()+" from EPANET project.");
    }
    
    _multipliers_.reserve(len);

    // Start from +1. See epanet documentation.
    for(int i=1; i<=len; ++i) {
        errorcode = EN_getpatternvalue(ph, index(), i, &val); 
        if (errorcode > 100) {
            throw std::runtime_error("Error retrieving value of pattern "+id()+" from EPANET project.");
        }
        _multipliers_.push_back(val);
    }
}

} // namespace wds
} // namespace bevarmejo
