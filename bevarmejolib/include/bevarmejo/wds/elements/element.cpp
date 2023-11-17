//
// element.cpp
//
// Created by Dennis Zanutto on 20/10/23.

#include <string>
#include <unordered_map>
#include <variant>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/variable.hpp"
#include "bevarmejo/wds/elements/temporal.hpp"

#include "element.hpp"

namespace bevarmejo {
namespace wds {

element::element() :
    _id_(""),
    _index_(0),
    _properties_()
    {
        _add_properties();
        _update_pointers();
    }

element::element(const std::string& id) :
    _id_(id),
    _index_(0),
    _properties_()
    {
        _add_properties();
        _update_pointers();
    }

// Copy constructor
element::element(const element& other) :
    _id_(other._id_),
    _index_(other._index_),
    _properties_(other._properties_)
    {
        _update_pointers();
    }

// Move constructor
element::element(element&& rhs) noexcept :
    _id_(std::move(rhs._id_)),
    _index_(rhs._index_),
    _properties_(std::move(rhs._properties_))
    {
        _update_pointers();
    }

// Copy assignment operator
element& element::operator=(const element& rhs) {
    if (this != &rhs) {
        _id_ = rhs._id_;
        _index_ = rhs._index_;
        _properties_ = rhs._properties_;
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
element& element::operator=(element&& rhs) noexcept {
    if (this != &rhs) {
        _id_ = std::move(rhs._id_);
        _index_ = rhs._index_;
        _properties_ = std::move(rhs._properties_);
        _update_pointers();
    }
    return *this;
}

element::~element() {
    _properties_.clear();
}

bool element::operator==(const element& rhs) const {
    return _id_ == rhs._id_;
}

void element::_update_pointers() {
    // If in derived classes you have pointers to properties or results,
    // you should override this function and update them here.
}

void element::_add_properties() {
    // If in derived classes you have properties, you should override this 
    // function and add them here.
    _properties_.clear();
}

} // namespace wds
} // namespace bevarmejo
