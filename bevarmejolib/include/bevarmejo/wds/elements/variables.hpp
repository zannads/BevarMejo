// 
// variable.hpp
//
// Created by Dennis Zanutto on 18/10/23.

#ifndef BEVARMEJOLIB__WDS_ELEMENTS__VARIABLES_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__VARIABLES_HPP

#include <unordered_map>

namespace bevarmejo {
namespace wds {
namespace vars {

template <typename VT>
class variables : public std::unordered_map<std::string, VT> {

    public:
        using inherited= std::unordered_map<std::string, VT>;

        variables() : inherited() {}

        // can't add a variable without the name! 
        variables(const std::string& name, const VT& value) : inherited() {
            (*this)[name] = value;
        }

        template <typename ...Params>
        variables(const std::string& name, Params&& ...params) : inherited() {
            (*this)[name] = VT(params...);
        }

        VT& get(const std::string& name) {return (*this).at(name);};

}; // class variables

} // namespace vars
} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__VARIABLES_HPP