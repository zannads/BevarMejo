#ifndef BEVARMEJOLIB__WDS_ELEMENTS__NODE_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__NODE_HPP

#include <memory>
#include <string>
#include <unordered_set>

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/network_element.hpp"


namespace bevarmejo::wds
{

class Node;
template <>
struct TypeTraits<Node>
{
    static constexpr const char* name = "Node";
    static constexpr unsigned int code = 111;
    static constexpr bool is_EN_complete = false;
};
    
// Forward definition of the Link class because Nodes have Links installed in them.
class Link;

class Node : public NetworkElement
{
// Node object
/************************************************************************
 * The bevarmejo::wds::Node class is the ancestor of all the nodes
 * of the WDS. It should be a pure virtual class, so it cannot be instantiated.
 */
    
/*------- Member types -------*/
public:
    using self_type = Node;
    using self_traits = TypeTraits<self_type>;
    using inherited = NetworkElement;
    using Link_ptr = const Link*;
    using ConnectedLinks = std::unordered_set<Link_ptr>;
    using HeadSeries = aux::QuantitySeries<double>;
    using PressureSeries = aux::QuantitySeries<double>;
private:
    friend class WaterDistributionSystem;

/*------- Member objects -------*/
protected:
    // === Properties ===
    double m__x_coord;
    double m__y_coord;
    double m__elevation; // or z coordinate

    ConnectedLinks m__links;

    // === Results ===
    HeadSeries m__head;
    PressureSeries m__pressure;
    
/*------- Member functions -------*/
// (constructor)
protected:
    Node() = delete;
    Node(const WaterDistributionSystem& wds, const EN_Name_t& name); // Constructor

// (destructor)
public:
    virtual ~Node() = default;

// clone()
public:

/*------- Operators -------*/
// operator=
public:

/*------- Element access -------*/
public:
    // === Read/Write properties ===
    double& x_coord();
    double x_coord() const;

    double& y_coord();
    double y_coord() const;

    double& elevation();
    double elevation() const;
    double& z_coord();
    double z_coord() const;

    const ConnectedLinks& links() const;

    // === Read-only properties ===
    virtual bool has_demand() const = 0;

    // === Results ===
    const HeadSeries& head() const;
    const PressureSeries& pressure() const;

/*------- Capacity -------*/
public:

/*------- Modifiers -------*/
public:
    virtual void clear_results() override;

    void retrieve_EN_index() override final;

    virtual void retrieve_EN_properties() override;

    virtual void retrieve_EN_results() override;

    void x_coord(double x);

    void y_coord(double y);

    void elevation(double z);

    void z_coord(double z);

    void connect_link(Link_ptr link);

    void disconnect_link(Link_ptr link);

}; // class Node

} // namespace bevarmejo::wds

#endif // BEVARMEJOLIB__WDS_ELEMENTS__NODE_HPP
