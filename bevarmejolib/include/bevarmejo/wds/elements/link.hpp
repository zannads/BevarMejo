#ifndef BEVARMEJOLIB__WDS_ELEMENTS__LINK_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__LINK_HPP

#include <string>

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/network_element.hpp"

namespace bevarmejo::wds
{

class Link;
template <>
struct TypeTraits<Link>
{
    static constexpr const char* name = "Link";
    static constexpr unsigned int code = 211;
    static constexpr bool is_EN_complete = false;
};

// Forward definition of the Node class because Links are installed between Nodes.
class Node;

class Link : public NetworkElement
{
// WDS Link
/*******************************************************************************
 * The wds::Link class represents a link in the network.
 ******************************************************************************/

/*------- Member types -------*/
public:
    using self_type = Link;
    using self_traits = TypeTraits<self_type>;
    using inherited = NetworkElement;
    using Node_ptr = const Node*;
    using StatusSeries = aux::QuantitySeries<int>;
    using FlowSeries = aux::QuantitySeries<double>;
private:
    friend class WaterDistributionSystem;

/*------- Member objects -------*/
protected:
    // === Properties ===
    Node_ptr m__from_node;
    Node_ptr m__to_node;
    StatusSeries m__initial_status; // Constant

    // === Results ===
    FlowSeries m__flow;

/*------- Member functions -------*/
// (constructor)
protected:
    Link() = delete;
    Link(const WaterDistributionSystem& wds, const EN_Name_t& name); // Constructor
    
// (destructor)
public:
    virtual ~Link() = default;

// clone()
public:

/*------- Operators -------*/
// operator=
public:

/*------- Element access -------*/
public:
    // === Read/Write properties ===
    Node_ptr from_node() const;

    Node_ptr to_node() const;

    StatusSeries& initial_status();
    const StatusSeries& initial_status() const;

    // === Results ===
    const FlowSeries& flow() const;

/*------- Capacity -------*/
public:

/*------- Modifiers -------*/
public:
    virtual void clear_results() override;

    void retrieve_EN_index() override final;
    virtual void retrieve_EN_properties() override;
    virtual void retrieve_EN_results() override;
private:
    void __retrieve_EN_properties();
    void __retrieve_EN_results();

    void from_node(Node_ptr a_node);

    void to_node(Node_ptr a_node);

    void initial_status(int a_status);

}; // class Link

} // namespace bevarmejo::wds

#endif // BEVARMEJOLIB__WDS_ELEMENTS__LINK_HPP
