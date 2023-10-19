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
            inherited::emplace(0,value);
        }

        temporal(const long time, const ET& value) : inherited() {
            inherited::emplace(time,value);
        }

        template <typename ...Params>
        temporal(const long time, Params&& ...params) : inherited() {
            inherited::emplace(time,ET(params...));
        }

        // Copy constructor
        temporal(const temporal& other) : inherited(other) {}

        // Move constructor
        temporal(temporal&& other) noexcept : inherited(std::move(other)) {}

        // Copy assignment operator
        temporal& operator=(const temporal& rhs) {
            if (this != &rhs) {
                inherited::operator=(rhs);
            }
            return *this;
        }

        // Move assignment operator
        temporal& operator=(temporal&& rhs) noexcept {
            if (this != &rhs) {
                inherited::operator=(std::move(rhs));
            }
            return *this;
        }

        virtual ~temporal() { inherited::clear(); }
       
        ET& when(const long time) {return (*this).at(time);};
        // but of course you have also find, at, [], contains etc..  
        
}; // class temporal

} // namespace vars
} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__TEMPORAL_HPP