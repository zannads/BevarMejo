#include <cassert>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "epanet2_2.h"

#include "bevarmejo/wds/data_structures/temporal.hpp"
#include "bevarmejo/wds/data_structures/variable.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"

#include "bevarmejo/wds/auxiliary/pattern.hpp"
#include "bevarmejo/wds/auxiliary/demand.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "junction.hpp"

namespace bevarmejo {
namespace wds {

Junction::Junction(const std::string& id, const WaterDistributionSystem& wds) : 
    inherited(id, wds),
    _demands_(),
    _demand_constant_(nullptr),
    _demand_requested_(nullptr),
    _demand_delivered_(nullptr),
    _demand_undelivered_(nullptr)
    {
        _add_properties();
        _add_results();
        _update_pointers();
    }

// Copy constructor
Junction::Junction(const Junction& other) : 
    inherited(other),
    _demands_(other._demands_),
    _demand_constant_(nullptr),
    _demand_requested_(nullptr),
    _demand_delivered_(nullptr),
    _demand_undelivered_(nullptr)
    {
        _update_pointers();
    }

// Move constructor
Junction::Junction(Junction&& rhs) noexcept : 
    inherited(std::move(rhs)),
    _demands_(std::move(rhs._demands_)),
    _demand_constant_(nullptr),
    _demand_requested_(nullptr),
    _demand_delivered_(nullptr),
    _demand_undelivered_(nullptr)
    {
        _update_pointers();
    }

// Copy assignment operator
Junction& Junction::operator=(const Junction& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        _demands_ = rhs._demands_;
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
Junction& Junction::operator=(Junction&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        _demands_ = std::move(rhs._demands_);
        _update_pointers();
    }
    return *this;
}

Junction::~Junction() { /* Everything is deleted by the inherited destructor */ }

Demand& Junction::demand(const std::string &a_category) {
    for (auto& d : _demands_) {
        if (d.category() == a_category) {
            return d;
        }
    }
    throw std::out_of_range("Demand category " + a_category + " not found in junction " + id());
}

void Junction::add_demand(const std::string &a_category, const double a_base_dem, const std::shared_ptr<Pattern> a_pattern) {
    _demands_.emplace_back(Demand(a_category, a_base_dem, a_pattern));
}

auto Junction::_find_demand(const std::string &a_category) const {
    for (auto it = _demands_.begin(); it != _demands_.end(); ++it) {
        if (it->category() == a_category) {
            return it;
        }
    }
    throw std::out_of_range("Demand category " + a_category + " not found in junction " + id());
}

void Junction::remove_demand(const std::string &a_category) {
    auto d_p_demand = _find_demand(a_category);
    if (d_p_demand != _demands_.end()) {
        _demands_.erase(d_p_demand);
    }
}

const bool Junction::has_demand() const {
    return _demand_constant_->value() > 0 || !_demands_.empty();
}

void Junction::__retrieve_EN_properties(EN_Project ph)  {
    inherited::__retrieve_EN_properties(ph);
    auto patterns= m__wds.patterns();

    int n_demands= 0;
    int errorcode= EN_getnumdemands(ph, this->index(), &n_demands);
    assert(errorcode < 100);

    for (std::size_t i= 1; i <= n_demands; ++i) {
        double base_demand= 0.0;
        errorcode= EN_getbasedemand(ph, this->index(), i, &base_demand);
        assert(errorcode < 100);

        int pattern_index= 0;
        errorcode= EN_getdemandpattern(ph, this->index(), i, &pattern_index);
        assert(errorcode < 100);

        char __pattern_id[EN_MAXID+1];
        errorcode= EN_getpatternid(ph, pattern_index, __pattern_id);
        assert(errorcode < 100);
        std::string pattern_id(__pattern_id);

        char __demand_category[EN_MAXID+1];
        errorcode= EN_getdemandname(ph, this->index(), i, __demand_category);
        assert(errorcode < 100);
        std::string demand_category(__demand_category);

        std::shared_ptr<Pattern> pattern= nullptr;
        // Pattern id can be "" if the demand is constant
        if (!pattern_id.empty()) {
            auto it= patterns.find(pattern_id);
            assert(it != patterns.end());
            pattern= *it;
        }
        
        add_demand(demand_category, base_demand, pattern);
    }
}

void Junction::retrieve_results(EN_Project ph, long t=0) {
    inherited::retrieve_results(ph, t);

    int errorcode = 0;
    double d_demand = 0;
    double d_dem_deficit = 0;
    errorcode = EN_getnodevalue(ph, index(), EN_DEMAND, &d_demand);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving demand for node " + id()+"\n");

    if (ph->parser.Unitsflag != LPS)
        d_demand = epanet::convert_flow_to_L_per_s(ph, d_demand);
    this->_demand_requested_->value().insert(std::make_pair(t, d_demand));

    errorcode = EN_getnodevalue(ph, index(), EN_DEMANDDEFICIT, &d_dem_deficit);
    if (errorcode > 100)
        throw std::runtime_error("Error retrieving demand deficit for node " + id()+"\n");

    if (ph->parser.Unitsflag != LPS)
        d_dem_deficit = epanet::convert_flow_to_L_per_s(ph, d_dem_deficit);
    // IF DDA is on and head is negative, the demand was not satisfied and it should go as a demand undelivered
    if (/*Assume we know it is DDA*/ this->_head_->value().at(t) < 0)
        this->_demand_undelivered_->value().insert(std::make_pair(t, d_demand));
    else
        this->_demand_undelivered_->value().insert(std::make_pair(t, d_dem_deficit));

    this->_demand_delivered_->value().insert(std::make_pair(t, 
        _demand_requested_->value().at(t) - _demand_undelivered_->value().at(t)));
}

void Junction::_add_properties() {
    inherited::_add_properties();

    properties().emplace(LDEMAND_CONSTANT, vars::var_real(vars::l__L_per_s,0));
}

void Junction::_add_results() {
    inherited::_add_results();

    results().emplace(LDEMAND_REQUESTED, vars::var_tseries_real(vars::l__L_per_s));
    results().emplace(LDEMAND_DELIVERED, vars::var_tseries_real(vars::l__L_per_s));
    results().emplace(LDEMAND_UNDELIVERED, vars::var_tseries_real(vars::l__L_per_s));
}

void Junction::_update_pointers() {
    inherited::_update_pointers();

    _demand_constant_ = &std::get<vars::var_real>(properties().at(LDEMAND_CONSTANT));

    _demand_requested_ = &std::get<vars::var_tseries_real>(results().at(LDEMAND_REQUESTED));
    _demand_delivered_ = &std::get<vars::var_tseries_real>(results().at(LDEMAND_DELIVERED));
    _demand_undelivered_ = &std::get<vars::var_tseries_real>(results().at(LDEMAND_UNDELIVERED));
}

} // namespace wds
} // namespace bevarmejo
