#ifndef BEVARMEJOLIB__WDS_ELEMENTS__NETWORK_ELEMENT_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__NETWORK_ELEMENT_HPP

#include <string>

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/element.hpp"

namespace bevarmejo::wds
{

class NetworkElement : public Element
{
    // WDS ancestor object.
    /************************************************************************
     * The bevarmejo::wds::NetworkElement class is the ancestor of networks 
     * elements of the WDS (nodes and links). It is a pure virtual class.
    */

/*------- Member types -------*/
public:
    using inherited = Element;
    using ResultsMap = aux::QuantitiesMap;
private:
    friend class WaterDistributionSystem;

/*------- Member objects -------*/
protected:
    ResultsMap m__ud_results; // User-defined Results of the network element.

/*------- Member functions -------*/
// (constructor)
protected:
    NetworkElement() = delete;
    NetworkElement(const WaterDistributionSystem& wds, const EN_Name_t& name); // Constructor
   
// (destructor)
public:
    virtual ~NetworkElement() = default;

// clone()
public:

/*------- Operators -------*/
// operator=
public:

/*------- Element access -------*/
public:
    const ResultsMap& results() const;

/*------- Capacity -------*/
public:

/*------- Modifiers -------*/
public:
    virtual void clear_results();

    virtual void retrieve_EN_results() = 0;
};

template <>
struct TypeTraits<NetworkElement>
{
    static constexpr const char* name = "NetworkElement";
    static constexpr const int code = 11;
    static constexpr bool is_EN_complete = false;
};

} // namespace bevarmejo::wds

#endif // BEVARMEJOLIB__WDS_ELEMENTS__NETWORK_ELEMENT_HPP
