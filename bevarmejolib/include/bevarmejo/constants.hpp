//
//  constants.hpp
//  bevarmejo cpp library
//
//  Created by Dennis Zanutto on 12/10/23.
//

#ifndef BEVARMEJOLIB__CONSTANTS_HPP
#define BEVARMEJOLIB__CONSTANTS_HPP

#include <cmath>

namespace bevarmejo {

constexpr double water_density_kg_per_m3 = 1000.0;
constexpr double gravity_m_per_s2 = 9.81;
constexpr double water_specific_weight_N_per_m3 = water_density_kg_per_m3 * gravity_m_per_s2;

constexpr int k__days_ina_year = 365;
constexpr int k__sec_per_hour = 3600;

constexpr double k__pi = M_PI;

} // namespace bevamejo
#endif // BEVARMEJOLIB__CONSTANTS_HPP