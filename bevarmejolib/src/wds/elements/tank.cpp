#include <cassert>
#include <stdexcept>
#include <string>
#include <utility>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/wds/epanet_helpers/en_help.hpp"

#include "bevarmejo/wds/epanet_helpers/en_time_options.hpp"
#include "bevarmejo/wds/auxiliary/time_series.hpp"
#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/source.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "tank.hpp"

namespace bevarmejo {
namespace wds {

Tank::Tank(const std::string& id, const WaterDistributionSystem& wds) :
    inherited(id, wds),
    m__diameter(wds.time_series(l__CONSTANT_TS)),
    m__min_volume(wds.time_series(l__CONSTANT_TS)),
    m__min_level(wds.time_series(l__CONSTANT_TS)),
    m__max_volume(wds.time_series(l__CONSTANT_TS)),
    m__max_level(wds.time_series(l__CONSTANT_TS)),
    m__can_overflow(wds.time_series(l__CONSTANT_TS)),
    m__initial_volume(wds.time_series(l__CONSTANT_TS)),
    m__initial_level(wds.time_series(l__CONSTANT_TS)),
    m__volume(wds.time_series(l__RESULT_TS)),
    m__level(wds.time_series(l__RESULT_TS)) { }

// Copy constructor
Tank::Tank(const Tank& other) : 
    inherited(other),
    m__diameter(other.m__diameter),
    m__min_volume(other.m__min_volume),
    m__min_level(other.m__min_level),
    m__max_volume(other.m__max_volume),
    m__max_level(other.m__max_level),
    m__can_overflow(other.m__can_overflow),
    m__initial_volume(other.m__initial_volume),
    m__initial_level(other.m__initial_level),
    m__volume(other.m__volume),
    m__level(other.m__level) { }

// Move constructor
Tank::Tank(Tank&& rhs) noexcept : 
    inherited(std::move(rhs)),
    m__diameter(std::move(rhs.m__diameter)),
    m__min_volume(std::move(rhs.m__min_volume)),
    m__min_level(std::move(rhs.m__min_level)),
    m__max_volume(std::move(rhs.m__max_volume)),
    m__max_level(std::move(rhs.m__max_level)),
    m__can_overflow(std::move(rhs.m__can_overflow)),
    m__initial_volume(std::move(rhs.m__initial_volume)),
    m__initial_level(std::move(rhs.m__initial_level)),
    m__volume(std::move(rhs.m__volume)),
    m__level(std::move(rhs.m__level)) { }

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
    m__diameter= val;

    errorcode = EN_getnodevalue(ph, index(), EN_MINVOLUME, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving min volume for node " + id()+"\n");

    if (ph->parser.Unitsflag == US)
        val *= M3perFT3;
    m__min_volume= val;

    errorcode = EN_getnodevalue(ph, index(), EN_MINLEVEL, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving min level for node " + id()+"\n");

    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    m__min_level= val;

    errorcode = EN_getnodevalue(ph, index(), EN_MAXLEVEL, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving max level for node " + id()+"\n");

    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    m__max_level= val;

    errorcode = EN_getnodevalue(ph, index(), EN_CANOVERFLOW, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving can overflow for node " + id()+"\n");

    // DIMLESS
    m__can_overflow= val;

    errorcode = EN_getnodevalue(ph, index(), EN_TANKLEVEL, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving initial level for node " + id()+"\n");

    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    m__initial_level= val;

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
    m__initial_volume= val;

    errorcode = EN_getnodevalue(ph, index(), EN_MAXVOLUME, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving max volume for node " + id()+"\n");

    if (ph->parser.Unitsflag == US)
        val *= M3perFT3;
    m__max_volume= val;
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
    m__level.commit(t, val);

    errorcode = EN_getnodevalue(ph, index(), EN_TANKVOLUME, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving volume for node " + id()+"\n");

    if (ph->parser.Unitsflag == US)
        val *= M3perFT3;
    m__volume.commit(t, val);
}

void Tank::clear_results() {
    inherited::clear_results();

    m__level.clear();
    m__volume.clear();
}

} // namespace wds
} // namespace bevarmejo