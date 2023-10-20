//
// 
#ifndef BEVARMEJOLIB__WDS_ELEMENTS__JUNCTION_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__JUNCTION_HPP

#include <string>

#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/variable.hpp"

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

protected:

    // should store demands in vector?? 
    // for now I will simply store a single demand object
    demand _demand_;

    // variables
    vars::var_real* _demand_constant_;

    // results
    vars::var_tseries_real* _demand_requested_;
    vars::var_tseries_real* _demand_delivered_;
    vars::var_tseries_real* _demand_undelivered_;

    // TODO: add properties as now demand can change
    void _add_results() override;
    void _update_pointers() override;

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
    
    // ----- override inherited pure virtual methods ----- // 
    const std::string& element_name() const override {return LNAME_JUNCTION;}
    const unsigned int& element_type() const override {return ELEMENT_JUNCTION;}

    // getters -- variables
    vars::var_real* demand_constant() {return _demand_constant_;}

    // getters -- results
    vars::var_tseries_real* demand_requested() {return _demand_requested_;}
    vars::var_tseries_real* demand_delivered() {return _demand_delivered_;}
    vars::var_tseries_real* demand_undelivered() {return _demand_undelivered_;}

    // TODO: Getter demands

}; // class junction

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__JUNCTION_HPP