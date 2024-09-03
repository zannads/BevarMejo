#include <cassert>
#include <string>
#include <stdexcept>
#include <utility>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/wds/epanet_helpers/en_help.hpp"

#include "bevarmejo/wds/epanet_helpers/en_time_options.hpp"
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
    m__diameter(wds.time_series(l__CONSTANT_TS)),
    m__roughness(wds.time_series(l__CONSTANT_TS)),
    m__minor_loss(wds.time_series(l__CONSTANT_TS)),
    m__bulk_coeff(wds.time_series(l__CONSTANT_TS)),
    m__wall_coeff(wds.time_series(l__CONSTANT_TS)),
    m__velocity(wds.time_series(l__RESULT_TS)) { }

// Copy constructor
DimensionedLink::DimensionedLink(const DimensionedLink& other) : 
    inherited(other),
    m__diameter(other.m__diameter),
    m__roughness(other.m__roughness),
    m__minor_loss(other.m__minor_loss),
    m__bulk_coeff(other.m__bulk_coeff),
    m__wall_coeff(other.m__wall_coeff),
    m__velocity(other.m__velocity) { }

// Move constructor
DimensionedLink::DimensionedLink(DimensionedLink&& rhs) noexcept : 
    inherited(std::move(rhs)),
    m__diameter(std::move(rhs.m__diameter)),
    m__roughness(std::move(rhs.m__roughness)),
    m__minor_loss(std::move(rhs.m__minor_loss)),
    m__bulk_coeff(std::move(rhs.m__bulk_coeff)),
    m__wall_coeff(std::move(rhs.m__wall_coeff)),
    m__velocity(std::move(rhs.m__velocity)) { }

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
    m__diameter= val;

    errorcode = EN_getlinkvalue(ph, index(), EN_ROUGHNESS, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving the roughness of link "+id()+" from EPANET project.");

    // for now only HW coeff is supported
    m__roughness= val;

    errorcode = EN_getlinkvalue(ph, index(), EN_MINORLOSS, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving the minor loss of link "+id()+" from EPANET project.");
    
    // DIMLESS
    m__minor_loss= val;

    errorcode = EN_getlinkvalue(ph, index(), EN_KBULK, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving the bulk coefficient of link "+id()+" from EPANET project.");
    
    // for now I don't care about this
    m__bulk_coeff= val;

    errorcode = EN_getlinkvalue(ph, index(), EN_KWALL, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving the wall coefficient of link "+id()+" from EPANET project.");
    
    // for now I don't care about this
    m__wall_coeff= val;
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

    m__velocity.commit(t, val);
}

void DimensionedLink::clear_results() {
    inherited::clear_results();

    m__velocity.clear();
}

} // namespace wds
} // namespace bevarmejo
