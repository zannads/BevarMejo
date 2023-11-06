#ifndef BEVARMEJOLIB__WDS_ELEMENTS__LINK_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__LINK_HPP

#include <string>
#include <variant>

#include "bevarmejo/wds/elements/variable.hpp"
#include "bevarmejo/wds/elements/element.hpp"

#include "bevarmejo/wds/elements/node.hpp"

namespace bevarmejo {
namespace wds {

/// WDS link
/*******************************************************************************
 * The wds::link class represents a link in the network.
 ******************************************************************************/

static const std::string L_INITIAL_STATUS= "Initial Status";
static const std::string L_FLOW= "Flow";

class node; // forward declaration

class link : public element {

public:
    using inherited= element;

protected:
    node* _node_start_;
    node* _node_end_;

    // pointers to variables
    vars::var_int* _initial_status_;

    // pointers to results 
    vars::var_tseries_real* _flow_;
    // TODO: water quality

    void _add_properties() override;
    void _add_results() override;
    void _update_pointers() override;

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

    node* from_node() const { return _node_start_; }
    void from_node(node* n);
    node* to_node() const { return _node_end_; }
    void to_node(node* n);

    // ----- load from EPANET ----- //
    void retrieve_index(EN_Project ph) override;
    void retrieve_properties(EN_Project ph) override;

    vars::var_int& initial_status() const { return *_initial_status_; }
    vars::var_tseries_real& flow() const { return *_flow_; }

};

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__LINK_HPP