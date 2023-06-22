/*----------------------
Author: DennisZ
descr: Model of the Hanoi problem, header file. 
----------------------*/

#ifndef model_hanoi_hpp
#define model_hanoi_hpp


#include <utility>
#include <vector>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numeric>

#include "pugixml.hpp"

#include <boost/filesystem.hpp>

#include "hanoi.hpp"

class model_hanoi{
public:
	model_hanoi();
    // ~model_hanoi(); for now nothing specific to destory in here
    
	// Other public functions for the optimization algorithm:
	// Number of objective functions
    std::vector<double>::size_type get_nobj() const;

    // Number of equality constraints
    std::vector<double>::size_type get_nec() const;

    // Number of INequality constraints
    std::vector<double>::size_type get_nic() const;

    // Number of integer decision variables 
    std::vector<double>::size_type get_nix() const;

    // Number of continous decision variables is automatically retrieved with get_bounds() and get_nix()
    
    // Mandatory public functions necessary for the optimization algorithm:
    // Implementation of the objective function.
    std::vector<double> fitness(const std::vector<double>& dv) const;

    // Implementation of the box bounds.
    std::pair<std::vector<double>, std::vector<double>> get_bounds() const;
    
    // Initialize the object on which it is called.
    void upload_settings(std::string settingsFile);
    void clear();
    
protected:
    // Variable containing the EPANET handler(s) and all the related network information.
    hanoi Hanoi;
    
    // Variable containing the available diameters for the decision variable
    std::vector<std::pair<double, double>> av_diams;
    
    
    // Objective function
    double cost() const;
    
    // Constraint on minimum pressure
    double minPressure(std::vector<double>& pressures) const;
    
    // Modify the EPANET object with the network applying the decision variable
    void applySolution(const std::vector<double>& dv) const;
    
    
    // Ad-hoc function to load the file containing the av_diams.
    void load_availDiam(const char * filename);
};

#endif /* hanoi_hpp */
