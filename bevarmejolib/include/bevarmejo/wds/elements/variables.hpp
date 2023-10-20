// 
// variable.hpp
//
// Created by Dennis Zanutto on 18/10/23.

#ifndef BEVARMEJOLIB__WDS_ELEMENTS__VARIABLES_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__VARIABLES_HPP

#include <unordered_map>

#include "bevarmejo/wds/elements/variable.hpp"
#include "bevarmejo/wds/elements/temporal.hpp"

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

        auto& get(const std::string& name) { return (*this).at(name).value(); }

        void emplace(const std::string& name) {
            inherited::emplace(name, VT());
        }
        template <typename ...Params>
        void emplace(const std::string& name, Params&& ...params) {
            inherited::emplace(name, VT(std::forward<Params>(params)...));
        }

}; // class variables

using variables_int= variables<var_int>;
using variables_real= variables<var_real>;

using variables_tseries_int= variables<var_tseries_int>;
using variables_tseries_real= variables<var_tseries_real>;

} // namespace vars
} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__VARIABLES_HPP