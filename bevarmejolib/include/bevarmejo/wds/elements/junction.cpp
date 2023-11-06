#include <string>

#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/results.hpp"
#include "bevarmejo/wds/elements/variables.hpp"
#include "bevarmejo/wds/elements/variable.hpp"
#include "bevarmejo/wds/elements/temporal.hpp"

#include "junction.hpp"

namespace bevarmejo {
namespace wds {

junction::junction(const std::string& id) : inherited(id),
                                            _demand_(),
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
junction::junction(const junction& other) : inherited(other),
                                            _demand_(other._demand_),
                                            _demand_constant_(nullptr),
                                            _demand_requested_(nullptr),
                                            _demand_delivered_(nullptr),
                                            _demand_undelivered_(nullptr)
                                            {
                                                _update_pointers();
                                            }

// Move constructor
junction::junction(junction&& rhs) noexcept : inherited(std::move(rhs)),
                                            _demand_(std::move(rhs._demand_)),
                                            _demand_constant_(nullptr),
                                            _demand_requested_(nullptr),
                                            _demand_delivered_(nullptr),
                                            _demand_undelivered_(nullptr)
                                            {
                                                _update_pointers();
                                            }

// Copy assignment operator
junction& junction::operator=(const junction& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        _demand_ = rhs._demand_;
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
junction& junction::operator=(junction&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        _demand_ = std::move(rhs._demand_);
        _update_pointers();
    }
    return *this;
}

junction::~junction() { /* Everything is deleted by the inherited destructor */ }

void junction::retrieve_properties(EN_Project ph) {
    inherited::retrieve_properties(ph);

    //TODO: get the demands 
}

void junction::_add_properties()
{
    inherited::_add_properties();
    properties().emplace(LDEMAND_CONSTANT, vars::var_real(vars::L_M3_PER_S,0));
}

void junction::_add_results()
{
    inherited::_add_results();
    results().temporal_reals().emplace(LDEMAND_REQUESTED, vars::L_M3_PER_S);
    results().temporal_reals().emplace(LDEMAND_DELIVERED, vars::L_M3_PER_S);
    results().temporal_reals().emplace(LDEMAND_UNDELIVERED, vars::L_M3_PER_S);
}

void junction::_update_pointers() {
    inherited::_update_pointers();

    _demand_constant_ = &(std::get<vars::var_real>(properties().at(LDEMAND_CONSTANT)));

    _demand_requested_ = &(results().temporal_reals().at(LDEMAND_REQUESTED));
    _demand_delivered_ = &(results().temporal_reals().at(LDEMAND_DELIVERED));
    _demand_undelivered_ = &(results().temporal_reals().at(LDEMAND_UNDELIVERED));
}

} // namespace wds
} // namespace bevarmejo
