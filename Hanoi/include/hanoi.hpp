//
//  hanoi.hpp
//  modelHanoi
//
//  Created by Dennis Zanutto on 09/06/23.
//

#ifndef hanoi_hpp
#define hanoi_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <stdexcept>

#include "epanet2_2.h"

class hanoi{

public:
    hanoi();
    
     ~hanoi();
    
    void init();
    void clear();
    
    void set_inpFile(std::string inpFile);
    
    // Run a simulation of the Hanoi problem and return the pressures at all nodes 
    std::vector<double> evaluate() const;
    
    // Handler for the project.
    // Public because I may want to modify it (e.g., apply a decision vector).
    // it is just faster than doing create an interface. I will be careful.
    mutable EN_Project ph;
    
protected:
    
    // Boolean to keep track if ph has been initialized or not.
    // I don't know why but I can't see it automatically using ph.
    bool phStatus;
    
    // Path to the inp file from which the project will be uploaded.
    std::string inpFilename;
};

#endif /* hanoi_hpp */
