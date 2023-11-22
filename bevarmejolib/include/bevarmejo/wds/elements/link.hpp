#ifndef BEVARMEJOLIB__WDS_ELEMENTS__LINK_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__LINK_HPP

#include <string>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/variable.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"

namespace bevarmejo {
namespace wds {

/// WDS link
/*******************************************************************************
 * The wds::link class represents a link in the network.
 ******************************************************************************/

static const std::string L_INITIAL_STATUS= "Initial Status";
static const std::string L_FLOW= "Flow";

class Node; // forward declaration

class link : public NetworkElement {

public:
    using inherited= NetworkElement;

/*--- Attributes ---*/
protected:
    /*--- Properties ---*/
    Node* _node_start_;
    Node* _node_end_;

    vars::var_int* _initial_status_;

    /*---  Results   ---*/ 
    vars::var_tseries_real* _flow_;
    // TODO: water quality

protected:
    void _add_properties() override;
    void _add_results() override;
    void _update_pointers() override;

/*--- Constructors ---*/
public:
    link() = delete;

    link(const std::string& id);

    // Copy constructor
    link(const link& other);

    // Move constructor
    link(link&& rhs) noexcept;

    // Copy assignment operator
    link& operator=(const link& rhs);

    // Move assignment operator
    link& operator=(link&& rhs) noexcept;

    // Destructor
    virtual ~link();

/*--- Getters and setters ---*/
public:
    /*--- Properties ---*/
    Node* from_node() const { return _node_start_; }
    void from_node(Node* a_node);

    Node* to_node() const { return _node_end_; }
    void to_node(Node* a_node);

    vars::var_int& initial_status() const { return *_initial_status_; }

    /*---  Results   ---*/
    const vars::var_tseries_real& flow() const { return *_flow_; }

/*--- Pure virtual methods override---*/

/*--- EPANET-dependent PVMs override ---*/
public:
    void retrieve_index(EN_Project ph) override;
    void retrieve_properties(EN_Project ph) override;
    void retrieve_results(EN_Project ph, long t) override;

}; // class link

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__LINK_HPP
