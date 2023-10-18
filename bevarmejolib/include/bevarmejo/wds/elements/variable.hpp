// 
// variable.hpp
//
// Created by Dennis Zanutto on 18/10/23.

#ifndef BEVARMEJOLIB__WDS_ELEMENTS__VARIABLE_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__VARIABLE_HPP

#include <string>

namespace bevarmejo {
namespace wds {
namespace vars {

class variable_core
{
private:
    std::string _unit_; // TODO: see if it possible to convert to boost.units
public:
    variable_core(): _unit_(""){};

    variable_core(const std::string& unit): _unit_(unit){};

    ~variable_core() {};

    const std::string unit() const {return _unit_;};
    // Assignement operator not possible except at construction.

}; // class variable_core

template<typename VT>
class variable : public variable_core {

    private:
        VT _value_;

    public:

        variable() = default;

        variable(const std::string unit):
            variable_core(unit){};

        template <typename ...Params>
        variable(const std::string unit, Params&&... params): 
            variable_core(unit), 
            _value_(std::forward<Params>(params)...){};

        ~variable() {};

        VT& value() {return _value_;};
        void value(const VT& value) {_value_ = value;};
        VT& operator()() {return _value_;};
}; // class variable

} // namespace vars
} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__VARIABLE_HPP