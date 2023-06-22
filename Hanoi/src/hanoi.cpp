//
//  hanoi.cpp
//  modelHanoi
//
//  Created by Dennis Zanutto on 09/06/23.
//

#include "hanoi.hpp"

hanoi::hanoi(){
    // Empty constructor.
    // EPANET project is not initialized
    ph = nullptr;
}

hanoi::~hanoi(){
}

void hanoi::clear(){
    // do I need to close before delete? or is it unnecessary?
    printf("deleting hanoi\n");
    EN_deleteproject(ph);
}

void hanoi::set_inpFile(const char* inpFile){
    inpFilename = inpFile;
    return;
}


void hanoi::init(){
    EN_createproject(&ph);
    
    int error = EN_open(ph, inpFilename, "", "");
    if (error>100)
        printf("File not found\n");
    
    return;
}

std::vector<double> hanoi::evaluate() const{
    // check it can be done
    
    // Simulation
    
    // Open the Hydraulic engine
    int error = EN_openH(ph);
    if (error>100)
        throw std::runtime_error("Hydraulics not opened.");
    
    // Initialize the Hydraulic engine
    // Don't save hydraulics, Re-initialize flows
    error = EN_initH(ph, EN_INITFLOW);
    if (error>100)
        throw std::runtime_error("Hydraulics not initialised.");
    
    // Run hydraulics
    long t{0};
    error = EN_runH(ph, &t);
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
        
        error = EN_getlinkindex(ph, juncFakeName.c_str(), &juncIdx);
        
        error = EN_getnodevalue(ph, juncIdx, EN_PRESSURE, &nodesPres[i]);
    }
    
    error = EN_closeH(ph);
    
    return nodesPres;
}
