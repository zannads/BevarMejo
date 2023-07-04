//
//  populationsHandler.cpp
//  BèvarMéjo
//
//  Created by Dennis Zanutto on 26/06/23.
//

#include "populationsHandler.hpp"

namespace bevarmejo {

template<>
void writeParamCouple<std::vector<double>> (std::ofstream& ofs, const char* paramName, std::vector<double> paramValue){
    std::string sJunk{paramName}; sJunk += "[";

    for( unsigned int i = 0; i<(paramValue.size()-1); ++i){
        sJunk += std::to_string(paramValue.at(i)); sJunk += ",";
    }
    sJunk += std::to_string(paramValue.back()); sJunk += "]";
    
    ofs <<sJunk <<std::endl;
}

void writeHeader(std::ofstream& ofs, pagmo::population& pop){
    
    std::string sJunk{};
    
    sJunk ="Current time: ";
    const auto now = std::chrono::system_clock::now();
    const time_t t_c = std::chrono::system_clock::to_time_t(now);
    sJunk += ctime(&t_c);
    ofs <<sJunk <<std::endl;
    
    //writeParamCouple(ofs, "Problem name: ", pop.get_problem().get_name());
    sJunk = "Problem name: ";
    sJunk += pop.get_problem().get_name();
    ofs << sJunk <<std::endl;
    
    writeParamCouple(ofs, "\tGlobal dimension: ", pop.get_problem().get_nx());
    
    writeParamCouple(ofs, "\tFitness dimension: ", pop.get_problem().get_nf());
    
    writeParamCouple(ofs, "\tNumber of objectives: ", pop.get_problem().get_nobj());
    
    writeParamCouple(ofs, "\tLower bounds: ", pop.get_problem().get_lb());
    
    writeParamCouple(ofs, "\tUpper bounds: ", pop.get_problem().get_ub());
    
    writeParamCouple(ofs, "\tFitness evaluations: ", pop.get_problem().get_fevals());
}

void saveFinalPopulation(std::filesystem::path& outFilename, pagmo::population& pop){

    /*
    if (!exists(outFilename) || !is_regular_file(outFilename)){
        filesystem::path default_outFilename = "finalPop_";
        default_outFilename += to_string( pop.get_seed() );
        default_outFilename += ".txt";
        
        cout << "Output file not existing or not a file (e.g., a directory)!" <<endl;
        cout << "Saving output on file with standard name: " <<default_outFilename.c_str() <<endl;
        
        outFilename = default_outFilename;
    }*/
    
    std::ofstream ofs;
    ofs.open(outFilename);
    if(!ofs.is_open()){
        std::cout <<"Problem with outfile" <<std::endl;
        std::cout << pop;
    }
    
    writeHeader(ofs, pop);

    // Write body
    ofs <<std::endl;
    writeParamCouple(ofs, "Population size: ", pop.size());
    
    ofs <<std::endl << "List of individuals: " <<std::endl;
    auto ids = pop.get_ID();
    auto dvs = pop.get_x();
    auto fis = pop.get_f();
    std::string sJunk{};
    for( unsigned int i = 0; i<pop.size(); ++i){
        sJunk = "#"; sJunk += std::to_string(i); sJunk += ":\n";
        ofs << sJunk;
        
        writeParamCouple(ofs, "\tID:\t", ids.at(i));
        
        writeParamCouple(ofs, "\tDecision vector:\t", dvs.at(i));
        
        writeParamCouple(ofs, "\tFitness vector:\t\t", fis.at(i));
    }
    
    ofs.close();
}

//void saveRuntimePopulation(std::filesystem::path filename, pagmo::population pop){TODO}

};
