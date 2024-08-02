#ifndef BEVARMEJOLIB__WDS_ELEMENTS__CURVE_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__CURVE_HPP

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/element.hpp"

namespace bevarmejo {
namespace wds {

class Curve : public Element {
    // WDS Curve
    /*******************************************************************************
     * The wds::Curve class represents a curve describing some property of the 
     * elements in the network (e.g. pump efficiency, valve headloss, etc.).
    ******************************************************************************/

/*--- Constructors ---*/
public:
    using inherited= Element;
    Curve() = delete;
    Curve(const std::string& id) :
        inherited(id) {}
    Curve(const Curve& other) :
        inherited(other) {}
    Curve(Curve&& rhs) noexcept :  
        inherited(std::move(rhs)) {}
    Curve& operator=(const Curve& rhs) {
        if (this != &rhs) {
            inherited::operator=(rhs);
        }
        return *this;
    }
    Curve& operator=(Curve&& rhs) noexcept {
        if (this != &rhs) {
            inherited::operator=(std::move(rhs));
        }
        return *this;
    }
    virtual ~Curve() {}
    std::unique_ptr<Curve> clone() const { return std::unique_ptr<Curve>(this->__clone()); }

private:
    virtual Curve* __clone() const = 0;  

/*--- Pure virtual methods override---*/
public:
    void retrieve_index(EN_Project ph) override {
        int curve_idx;
        int errco = EN_getcurveindex(ph, id().c_str(), &curve_idx);
        this->index(curve_idx);
    }
}; // class Curve

template <typename X, typename Y>
class SpecificCurve : public Curve {
    // WDS SpecificCurve
    /*******************************************************************************
     * The wds::SpecificCurve class represents a curve describing some property of 
     * the elements in the network (e.g. pump efficiency, valve headloss, etc.).
    ******************************************************************************/
public:
    using inherited= Curve;
    using Container= std::map<X, Y>;
/*--- Attributes ---*/
protected:
    Container _curve_;

/*--- Constructors ---*/
public:
    SpecificCurve() = delete;
    SpecificCurve(const std::string& id) :
        inherited(id),
        _curve_() {}
    SpecificCurve(const SpecificCurve& other) :
        inherited(other),
        _curve_(other._curve_) {}
    SpecificCurve(SpecificCurve&& other) noexcept :  
        inherited(std::move(other)),
        _curve_(std::move(other._curve_)) {}
    SpecificCurve& operator=(const SpecificCurve& rhs) {
        if (this != &rhs) {
            inherited::operator=(rhs);
            _curve_ = rhs._curve_;
        }
        return *this;
    }
    SpecificCurve& operator=(SpecificCurve&& rhs) noexcept {
        if (this != &rhs) {
            inherited::operator=(std::move(rhs));
            _curve_ = std::move(rhs._curve_);
        }
        return *this;
    }
    virtual ~SpecificCurve() { _curve_.clear(); }

/*--- Element Access ---*/
public:
    Container& curve() {return _curve_;}
    const Container& curve() const {return _curve_;}

    Y& at(const X& x) {return _curve_.at(x);}
    const Y& at(const X& x) const {return _curve_.at(x);}
    Y& operator[](const X& x) {return _curve_[x];}
    // Y& operator[](X&& x) {return _curve_[std::move(x)];} TODO: look into this

/*--- Iterators ---*/
public:
    typename Container::iterator begin() noexcept {return _curve_.begin();}
    typename Container::const_iterator begin() const noexcept {return _curve_.begin();}
    typename Container::const_iterator cbegin() const noexcept {return _curve_.cbegin();}
    typename Container::iterator end() noexcept {return _curve_.end();}
    typename Container::const_iterator end() const noexcept {return _curve_.end();}
    typename Container::const_iterator cend() const noexcept {return _curve_.cend();}
    typename Container::reverse_iterator rbegin() noexcept {return _curve_.rbegin();}
    typename Container::const_reverse_iterator rbegin() const noexcept {return _curve_.rbegin();}
    typename Container::const_reverse_iterator crbegin() const noexcept {return _curve_.crbegin();}
    typename Container::reverse_iterator rend() noexcept {return _curve_.rend();}
    typename Container::const_reverse_iterator rend() const noexcept {return _curve_.rend();}
    typename Container::const_reverse_iterator crend() const noexcept {return _curve_.crend();}

/*--- Capacity ---*/
    bool empty() const noexcept {return _curve_.empty();}
    typename Container::size_type size() const noexcept {return _curve_.size();}
    typename Container::size_type max_size() const noexcept {return _curve_.max_size();}

/*--- Modifiers ---*/
    void clear() noexcept {_curve_.clear();}
    std::pair<typename Container::iterator, bool> insert(const typename Container::value_type& val) {return _curve_.insert(val);}

/*--- Lookup ---*/
    typename Container::size_type count(const X& x) const {return _curve_.count(x);}
    typename Container::iterator find(const X& x) {return _curve_.find(x);}
    typename Container::const_iterator find(const X& x) const {return _curve_.find(x);}
    bool contains(const X& x) const {return _curve_.find(x) != _curve_.end();}

    //TODO: finish all definitions

/*--- Pure virtual methods override ---*/
public:
    /*--- Properties ---*/
private:
    virtual void __retrieve_EN_properties(EN_Project ph) override{
        assert(this->index()>0);
        int n_points;
        int errco = EN_getcurvelen(ph, this->index(), &n_points);
        assert(errco<=100);
        for (int i=1; i<=n_points; ++i) {
            double __x, __y;
            errco = EN_getcurvevalue(ph, this->index(), i, &__x, &__y);
            assert(errco<=100);

            _curve_.insert(std::make_pair(X{__x}, Y{__y}));
        }
    }

}; // class SpecificCurve

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__CURVE_HPP
    