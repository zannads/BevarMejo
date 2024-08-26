//
// element.cpp
//
// Created by Dennis Zanutto on 20/10/23.

#include <string>
#include <unordered_map>
#include <utility>

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "element.hpp"

namespace bevarmejo {
namespace wds {

Element::Element() :
    _id_(""),
    _index_(0),
    m__ud_properties() { }

Element::Element(const std::string& id) :
    _id_(id),
    _index_(0),
    m__ud_properties() { }

// Copy constructor
Element::Element(const Element& other) :
    _id_(other._id_),
    _index_(other._index_),
    m__ud_properties() {
        for (auto& [key, ptr] : other.m__ud_properties) {
            m__ud_properties[key] = ptr->clone();
        }
    }

// Move constructor
Element::Element(Element&& rhs) noexcept :
    _id_(std::move(rhs._id_)),
    _index_(rhs._index_),
    m__ud_properties(std::move(rhs.m__ud_properties)) { }

// Copy assignment operator
Element& Element::operator=(const Element& rhs) {
    if (this != &rhs) {
        _id_ = rhs._id_;
        _index_ = rhs._index_;
        m__ud_properties.clear();
        for (auto& [key, ptr] : rhs.m__ud_properties) {
            m__ud_properties[key] = ptr->clone();
        }
    }
    return *this;
}

// Move assignment operator
Element& Element::operator=(Element&& rhs) noexcept {
    if (this != &rhs) {
        _id_ = std::move(rhs._id_);
        _index_ = rhs._index_;
        m__ud_properties = std::move(rhs.m__ud_properties);
    }
    return *this;
}

bool Element::operator==(const Element& rhs) const {
    return _id_ == rhs._id_;
}

} // namespace wds
} // namespace bevarmejo
