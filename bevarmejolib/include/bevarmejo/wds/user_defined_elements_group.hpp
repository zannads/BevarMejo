#ifndef BEVARMEJOLIB__WDS__USER_DEFINED_ELEMENTS_GROUP_HPP
#define BEVARMEJOLIB__WDS__USER_DEFINED_ELEMENTS_GROUP_HPP

#include <memory>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <utility>
#include <vector>

#include "bevarmejo/io.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/link.hpp"
#include "bevarmejo/wds/elements/junction.hpp"
#include "bevarmejo/wds/elements/source.hpp"
#include "bevarmejo/wds/elements/tank.hpp"
#include "bevarmejo/wds/elements/reservoir.hpp"
#include "bevarmejo/wds/elements/dimensioned_link.hpp"
#include "bevarmejo/wds/elements/pipe.hpp"
#include "bevarmejo/wds/elements/pump.hpp"
//#include "bevarmejo/wds/elements/valve.hpp"
#include "bevarmejo/wds/elements/pattern.hpp"
#include "bevarmejo/wds/elements/curve.hpp"
#include "bevarmejo/wds/elements_group.hpp"

namespace bevarmejo {
namespace wds {

template <typename T>
class UserDefinedElementsGroup : public ElementsGroup<T> {

public:
    using inherithed = ElementsGroup<T>;
protected:
    std::vector<std::weak_ptr<T>> _ord_elems_;
    std::string _comment_;

// Since the Elements Group is a set, the order of the elements is not guaranteed.
// I may want to access the elements in the order they were loaded from file.
// To guarantee this behaviour, I will store the ids in a vector. However, I need to 
// change the whole behaviour of the class.

/*--- Constructors ---*/
public:
    UserDefinedElementsGroup() : 
        inherithed(), 
        _ord_elems_(), 
        _comment_() { }

    // Copy constructor
    UserDefinedElementsGroup(const UserDefinedElementsGroup& other) : 
        inherithed(other), 
        _ord_elems_(other._ord_elems_), 
        _comment_(other._comment_) { }

    // Move constructor
    UserDefinedElementsGroup(UserDefinedElementsGroup&& other) noexcept : 
        inherithed(std::move(other)), 
        _ord_elems_(std::move(other._ord_elems_)), 
        _comment_(std::move(other._comment_)) { }

    // Copy assignment operator
    UserDefinedElementsGroup& operator=(const UserDefinedElementsGroup& rhs) {
        if (this != &rhs) {
            inherithed::operator=(rhs);
            _ord_elems_ = rhs._ord_elems_;
            _comment_ = rhs._comment_;
        }
        return *this;
    }

    // Move assignment operator
    UserDefinedElementsGroup& operator=(UserDefinedElementsGroup&& rhs) noexcept {
        if (this != &rhs) {
            inherithed::operator=(std::move(rhs));
            _ord_elems_ = std::move(rhs._ord_elems_);
            _comment_ = std::move(rhs._comment_);
        }
        return *this;
    }

    // Destructor
    ~UserDefinedElementsGroup() = default;

/*--- Getters and setters ---*/
public:
    std::string& comment() { return _comment_; }
    const std::string& comment() const { return _comment_; }
    void comment(const std::string& comment) { _comment_ = comment; }


/*--- Methods ---*/

}; // class UserDefinedElementsGroup

std::tuple<int, std::vector<std::string>, std::string> __load_egroup_data_from_stream(std::istream& is);

using Subnetwork = UserDefinedElementsGroup<NetworkElement>;


} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS__USER_DEFINED_ELEMENTS_GROUP_HPP
