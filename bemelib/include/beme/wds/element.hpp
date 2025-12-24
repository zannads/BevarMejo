#ifndef BEVARMEJOLIB__WDS_ELEMENTS__ELEMENT_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__ELEMENT_HPP

#include <string>

#include "bevarmejo/wds/utility/quantity_series.hpp"

namespace bevarmejo
{
// Forward definition of the WDS class.
class WaterDistributionSystem;

namespace wds
{

// Default Element Trait 
template <typename T>
struct TypeTraits
{
    static constexpr const char* name = "Unknown";
    static constexpr unsigned int code = 0;
    static constexpr bool is_EN_complete = false;
};

class Element;
template <>
struct TypeTraits<Element>
{
    static constexpr const char* name = "Element";
    static constexpr unsigned int code = 1;
    static constexpr bool is_EN_complete = false;
};

class Element
{
    // WDS ancestor object.
    /************************************************************************
     * The bevarmejo::wds::Element class is the ancestor of all the elements
     * of the WDS. It is a pure virtual class, so it cannot be instantiated.
    */

/*------- Member types -------*/
public:
    using self_type = Element;
    using self_traits = TypeTraits<self_type>;
    using EN_Name_t = std::string; // Type for the name of the element compatible with the EPANET ID.
    using EN_Index_t = int; // Type for the index of the element in the EPANET project.
    using PropertiesMap = aux::QuantitiesMap;
private:
    friend class WaterDistributionSystem; // The WDS class is the only one that can create elements.

/*------- Member objects -------*/
protected:
    // Const reference to the WDS object that owns the element.
    const WaterDistributionSystem& m__wds;

    // Properties for EPANET compatibility and cache purposes.
    // The WDS class takes care of the uniqueness of the ID/name of the elements.
    // However, there are situations where an element should know its name/ID
    // (for example to retrieve its index in the EPANET project).
    // As I expose the WDS class as a const reference, I could search for 
    // the element in the WDS class every time I need its name, but it is
    // not efficient. So, I store a copy of the name here.

    EN_Name_t m__name; // User-defined name for the element compatible with the EPANET ID.
    EN_Index_t m__en_index; // Index in the EPANET project for cache purposes.
    
    PropertiesMap m__ud_properties; // User-defined Properties of the element.
    
/*------- Member functions -------*/
// (constructor)
public:
    Element() = delete;
    Element(const WaterDistributionSystem& wds, const EN_Name_t& name); // Constructor
    
// (destructor)
public:
    virtual ~Element() = default;

// clone()
public:

/*------- Operators -------*/
// operator=
public:
        
/*------- Element access -------*/
public:
    virtual const char* type_name() const = 0;

    virtual unsigned int type_code() const = 0;

    const EN_Name_t& name() const;
    const EN_Name_t& EN_id() const;

    EN_Index_t EN_index() const;

    const PropertiesMap& ud_properties() const;

/*------- Capacity -------*/
public:

/*------- Modifiers -------*/
public:
    virtual void retrieve_EN_index() = 0;

    virtual void retrieve_EN_properties();
};

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__ELEMENT_HPP
