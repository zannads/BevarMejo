//
// 
#ifndef BEVARMEJOLIB__WDS_ELEMENTS__JUNCTION_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__JUNCTION_HPP

#include <string>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/variable.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"

#include "bevarmejo/wds/elements/pattern.hpp"
#include "bevarmejo/wds/elements/demand.hpp"

namespace bevarmejo {
namespace wds {

static const std::string LDEMAND_CONSTANT= "Demand (constant)";
static const std::string LDEMAND_REQUESTED= "Demand (requested)";
static const std::string LDEMAND_DELIVERED= "Demand (delivered)";
static const std::string LDEMAND_UNDELIVERED= "Demand (undelivered)";

/// WDS junction
/*******************************************************************************
 * The wds::junction class represents a demand node in the network.
 ******************************************************************************/

static const std::string LNAME_JUNCTION= "Junction";

class junction : public node {    
public:
    using inherited= node;
    using DemandContainer = std::vector<demand>;

/*--- Attributes ---*/
protected:
    /*--- Properties ---*/
    // TODO: from containing demand object to containing DemandContainer object
    demand _demand_;

    vars::var_real* _demand_constant_;

    /*---  Results   ---*/
    vars::var_tseries_real* _demand_requested_;
    vars::var_tseries_real* _demand_delivered_;
    vars::var_tseries_real* _demand_undelivered_;

protected:
    void _add_properties() override;
    void _add_results() override;
    void _update_pointers() override;

 /*--- Constructors ---*/
public:
    junction() = delete;
    junction(const std::string& id);

    // Copy constructor
    junction(const junction& other);

    // Move constructor
    junction(junction&& rhs) noexcept;

    // Copy assignment operator
    junction& operator=(const junction& rhs);

    // Move assignment operator
    junction& operator=(junction&& rhs) noexcept;

    ~junction() override;
    
/*--- Getters and setters ---*/
public:
    /*--- Properties ---*/
    const demand& demands() const {return _demand_;}
    void demands(const demand& a_demand) {_demand_ = a_demand;}
    // TODO: methods for DemandContainer
    vars::var_real& demand_constant() {return *_demand_constant_;}

    /*---  Results   ---*/
    const vars::var_tseries_real& demand_requested() const {return *_demand_requested_;}
    const vars::var_tseries_real& demand_delivered() const {return *_demand_delivered_;}
    const vars::var_tseries_real& demand_undelivered() const {return *_demand_undelivered_;}

/*--- Pure virtual methods override---*/
public:
    /*--- Properties ---*/
    const std::string& element_name() const override {return LNAME_JUNCTION;}
    const unsigned int element_type() const override {return ELEMENT_JUNCTION;}

/*--- EPANET-dependent PVMs ---*/
public:
    /*--- Properties ---*/
    void retrieve_properties(EN_Project ph) override;

    /*--- Results ---*/
    void retrieve_results(EN_Project ph, long t) override;

}; // class junction

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__JUNCTION_HPP