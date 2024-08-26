//
// 
#ifndef BEVARMEJOLIB__WDS_ELEMENTS__JUNCTION_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__JUNCTION_HPP

#include <string>
#include <unordered_map>

#include "epanet2_2.h"

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/node.hpp"

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
    using FlowSeries= aux::QuantitySeries<double>;
    using Demands= std::unordered_map<std::string, FlowSeries>;

/*--- Attributes ---*/
protected:
    /*--- Properties ---*/
    Demands m__demands;

    /*---  Results   ---*/
    FlowSeries m__demand;
    FlowSeries m__consumption;
    FlowSeries m__undelivered_demand;

 /*--- Constructors ---*/
public:
    Junction() = delete;
    Junction(const std::string& id, const WaterDistributionSystem& wds);

    // Copy constructor
    Junction(const Junction& other);

    // Move constructor
    Junction(Junction&& rhs) noexcept;

    // Copy assignment operator
    Junction& operator=(const Junction& rhs);

    // Move assignment operator
    Junction& operator=(Junction&& rhs) noexcept;

    virtual ~Junction() = default;
    
/*--- Getters and setters ---*/
public:
    /*--- Properties ---*/
    Demands& demands() {return m__demands;}
    const Demands& demands() const {return m__demands;}

    FlowSeries& demand(const std::string& a_category);
    const FlowSeries& demand(const std::string& a_category) const;

    /*---  Results   ---*/
    const aux::QuantitySeries<double>& demand_requested() const { return m__demand; }
    const aux::QuantitySeries<double>& demand_delivered() const { return consumption(); }
    const aux::QuantitySeries<double>& consumption() const { return m__consumption; }
    const aux::QuantitySeries<double>& demand_undelivered() const { return m__undelivered_demand; }

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
protected:
    void __retrieve_EN_properties(EN_Project ph) override;
public:
    /*--- Results ---*/
    void retrieve_results(EN_Project ph, long t) override;

}; // class Junction

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__JUNCTION_HPP