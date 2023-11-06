//
// element.cpp
//
// Created by Dennis Zanutto on 20/10/23.

#include <string>

#include "bevarmejo/wds/elements/results.hpp"

#include "element.hpp"

namespace bevarmejo {
namespace wds {

element::element() :
    _id_(""),
    _index_(0),
    _properties_(),
    _results_()
    {
        _add_properties();
        _add_results();
        _update_pointers();
    }

element::element(const std::string& id) :
    _id_(id),
    _index_(0),
    _properties_(),
    _results_()
    {
        _add_properties();
        _add_results();
        _update_pointers();
    }

// Copy constructor
element::element(const element& other) :
    _id_(other._id_),
    _index_(other._index_),
    _properties_(other._properties_),
    _results_(other._results_)
    {
        _update_pointers();
    }

// Move constructor
element::element(element&& rhs) noexcept :
    _id_(std::move(rhs._id_)),
    _index_(rhs._index_),
    _properties_(std::move(rhs._properties_)),
    _results_(std::move(rhs._results_))
    {
        _update_pointers();
    }

// Copy assignment operator
element& element::operator=(const element& rhs) {
    if (this != &rhs) {
        _id_ = rhs._id_;
        _index_ = rhs._index_;
        _properties_ = rhs._properties_;
        _results_ = rhs._results_;
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
        _results_ = std::move(rhs._results_);
        _update_pointers();
    }
    return *this;
}

element::~element() {
    _properties_.clear();
    _results_.clear();
}

bool element::operator==(const element& rhs) const {
    return _id_ == rhs._id_;
}

void element::_update_pointers() {
    // there are actually no pointers to update with this setup
}

void element::_add_properties() {
    // override to add properties to the properties object
    _properties_.clear();
}

void element::_add_results() {
    // override to add results to the results object
    _results_.clear();
}

} // namespace wds
} // namespace bevarmejo
