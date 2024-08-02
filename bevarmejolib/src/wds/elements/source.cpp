#include <cassert>
#include <string>
#include <variant>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/wds/data_structures/variable.hpp"
#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/element.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "source.hpp"

namespace bevarmejo {
namespace wds {

Source::Source(const std::string& id, const WaterDistributionSystem& wds) : 
    inherited(id, wds), 
    _inflow_(nullptr),
    _source_elevation_(nullptr),
    m__inflow(wds.time_series(l__RESULT_TS)),
    m__source_elevation(wds.time_series(l__RESULT_TS))
    {
        _add_results();
        _update_pointers();
    }

// Copy constructor
Source::Source(const Source& other) : 
    inherited(other), 
    _inflow_(nullptr),
    _source_elevation_(nullptr),
    m__inflow(other.m__inflow),
    m__source_elevation(other.m__source_elevation)
    {
        _update_pointers();
    }

// Move constructor
Source::Source(Source&& rhs) noexcept : 
    inherited(std::move(rhs)), 
    _inflow_(nullptr),
    _source_elevation_(nullptr),
    m__inflow(std::move(rhs.m__inflow)),
    m__source_elevation(std::move(rhs.m__source_elevation))
    {
        _update_pointers();
    }

// Copy assignment operator
Source& Source::operator=(const Source& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        m__inflow = rhs.m__inflow;
        m__source_elevation = rhs.m__source_elevation;
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
Source& Source::operator=(Source&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        m__inflow = std::move(rhs.m__inflow);
        m__source_elevation = std::move(rhs.m__source_elevation);
        _update_pointers();
    }
    return *this;
}

void Source::retrieve_results(EN_Project ph, long t) {
    inherited::retrieve_results(ph, t);
    assert(index()!= 0);

    int errorcode = 0;
    double val = 0.0;

    errorcode = EN_getnodevalue(ph, index(), EN_DEMAND, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving demand for node " + id()+"\n");

    if (ph->parser.Flowflag != LPS)
        val = epanet::convert_flow_to_L_per_s(ph, val);
    this->_inflow_->value().insert(std::make_pair(t, val));

    errorcode = EN_getnodevalue(ph, index(), EN_HEAD, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving elevation for node " + id()+"\n");

    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    this->_source_elevation_->value().insert(std::make_pair(t, val));
}

void Source::_add_results() {
    inherited::_add_results();

    results().emplace(LINFLOW, vars::var_tseries_real(vars::l__L_per_s));
    results().emplace(LSOURCE_ELEV, vars::var_tseries_real(vars::l__m));
}

void Source::_update_pointers() {
    inherited::_update_pointers();

    _inflow_ = &std::get<vars::var_tseries_real>(results().at(LINFLOW));
    _source_elevation_ = &std::get<vars::var_tseries_real>(results().at(LSOURCE_ELEV));
}

} // namespace wds
} // namespace bevarmejo
