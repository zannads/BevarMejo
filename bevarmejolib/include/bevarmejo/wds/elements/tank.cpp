#include <cassert>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/variable.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/source.hpp"

#include "tank.hpp"

namespace bevarmejo {
namespace wds {

Tank::Tank(const std::string& id) :
    inherited(id),
    _diameter_(nullptr),
    _min_volume_(nullptr),
    _volume_curve_(nullptr),
    _min_level_(nullptr),
    _max_level_(nullptr),
    _can_overflow_(nullptr),
    _initial_level_(nullptr),
    _level_(nullptr),
    _initial_volume_(nullptr),
    _volume_(nullptr),
    _max_volume_(nullptr)
    {
        _add_properties();
        _add_results();
        _update_pointers();
    }

// Copy constructor
Tank::Tank(const Tank& other) : 
    inherited(other),
    _diameter_(nullptr),
    _min_volume_(nullptr),
    _volume_curve_(other._volume_curve_),
    _min_level_(nullptr),
    _max_level_(nullptr),
    _can_overflow_(nullptr),
    _initial_level_(nullptr),
    _level_(nullptr),
    _initial_volume_(nullptr),
    _volume_(nullptr),
    _max_volume_(nullptr)
    {
        _update_pointers();
    }

// Move constructor
Tank::Tank(Tank&& rhs) noexcept : 
    inherited(std::move(rhs)),
    _diameter_(nullptr),
    _min_volume_(nullptr),
    _volume_curve_(std::move(rhs._volume_curve_)),
    _min_level_(nullptr),
    _max_level_(nullptr),
    _initial_level_(nullptr),
    _level_(nullptr),
    _can_overflow_(nullptr),
    _initial_volume_(nullptr),
    _volume_(nullptr),
    _max_volume_(nullptr)
    {
        _update_pointers();
    }

// Copy assignment operator
Tank& Tank::operator=(const Tank& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
Tank& Tank::operator=(Tank&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        _update_pointers();
    }
    return *this;
}

Tank::~Tank() {/* Everything is deleted by the inherited destructor */}

void Tank::retrieve_properties(EN_Project ph) {
    inherited::retrieve_properties(ph);

    int errorcode;
    double val;
    errorcode = EN_getnodevalue(ph, index(), EN_DIAMETER, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving diameter for node " + id()+"\n");
    this->_diameter_->value(val);

    errorcode = EN_getnodevalue(ph, index(), EN_MINVOLUME, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving min volume for node " + id()+"\n");
    this->_min_volume_->value(val);

    errorcode = EN_getnodevalue(ph, index(), EN_MINLEVEL, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving min level for node " + id()+"\n");
    this->_min_level_->value(val);

    errorcode = EN_getnodevalue(ph, index(), EN_MAXLEVEL, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving max level for node " + id()+"\n");
    this->_max_level_->value(val);

    errorcode = EN_getnodevalue(ph, index(), EN_CANOVERFLOW, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving can overflow for node " + id()+"\n");
    this->_can_overflow_->value(val);

    errorcode = EN_getnodevalue(ph, index(), EN_TANKLEVEL, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving initial level for node " + id()+"\n");
    this->_initial_level_->value(val);
}

void Tank::retrieve_results(EN_Project ph, long t) {
    inherited::retrieve_results(ph, t);

    int errorcode;
    double val;
    errorcode = EN_getnodevalue(ph, index(), EN_TANKLEVEL, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving level for node " + id()+"\n");
    this->_level_->value().insert(std::make_pair(t, val));

    errorcode = EN_getnodevalue(ph, index(), EN_INITVOLUME, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving initial volume for node " + id()+"\n");
    this->_initial_volume_->value(val);

    errorcode = EN_getnodevalue(ph, index(), EN_TANKVOLUME, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving volume for node " + id()+"\n");
    this->_volume_->value().insert(std::make_pair(t, val));

    errorcode = EN_getnodevalue(ph, index(), EN_MAXVOLUME, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving max volume for node " + id()+"\n");
    this->_max_volume_->value(val);
}

void Tank::_add_properties() {
    inherited::_add_properties();

    properties().emplace(l__DIAMETER, vars::var_real(vars::l__m,0));
    properties().emplace(l__MIN_VOLUME, vars::var_real(vars::l__m3,0));
    properties().emplace(l__MIN_LEVEL, vars::var_real(vars::l__m,0));
    properties().emplace(l__MAX_LEVEL, vars::var_real(vars::l__m,0));
    properties().emplace(l__CAN_OVERFLOW, vars::var_int(vars::L_DIMLESS,0));
    properties().emplace(l__INITIAL_LEVEL, vars::var_real(vars::l__m,0));
}

void Tank::_add_results() {
    inherited::_add_results();

    results().emplace(l__LEVEL, vars::var_tseries_real(vars::l__m));
    results().emplace(l__INITIAL_VOLUME, vars::var_real(vars::l__m3,0));
    results().emplace(l__VOLUME, vars::var_tseries_real(vars::l__m3));
    results().emplace(l__MAX_VOLUME, vars::var_real(vars::l__m3,0));
}

void Tank::_update_pointers() {
    inherited::_update_pointers();

    _diameter_ = &std::get<vars::var_real>(properties().at(l__DIAMETER));
    _min_volume_ = &std::get<vars::var_real>(properties().at(l__MIN_VOLUME));
    _min_level_ = &std::get<vars::var_real>(properties().at(l__MIN_LEVEL));
    _max_level_ = &std::get<vars::var_real>(properties().at(l__MAX_LEVEL));
    _can_overflow_ = &std::get<vars::var_int>(properties().at(l__CAN_OVERFLOW));
    _initial_level_ = &std::get<vars::var_real>(properties().at(l__INITIAL_LEVEL));

    _level_ = &std::get<vars::var_tseries_real>(results().at(l__LEVEL));
    _initial_volume_ = &std::get<vars::var_real>(results().at(l__INITIAL_VOLUME));
    _volume_ = &std::get<vars::var_tseries_real>(results().at(l__VOLUME));
    _max_volume_ = &std::get<vars::var_real>(results().at(l__MAX_VOLUME));
}

} // namespace wds
} // namespace bevarmejo