#include <cassert>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "epanet2_2.h"

#include "bevarmejo/wds/data_structures/temporal.hpp"
#include "bevarmejo/wds/data_structures/variable.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/source.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "tank.hpp"

namespace bevarmejo {
namespace wds {

Tank::Tank(const std::string& id, const WaterDistributionSystem& wds) :
    inherited(id, wds),
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
    _max_volume_(nullptr),
    m__diameter(wds.time_series(l__CONSTANT_TS)),
    m__min_volume(wds.time_series(l__CONSTANT_TS)),
    m__min_level(wds.time_series(l__CONSTANT_TS)),
    m__max_volume(wds.time_series(l__CONSTANT_TS)),
    m__max_level(wds.time_series(l__CONSTANT_TS)),
    m__can_overflow(wds.time_series(l__CONSTANT_TS)),
    m__initial_volume(wds.time_series(l__CONSTANT_TS)),
    m__initial_level(wds.time_series(l__CONSTANT_TS)),
    m__volume(wds.time_series(l__RESULT_TS)),
    m__level(wds.time_series(l__RESULT_TS))
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
    _max_volume_(nullptr),
    m__diameter(other.m__diameter),
    m__min_volume(other.m__min_volume),
    m__min_level(other.m__min_level),
    m__max_volume(other.m__max_volume),
    m__max_level(other.m__max_level),
    m__can_overflow(other.m__can_overflow),
    m__initial_volume(other.m__initial_volume),
    m__initial_level(other.m__initial_level),
    m__volume(other.m__volume),
    m__level(other.m__level)
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
    _max_volume_(nullptr),
    m__diameter(std::move(rhs.m__diameter)),
    m__min_volume(std::move(rhs.m__min_volume)),
    m__min_level(std::move(rhs.m__min_level)),
    m__max_volume(std::move(rhs.m__max_volume)),
    m__max_level(std::move(rhs.m__max_level)),
    m__can_overflow(std::move(rhs.m__can_overflow)),
    m__initial_volume(std::move(rhs.m__initial_volume)),
    m__initial_level(std::move(rhs.m__initial_level)),
    m__volume(std::move(rhs.m__volume)),
    m__level(std::move(rhs.m__level))
    {
        _update_pointers();
    }

// Copy assignment operator
Tank& Tank::operator=(const Tank& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        m__diameter = rhs.m__diameter;
        m__min_volume = rhs.m__min_volume;
        m__min_level = rhs.m__min_level;
        m__max_volume = rhs.m__max_volume;
        m__max_level = rhs.m__max_level;
        m__can_overflow = rhs.m__can_overflow;
        m__initial_volume = rhs.m__initial_volume;
        m__initial_level = rhs.m__initial_level;
        m__volume = rhs.m__volume;
        m__level = rhs.m__level;
        
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
Tank& Tank::operator=(Tank&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        m__diameter = std::move(rhs.m__diameter);
        m__min_volume = std::move(rhs.m__min_volume);
        m__min_level = std::move(rhs.m__min_level);
        m__max_volume = std::move(rhs.m__max_volume);
        m__max_level = std::move(rhs.m__max_level);
        m__can_overflow = std::move(rhs.m__can_overflow);
        m__initial_volume = std::move(rhs.m__initial_volume);
        m__initial_level = std::move(rhs.m__initial_level);
        m__volume = std::move(rhs.m__volume);
        m__level = std::move(rhs.m__level);

        _update_pointers();
    }
    return *this;
}


void Tank::__retrieve_EN_properties(EN_Project ph) {
    inherited::__retrieve_EN_properties(ph);

    double val= 0.0;
    int errorcode = EN_getnodevalue(ph, index(), EN_DIAMETER, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving diameter for node " + id()+"\n");

    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    this->_diameter_->value(val);
    m__diameter.value()= val;

    errorcode = EN_getnodevalue(ph, index(), EN_MINVOLUME, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving min volume for node " + id()+"\n");

    if (ph->parser.Unitsflag == US)
        val *= M3perFT3;
    this->_min_volume_->value(val);
    m__min_volume.value()= val;

    errorcode = EN_getnodevalue(ph, index(), EN_MINLEVEL, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving min level for node " + id()+"\n");

    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    this->_min_level_->value(val);
    m__min_level.value()= val;

    errorcode = EN_getnodevalue(ph, index(), EN_MAXLEVEL, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving max level for node " + id()+"\n");

    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    this->_max_level_->value(val);
    m__max_level.value()= val;

    errorcode = EN_getnodevalue(ph, index(), EN_CANOVERFLOW, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving can overflow for node " + id()+"\n");

    // DIMLESS
    this->_can_overflow_->value(val);
    m__can_overflow.value()= val;

    errorcode = EN_getnodevalue(ph, index(), EN_TANKLEVEL, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving initial level for node " + id()+"\n");

    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    this->_initial_level_->value(val);
    m__initial_level.value()= val;

    { // Assign EN curves 
        auto curves= m__wds.curves();
        errorcode = EN_getnodevalue(ph, this->index(), EN_VOLCURVE, &val);
        assert(errorcode <= 100);

        if (val != 0.0) {
            char curve_id[EN_MAXID+1];
            errorcode = EN_getcurveid(ph, static_cast<int>(val), curve_id);
            assert(errorcode <= 100);

            auto it = curves.find(curve_id);
            assert(it != curves.end());

            std::shared_ptr<VolumeCurve> volume_curve = std::dynamic_pointer_cast<VolumeCurve>(*it);
            assert(volume_curve != nullptr);
            // EPANET says that this curve with this ID should be a volume curve.
            // If this assertion fails it means there are some inconsistencies in the upload from EPANET.
            this->volume_curve(volume_curve);
        }
        else this->volume_curve(nullptr);
    }

    // Assign Read-only properties (Results but not really).
    errorcode = EN_getnodevalue(ph, index(), EN_INITVOLUME, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving initial volume for node " + id()+"\n");

    if (ph->parser.Unitsflag == US)
        val *= M3perFT3;
    this->_initial_volume_->value(val);
    m__initial_volume.value()= val;

    errorcode = EN_getnodevalue(ph, index(), EN_MAXVOLUME, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving max volume for node " + id()+"\n");

    if (ph->parser.Unitsflag == US)
        val *= M3perFT3;
    this->_max_volume_->value(val);
    m__max_volume.value()= val;
}

void Tank::retrieve_results(EN_Project ph, long t) {
    inherited::retrieve_results(ph, t);

    int errorcode;
    double val;

    errorcode = EN_getnodevalue(ph, index(), EN_TANKLEVEL, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving level for node " + id()+"\n");

    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    this->_level_->value().insert(std::make_pair(t, val));

    errorcode = EN_getnodevalue(ph, index(), EN_TANKVOLUME, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving volume for node " + id()+"\n");

    if (ph->parser.Unitsflag == US)
        val *= M3perFT3;
    this->_volume_->value().insert(std::make_pair(t, val));
}

void Tank::_add_properties() {
    inherited::_add_properties();

    properties().emplace(l__DIAMETER, vars::var_real(vars::l__m,0));
    properties().emplace(l__MIN_VOLUME, vars::var_real(vars::l__m3,0));
    properties().emplace(l__MIN_LEVEL, vars::var_real(vars::l__m,0));
    properties().emplace(l__MAX_LEVEL, vars::var_real(vars::l__m,0));
    properties().emplace(l__CAN_OVERFLOW, vars::var_int(vars::l__DIMLESS,0));
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