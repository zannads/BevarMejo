#include <cassert>
#include <memory>
#include <string>

#include "epanet2_2.h"

#include "bevarmejo/wds/epanet_helpers/en_help.hpp"

#include "bevarmejo/utility/bemexcept.hpp"

#include "bevarmejo/wds/auxiliary/pattern.hpp"
#include "bevarmejo/wds/auxiliary/curve.hpp"
#include "bevarmejo/wds/auxiliary/curves.hpp"
#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/link.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "pump.hpp"

namespace bevarmejo::wds
{

/*------- Member functions -------*/
// (constructor)

Pump::Pump(const WaterDistributionSystem& wds, const EN_Name_t& name) :
    inherited(wds, name),
    m__init_setting(wds.time_series(label::__CONSTANT_TS)),
    m__power_rating(wds.time_series(label::__CONSTANT_TS)),
    m__energy_cost(wds.time_series(label::__CONSTANT_TS)),
    m__speed_pattern(nullptr),
    m__energy_cost_pattern(nullptr),
    m__pump_curve(nullptr),
    m__efficiency_curve(nullptr),
    m__instant_energy(wds.time_series(label::__RESULTS_TS)),
    m__state(wds.time_series(label::__RESULTS_TS)),
    m__efficiency(wds.time_series(label::__RESULTS_TS))
{ }

// (EPANET constructor)
auto Pump::make_from_EN_for(const WaterDistributionSystem& wds, const EN_Name_t& name) -> std::unique_ptr<Pump>
{
    auto p_pump = std::make_unique<Pump>(wds, name);
    p_pump->retrieve_EN_index();
    p_pump->retrieve_EN_properties();

    return std::move(p_pump);
}

/*------- Operators -------*/

/*------- Element access -------*/
const char* Pump::type_name() const
{
    return self_traits::name;
}

unsigned int Pump::type_code() const
{
    return self_traits::code;
}

/*------- Capacity -------*/

/*------- Modifiers -------*/
void Pump::clear_results()
{
    inherited::clear_results();

    m__instant_energy.clear();
    m__state.clear();
    m__efficiency.clear();
}

void Pump::retrieve_EN_properties()
{
    this->__retrieve_EN_properties();

    inherited::retrieve_EN_properties();
}

void Pump::__retrieve_EN_properties()
{
    assert(m__en_index > 0);
    assert(m__wds.ph() != nullptr);

    auto ph = m__wds.ph();

    double val = 0.0;
    int errorcode = EN_getlinkvalue(ph, m__en_index, EN_INITSETTING, &val);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the properties of the pump.",
        "Error while retrieving value: EN_INITSETTING",
        "Error code: ", errorcode,
        "Pump ID: ", m__name);

    m__init_setting = val;

    errorcode = EN_getlinkvalue(ph, m__en_index, EN_PUMP_POWER, &val);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the properties of the pump.",
        "Error while retrieving value: EN_PUMP_POWER",
        "Error code: ", errorcode,
        "Pump ID: ", m__name);

    m__power_rating = val;

    errorcode = EN_getlinkvalue(ph, m__en_index, EN_PUMP_ECOST, &val);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the properties of the pump.",
        "Error while retrieving value: EN_PUMP_ECOST",
        "Error code: ", errorcode,
        "Pump ID: ", m__name);

    m__energy_cost = val;

    // Assign EN Pattern
    errorcode = EN_getlinkvalue(ph, this->m__en_index, EN_LINKPATTERN, &val);
    assert(errorcode < 100);
    char pattern_id[EN_MAXID+1];
    errorcode = EN_getpatternid(ph, static_cast<int>(val), pattern_id);
    assert(errorcode < 100);
    
    m__speed_pattern = m__wds.patterns().get(pattern_id);
    
    // Assign EN Curves (Pump curve and Efficiency curve)
    errorcode= EN_getlinkvalue(ph, this->m__en_index, EN_PUMP_HCURVE, &val);
    assert(errorcode < 100);
    
    if (val == 0.0)
        m__pump_curve = nullptr;
    else
    {
        char curve_id[EN_MAXID+1];
        errorcode = EN_getcurveid(ph, static_cast<int>(val), curve_id);
        assert(errorcode < 100);

        m__pump_curve = m__wds.curves().get<PumpCurve>(curve_id);
    }

    errorcode = EN_getlinkvalue(ph, this->m__en_index, EN_PUMP_ECURVE, &val);
    assert(errorcode < 100);

    if (val == 0.0)
        m__efficiency_curve = nullptr;
    else
    {
        char curve_id[EN_MAXID+1];
        errorcode = EN_getcurveid(ph, static_cast<int>(val), curve_id);
        assert(errorcode < 100);
        
        m__efficiency_curve = m__wds.curves().get<EfficiencyCurve>(curve_id);
    }
}

void Pump::retrieve_EN_results()
{
    this->__retrieve_EN_results();

    inherited::retrieve_EN_results();
}

void Pump::__retrieve_EN_results()
{
    assert(m__en_index > 0);
    assert(m__wds.ph() != nullptr);

    auto ph = m__wds.ph();
    auto t = m__wds.current_result_time();

    double val;
    int errorcode = EN_getlinkvalue(ph, m__en_index, EN_ENERGY, &val);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the results of the pump.",
        "Error while retrieving value: EN_ENERGY",
        "Error code: ", errorcode,
        "Pump ID: ", m__name);

    m__instant_energy.commit(t, val);

    errorcode = EN_getlinkvalue(ph, m__en_index, EN_STATUS, &val);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the results of the pump.",
        "Error while retrieving value: EN_STATUS",
        "Error code: ", errorcode,
        "Pump ID: ", m__name);

    m__state.commit(t, static_cast<int>(val));

    errorcode = EN_getlinkvalue(ph, m__en_index, EN_PUMP_EFFIC, &val);
    beme_throw_if(errorcode > 100, std::runtime_error,
        "Impossible to retrieve the results of the pump.",
        "Error while retrieving value: EN_PUMP_EFFIC",
        "Error code: ", errorcode,
        "Pump ID: ", m__name);

    m__efficiency.commit(t, val);

    // Note on conversions:
    // // Instantenous Energy is always in kW (even if they say kWh)        
    // // Status is dimensionless
    // // Efficiency is dimensionless
}

} // namespace bevarmejo::wds
