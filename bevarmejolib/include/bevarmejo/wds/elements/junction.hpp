#ifndef BEVARMEJOLIB__WDS_ELEMENTS__JUNCTION_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__JUNCTION_HPP

#include <string>
#include <unordered_map>

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/node.hpp"

namespace bevarmejo::wds
{

class Junction;
template <>
struct TypeTraits<Junction>
{
    static constexpr const char* name = "Junction";
    static constexpr unsigned int code = 1111;
    static constexpr bool is_EN_complete = true;
};

class Junction final : public Node
{
    /// WDS Junction
    /*******************************************************************************
     * The wds::Junction class represents a demand node in the network.
     ******************************************************************************/

/*------- Member types -------*/
public:
    using self_type = Junction;
    using self_traits = TypeTraits<self_type>;
    using inherited = Node;
    using FlowSeries = aux::QuantitySeries<double>;
    using Demands= std::unordered_map<std::string, FlowSeries>;
private:
    friend class WaterDistributionSystem;

/*------- Member objects -------*/
protected:
    // === Properties ===
    Demands m__demands;

    // === Results ===
    FlowSeries m__demand;
    FlowSeries m__consumption;
    FlowSeries m__undelivered_demand;

/*------- Member functions -------*/
// (constructor)
public:
    Junction() = delete;
    Junction(const WaterDistributionSystem& wds, const EN_Name_t& name); // Constructor

// (destructor)
public:
    virtual ~Junction() = default;

// clone()

// (EPANET constructor)
public:
    static auto make_from_EN_for(const WaterDistributionSystem& wds, const EN_Name_t& name) -> std::unique_ptr<Junction>;

/*------- Operators -------*/
// operator=
public:

/*------- Element access -------*/
public:
    // === Read/Write properties ===
    const char* type_name() const override;

    unsigned int type_code() const override;

    Demands& demands();
    const Demands& demands() const;

    FlowSeries& demand(const std::string& a_category);
    const FlowSeries& demand(const std::string& a_category) const;

    // === Read-only properties ===
    bool has_demand() const override;
    const FlowSeries& demand() const; // Total demand
    const FlowSeries& demand_requested() const; // As total demand

    // === Results ===
    const FlowSeries& demand_delivered() const; // Share of the total demand that was delivered (consumption)
    const FlowSeries& consumption() const; // Share of the total demand that was delivered (consumption)
    const FlowSeries& demand_undelivered() const; // Share of the total demand that was not delivered
    
/*------- Capacity -------*/
public:

/*------- Modifiers -------*/
public:
    void clear_results() override;

    void retrieve_EN_properties() override;
    void retrieve_EN_results() override;
private:
    void __retrieve_EN_properties();
    void __retrieve_EN_results();

}; // class Junction

} // namespace bevarmejo::wds

#endif // BEVARMEJOLIB__WDS_ELEMENTS__JUNCTION_HPP