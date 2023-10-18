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
        using inherited= std::map<long,ET>;

        temporal() : inherited() {}

        temporal(const ET& value) : inherited() {
            (*this)[0] = value;
        }

        temporal(const long time, const ET& value) : inherited() {
            (*this)[time] = value;
        }

        template <typename ...Params>
        temporal(const long time, Params&& ...params) : inherited() {
            (*this)[time] = ET(params...);
        }

        ET& when(const long time) {return (*this).at(time);};
        // but of course you have also find, at, [], contains etc..  
        
}; // class temporal

} // namespace vars
} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__TEMPORAL_HPP