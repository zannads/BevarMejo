#ifndef BEVARMEJOLIB__WDS_ELEMENTS__SOURCE_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__SOURCE_HPP

#include <string>

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/node.hpp"

namespace bevarmejo::wds
{

class Source;
template <>
struct TypeTraits<Source>
{
    static constexpr const char* name = "Source";
    static constexpr unsigned int code = 2111;
    static constexpr bool is_EN_complete = false;
};

class Source : public Node
{
    /// WDS Source
    /*******************************************************************************
     * The wds::Source class represents a fix pressure node in the network. 
     * It can either be a tank or a reservoir.
     ******************************************************************************/

/*------- Member types -------*/
public:
    using self_type = Source;
    using self_traits = TypeTraits<self_type>;
    using inherited = Node;
    using FlowSeries = aux::QuantitySeries<double>;
    using ElevationSeries = aux::QuantitySeries<double>;
private:
    friend class WaterDistributionSystem;

/*------- Member objects -------*/
protected:
    // === Properties ===

    // === Results ===
    FlowSeries m__inflow;
    ElevationSeries m__source_elevation;

/*------- Member functions -------*/
// (constructor)
protected:
    Source() = delete;
    Source(const WaterDistributionSystem& wds, const EN_Name_t& name); // Constructor

// (destructor)
public:
    virtual ~Source() = default;

// clone()
public:

/*------- Operators -------*/
// operator=
public:

/*------- Element access -------*/
public:
    // === Read/Write properties ===

    // === Read-only properties ===
    bool has_demand() const override final;

    // === Results ===
    const FlowSeries& inflow() const;
    const ElevationSeries& source_elevation() const;

/*------- Capacity -------*/
public:

/*------- Modifiers -------*/
public:
    virtual void clear_results() override;

    virtual void retrieve_EN_results() override;
private:
    void __retrieve_EN_results();
}; // class Source
    
} // namespace bevarmejo::wds

#endif // BEVARMEJOLIB__WDS_ELEMENTS__SOURCE_HPP
