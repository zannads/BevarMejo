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
    
    hanoi(std::string inpFile);
    
     ~hanoi();
    
    void init();
    
    void set_inpFile(std::string inpFile);
    
    // Run a simulation of the Hanoi problem and return the pressures att all nodes 
    std::vector<double> evaluate();
    
    // Handler for the project.
    EN_Project ph;
protected:
    
    
    
    // Path to the inp file from which the project will be uploaded.
    std::string inpFilename;
    
    
};

#endif /* hanoi_hpp */
