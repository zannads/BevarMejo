/*----------------------
Author: DennisZ
descr: Model of the Hanoi problem.
----------------------*/

#include "model_hanoi.hpp"

model_hanoi::model_hanoi(){
	//TODO
    Hanoi.set_inpFile("/Users/denniszanutto/data/BevarMejoData/networks/hanoi.inp");
}

// Number of objective functions
double model_hanoi::get_nobj() const{
	return 1;
}

// Number of equality constraints
double model_hanoi::get_nec() const{
	return 0;
}

// Number of INequality constraints
double model_hanoi::get_nic() const{
	return 0;
}

// Number of integer decision variables 
double model_hanoi::get_nix() const{
	//TODO
	return 1;
}

    // Number of continous decision variables is automatically retrieved with get_bounds() and get_nix()

    // Implementation of the objective function.
    std::vector<double> model_hanoi::fitness(const std::vector<double>& dv) {
        std::vector<double> fit(1,0);
        
    	// Load the file
        Hanoi.init();
        
        // Fit the solution
        applySolution(dv);

    	// Run the simulation
        std::vector<double> nodePressures = Hanoi.evaluate();

    	// Get the result of the functions defined on it
        fit[0] = cost();
        // fit[1] = minPress();
        
    	// Thank you Hanoi
        EN_close(Hanoi.ph);
    	return fit;
    }

    // Implementation of the box bounds.
    std::pair<std::vector<double>, std::vector<double>> model_hanoi::get_bounds() const{
    	return {{0.}, {1.}};
    }

void model_hanoi::applySolution(const std::vector<double>& dv){

    int error;
    
    int nLinks;
    error = EN_getcount(Hanoi.ph, EN_LINKCOUNT, &nLinks);
    if (error > 100)
        throw std::runtime_error("Number of links not retrieved.");
    
    
    for( int i=0; i<dv.size(); ++i){
        
        
    }
}


double model_hanoi::cost(){
    // C_{ij} = 1.1 * D_{ij}^1.5 * L_{ij}
    
    // Get number of link
    int error;
    int nLinks;
    
    error = EN_getcount(Hanoi.ph, EN_LINKCOUNT, &nLinks);
    if (error > 100)
        throw std::runtime_error("Number of links not retrieved.");
    
    double totalcost{0.}, diam{0.}, leng{0.};
    
    int linkType;
    int linkIdx;
    // Per each link, if it is a pipe add to total cost
    for (int link = 0; link < nLinks; ++link){
        
        error = EN_getlinkindex(Hanoi.ph, "1", &linkIdx);
        
        error = EN_getlinktype(Hanoi.ph, linkIdx, &linkType);
        if (error > 100)
            throw std::runtime_error("Link's type not retrieved.");
        
        if (linkType == EN_PIPE){
            // Get diameter and length
            error = EN_getlinkvalue(Hanoi.ph, linkIdx, EN_DIAMETER, &diam);
            error = EN_getlinkvalue(Hanoi.ph, linkIdx, EN_LENGTH, &leng);
            
            // Compute
            totalcost += pow(diam, 1.5)*leng;
        }
    }
    totalcost *= 1.1;
        
    return totalcost;
}
