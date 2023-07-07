/*----------------------
Author: DennisZ
descr: Model of the Hanoi problem, header file. 
----------------------*/

#ifndef HANOI_MODEL_HANOI_HPP
#define HANOI_MODEL_HANOI_HPP

#include <iostream>
#include <filesystem>
#include <memory>
#include <utility>
#include <vector>

#include "hanoi.hpp"

namespace fsys = std::filesystem;
namespace bevarmejo {

struct diamData{
    double inches;
    double millimeters;
    double inches_cost;
};

class ModelHanoi{
public:
    ModelHanoi();
    
    ModelHanoi(fsys::path settings_file);
    
    ModelHanoi(const ModelHanoi &src);
    
    ModelHanoi(ModelHanoi &&src);
    
    //~ModelHanoi();// for now nothing specific to destory in here
    
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
    
    //
    std::string get_extra_info() const;
    
    // Mandatory public functions necessary for the optimization algorithm:
    // Implementation of the objective function.
    std::vector<double> fitness(const std::vector<double>& dv) const;
    
    // Implementation of the box bounds.
    std::pair<std::vector<double>, std::vector<double>> get_bounds() const;
     
protected:
    // Variable containing the EPANET handler(s) and all the related network information.
    std::shared_ptr<bevarmejo::Hanoi> _hanoi_;
    
    // Variable containing the available diameters for the decision variable
    std::vector<diamData> _av_diams_;
    
    // Objective function
    double cost() const;
    
    // Constraint on minimum pressure
    double minPressure(std::vector<double>& pressures) const;
    
    // Modify the EPANET object with the network applying the decision variable
    void applySolution(const std::vector<double>& dv) const;
    
    
    // Ad-hoc function to load the file containing the av_diams.
    void load_availDiam(std::string filename);
};

} /* namespace bevarmejo */

#endif /* HANOI_MODEL_HANOI_HPP */
