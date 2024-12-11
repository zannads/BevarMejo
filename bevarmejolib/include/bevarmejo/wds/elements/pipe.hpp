#ifndef BEVARMEJOLIB__WDS_ELEMENTS__PIPE_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__PIPE_HPP

#include <string>

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/dimensioned_link.hpp"

namespace bevarmejo::wds
{

class Pipe;
template <>
struct TypeTraits<Pipe>
{
    static constexpr const char* name = "Pipe";
    static constexpr unsigned int code = 11211;
    static constexpr bool is_EN_complete = true;
};

class Pipe final : public DimensionedLink
{
/// WDS Pipe
/*******************************************************************************
 * The wds::Pipe class represents a pipe in the network.
 ******************************************************************************/

/*------- Member types -------*/
public:
    using self_type = Pipe;
    using self_traits = TypeTraits<self_type>;
    using inherited = DimensionedLink;
private:
    friend class WaterDistributionSystem;

/*------- Member objects -------*/
protected:
    // === Properties ===
    aux::QuantitySeries<double> m__length; // Constant

    // === Results ===

/*------- Member functions -------*/
// (constructor)
public:
    Pipe() = delete;
    Pipe(const WaterDistributionSystem& wds, const EN_Name_t& name); // Constructor
    Pipe(const Pipe&) = delete;
    Pipe(Pipe&&) = delete;
    Pipe(const WaterDistributionSystem& wds, const EN_Name_t& name, const Pipe& other);
    
// (destructor)
public:
    virtual ~Pipe() = default;

// clone()
public:

// (EPANET constructor)
public:
    static auto make_from_EN_for(const WaterDistributionSystem& wds, const EN_Name_t& name) -> std::unique_ptr<Pipe>;

/*------- Operators -------*/
// operator=
public:

/*------- Element access -------*/
public:
    // === Read/Write properties ===
    const char* type_name() const override;

    unsigned int type_code() const override;

    aux::QuantitySeries<double>& length() {return m__length;}
    const aux::QuantitySeries<double>& length() const {return m__length;}
    void length(double a_length) {m__length.value(a_length);}

    // === Read-only properties ===

    // === Results ===

/*------- Capacity -------*/
public:

/*------- Modifiers -------*/
public:
    void retrieve_EN_properties() override;
private:
    void __retrieve_EN_properties();

}; // class Pipe

} // namespace bevarmejo::wds

#endif // BEVARMEJOLIB__WDS_ELEMENTS__PIPE_HPP
