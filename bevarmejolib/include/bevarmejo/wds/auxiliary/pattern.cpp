#include <cassert>
#include <string>
#include <vector>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/element.hpp"

#include "pattern.hpp"

namespace bevarmejo {
namespace wds {

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
    double val = 0.0;
    int len = 0;
    int errorcode = EN_getpatternlen(ph, index(), &len);
    if (errorcode > 100) {
        throw std::runtime_error("Error retrieving length of pattern "+id()+" from EPANET project.");
    }
    m__multipliers.clear();
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
