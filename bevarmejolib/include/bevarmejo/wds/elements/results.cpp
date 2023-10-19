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
