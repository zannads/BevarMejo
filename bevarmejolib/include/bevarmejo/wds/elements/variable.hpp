// 
// variable.hpp
//
// Created by Dennis Zanutto on 18/10/23.

#ifndef BEVARMEJOLIB__WDS_ELEMENTS__VARIABLE_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__VARIABLE_HPP

#include <string>

#include "bevarmejo/wds/elements/temporal.hpp"

namespace bevarmejo {
namespace wds {
namespace vars {

static const std::string L_DIMLESS = "";
static const std::string l_DIMLESS = "";
static const std::string L_METER = "m";
static const std::string l__m = "m";
static const std::string l__m3 = "m3";
static const std::string l__m3_per_s = "m3/s";
static const std::string L_M3_PER_S = "m/s";
static const std::string l_L_per_s = "L/s";
static const std::string L_KWH = "kWh";
static const std::string l_kWh = "kWh";
static const std::string L_JOUL = "J";
static const std::string l__J = "J";
static const std::string l__W = "W";
static const std::string l__Euro = "€";

class variable_core
{
private:
    std::string _unit_; // TODO: see if it possible to convert to boost.units
public:
    variable_core() : _unit_(L_DIMLESS) {}

    variable_core(const std::string& unit): _unit_(unit){}

    // Copy constructor
    variable_core(const variable_core& other) : _unit_(other._unit_) {}

    // Move constructor
    variable_core(variable_core&& other) : _unit_(std::move(other._unit_)) {}

    // Copy assignment operator
    variable_core& operator=(const variable_core& other) {
        if (this != &other) {
            _unit_ = other._unit_;
        }
        return *this;
    }

    // Move assignment operator
    variable_core& operator=(variable_core&& other) {
        if (this != &other) {
            _unit_ = std::move(other._unit_);
        }
        return *this;
    }    

    virtual ~variable_core() {}


    const std::string unit() const {return _unit_;} 
    // Assignement operator not possible except at construction.

}; // class variable_core

template<typename VT>
class variable : public variable_core {
    public:
        using inherited= variable_core;

    private:
        VT _value_;

    public:

        variable() : inherited(),
                     _value_() {}

        variable(const std::string unit) : inherited(unit),
                                            _value_() {}

        variable(const VT& value) : inherited(),
                                    _value_(value) {}

        template <typename ...Params>
        variable(const std::string unit, Params&&... params) : inherited(unit), 
                                                                _value_(std::forward<Params>(params)...) {}

        // Copy constructor
        variable(const variable& other) : inherited(other),
                                            _value_(other._value_) {}

        // Move constructor
        variable(variable&& other) : inherited(std::move(other)),
                                    _value_(std::move(other._value_)) {}

        // Copy assignment operator
        variable& operator=(const variable& other) {
            if (this != &other) {
                inherited::operator=(other);
                _value_ = other._value_;
            }
            return *this;
        }

        // Move assignment operator
        variable& operator=(variable&& other) {
            if (this != &other) {
                inherited::operator=(std::move(other));
                _value_ = std::move(other._value_);
            }
            return *this;
        }

        virtual ~variable() {}

        VT& value() {return _value_;}
        const VT& value() const {return _value_;}
        void value(const VT& value) {_value_ = value;}
        VT& operator()() {return _value_;} 

}; // class variable

using var_int= variable<int>;
using var_real= variable<double>;

using var_tseries_int= variable<timeseries_int>;
using var_tseries_real= variable<timeseries_real>;

} // namespace vars
} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__VARIABLE_HPP