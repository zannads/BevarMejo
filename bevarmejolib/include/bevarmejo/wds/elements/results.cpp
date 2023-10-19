//
// results.cpp
//
// Created by Dennis Zanutto on 19/10/23.

#include <string>

#include "temporal.hpp"
#include "variable.hpp"
#include "variables.hpp"

#include "results.hpp"

namespace bevarmejo {
namespace wds {

results::results() :
    _strings_(),
    _integers_(),
    _reals_(),
    _temporal_integers_(),
    _temporal_reals_()
    {}

// Copy constructor
results::results(const results& other) :
    _strings_(other._strings_),
    _integers_(other._integers_),
    _reals_(other._reals_),
    _temporal_integers_(other._temporal_integers_),
    _temporal_reals_(other._temporal_reals_)
    {}

// Move constructor
results::results(results&& rhs) noexcept :
    _strings_(std::move(rhs._strings_)),
    _integers_(std::move(rhs._integers_)),
    _reals_(std::move(rhs._reals_)),
    _temporal_integers_(std::move(rhs._temporal_integers_)),
    _temporal_reals_(std::move(rhs._temporal_reals_))
    {}

// Copy assignment operator
results& results::operator=(const results& rhs) {
    if (this != &rhs) {
        _strings_ = rhs._strings_;
        _integers_ = rhs._integers_;
        _reals_ = rhs._reals_;
        _temporal_integers_ = rhs._temporal_integers_;
        _temporal_reals_ = rhs._temporal_reals_;
    }
    return *this;
}

// Move assignment operator
results& results::operator=(results&& rhs) noexcept {
    if (this != &rhs) {
        _strings_ = std::move(rhs._strings_);
        _integers_ = std::move(rhs._integers_);
        _reals_ = std::move(rhs._reals_);
        _temporal_integers_ = std::move(rhs._temporal_integers_);
        _temporal_reals_ = std::move(rhs._temporal_reals_);
    }
    return *this;
}

results::~results() {
    this->clear();
}

void results::add(const results& rhs) {
    _strings_.insert(rhs._strings_.begin(), rhs._strings_.end());
    _integers_.insert(rhs._integers_.begin(), rhs._integers_.end());
    _reals_.insert(rhs._reals_.begin(), rhs._reals_.end());
    _temporal_integers_.insert(rhs._temporal_integers_.begin(), rhs._temporal_integers_.end());
    _temporal_reals_.insert(rhs._temporal_reals_.begin(), rhs._temporal_reals_.end());
}

const std::size_t results::size() const {
    return _strings_.size() + _integers_.size() + _reals_.size() + _temporal_integers_.size() + _temporal_reals_.size();
}

void results::clear() {
    _strings_.clear();
    _integers_.clear();
    _reals_.clear();
    _temporal_integers_.clear();
    _temporal_reals_.clear();
}

} // namespace wds 
} // namespace bevarmejo
