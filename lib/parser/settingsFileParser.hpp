//
//  settingsFileParser.hpp
//  hanoiOptimization
//
//  Created by Dennis Zanutto on 21/06/23.
//

#ifndef settingsFileParser_hpp
#define settingsFileParser_hpp

#include <iostream>
#include <string>

#include "pugixml.hpp"

#include <boost/filesystem.hpp>

namespace bevarmejo{

template<typename T>
T from_XML_to_settingsStruct(const std::string xmlFile, const T settingsStructType);



} // namespace bevarmejo

template<typename T>
T bevarmejo::from_XML_to_settingsStruct(const std::string xmlFile, const T settingsStructType){
    
    T outSettingsStruct;
    
    
    // Load the file and check
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(xmlFile.c_str());
    
    if (result.status != pugi::status_ok){
        throw std::runtime_error(result.description());
    }
    
    std::cout <<doc.child("rootDataFolder").name() <<" " <<doc.child("rootDataFolder").value() <<" " <<doc.child("rootDataFolder").child_value();
    
    // Start parsing the xml_document object
    // TODO generic structure
    
    // for now let's just do for Hanoi assuming I know it works
    //const char* rootDataFolder =
    
    outSettingsStruct.rootDataFolder = doc.child("rootDataFolder").child_value();
    // Return the object
    return outSettingsStruct;
}

#endif /* settingsFileParser_hpp */
