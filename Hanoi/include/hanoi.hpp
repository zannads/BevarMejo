//
//  hanoi.hpp
//  modelHanoi
//
//  Created by Dennis Zanutto on 09/06/23.
//

#ifndef HANOI__HANOI_HPP
#define HANOI__HANOI_HPP

#include <stdio.h>
#include <vector>

#include "bevarmejo/wds/water_distribution_system.hpp"

namespace bevarmejo {
class Hanoi : public bevarmejo::WaterDistributionSystem {
    
public:
    // Run a simulation of the Hanoi problem and return the pressures at all nodes
    std::vector<double> evaluate() const;
};

} /* namespace bevarmejo */
#endif /* HANOI__HANOI_HPP */
