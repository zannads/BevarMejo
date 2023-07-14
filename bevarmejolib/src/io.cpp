//
//  io.cpp
//  BèvarMéjo
//
//  Created by Dennis Zanutto on 06/07/23.
//

#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "io.hpp"

namespace bevarmejo {

   /* template <typename T>
    NMatrix<T>& NMatrix<T>::load(std::istream& is, const std::string_view tag) 

    template <typename T>
    std::size_t NMatrix<T>::ndims() const 

    template <typename T>
    std::size_t NMatrix<T>::size() const 

    template <typename T>
    std::vector<std::size_t> NMatrix<T>::get_dimensions() const

    template <typename T>
    std::vector<std::size_t> NMatrix<T>::load_dimensions(std::istream& is) 
    template <typename T>
    std::vector<T> NMatrix<T>::get_1D_data() const 

    template <typename T>
    std::vector<T> NMatrix<T>::load_data(std::istream& is,
            const std::vector<std::size_t>& dimensions)

    template <typename T>
    std::size_t NMatrix<T>::NIndices_to_index(const std::vector<std::size_t>& indices) const

    */


    std::stringstream get_data_from_tag(std::istream& is, const std::string_view tag) {
        std::stringstream ss;
        std::string line;

        // load everything from where you find the tag until the first empty line
        while (std::getline(is, line)) {
            if (line.find(tag) != std::string::npos) {
                while (std::getline(is, line)) {
                    if (line.empty()) {
                        is.clear();
                        is.seekg(0, std::ios::beg);
                        return ss;
                    }
                    ss << line << std::endl;
                }
            }
        }
        is.clear();
        is.seekg(0, std::ios::beg);
        return ss;
    }

} /* namespace bevarmejo */
