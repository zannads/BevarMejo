#include <cassert>
#include <string>
#include <unordered_map>
#include <variant>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/variable.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/link.hpp"

#include "dimensioned_link.hpp"

namespace bevarmejo {
namespace wds {

dimensioned_link::dimensioned_link(const std::string& id) : 
    inherited(id),
    _diameter_(nullptr),
    _roughness_(nullptr),
    _minor_loss_(nullptr),
    _bulk_coeff_(nullptr),
    _wall_coeff_(nullptr),
    _velocity_(nullptr)
    {
        _add_properties();
        _add_results();
        _update_pointers();
    }

// Copy constructor
dimensioned_link::dimensioned_link(const dimensioned_link& other) : 
    inherited(other),
    _diameter_(nullptr),
    _roughness_(nullptr),
    _minor_loss_(nullptr),
    _bulk_coeff_(nullptr),
    _wall_coeff_(nullptr),
    _velocity_(nullptr)
    {
        _update_pointers();
    }

// Move constructor
dimensioned_link::dimensioned_link(dimensioned_link&& rhs) noexcept : 
    inherited(std::move(rhs)),
    _diameter_(nullptr),
    _roughness_(nullptr),
    _minor_loss_(nullptr),
    _bulk_coeff_(nullptr),
    _wall_coeff_(nullptr),
    _velocity_(nullptr)
    {
        _update_pointers();
    }

// Copy assignment operator
dimensioned_link& dimensioned_link::operator=(const dimensioned_link& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
dimensioned_link& dimensioned_link::operator=(dimensioned_link&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        _update_pointers();
    }
    return *this;
}

dimensioned_link::~dimensioned_link() {/* results are cleared when the inherited destructor is called*/ }

void dimensioned_link::retrieve_properties(EN_Project ph) {
    inherited::retrieve_properties(ph);
    
    assert(index()!=0);

    int errorcode = 0;
    double val = 0;

    errorcode = EN_getlinkvalue(ph, index(), EN_DIAMETER, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving the diameter of link "+id()+" from EPANET project.");
    _diameter_->value(val);

    errorcode = EN_getlinkvalue(ph, index(), EN_ROUGHNESS, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving the roughness of link "+id()+" from EPANET project.");
    _roughness_->value(val);

    errorcode = EN_getlinkvalue(ph, index(), EN_MINORLOSS, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving the minor loss of link "+id()+" from EPANET project.");
    _minor_loss_->value(val);

    errorcode = EN_getlinkvalue(ph, index(), EN_KBULK, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving the bulk coefficient of link "+id()+" from EPANET project.");
    _bulk_coeff_->value(val);

    errorcode = EN_getlinkvalue(ph, index(), EN_KWALL, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving the wall coefficient of link "+id()+" from EPANET project.");
    _wall_coeff_->value(val);
}

void dimensioned_link::retrieve_results(EN_Project ph, long t) {
    inherited::retrieve_results(ph, t);

    int errorcode = 0;  
    double d_velocity = 0;
    errorcode = EN_getlinkvalue(ph, index(), EN_VELOCITY, &d_velocity);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving the velocity of link "+id()+" from EPANET project.");
    this->_velocity_->value().insert(std::make_pair(t, d_velocity));
}

void dimensioned_link::_add_properties() {
    inherited::_add_properties();

    properties().emplace(L_DIAMETER, vars::var_real(vars::L_METER, kmin_diameter));
    properties().emplace(L_ROUGHNESS, vars::var_real("?", 0));
    properties().emplace(L_MINOR_LOSS, vars::var_real(vars::L_DIMLESS, 0));
    properties().emplace(L_BULK_COEFF, vars::var_real(vars::L_DIMLESS, 0));
    properties().emplace(L_WALL_COEFF, vars::var_real(vars::L_DIMLESS, 0));
}

void dimensioned_link::_add_results() {
    inherited::_add_results();

    results().emplace(L_VELOCITY, vars::var_tseries_real("m/s"));
}

void dimensioned_link::_update_pointers() {
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
