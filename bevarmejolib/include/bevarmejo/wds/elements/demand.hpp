
#ifndef BEVARMEJOLIB__WDS_ELEMENTS__DEMAND_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__DEMAND_HPP

#include <functional>
#include <memory>
#include <string>

#include "bevarmejo/wds/elements/variable.hpp"
#include "bevarmejo/wds/elements/temporal.hpp"

#include "bevarmejo/wds/elements/pattern.hpp"

namespace bevarmejo {   
namespace wds {

static const std::string L_DEMAND = "Demand";

class Demand {

/*--- Attributes ---*/
private:
    vars::var_real _base_dem_;
    std::shared_ptr<Pattern> _pattern_;
    std::string _category_;

/*--- Constructors ---*/
public:
    Demand() :  _base_dem_(vars::L_M3_PER_S, 0.0), 
                _pattern_(nullptr),
                _category_("") {}

    Demand(const std::string& a_category) :
        _base_dem_(vars::L_M3_PER_S, 0.0),
        _pattern_(nullptr),
        _category_(a_category) {}

    Demand(const std::string& a_category, const double a_base_dem, const std::shared_ptr<Pattern> a_pattern) : 
        _base_dem_(vars::L_M3_PER_S, a_base_dem), 
        _pattern_(a_pattern),
        _category_(a_category) {}
    
    Demand(const Demand& other) : 
        _base_dem_(other._base_dem_), 
        _pattern_(other._pattern_),
        _category_(other._category_) {}

    Demand(Demand&& rhs) noexcept : 
        _base_dem_(std::move(rhs._base_dem_)), 
        _pattern_(std::move(rhs._pattern_)),
        _category_(std::move(rhs._category_)) {}

    Demand& operator=(const Demand& rhs) {
        if (this != &rhs) {
            _base_dem_ = rhs._base_dem_;
            _pattern_ = rhs._pattern_;
            _category_ = rhs._category_;
        }
        return *this;
    }

    Demand& operator=(Demand&& rhs) noexcept {
        if (this != &rhs) {
            _base_dem_ = std::move(rhs._base_dem_);
            _pattern_ = std::move(rhs._pattern_);
            _category_ = std::move(rhs._category_);
        }
        return *this;
    }

    virtual ~Demand() { _pattern_.reset(); } // release ownership 

/*--- Getters and setters ---*/
public:
    vars::var_real& base_demand() { return _base_dem_; }
    void base_demand(double bd) { _base_dem_.value(bd); }

    std::shared_ptr<Pattern> who_is_yr_pattern() const {return _pattern_;}
    void change_pattern(std::shared_ptr<Pattern> a_pattern) {_pattern_ = a_pattern;}
    const std::string& category() const {return _category_;}
    void category(const std::string& a_category) {_category_ = a_category;}

    //const double when(const long time) {return _base_dem_.value() * _pattern_->when(time);}

    const std::string& id() {return _pattern_->id(); }

}; // class Demand

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__DEMAND_HPP
