#include <cassert>
#include <string>
#include <variant>

#include "bevarmejo/wds/elements/variable.hpp"
#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/element.hpp"

#include "source.hpp"

namespace bevarmejo {
namespace wds {

source::source(const std::string& id) : 
    inherited(id), 
    _inflow_(nullptr),
    _source_elevation_(nullptr) 
    {
        _add_results();
        _update_pointers();
    }

// Copy constructor
source::source(const source& other) : 
    inherited(other), 
    _inflow_(nullptr),
    _source_elevation_(nullptr) 
    {
        _update_pointers();
    }

// Move constructor
source::source(source&& rhs) noexcept : 
    inherited(std::move(rhs)), 
    _inflow_(nullptr),
    _source_elevation_(nullptr) 
    {
        _update_pointers();
    }

// Copy assignment operator
source& source::operator=(const source& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
source& source::operator=(source&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        _update_pointers();
    }
    return *this;
}

source::~source() {/* results are cleared when the inherited destructor is called*/ }

void source::retrieve_results(EN_Project ph, long t) {
    inherited::retrieve_results(ph, t);
    assert(index()!= 0);

    int errorcode = 0;
    double val = 0.0;
    errorcode = EN_getnodevalue(ph, index(), EN_DEMAND, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving demand for node " + id()+"\n");
    this->_inflow_->value().insert(std::make_pair(t, val));

    errorcode = EN_getnodevalue(ph, index(), EN_ELEVATION, &val);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving elevation for node " + id()+"\n");
    this->_source_elevation_->value().insert(std::make_pair(t, val));
}

void source::_add_results() {
    inherited::_add_results();

    results().emplace(LINFLOW, vars::var_tseries_real(vars::L_M3_PER_S));
    results().emplace(LSOURCE_ELEV, vars::var_tseries_real(vars::L_METER));
}

void source::_update_pointers() {
    inherited::_update_pointers();

    _inflow_ = &std::get<vars::var_tseries_real>(results().at(LINFLOW));
    _source_elevation_ = &std::get<vars::var_tseries_real>(results().at(LSOURCE_ELEV));
}

} // namespace wds
} // namespace bevarmejo
