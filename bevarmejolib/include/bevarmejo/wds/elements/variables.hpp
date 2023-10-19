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
            inherited::emplace(name,value);
        }

        template <typename ...Params>
        variables(const std::string& name, Params&& ...params) : inherited() {
            inherited::emplace(name, VT(std::forward<Params>(params)...));
        }

        // Copy constructor
        variables(const variables& other) : inherited(other) {}

        // Move constructor
        variables(variables&& other) noexcept : inherited(std::move(other)) {}

        // Copy assignment operator
        variables& operator=(const variables& rhs) {
            if (this != &rhs) {
                inherited::operator=(rhs);
            }
            return *this;
        }

        // Move assignment operator
        variables& operator=(variables&& rhs) noexcept {
            if (this != &rhs) {
                inherited::operator=(std::move(rhs));
            }
            return *this;
        }

        virtual ~variables() { inherited::clear(); }

        VT& get(const std::string& name) { return (*this).at(name); }

        auto& get_v(const std::string& name) { return (*this).at(name).value(); };

        void emplace(const std::string& name) {
            inherited::emplace(name, VT());
        }
        template <typename ...Params>
        void emplace(const std::string& name, Params&& ...params) {
            inherited::emplace(name, VT(std::forward<Params>(params)...));
        }

}; // class variables

} // namespace vars
} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__VARIABLES_HPP