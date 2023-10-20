
#ifndef BEVARMEJOLIB__WDS_ELEMENTS__DEMAND_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__DEMAND_HPP

#include <memory>
#include <string>

#include "bevarmejo/wds/elements/variable.hpp"
#include "bevarmejo/wds/elements/pattern.hpp"

namespace bevarmejo {   
namespace wds {

static const std::string L_DEMAND = "demand";

class demand {

private:
    vars::var_real _base_dem_;
    std::shared_ptr<pattern> _pattern_;

public:
    demand() : _base_dem_(vars::L_M3_PER_S, 0.0), _pattern_(std::make_shared<pattern>()) {}
    
    demand(const demand& other) : _base_dem_(other._base_dem_), _pattern_(other._pattern_) {}

    demand(demand&& rhs) noexcept : _base_dem_(std::move(rhs._base_dem_)), _pattern_(std::move(rhs._pattern_)) {}

    demand& operator=(const demand& rhs) {
        if (this != &rhs) {
            _base_dem_ = rhs._base_dem_;
            _pattern_ = rhs._pattern_;
        }
        return *this;
    }

    demand& operator=(demand&& rhs) noexcept {
        if (this != &rhs) {
            _base_dem_ = std::move(rhs._base_dem_);
            _pattern_ = std::move(rhs._pattern_);
        }
        return *this;
    }

    virtual ~demand() { _pattern_.reset(); } // release ownership 

    vars::var_real& base_demand() { return _base_dem_; }
    void base_demand_val(double bd) { _base_dem_.value(bd); }

    std::shared_ptr<pattern> who_is_yr_pattern() const {return _pattern_;}
    void change_pattern(std::shared_ptr<pattern> a_pattern) {_pattern_ = a_pattern;}

    const double when(const long time) {return _base_dem_.value() * _pattern_->when(time);}

    const std::string& id() {return _pattern_->id(); }

}; // class demand

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__DEMAND_HPP