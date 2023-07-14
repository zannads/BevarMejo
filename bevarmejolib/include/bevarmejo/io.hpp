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
#include <string_view>
#include <utility>
#include <vector>

namespace bevarmejo {

template <typename T>
struct NMatrix {
    std::vector<std::size_t> dimensions;
    std::vector<T> data;

    NMatrix<T>& load(std::istream& is, const std::string_view tag) {
        dimensions.clear();
        data.clear();

        // I look in the file until I find the tag
        std::string line;
        while (std::getline(is, line))
        {
            if (line.find(tag) != std::string::npos) {
                // Once I find it, data are in the following structure:
                // #tag n1 n2 n3 ... nN
                // data11, data12, data13, ... data1n2
                // data21, data22, data23, ... data2n2
                // ...
                // datan11, datan12, datan13, ... datan1n2
                // ---new_element--- (in dimension n3)
                // data again
                // ___NEW_DIMENSION___ (finished loading third dimension)
                // data again

                std::stringstream ss(line);

                // consume the name #tag
                std::string name;
                ss >> name;
                // I know dimensions follow immediately after the name in the same row,
                // thus, continue reading from the same stringstream
                this->dimensions = load_dimensions(ss);

                // Data are from the next line onwards,
                // thus, continue reading from is again
                this->data = load_data(is, this->dimensions);

                // TODO: Check if data are consistent with dimensions 
                // If the number of elements is less than the expected, 
                // the remaining elements are set to 0 (or whatever the default value is)
                // If the number of elements is more than the expected, the ones after are ignored 
            }
        }
        is.clear();
        is.seekg(0, std::ios::beg);

        return *this;
    }
    std::size_t ndims() const { return this->dimensions.size(); }

    std::size_t size() const {
        std::size_t size = 1;
        for (auto& dim : this->dimensions) {
            size *= dim;
        }
        return size;
    }

    std::vector<std::size_t> get_dimensions() const { return this->dimensions; }

    std::vector<std::size_t> load_dimensions(std::istream& is) {
        std::vector<std::size_t> dimensions;

        std::string line;
        std::getline(is, line);

        // case 1: no numbers after the name
        if (line.empty()) {
            dimensions.push_back(1);
            dimensions.push_back(1);
            return dimensions;
        }
        // case 2: special simbol - after the name
        if (line.find("-") != std::string::npos) {
            return dimensions;
        }
        // case 3: numbers after the name
        std::stringstream ss(line);
        std::size_t n;
        while (ss >> n) {
            dimensions.push_back(n);
        }
        dimensions.push_back(1);
        return dimensions;
    }

    std::vector<T> get_1D_data() const { return this->data; }

    std::vector<T> load_data(std::istream& is, const std::vector<std::size_t>& dimensions) {

        // case 2: special simbol - after the name
        if (dimensions.size() == 0) {
            return std::vector<T>();
        }

        // case 1, 3: numbers after the name (implicit or explicit)
        std::size_t n_required = this->size();
        std::vector<T> data(n_required);
        std::size_t n_loaded = 0;
        std::vector<std::size_t> indices(this->ndims() - 1, 0); // indices of the current element
        // I never need the last dimension as it is always 1
        std::string line;

        while (std::getline(is, line) && n_loaded < n_required) {
            // Alwys remove \t in case someone added them
            // (if parsed into string object they will appear as \t)
            line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());
            // Here I check if there are the special separators for the dimensions
            // (---new_element--- and ___NEW_DIMENSION___)
            // TODO: check separators and correct the position of the indices

            // For now only the first and second dimension are supported
            // special case for 1D matrices
            if (indices.size() > 1)
                indices[1] = 0; // reset the second dimension as each line contains all the elements of that dimension

            /* From here on I always work with bidimensional matrices */
            // put the current row back in a stream to parse it
            std::stringstream ss(line);
            T value;
            // If second dimension is 1, or I am at the last element of the second
            // dimension just load with normal getline, i.e. separator is \n
            // Otherwise, I need to use the comma separator and save them in the
            // correct loaction
            char separator = (indices.size() == 1) || (indices[1] == (dimensions[1] - 1)) ? '\n' : ',';

            while (std::getline(ss, line, separator)) {
                //TODO: here if type is string it takes only the first word
                std::stringstream ss2(line);
                ss2 >> value;
                data[this->NIndices_to_index(indices)] = value;
                if (indices.size() > 1)
                    ++indices[1];   // Next column of this bidimensional slice
                ++n_loaded;     // I loaded one more element
            }

            ++indices[0];       // Next row of this bidimensional slice
        }

        return data;
    }

    std::size_t NIndices_to_index(const std::vector<std::size_t>& indices) const {
        // TODO: check that indices are a vector smaller than dimensions by one

        std::size_t index = 0;
        std::size_t stride = 1;
        for (std::size_t i = 0; i < indices.size(); ++i) {
            index += indices[i] * stride;
            stride *= this->dimensions[i];
        }
        return index;
    }
};


std::stringstream get_data_from_tag(std::istream& is, const std::string_view tag);


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
