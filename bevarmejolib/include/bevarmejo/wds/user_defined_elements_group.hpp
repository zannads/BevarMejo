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
#include "bevarmejo/wds/data_structures/pattern.hpp"
#include "bevarmejo/wds/auxiliary/curve.hpp"
#include "bevarmejo/wds/elements_group.hpp"

namespace bevarmejo {
namespace wds {

template <typename T>
class UserDefinedElementsGroup : public ElementsGroup<T> {

public:
    using inherithed = ElementsGroup<T>;
    using Container = std::vector<std::weak_ptr<T>>;
protected:
    Container _ord_elems_;
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
public:
/*--- Iterators ---*/
    typename Container::iterator begin() noexcept { return _ord_elems_.begin(); }
    typename Container::const_iterator begin() const noexcept { return _ord_elems_.begin(); }
    typename Container::const_iterator cbegin() const noexcept { return _ord_elems_.cbegin(); }
    typename Container::iterator end() noexcept { return _ord_elems_.end(); }
    typename Container::const_iterator end() const noexcept { return _ord_elems_.end(); }
    typename Container::const_iterator cend() const noexcept { return _ord_elems_.cend(); }
    typename Container::reverse_iterator rbegin() noexcept { return _ord_elems_.rbegin(); }
    typename Container::const_reverse_iterator rbegin() const noexcept { return _ord_elems_.rbegin(); }
    typename Container::const_reverse_iterator crbegin() const noexcept { return _ord_elems_.crbegin(); }
    typename Container::reverse_iterator rend() noexcept { return _ord_elems_.rend(); }
    typename Container::const_reverse_iterator rend() const noexcept { return _ord_elems_.rend(); }
    typename Container::const_reverse_iterator crend() const noexcept { return _ord_elems_.crend(); }

/*--- Capacity ---*/
// empty, size, from Inherithed class
    std::size_t max_size() const noexcept { // return the minimum between the two max_sizes
        return _ord_elems_.max_size() > inherithed::max_size() ? inherithed::max_size() : _ord_elems_.max_size(); }

/*--- Modifiers ---*/
    void clear_elements() noexcept { inherithed::clear_elements(); _ord_elems_.clear(); }
    void clear () noexcept { clear_elements(); _comment_.clear(); }

    // Set like functions 
    std::pair<typename Container::iterator, bool> insert(const std::shared_ptr<T>& element) {
        if ( element== nullptr)
            return std::make_pair(_ord_elems_.end(), false);
        
        // First introduce it in the set, if it works, then you can add it in the last position of the vector.
        auto res = inherithed::insert(element);
        
        if (! res.second) 
            return std::make_pair(_ord_elems_.end(), false);
        
        _ord_elems_.push_back(element);
        
        // I know it has been insert at the end, no need to search for it.
        return std::make_pair(_ord_elems_.end()--, true);
    }

    std::size_t remove(const std::shared_ptr<T>& element) {
        if (element == nullptr)
            return 0;
        
        // Remove it from the set first as it is faster usign the hash.
        // If it is not in the set, it is not in the vector either.
        if (inherithed::remove(element) == 0)
            return 0;
        
        // Remove it from the vector.
        auto it = std::find_if(_ord_elems_.begin(), _ord_elems_.end(), 
            [&element](const std::weak_ptr<T>& weak_element) { return weak_element.lock() == element; });
        _ord_elems_.erase(it);

        return 1;
    }

    std::size_t remove(const std::string& id) {
        // Since with the ID I need to iterate thorugh al the elements, I better use the vector which is faster.
        auto it = std::find_if(_ord_elems_.begin(), _ord_elems_.end(), 
            [&id](const std::weak_ptr<T>& weak_element) { 
                auto sp = weak_element.lock();
                return sp && sp->id() == id; });
        
        if (it == _ord_elems_.end())
            return 0; // Not found

        // Now I know where it is in the vector, so I can use lock and removing it from the set.
        inherithed::remove(it->lock()); // I don't need to check if it has been removed or not. 

        _ord_elems_.erase(it);

        return 1;
    }

/*--- Lookup ---*/
// count and contains from Inherithed class.
    typename Container::iterator find(const std::shared_ptr<T>& element) {
        return std::find_if(_ord_elems_.begin(), _ord_elems_.end(), 
            [&element](const std::weak_ptr<T>& weak_element) { 
                auto sp = weak_element.lock();
                return sp && sp == element; });
    }
    typename Container::const_iterator find(const std::shared_ptr<T>& element) const {
        return std::find_if(_ord_elems_.begin(), _ord_elems_.end(), 
            [&element](const std::weak_ptr<T>& weak_element) { 
                auto sp = weak_element.lock();
                return sp && sp == element; });
    }
    typename Container::iterator find(const std::string& id) {
        return std::find_if(_ord_elems_.begin(), _ord_elems_.end(), 
            [&id](const std::weak_ptr<T>& weak_element) { 
                auto sp = weak_element.lock();
                return sp && sp->id() == id; });
    }
    typename Container::const_iterator find(const std::string& id) const {
        return std::find_if(_ord_elems_.begin(), _ord_elems_.end(), 
            [&id](const std::weak_ptr<T>& weak_element) { 
                auto sp = weak_element.lock();
                return sp && sp->id() == id; });
    }
    typename inherithed::Container::iterator find_in_set(const std::shared_ptr<T>& element) {
        return inherithed::find(element);
    }
    typename inherithed::Container::const_iterator find_in_set(const std::shared_ptr<T>& element) const {
        return inherithed::find(element);
    }
    typename inherithed::Container::iterator find_in_set(const std::string& id) {
        return inherithed::find(id);
    }
    typename inherithed::Container::const_iterator find_in_set(const std::string& id) const {
        return inherithed::find(id);
    }

    T& element(const std::string& id) {
        auto it = find(id);
        if (it == _ord_elems_.end() || it->lock() == nullptr) 
            throw std::out_of_range("The element with id " + id + " is not in the group.");

        return *it->lock();
    }

    const T& element(const std::string& id) const {
        auto it = find(id);
        if (it == _ord_elems_.end() || it->lock() == nullptr)
            throw std::out_of_range("The element with id " + id + " is not in the group.");

        return *it->lock();
    }

    void reserve(std::size_t new_cap) {
        inherithed::reserve(new_cap);
        _ord_elems_.reserve(new_cap);
    }

}; // class UserDefinedElementsGroup

std::tuple<int, std::vector<std::string>, std::string> __load_egroup_data_from_stream(std::istream& is);

using Subnetwork = UserDefinedElementsGroup<NetworkElement>;


} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS__USER_DEFINED_ELEMENTS_GROUP_HPP
