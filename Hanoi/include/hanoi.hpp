//
//  hanoi.hpp
//  modelHanoi
//
//  Created by Dennis Zanutto on 09/06/23.
//

#ifndef hanoi__hanoi_hpp
#define hanoi__hanoi_hpp

#include <stdio.h>
#include <vector>

#include "classes/water_distribution_system.hpp"

namespace bevarmejo {
class Hanoi : public bevarmejo::WaterDistributionSystem {
    
public:
    // Run a simulation of the Hanoi problem and return the pressures at all nodes
    std::vector<double> evaluate() const;
};

} /* namespace bevarmejo */
#endif /* hanoi__hanoi_hpp */
