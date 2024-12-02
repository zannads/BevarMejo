#ifndef BEVARMEJOLIB__WDS_ELEMENTS__CURVE_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__CURVE_HPP

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/element.hpp"

namespace bevarmejo::wds
{

class Curve : public Element
{
    // WDS Curve
    /*******************************************************************************
     * The wds::Curve class represents a curve describing some property of the 
     * elements in the network (e.g. pump efficiency, valve headloss, etc.).
     * Curve is a common class to gather all the different types of curves that
     * can be used in the network.
    ******************************************************************************/

/*------- Member types -------*/
public:
    using inherited = Element;
private:
    friend class WaterDistributionSystem;

/*------- Member objects -------*/
protected:

/*------- Member functions -------*/
// (constructor)
protected:
    Curve() = delete;
    Curve(const WaterDistributionSystem& wds, const EN_Name_t& name); // Constructor
    
// (destructor)
public:
    virtual ~Curve() = default;

// clone()
public:

/*------- Operators -------*/
// operator=
public:

/*------- Element access -------*/
public:

/*------- Capacity -------*/
public:

/*------- Modifiers -------*/
public:
    void retrieve_EN_index() override final;

}; // class Curve

template <>
struct TypeTraits<Curve>
{
    static constexpr const char* name = "Curve";
    static constexpr unsigned int code = 21;
    static constexpr bool is_EN_complete = false;
};

// Default trait for the curve (name and type (code))
template <typename X, typename Y>
class SpecificCurve;

template <typename X, typename Y>
struct TypeTraits<SpecificCurve<X, Y>>
{
    static constexpr const char* name = "SpecificCurve";
    static constexpr unsigned int code = 21;
    static constexpr bool is_EN_complete = false;
};


template <typename X, typename Y>
class SpecificCurve : public Curve
{
    // WDS SpecificCurve
    /*******************************************************************************
     * The wds::SpecificCurve class represents a curve describing some property of 
     * the elements in the network (e.g. pump efficiency, valve headloss, etc.).
    ******************************************************************************/

/*------- Member types -------*/
public:
    using self_type = SpecificCurve<X, Y>;
    using self_traits = TypeTraits<self_type>;
    using inherited = Curve;
    using Container = std::map<X, Y>;
private:
    friend class WaterDistributionSystem;

/*------- Member objects -------*/
protected:
    Container m__curve; // The curve data

/*------- Member functions -------*/
// (constructor)
protected:
    SpecificCurve() = delete;
    SpecificCurve(const WaterDistributionSystem& wds, const EN_Name_t& name) : 
        inherited(wds, name),
        m__curve()
    {
        static_assert(self_traits::is_EN_complete, "You are attempting to create an invalid curve.");
    }
    template <typename... Args>
    SpecificCurve(const WaterDistributionSystem& wds, const EN_Name_t& name, Args&&... args) :
        inherited(wds, name),
        m__curve(std::forward<Args>(args)...)
    {
        static_assert(self_traits::is_EN_complete, "You are attempting to create an invalid curve.");
    }

// (destructor)
public:
    virtual ~SpecificCurve() = default;

// clone()
public:

/*------- Operators -------*/
// operator=
public:

/*------- Element access -------*/
public:
    const char* type_name() const override final
    {
        return self_traits::name;
    }
    unsigned int type_code() const override final
    {
        return self_traits::code;
    }

    Container& curve() {return m__curve;}
    const Container& curve() const {return m__curve;}

    Y& at(const X& x) {return m__curve.at(x);}
    const Y& at(const X& x) const {return m__curve.at(x);}
    Y& operator[](const X& x) {return m__curve[x];}
    // Y& operator[](X&& x) {return m__curve[std::move(x)];} TODO: look into this

/*--- Iterators ---*/
public:
    typename Container::iterator begin() noexcept {return m__curve.begin();}
    typename Container::const_iterator begin() const noexcept {return m__curve.begin();}
    typename Container::const_iterator cbegin() const noexcept {return m__curve.cbegin();}
    typename Container::iterator end() noexcept {return m__curve.end();}
    typename Container::const_iterator end() const noexcept {return m__curve.end();}
    typename Container::const_iterator cend() const noexcept {return m__curve.cend();}
    typename Container::reverse_iterator rbegin() noexcept {return m__curve.rbegin();}
    typename Container::const_reverse_iterator rbegin() const noexcept {return m__curve.rbegin();}
    typename Container::const_reverse_iterator crbegin() const noexcept {return m__curve.crbegin();}
    typename Container::reverse_iterator rend() noexcept {return m__curve.rend();}
    typename Container::const_reverse_iterator rend() const noexcept {return m__curve.rend();}
    typename Container::const_reverse_iterator crend() const noexcept {return m__curve.crend();}

/*------- Capacity -------*/
    bool empty() const noexcept {return m__curve.empty();}
    typename Container::size_type size() const noexcept {return m__curve.size();}
    typename Container::size_type max_size() const noexcept {return m__curve.max_size();}

/*------- Modifiers -------*/
    void clear() noexcept {m__curve.clear();}
    std::pair<typename Container::iterator, bool> insert(const typename Container::value_type& val) {return m__curve.insert(val);}

    void retrieve_EN_properties() override final
    {
        assert(m__en_index > 0 && "The index is not set.");
        assert(m__wds.ph() != nullptr && "The EPANET project handle is not set.");

        int n_points;
        int errorcode = EN_getcurvelen(ph, this->index(), &n_points);
        assert(errorcode <= 100);

        for (auto i = 1; i <= n_points; ++i)
        {
            double x, y;
            errorcode = EN_getcurvevalue(ph, this->index(), i, &x, &y);
            assert(errorcode <= 100);

            _curve_.emplace(x, y);
        }
    }

/*------- Lookup -------*/
    typename Container::size_type count(const X& x) const {return m__curve.count(x);}
    typename Container::iterator find(const X& x) {return m__curve.find(x);}
    typename Container::const_iterator find(const X& x) const {return m__curve.find(x);}
    bool contains(const X& x) const {return m__curve.find(x) != m__curve.end();}

}; // class SpecificCurve

} // namespace bevarmejo::wds

#endif // BEVARMEJOLIB__WDS_ELEMENTS__CURVE_HPP
    