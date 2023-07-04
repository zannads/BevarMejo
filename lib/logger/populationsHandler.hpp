//
//  populationsHandler.hpp
//  BèvarMéjo
//
//  Created by Dennis Zanutto on 26/06/23.
//

#ifndef populationsHandler_hpp
#define populationsHandler_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

#include <filesystem>

#include <pagmo/population.hpp>

namespace bevarmejo {

template<typename T>
void writeParamCouple(std::ofstream& ofs,  const char*  paramName, T paramValue){
    std::string sJunk{paramName};
    sJunk += std::to_string(paramValue);
    ofs <<sJunk <<std::endl;
}


void writeHeader(std::ofstream& ofs, pagmo::population& pop);

void saveFinalPopulation(std::filesystem::path& outFilename, pagmo::population& pop);

void saveRuntimePopulation(std::filesystem::path& runtFilename, pagmo::population& pop);
};

#endif /* populationsHandler_hpp */
