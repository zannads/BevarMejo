//
//  io.hpp
//  BèvarMéjo
//
//  Created by Dennis Zanutto on 06/07/23.
//

#ifndef BEVARMEJOLIB_IO_HPP
#define BEVARMEJOLIB_IO_HPP

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace bevarmejo {

namespace detail {

/* BASIC TYPES stream
* For basic types, if the type we pass doesn't have a >> operator
* a compile-error will appear
* CAREFUL: with strings when parsing as input only the first word
* before the space is parsed in, but not in out...
*/

template <typename T>
inline void stream_input(std::istream& is, T& value) {
    is >> value;
}

template <typename T>
inline void stream_output(std::ostream& os, const T& value) {
    os << value;
}

/* TYPE SPECIFIC implementations */

/* TODO: bool */

/* TODO: pair */

/* CONTAINERS */

// Helpers for single elements containers: array, vector, list, set, etc..
template<typename It>
inline void stream_input_single_range(std::istream& is, It begin, const It end,
                                      const std::string_view opener = " ",
                                       const std::string_view delimeter = " ",
                                       const std::string_view closer = " ") {
    
    // Move to the opener
    is.ignore(1000, opener[0]);

    // Now I am in the best position to start parsing. 
    // I will parse until the stream ends
    while(begin!= end) {
        // Parse the first element
        stream_input(is, *begin);
        
        // Unless it is the last look for the delimeter, otherwise make sure you will exit and then look for the closer
        if (++begin != end){
            // Move to the delimeter
            is.ignore(1000, delimeter[0]);
        }
    }
    
    // Move to the closer
    is.ignore(1000, closer[0]);
}

template<typename It>
inline void stream_output_single_range(std::ostream& os, It begin, const It end,
                                      const std::string_view opener = " ",
                                       const std::string_view delimeter = " ",
                                       const std::string_view closer = " ") {
    
    os << opener;
    
    while (begin != end) {
        stream_output(os, *begin);
        
        // Unless it is the last put the delimeter,
        // Note that if it is empty you never enter the while in first place
        if (++begin != end){
            os << delimeter;
        }
    }
    
    os << closer;
    return;
}

// Helpers for couple elements containers: map, etc ...
template<typename It>
inline void stream_input_couple_range(std::istream& is, It begin, const It end,
                                      const std::string_view opener = " ",
                                       const std::string_view delimeter = " ",
                                       const std::string_view closer = " ",
                                      const std::string_view el_delimeter = " ") {
    
    // Move to the opener
    is.ignore(1000, opener[0]);

    // Now I am in the best position to start parsing. 
    // I will parse until the stream ends
    while(begin!= end) {
        // Parse the first element
        stream_input(is, begin->first);
        
        // Move to the delimeter
        is.ignore(1000, el_delimeter[0]);
        
        // Parse the second element
        stream_input(is, begin->second);
        
        // Unless it is the last look for the delimeter, otherwise make sure you will exit and then look for the closer
        if (++begin != end){
            // Move to the delimeter
            is.ignore(1000, delimeter[0]);
        }
    }

    // Move to the closer
    is.ignore(1000, closer[0]);
}

template<typename It>
inline void stream_output_couple_range(std::ostream& os, It begin, const It end,
                                       const std::string_view opener = " ",
                                      const std::string_view delimeter = " ",
                                      const std::string_view closer = " ",
                                       const std::string_view el_delimeter = " ") {
    
    os << opener;
    
    while (begin != end) {
        stream_output(os, begin->first);
        stream_output(os, el_delimeter);
        stream_output(os, begin->second);
        
        // Unless it is the last put the delimeter,
        // Note that if it is empty you never enter the while in first place 
        if (++begin != end){
            os << delimeter;
        }
    }
    
    os << closer;
    return;
}

/* VECTOR
* Structure of vectors:
* [1, 2, 3, 5]
*/
template <typename U>
inline void stream_input(std::istream& is, std::vector<U> &v) {
    stream_input_single_range(is, v.begin(), v.end(), "[", ",", "]" );
}

template <class U>
inline void stream_output(std::ostream &os, const std::vector<U> &v) {
    stream_output_single_range(os, v.begin(), v.end(), "[", ", ", "]" );
}

/* MAP
* Structure of maps:
* {1| 2
*  2 | 3
*  3 | 5} 
*/
template <class Key, class U>
inline void stream_output(std::ostream &os, const std::map<Key,U> &m) {
    stream_output_couple_range(os, m.begin(), m.end(), "{", "\n", "}", " | ");
}

/* STREAM IN and OUT
* Simply write or load the same element with the appropriate structure.
* Wrapper for type specific implementations.
* Possible output:
* [1, 3, 5]
*/

template <typename T, typename ...Args>
inline void stream_input(std::istream& is, T& value, Args& ...args){
    stream_input(is, value);
    stream_input(is, args...);
}

template <typename T, typename ...Args>
inline void stream_output(std::ostream& os, const T& value, Args& ...args){
    stream_output(os, value);
    stream_output(os, args...);
}

} /* namespace detail */


/* STREAM IN and OUT
* Simply write or load the same element with the appropriate structure.
* Wrapper of implementation function in detail namespace.
* Possible output:
* [1, 3, 5]
*/

template <typename... Args>
inline void stream_in(std::istream &is, Args &...args) {
    detail::stream_input(is, args...);
}

template <typename... Args>
inline void stream_out(std::ostream &os, const Args &...args) {
    detail::stream_output(os, args...);
}

/* STREAM PARAM
* Special type of stream out for couples parameter:
* Possible output:
* Seed : 3 \n
*
*/
 
template <typename T>
inline void stream_param(std::ostream &os, const std::string& param_name, const T& param_value){
    detail::stream_output(os, param_name, " : ", param_value, "\n");
}

/* LOAD dimensions from TAG
* Special type of stream input for tag */
inline std::size_t load_dimensions(std::istream& is, const std::string_view tag) {
    std::size_t dimensions;

    std::string line;
    while (getline(is, line) && line.find(tag) != std::string::npos );;

    // If I'm here either it has finished or it has found it
    if (is.eof()) {
        std::ostringstream oss;
        oss << "Tag " << tag << " not found." << std::endl;
        throw std::runtime_error(oss.str());
    }

    // If I'm here, I have found the tag
    std::istringstream iss(line);
    // consume the tag
    stream_in(iss, line);
    // the size is: 
    stream_in(iss, dimensions);

    // When I don't write anything, the dimensions is 0, but I mean it to be 1
    dimensions = dimensions == 0 ? 1 : dimensions;
    
    return dimensions;
}


} /* namespace bevarmejo */

#endif /* BEVARMEJOLIB_IO_HPP */
