#ifndef BEVARMEJOLIB__WDS_ELEMENTS__RESERVOIR_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__RESERVOIR_HPP

#include <string>

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/source.hpp"

namespace bevarmejo::wds
{

class Reservoir;
template <>
struct TypeTraits<Reservoir>
{
    static constexpr const char* name = "Reservoir";
    static constexpr unsigned int code = 12111;
    static constexpr bool is_EN_complete = true;
};

class Reservoir final : public Source
{
    /// WDS Reservoir
    /*******************************************************************************
     * The wds::Reservoir class represents a Reservoir in the network. It is a static element.
     ******************************************************************************/

/*------- Member types -------*/
public:
    using self_type = Reservoir;
    using self_traits = TypeTraits<self_type>;
    using inherited = Source;
private:
    friend class WaterDistributionSystem;

/*------- Member objects -------*/
protected:
    // === Properties ===

    // === Results ===

/*------- Member functions -------*/
// (constructor)
public:
    Reservoir() = delete;
    Reservoir(const WaterDistributionSystem& wds, const EN_Name_t& name); // Constructor

// (destructor)
public:
    virtual ~Reservoir() = default;

// clone()
public:

// (EPANET constructor)
public:
    static auto make_from_EN_for(const WaterDistributionSystem& wds, const EN_Name_t& name) -> std::unique_ptr<Reservoir>;

/*------- Operators -------*/
// operator=
public:

/*------- Element access -------*/
public:
    // === Read/Write properties ===
    const char* type_name() const override;

    unsigned int type_code() const override;

    // === Read-only properties ===

    // === Results ===
    
/*------- Capacity -------*/
public:

/*------- Modifiers -------*/
public:

}; // class Reservoir

} // namespace bevarmejo::wds

#endif // BEVARMEJOLIB__WDS_ELEMENTS__RESERVOIR_HPP
