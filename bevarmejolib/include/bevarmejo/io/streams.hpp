#pragma once

#include <algorithm>
#include <iterator>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace bevarmejo {
namespace io {

namespace detail {

/* Arithmetic symbols that indicate the presence of a number other than a digit:
* ^: maximum
* ~: minimum
* +: positive number
* -: negative number
*/
constexpr std::string_view arithmetic_symbols = "^~+-"; 

/* BASIC TYPES stream
* For basic types, if the type we pass doesn't have a >> operator
* a compile-error will appear
* CAREFUL: with strings when parsing as input only the first word
* before the space is parsed in, but not in out...
*/

template <typename T>
inline void stream_input(std::istream& is, T& value) {
    while (!is.eof() 
        && !std::isdigit(is.peek()) 
        && arithmetic_symbols.find(is.peek()) == std::string_view::npos) {

		is.ignore();
	}
    if (is.eof())
		return;

    if (is.peek() == '^') {
        is.ignore();
        value = std::numeric_limits<T>::max();
    } else if (is.peek() == '~') {
        is.ignore();
		value = std::numeric_limits<T>::min();
    }
    else if (is.peek() == '+') {
		is.ignore();
		is >> value;
    }
    else if (is.peek() == '-') {
		is >> value;
    }
    else if (std::isdigit(is.peek())) {
		is >> value;
	}
}

template <>
inline void stream_input(std::istream& is, std::string& value) {
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
    while(begin!= end && is ) {
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

// temporary solution for vector of strings (needed for IDs)
template <>
inline void stream_input(std::istream& is, std::vector<std::string>& v) {
	stream_input_single_range(is, v.begin(), v.end(), " ", "\n", "\n" );
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

} // namespace detail 


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

} // namespace io
} // namespace bevarmejo
