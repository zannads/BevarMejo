#include <cassert>
#include <string>
#include <unordered_map>
#include <variant>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/wds/data_structures/temporal.hpp"
#include "bevarmejo/wds/data_structures/variable.hpp"

#include "bevarmejo/wds/auxiliary/time_series.hpp"
#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/link.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "dimensioned_link.hpp"

namespace bevarmejo {
namespace wds {

DimensionedLink::DimensionedLink(const std::string& id, const WaterDistributionSystem& wds) : 
    inherited(id, wds),
    _diameter_(nullptr),
    _roughness_(nullptr),
    _minor_loss_(nullptr),
    _bulk_coeff_(nullptr),
    _wall_coeff_(nullptr),
    _velocity_(nullptr),
    m__diameter(wds.time_series(l__CONSTANT_TS)),
    m__roughness(wds.time_series(l__CONSTANT_TS)),
    m__minor_loss(wds.time_series(l__CONSTANT_TS)),
    m__bulk_coeff(wds.time_series(l__CONSTANT_TS)),
    m__wall_coeff(wds.time_series(l__CONSTANT_TS)),
    m__velocity(wds.time_series(l__RESULT_TS))
    {
        _add_properties();
        _add_results();
        _update_pointers();
    }

// Copy constructor
DimensionedLink::DimensionedLink(const DimensionedLink& other) : 
    inherited(other),
    _diameter_(nullptr),
    _roughness_(nullptr),
    _minor_loss_(nullptr),
    _bulk_coeff_(nullptr),
    _wall_coeff_(nullptr),
    _velocity_(nullptr),
    m__diameter(other.m__diameter),
    m__roughness(other.m__roughness),
    m__minor_loss(other.m__minor_loss),
    m__bulk_coeff(other.m__bulk_coeff),
    m__wall_coeff(other.m__wall_coeff),
    m__velocity(other.m__velocity)
    {
        _update_pointers();
    }

// Move constructor
DimensionedLink::DimensionedLink(DimensionedLink&& rhs) noexcept : 
    inherited(std::move(rhs)),
    _diameter_(nullptr),
    _roughness_(nullptr),
    _minor_loss_(nullptr),
    _bulk_coeff_(nullptr),
    _wall_coeff_(nullptr),
    _velocity_(nullptr),
    m__diameter(std::move(rhs.m__diameter)),
    m__roughness(std::move(rhs.m__roughness)),
    m__minor_loss(std::move(rhs.m__minor_loss)),
    m__bulk_coeff(std::move(rhs.m__bulk_coeff)),
    m__wall_coeff(std::move(rhs.m__wall_coeff)),
    m__velocity(std::move(rhs.m__velocity))
    {
        _update_pointers();
    }

// Copy assignment operator
DimensionedLink& DimensionedLink::operator=(const DimensionedLink& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        m__diameter = rhs.m__diameter;
        m__roughness = rhs.m__roughness;
        m__minor_loss = rhs.m__minor_loss;
        m__bulk_coeff = rhs.m__bulk_coeff;
        m__wall_coeff = rhs.m__wall_coeff;
        m__velocity = rhs.m__velocity;

        _update_pointers();
    }
    return *this;
}

// Move assignment operator
DimensionedLink& DimensionedLink::operator=(DimensionedLink&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        m__diameter = std::move(rhs.m__diameter);
        m__roughness = std::move(rhs.m__roughness);
        m__minor_loss = std::move(rhs.m__minor_loss);
        m__bulk_coeff = std::move(rhs.m__bulk_coeff);
        m__wall_coeff = std::move(rhs.m__wall_coeff);
        m__velocity = std::move(rhs.m__velocity);

        _update_pointers();
    }
    return *this;
}

void DimensionedLink::__retrieve_EN_properties(EN_Project ph) {
    inherited::__retrieve_EN_properties(ph);
    
    assert(index()!=0);

    int errorcode = 0;
    double val = 0;

    errorcode = EN_getlinkvalue(ph, index(), EN_DIAMETER, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving the diameter of link "+id()+" from EPANET project.");

    if (ph->parser.Unitsflag == US)
        val = val/12*MperFT*1000; // from inches to ft, then to m, finally to mm
    _diameter_->value(val);

    errorcode = EN_getlinkvalue(ph, index(), EN_ROUGHNESS, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving the roughness of link "+id()+" from EPANET project.");

    // for now only HW coeff is supported
    _roughness_->value(val);

    errorcode = EN_getlinkvalue(ph, index(), EN_MINORLOSS, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving the minor loss of link "+id()+" from EPANET project.");
    
    // DIMLESS
    _minor_loss_->value(val);

    errorcode = EN_getlinkvalue(ph, index(), EN_KBULK, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving the bulk coefficient of link "+id()+" from EPANET project.");
    
    // for now I don't care about this
    _bulk_coeff_->value(val);

    errorcode = EN_getlinkvalue(ph, index(), EN_KWALL, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving the wall coefficient of link "+id()+" from EPANET project.");
    
    // for now I don't care about this
    _wall_coeff_->value(val);
}

void DimensionedLink::retrieve_results(EN_Project ph, long t) {
    inherited::retrieve_results(ph, t);

    int errorcode = 0;  
    double val = 0;
    errorcode = EN_getlinkvalue(ph, index(), EN_VELOCITY, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving the velocity of link "+id()+" from EPANET project.");

    // Before saving I need to conver it to m/s
    if (ph->parser.Unitsflag == US)
        val = val*MperFT; // from ft/s to m/s

    this->_velocity_->value().insert(std::make_pair(t, val));
}

void DimensionedLink::_add_properties() {
    inherited::_add_properties();

    properties().emplace(L_DIAMETER, vars::var_real(vars::l__mm, kmin_diameter));
    properties().emplace(L_ROUGHNESS, vars::var_real("?", 0));
    properties().emplace(L_MINOR_LOSS, vars::var_real(vars::l__DIMLESS, 0));
    properties().emplace(L_BULK_COEFF, vars::var_real(vars::l__DIMLESS, 0));
    properties().emplace(L_WALL_COEFF, vars::var_real(vars::l__DIMLESS, 0));
}

void DimensionedLink::_add_results() {
    inherited::_add_results();

    results().emplace(L_VELOCITY, vars::var_tseries_real("m/s"));
}

void DimensionedLink::_update_pointers() {
    inherited::_update_pointers();

    _diameter_ = &std::get<vars::var_real>(properties().at(L_DIAMETER));
    _roughness_ = &std::get<vars::var_real>(properties().at(L_ROUGHNESS));
    _minor_loss_ = &std::get<vars::var_real>(properties().at(L_MINOR_LOSS));
    _bulk_coeff_ = &std::get<vars::var_real>(properties().at(L_BULK_COEFF));
    _wall_coeff_ = &std::get<vars::var_real>(properties().at(L_WALL_COEFF));

    _velocity_ = &std::get<vars::var_tseries_real>(results().at(L_VELOCITY));
}

} // namespace wds
} // namespace bevarmejo
