#include <cassert>

#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/element.hpp"

#include "pattern.hpp"

namespace bevarmejo {
namespace wds {

pattern::pattern() : inherited(),
                     _data_()
                    {
                        _add_results();
                        _update_pointers();
                    }

pattern::pattern(const std::string& id) : inherited(id),
                                            _data_() 
                                            {
                                                _add_results();
                                                _update_pointers();
                                            }

// Copy constructor
pattern::pattern(const pattern& other) : inherited(other),
                                         _data_(other._data_) 
                                            {
                                                _update_pointers();
                                            }

// Move constructor
pattern::pattern(pattern&& rhs) noexcept : inherited(std::move(rhs)),
                                           _data_(std::move(rhs._data_))
                                            {
                                                _update_pointers();
                                            }

// Copy assignment operator
pattern& pattern::operator=(const pattern& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        _data_ = rhs._data_;
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
pattern& pattern::operator=(pattern&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        _data_ = std::move(rhs._data_);
        _update_pointers();
    }
    return *this;
}

void pattern::retrieve_index(EN_Project ph) {
    int en_index = 0;
    int errorcode = EN_getpatternindex(ph, id().c_str(), &en_index);
    if (errorcode > 100) {
        throw std::runtime_error("Error retrieving index of pattern "+id()+" from EPANET project.");
    }
    this->index(en_index);
}

void pattern::retrieve_properties(EN_Project ph) {
    assert(index()!= 0);

    int en_index = 0;
    int errorcode = 0;
    double val = 0.0;
    int len = 0;
    errorcode = EN_getpatternlen(ph, index(), &len);
    if (errorcode > 100) {
        throw std::runtime_error("Error retrieving length of pattern "+id()+" from EPANET project.");
    }
    
    _data_.reserve(len);

    // Start from +1. See epanet documentation.
    for(int i=1; i<=len; ++i) {
        errorcode = EN_getpatternvalue(ph, index(), i, &val); 
        if (errorcode > 100) {
            throw std::runtime_error("Error retrieving value of pattern "+id()+" from EPANET project.");
        }
        _data_.push_back(val);
    }
}

} // namespace wds
} // namespace bevarmejo
