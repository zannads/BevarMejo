//
//  hanoi.cpp
//  modelHanoi
//
//  Created by Dennis Zanutto on 09/06/23.
//


#include <stdio.h>
#include <stdexcept>
#include <string>
#include <vector>

#include "epanet2_2.h"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "hanoi.hpp"

namespace bevarmejo {

std::vector<double> Hanoi::evaluate() const{
    // check it can be done
    
    // Simulation
    
    // Open the Hydraulic engine
    int error = EN_openH(ph_);
    if (error>100)
        throw std::runtime_error("Hydraulics not opened.");
    
    // Initialize the Hydraulic engine
    // Don't save hydraulics, Re-initialize flows
    error = EN_initH(ph_, EN_INITFLOW);
    if (error>100)
        throw std::runtime_error("Hydraulics not initialised.");
    
    // Run hydraulics
    long t{0};
    error = EN_runH(ph_, &t);
    if (error>100)
        throw std::runtime_error("Simulation failed.");
    
    // Save data, it's easy, pressure of all junctions, i.e., nodes from id 2 to 32
    // since ids are only simple numbers I transform them directly from int to string
    int nJun = 31;
    int juncIdx{0};
    std::string juncFakeName;
    std::vector<double> nodesPres(31,0);
    for (int i = 0; i<nJun; ++i) {
        juncFakeName = std::to_string(i+2);
        
        error = EN_getnodeindex(ph_, juncFakeName.c_str(), &juncIdx);
        
        error = EN_getnodevalue(ph_, juncIdx, EN_PRESSURE, &nodesPres[i]);
    }
    
    error = EN_closeH(ph_);
    
    return nodesPres;
}

} /* namespace bevarmejo */
