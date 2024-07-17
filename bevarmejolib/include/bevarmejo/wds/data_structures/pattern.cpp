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
    m__multipliers(),
    m__start_time_s_(nullptr),
    m__timestep__s_(nullptr) { }

Pattern::Pattern(const std::string &id, long* ap__start_time_s, long* ap__timestep__s) :
    inherited(id),
    m__multipliers(),
    m__start_time_s_(ap__start_time_s),
    m__timestep__s_(ap__timestep__s) { }

// Copy constructor
Pattern::Pattern(const Pattern& other) :
    inherited(other), 
    m__multipliers(other.m__multipliers),
    m__start_time_s_(other.m__start_time_s_),
    m__timestep__s_(other.m__timestep__s_) { }

// Move constructor
Pattern::Pattern(Pattern&& rhs) noexcept : 
    inherited(std::move(rhs)),
    m__multipliers(std::move(rhs.m__multipliers)),
    m__start_time_s_(rhs.m__start_time_s_),
    m__timestep__s_(rhs.m__timestep__s_) { }

// Copy assignment operator
Pattern& Pattern::operator=(const Pattern& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        m__multipliers = rhs.m__multipliers;
        m__start_time_s_ = rhs.m__start_time_s_;
        m__timestep__s_ = rhs.m__timestep__s_;
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
Pattern& Pattern::operator=(Pattern&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        m__multipliers = std::move(rhs.m__multipliers);
        m__start_time_s_ = rhs.m__start_time_s_;
        m__timestep__s_ = rhs.m__timestep__s_;
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

    // If debug, check that you are also already pointing to the right time values.
    assert(m__start_time_s_ != nullptr);
    assert(m__timestep__s_ != nullptr);
    int errorcode = 0;
#ifndef NDEBUG
    long a__time=0;
    errorcode = EN_gettimeparam(ph, EN_PATTERNSTART, &a__time);
    assert(errorcode < 100);
    assert(a__time == *m__start_time_s_);
    errorcode = EN_gettimeparam(ph, EN_PATTERNSTEP, &a__time);
    assert(errorcode < 100);
    assert(a__time == *m__timestep__s_);
#endif

    int en_index = 0;
    double val = 0.0;
    int len = 0;
    errorcode = EN_getpatternlen(ph, index(), &len);
    if (errorcode > 100) {
        throw std::runtime_error("Error retrieving length of pattern "+id()+" from EPANET project.");
    }
    
    m__multipliers.reserve(len);

    // Start from +1. See epanet documentation.
    for(int i=1; i<=len; ++i) {
        errorcode = EN_getpatternvalue(ph, index(), i, &val); 
        if (errorcode > 100) {
            throw std::runtime_error("Error retrieving value of pattern "+id()+" from EPANET project.");
        }
        m__multipliers.push_back(val);
    }
}

} // namespace wds
} // namespace bevarmejo
