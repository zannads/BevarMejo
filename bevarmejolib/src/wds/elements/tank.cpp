#include <cassert>
#include <memory>
#include <string>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/wds/epanet_helpers/en_help.hpp"

#include "bevarmejo/wds/auxiliary/curve.hpp"
#include "bevarmejo/wds/auxiliary/curves.hpp"
#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/source.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "tank.hpp"

namespace bevarmejo::wds
{

/*------- Member functions -------*/
// (constructor)
Tank::Tank(const WaterDistributionSystem& wds, const EN_Name_t& name) :
    inherited(wds, name),
    m__diameter(wds.time_series(label::__CONSTANT_TS)),
    m__volume_curve(nullptr),
    m__min_volume(wds.time_series(label::__CONSTANT_TS)),
    m__min_level(wds.time_series(label::__CONSTANT_TS)),
    m__max_level(wds.time_series(label::__CONSTANT_TS)),
    m__can_overflow(wds.time_series(label::__CONSTANT_TS)),
    m__initial_level(wds.time_series(label::__CONSTANT_TS)),
    m__initial_volume(wds.time_series(label::__CONSTANT_TS)),
    m__max_volume(wds.time_series(label::__CONSTANT_TS)),
    m__level(wds.time_series(label::__RESULTS_TS)),
    m__volume(wds.time_series(label::__RESULTS_TS))
{ }

/*------- Operators -------*/

/*------- Element access -------*/
const char* Tank::type_name() const
{
    return self_traits::name;
}

unsigned int Tank::type_code() const
{
    return self_traits::code;
}

/*------- Capacity -------*/

/*------- Modifiers -------*/
void Tank::clear_results()
{
    inherited::clear_results();

    m__level.clear();
    m__volume.clear();
}

void Tank::retrieve_EN_properties()
{
    this->__retrieve_EN_properties();

    inherited::retrieve_EN_properties();
}

void Tank::__retrieve_EN_properties()
{
    assert(m__en_index > 0);
    assert(m__wds.ph() != nullptr);

    auto ph = m__wds.ph();

    double val = 0.0;
    int errorcode = EN_getnodevalue(ph, m__en_index, EN_DIAMETER, &val);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the properties of the tank.",
        "Error while retrieving value: EN_DIAMETER",
        "Error code: ", errorcode,
        "Tank ID: ", m__name);

    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    m__diameter = val;

    errorcode = EN_getnodevalue(ph, m__en_index, EN_MINVOLUME, &val);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the properties of the tank.",
        "Error while retrieving value: EN_MINVOLUME",
        "Error code: ", errorcode,
        "Tank ID: ", m__name);

    if (ph->parser.Unitsflag == US)
        val *= M3perFT3;
    m__min_volume = val;

    errorcode = EN_getnodevalue(ph, m__en_index, EN_MINLEVEL, &val);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the properties of the tank.",
        "Error while retrieving value: EN_MINLEVEL",
        "Error code: ", errorcode,
        "Tank ID: ", m__name);

    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    m__min_level = val;

    errorcode = EN_getnodevalue(ph, m__en_index, EN_MAXLEVEL, &val);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the properties of the tank.",
        "Error while retrieving value: EN_MAXLEVEL",
        "Error code: ", errorcode,
        "Tank ID: ", m__name);

    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    m__max_level = val;

    errorcode = EN_getnodevalue(ph, m__en_index, EN_CANOVERFLOW, &val);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the properties of the tank.",
        "Error while retrieving value: EN_CANOVERFLOW",
        "Error code: ", errorcode,
        "Tank ID: ", m__name);

    // DIMLESS
    m__can_overflow = val;

    errorcode = EN_getnodevalue(ph, m__en_index, EN_TANKLEVEL, &val);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the properties of the tank.",
        "Error while retrieving value: EN_TANKLEVEL",
        "Error code: ", errorcode,
        "Tank ID: ", m__name);

    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    m__initial_level = val;

    // Assign EN curves 
    errorcode = EN_getnodevalue(ph, m__en_index, EN_VOLCURVE, &val);
    assert(errorcode <= 100);

    if (val != 0.0) {
        char curve_id[EN_MAXID+1];
        errorcode = EN_getcurveid(ph, static_cast<int>(val), curve_id);
        assert(errorcode <= 100);

        // EPANET says that this curve with this ID should be a volume curve.
        // If this assertion fails it means there are some inconsistencies in the upload from EPANET.
        m__volume_curve = m__wds.curves().get<VolumeCurve>(curve_id);
    }
    else m__volume_curve = nullptr;
    

    // Assign Read-only properties (Results but not really).
    errorcode = EN_getnodevalue(ph, m__en_index, EN_INITVOLUME, &val);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the properties of the tank.",
        "Error while retrieving value: EN_INITVOLUME",
        "Error code: ", errorcode,
        "Tank ID: ", m__name);

    if (ph->parser.Unitsflag == US)
        val *= M3perFT3;
    m__initial_volume = val;

    errorcode = EN_getnodevalue(ph, m__en_index, EN_MAXVOLUME, &val);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the properties of the tank.",
        "Error while retrieving value: EN_MAXVOLUME",
        "Error code: ", errorcode,
        "Tank ID: ", m__name);

    if (ph->parser.Unitsflag == US)
        val *= M3perFT3;
    m__max_volume = val;
}

void Tank::retrieve_EN_results()
{
    this->__retrieve_EN_results();

    inherited::retrieve_EN_results();
}

void Tank::__retrieve_EN_results()
{
    assert(m__en_index > 0);
    assert(m__wds.ph() != nullptr);

    auto ph = m__wds.ph();
    auto t = m__wds.current_result_time();

    double val;
    int errorcode = EN_getnodevalue(ph, m__en_index, EN_TANKLEVEL, &val);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the results of the tank.",
        "Error while retrieving value: EN_TANKLEVEL",
        "Error code: ", errorcode,
        "Tank ID: ", m__name);

    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    m__level.commit(t, val);

    errorcode = EN_getnodevalue(ph, m__en_index, EN_TANKVOLUME, &val);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the results of the tank.",
        "Error while retrieving value: EN_TANKVOLUME",
        "Error code: ", errorcode,
        "Tank ID: ", m__name);

    if (ph->parser.Unitsflag == US)
        val *= M3perFT3;
    m__volume.commit(t, val);
}

} // namespace bevarmejo::wds
