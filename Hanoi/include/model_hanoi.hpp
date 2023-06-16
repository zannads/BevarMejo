/*----------------------
Author: DennisZ
descr: Model of the Hanoi problem, header file. 
----------------------*/

#ifndef model_hanoi_hpp
#define model_hanoi_hpp


#include <utility>
#include <vector>
#include <cmath>

#include "hanoi.hpp"

class model_hanoi{
public:
	model_hanoi();
	//virtual ~model_hanoi();

	// Other public functions necessary for the optimization algorithm:
	// Number of objective functions
    double get_nobj() const;

    // Number of equality constraints
    double get_nec() const;

    // Number of INequality constraints
    double get_nic() const;

    // Number of integer decision variables 
    double get_nix() const;

    // Number of continous decision variables is automatically retrieved with get_bounds() and get_nix()

    // Implementation of the objective function.
    std::vector<double> fitness(const std::vector<double>& dv);

    // Implementation of the box bounds.
    std::pair<std::vector<double>, std::vector<double>> get_bounds() const;
    hanoi Hanoi;
protected:
    
    double cost();
    
    void applySolution(const std::vector<double>& dv);
};

#endif /* hanoi_hpp */
