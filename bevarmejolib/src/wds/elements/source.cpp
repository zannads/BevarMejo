#include <cassert>
#include <string>
#include <variant>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "source.hpp"

namespace bevarmejo {
namespace wds {

Source::Source(const std::string& id, const WaterDistributionSystem& wds) : 
    inherited(id, wds),
    m__inflow(wds.time_series(label::__RESULTS_TS)),
    m__source_elevation(wds.time_series(label::__RESULTS_TS)) { }

// Copy constructor
Source::Source(const Source& other) : 
    inherited(other),
    m__inflow(other.m__inflow),
    m__source_elevation(other.m__source_elevation) { }

// Move constructor
Source::Source(Source&& rhs) noexcept : 
    inherited(std::move(rhs)),
    m__inflow(std::move(rhs.m__inflow)),
    m__source_elevation(std::move(rhs.m__source_elevation)) { }

// Copy assignment operator
Source& Source::operator=(const Source& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        
        m__inflow = rhs.m__inflow;
        m__source_elevation = rhs.m__source_elevation;
    }
    return *this;
}

// Move assignment operator
Source& Source::operator=(Source&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));

        m__inflow = std::move(rhs.m__inflow);
        m__source_elevation = std::move(rhs.m__source_elevation);
    }
    return *this;
}

void Source::retrieve_results(EN_Project ph, long t) {
    inherited::retrieve_results(ph, t);
    assert(index()!= 0);

    double val = 0.0;
    int errorcode = EN_getnodevalue(ph, index(), EN_DEMAND, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving demand for node " + id()+"\n");

    if (ph->parser.Flowflag != LPS)
        val = epanet::convert_flow_to_L_per_s(ph, val);
    m__inflow.commit(t, val);

    errorcode = EN_getnodevalue(ph, index(), EN_HEAD, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving elevation for node " + id()+"\n");

    if (ph->parser.Unitsflag == US)
        val *= MperFT;
    m__source_elevation.commit(t, val);
}

void Source::clear_results() {
    inherited::clear_results();

    m__inflow.clear();
    m__source_elevation.clear();
}

} // namespace wds
} // namespace bevarmejo
