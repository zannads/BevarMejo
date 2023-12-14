//
// 
#ifndef BEVARMEJOLIB__WDS_ELEMENTS__JUNCTION_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__JUNCTION_HPP

#include <string>
#include <vector>

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

/// WDS Junction
/*******************************************************************************
 * The wds::Junction class represents a demand node in the network.
 ******************************************************************************/

static const std::string LNAME_JUNCTION= "Junction";

class Junction : public Node {    
public:
    using inherited= Node;
    using DemandContainer = std::vector<Demand>;

/*--- Attributes ---*/
protected:
    /*--- Properties ---*/
    DemandContainer _demands_;

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
    Junction() = delete;
    Junction(const std::string& id);

    // Copy constructor
    Junction(const Junction& other);

    // Move constructor
    Junction(Junction&& rhs) noexcept;

    // Copy assignment operator
    Junction& operator=(const Junction& rhs);

    // Move assignment operator
    Junction& operator=(Junction&& rhs) noexcept;

    ~Junction() override;
    
/*--- Getters and setters ---*/
public:
    /*--- Properties ---*/
    DemandContainer& demands() {return _demands_;}
    const DemandContainer& demands() const {return _demands_;}

    Demand& demand(const std::string& a_category);

    void add_demand(const Demand& a_demand) {_demands_.push_back(a_demand);}
    void add_demand(const std::string& a_category, const double a_base_dem, const std::shared_ptr<Pattern> a_pattern);
    void remove_demand(const std::string& a_category);
private:
    auto _find_demand(const std::string& a_category) const;
public:    
    vars::var_real& demand_constant() {return *_demand_constant_;}

    /*---  Results   ---*/
    const vars::var_tseries_real& demand_requested() const {return *_demand_requested_;}
    const vars::var_tseries_real& demand_delivered() const {return *_demand_delivered_;}
    const vars::var_tseries_real& demand_undelivered() const {return *_demand_undelivered_;}

/*--- Methods ---*/
public:
    const bool has_demand() const override;
    
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

}; // class Junction

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__JUNCTION_HPP