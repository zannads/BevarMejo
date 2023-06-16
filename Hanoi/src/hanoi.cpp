//
//  hanoi.cpp
//  modelHanoi
//
//  Created by Dennis Zanutto on 09/06/23.
//

#include "hanoi.hpp"

hanoi::hanoi(){
    EN_createproject(&ph);
    inpFilename = "";
}

hanoi::hanoi(std::string inpFile){
    EN_createproject(&ph);
    inpFilename = inpFile;
    
    int ec = EN_open(ph, inpFilename.c_str(), "", "");
    if (ec>100)
        printf("File not found\n");
}

hanoi::~hanoi(){
    // do I need to close before delete? or is it unnecessary?
    EN_deleteproject(ph);
}

void hanoi::set_inpFile(std::string inpFile){
    inpFilename = inpFile;
    return;
}


void hanoi::init(){
    
    int error = EN_open(ph, inpFilename.c_str(), "", "");
    if (error>100)
        printf("File not found\n");
    
    return;
}

std::vector<double> hanoi::evaluate(){
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
    
    int nNodes;
    error = EN_getcount(ph, EN_NODECOUNT, &nNodes);
    
    long t{0};
    error = EN_runH(ph, &t);
    if (error>100)
        throw std::runtime_error("Simulation failed.");
    
    std::vector<double> nodesPres(nNodes,0);
    for (int nodeIdx = 0; nodeIdx<nNodes; ++nodeIdx) {
        error = EN_getnodevalue(ph, nodeIdx, EN_PRESSURE, &nodesPres[nodeIdx]);
    }
    
    error = EN_closeH(ph);
    
    return nodesPres;
}
