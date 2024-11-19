#ifndef BEVARMEJOLIB__WDS_ELEMENTS__LINK_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__LINK_HPP

#include <string>

#include "epanet2_2.h"

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/network_element.hpp"

namespace bevarmejo {
namespace wds {

/// WDS Link
/*******************************************************************************
 * The wds::Link class represents a link in the network.
 ******************************************************************************/

static const std::string L_INITIAL_STATUS= "Initial Status";
static const std::string L_FLOW= "Flow";

class Node; // forward declaration

class Link : public NetworkElement {

public:
    using inherited= NetworkElement;

/*--- Attributes ---*/
protected:
    /*--- Properties ---*/
    const Node* _node_start_;
    const Node* _node_end_;

    aux::QuantitySeries<int> m__initial_status; // Constant

    /*---  Results   ---*/ 
    aux::QuantitySeries<double> m__flow;
    // TODO: water quality

/*--- Constructors ---*/
public:
    Link() = delete;

    Link(const std::string& id, const WaterDistributionSystem& wds);

    // Copy constructor
    Link(const Link& other);

    // Move constructor
    Link(Link&& rhs) noexcept;

    // Copy assignment operator
    Link& operator=(const Link& rhs);

    // Move assignment operator
    Link& operator=(Link&& rhs) noexcept;

    // Destructor
    virtual ~Link() = default;

/*--- Getters and setters ---*/
public:
    /*--- Properties ---*/
    const Node* from_node() const { return _node_start_; }
    void start_node(const Node* a_node) { _node_start_ = a_node; }
    const Node* to_node() const { return _node_end_; }
    void end_node(const Node* a_node) { _node_end_ = a_node; }

    aux::QuantitySeries<int>& initial_status() { return m__initial_status; }
    const aux::QuantitySeries<int>& initial_status() const { return m__initial_status; }

    /*---  Results   ---*/
    const aux::QuantitySeries<double>& flow() const { return m__flow; }

/*--- Pure virtual methods override---*/
    virtual void clear_results() override;

/*--- EPANET-dependent PVMs override ---*/
public:
    void retrieve_index(EN_Project ph) override;
    void retrieve_results(EN_Project ph, long t) override;
protected:
    void __retrieve_EN_properties(EN_Project ph) override;

}; // class Link

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__LINK_HPP
