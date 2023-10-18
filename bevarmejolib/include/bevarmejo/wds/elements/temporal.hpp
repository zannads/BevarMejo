//
// temporal.hpp
//
// Created by Dennis Zanutto on 18/10/23.

#ifndef BEVARMEJOLIB__WDS_ELEMENTS__TEMPORAL_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__TEMPORAL_HPP

#include <map>

namespace bevarmejo {
namespace wds {
namespace vars {

template <typename ET>
class temporal : public std::map<long,ET> {

    public:
        ET& when(const long time) {return (*this)[time];};

}; // class temporal

} // namespace vars
} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__TEMPORAL_HPP