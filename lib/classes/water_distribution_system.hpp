//
//  water_distribution_system.hpp
//  hanoiOptimization
//
//  Created by Dennis Zanutto on 04/07/23.
//

#ifndef lib_classes__water_distribution_system_hpp
#define lib_classes__water_distribution_system_hpp

#include <stdio.h>
//#include <filesystem>
#include <string>

#include "epanet2_2.h"

namespace bevarmejo {

class WaterDistributionSystem{
public:
    // Handler for the project.
    // Public because I may want to modify it (e.g., apply a decision vector).
    // it is just faster than doing create an interface. I will be careful.
    mutable EN_Project ph_;
    
protected:
    // Path to the inp file from which the project will be uploaded.
    std::string _inp_filename_;
    
public:
    
    // Default constructor
    WaterDistributionSystem();
    
    // Constructor from .inp file as a path reference
   // WaterDistributionSystem(const std::filesystem::path& inp_filename);
    
    // Constructor from .inp file
    WaterDistributionSystem(std::string inp_filename);
    
    // Copy constructor
    // this is not actually a copy constructor but rather a reinitialization one.
    WaterDistributionSystem(const WaterDistributionSystem &src);
    
    // Move constructor
    WaterDistributionSystem(WaterDistributionSystem &&src);
    
    // Copy Assignement operator
    WaterDistributionSystem& operator=(const WaterDistributionSystem& rhs);
    
    // Move Assignement operator
    WaterDistributionSystem& operator=(WaterDistributionSystem&& rhs);
    
    ~WaterDistributionSystem();
    
    // Equivalent to constuctor from .inp file
    void init();
    
    // Set/get _inpFilename
    //void set_inpfile(const std::string& inp_filename);
    void set_inpfile(const std::string inp_filename);
    std::string get_inpfile() const;
    
    //Run and stuff
    
};

} /* namespace bevarmejo */

#endif /* lib_classes__water_distribution_system_hpp */