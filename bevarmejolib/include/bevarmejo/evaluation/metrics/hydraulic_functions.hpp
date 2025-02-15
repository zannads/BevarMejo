#pragma once

#include "bevarmejo/wds/water_distribution_system.hpp"

namespace bevarmejo::eval::metrics
{

class HeadDeficiency
{
/*------- Member objects -------*/
private:
    double m__min_head = 14.0;
    bool m__relative = false;

/*------- Member functions -------*/
public:
    HeadDeficiency() = default;
    HeadDeficiency(const double min_head, const bool relative);
    HeadDeficiency(const HeadDeficiency& other) = default;
    HeadDeficiency(HeadDeficiency&& other) noexcept = default;
    HeadDeficiency& operator=(const HeadDeficiency& other) = default;
    HeadDeficiency& operator=(HeadDeficiency&& other) noexcept = default;
    ~HeadDeficiency() = default;

/*------- Compute method -------*/
public:
    wds::aux::QuantitySeries<double> operator()(const wds::Junction& a_junction) const;
};    


} // namespace bevarmejo::eval::metrics
