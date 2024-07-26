//
// element.cpp
//
// Created by Dennis Zanutto on 20/10/23.

#include <string>
#include <unordered_map>
#include <variant>

#include "epanet2_2.h"

#include "bevarmejo/wds/data_structures/variable.hpp"
#include "bevarmejo/wds/data_structures/temporal.hpp"

#include "element.hpp"

namespace bevarmejo {
namespace wds {

Element::Element() :
    _id_(""),
    _index_(0),
    _properties_()
    {
        _add_properties();
        _update_pointers();
    }

Element::Element(const std::string& id) :
    _id_(id),
    _index_(0),
    _properties_()
    {
        _add_properties();
        _update_pointers();
    }

// Copy constructor
Element::Element(const Element& other) :
    _id_(other._id_),
    _index_(other._index_),
    _properties_(other._properties_)
    {
        _update_pointers();
    }

// Move constructor
Element::Element(Element&& rhs) noexcept :
    _id_(std::move(rhs._id_)),
    _index_(rhs._index_),
    _properties_(std::move(rhs._properties_))
    {
        _update_pointers();
    }

// Copy assignment operator
Element& Element::operator=(const Element& rhs) {
    if (this != &rhs) {
        _id_ = rhs._id_;
        _index_ = rhs._index_;
        _properties_ = rhs._properties_;
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
Element& Element::operator=(Element&& rhs) noexcept {
    if (this != &rhs) {
        _id_ = std::move(rhs._id_);
        _index_ = rhs._index_;
        _properties_ = std::move(rhs._properties_);
        _update_pointers();
    }
    return *this;
}

Element::~Element() {
    _properties_.clear();
}

bool Element::operator==(const Element& rhs) const {
    return _id_ == rhs._id_;
}

void Element::_update_pointers() {
    // If in derived classes you have pointers to properties or results,
    // you should override this function and update them here.
}

void Element::_add_properties() {
    // If in derived classes you have properties, you should override this 
    // function and add them here.
    _properties_.clear();
}

} // namespace wds
} // namespace bevarmejo
