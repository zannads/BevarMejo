#include <cassert>
#include <memory>
#include <string>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/wds/utility/epanet/en_help.hpp"

#include "bevarmejo/wds/elements/curve.hpp"
#include "bevarmejo/wds/elements/curves.hpp"
#include "bevarmejo/wds/utility/quantity_series.hpp"

#include "bevarmejo/wds/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/network_elements/nodes/source.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "bevarmejo/wds/elements/network_elements/nodes/tank.hpp"

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

// (EPANET constructor)
auto Tank::make_from_EN_for(const WaterDistributionSystem& wds, const EN_Name_t& name) -> std::unique_ptr<Tank>
{
    auto p_tank = std::make_unique<Tank>(wds, name);
    p_tank->retrieve_EN_index();
    p_tank->retrieve_EN_properties();

    return std::move(p_tank);
}

/*------- Operators -------*/

auto Tank::volume(
    double a_level
) const -> aux::QuantitySeries<double>
{
    beme_throw_if(
        a_level < m__min_level.value() || a_level > m__max_level.value(),
        std::out_of_range,
        "Can not compute the tank volume.",
        "The level is out of range.",
        "Level: ", std::to_string(a_level),
        "Min level: ", std::to_string(m__min_level.value()),
        "Max level: ", std::to_string(m__max_level.value())
    );

    if (m__volume_curve)
        assertm(false, "Volume curve not implemented yet");
    
    // Else, let's use the cylindrical tank assumption.
    // as in EPANET; the min_level is not related to the min volume...
    auto delta_l = a_level - m__min_level.value();
    auto area = m__diameter.value() * m__diameter.value() / 4.0 * k__pi;
    
    return aux::QuantitySeries<double>(m__wds.time_series(label::__CONSTANT_TS),
        delta_l*area + m__min_volume.value());
}

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
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the properties of the tank.",
        "Error originating from the EPANET API while retrieving value: EN_DIAMETER",
        "Tank ID: ", m__name);

    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    m__diameter = val;

    errorcode = EN_getnodevalue(ph, m__en_index, EN_MINVOLUME, &val);
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the properties of the tank.",
        "Error originating from the EPANET API while retrieving value: EN_MINVOLUME",
        "Tank ID: ", m__name);

    if (ph->parser.Unitsflag == US)
        val *= M3perFT3;
    m__min_volume = val;

    errorcode = EN_getnodevalue(ph, m__en_index, EN_MINLEVEL, &val);
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the properties of the tank.",
        "Error originating from the EPANET API while retrieving value: EN_MINLEVEL",
        "Tank ID: ", m__name);

    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    m__min_level = val;

    errorcode = EN_getnodevalue(ph, m__en_index, EN_MAXLEVEL, &val);
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the properties of the tank.",
        "Error originating from the EPANET API while retrieving value: EN_MAXLEVEL",
        "Tank ID: ", m__name);

    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    m__max_level = val;

    errorcode = EN_getnodevalue(ph, m__en_index, EN_CANOVERFLOW, &val);
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the properties of the tank.",
        "Error originating from the EPANET API while retrieving value: EN_CANOVERFLOW",
        "Tank ID: ", m__name);

    // DIMLESS
    m__can_overflow = val;

    errorcode = EN_getnodevalue(ph, m__en_index, EN_TANKLEVEL, &val);
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the properties of the tank.",
        "Error originating from the EPANET API while retrieving value: EN_TANKLEVEL",
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
        m__volume_curve = m__wds.get_curve<VolumeCurve>(curve_id);
    }
    else m__volume_curve = nullptr;
    

    // Assign Read-only properties (Results but not really).
    errorcode = EN_getnodevalue(ph, m__en_index, EN_INITVOLUME, &val);
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the properties of the tank.",
        "Error originating from the EPANET API while retrieving value: EN_INITVOLUME",
        "Tank ID: ", m__name);

    if (ph->parser.Unitsflag == US)
        val *= M3perFT3;
    m__initial_volume = val;

    errorcode = EN_getnodevalue(ph, m__en_index, EN_MAXVOLUME, &val);
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the properties of the tank.",
        "Error originating from the EPANET API while retrieving value: EN_MAXVOLUME",
        "Tank ID: ", m__name);

    if (ph->parser.Unitsflag == US)
        val *= M3perFT3;
    m__max_volume = val;
}

void Tank::retrieve_EN_results()
{
    inherited::retrieve_EN_results();

    this->__retrieve_EN_results();
}

void Tank::__retrieve_EN_results()
{
    assert(m__en_index > 0);
    assert(m__wds.ph() != nullptr);

    auto ph = m__wds.ph();
    auto t = m__wds.current_result_time();

    double val;
    int errorcode = EN_getnodevalue(ph, m__en_index, EN_TANKLEVEL, &val);
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the results of the tank.",
        "Error originating from the EPANET API while retrieving value: EN_TANKLEVEL",
        "Tank ID: ", m__name);

    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    m__level.commit(t, val);

    errorcode = EN_getnodevalue(ph, m__en_index, EN_TANKVOLUME, &val);
    beme_throw_if_EN_error(errorcode,
        "Impossible to retrieve the results of the tank.",
        "Error originating from the EPANET API while retrieving value: EN_TANKVOLUME",
        "Tank ID: ", m__name);

    if (ph->parser.Unitsflag == US)
        val *= M3perFT3;
    m__volume.commit(t, val);
}

} // namespace bevarmejo::wds
