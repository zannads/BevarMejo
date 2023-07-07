//
//  io.hpp
//  BèvarMéjo
//
//  Created by Dennis Zanutto on 06/07/23.
//

#ifndef BEVARMEJOLIB_IO_HPP
#define BEVARMEJOLIB_IO_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace bevarmejo {

template <typename T, typename U>
inline void stream_param(std::ostream &os, const T& param_name, const U& param_value){
    os << param_name <<param_value <<std::endl;
}

template <typename T, typename U>
inline void stream_param(std::ostream &os, const T& vector_name, const std::vector<U>& vector_value ){
    
    os << vector_name;
    
    os << "[";
    
    auto end= vector_value.end();
    for(auto i= vector_value.begin(); i != end; ){
        os << *i;
        if (++i != end ){
            os << ", ";
        }
    }
    
    os << "]\n";
    
    
    return;
}

}/* namespace bevarmejo */

#endif /* BEVARMEJOLIB_IO_HPP */
