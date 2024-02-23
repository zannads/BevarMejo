//
//  subnetwork.hpp
//  bevarmejo cpp library
//
//  Created by Dennis Zanutto on 12/07/23.
//

#ifndef BEVARMEJOLIB__WDS__ELEMENTS_GROUP_HPP
#define BEVARMEJOLIB__WDS__ELEMENTS_GROUP_HPP

#include <memory>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <unordered_set>
#include <utility>

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

namespace bevarmejo {
namespace wds {

template <typename T>
class ElementsGroup {

/*--- Attributes ---*/
protected:
	using Container = std::unordered_set<std::shared_ptr<T>>;
	Container _elements_; 

	// On the motivation of using a set instead of a vector:
	// 1. I want to avoid duplicates
	// 2. I want to be able to remove elements from the subnetwork easily 
	//     wihtout having to iterate thorugh the whole collection.
	// 3. I don't really care on which order the elements are stored.
	// I don't really care about the performance of inserting and deleting as they 
	// less frequent than access. 

/*--- Constructors ---*/
public:
	ElementsGroup() : _elements_() { }

	// Copy constructor
	ElementsGroup(const ElementsGroup& other) : _elements_(other._elements_) { }

	// Move constructor
	ElementsGroup(ElementsGroup&& other) noexcept :_elements_(std::move(other._elements_)) { }

	// Copy assignment operator
	ElementsGroup& operator=(const ElementsGroup& rhs) {
		if (this != &rhs) {
			_elements_ = rhs._elements_;
		}
		return *this;
	}

	// Move assignment operator
	ElementsGroup& operator=(ElementsGroup&& rhs) noexcept {
		if (this != &rhs) {
			_elements_ = std::move(rhs._elements_);
		}
		return *this;
	}

	// TODO: set with operator=?

	// Destructor
	~ElementsGroup() { _elements_.clear(); }

/*--- Getters and setters ---*/
public:
	
/*--- Methods ---*/
public:
/*--- Iterators ---*/
	typename Container::iterator begin() noexcept { return _elements_.begin(); }
	typename Container::const_iterator begin() const noexcept { return _elements_.begin(); }
	typename Container::const_iterator cbegin() const noexcept { return _elements_.cbegin(); }
	typename Container::iterator end() noexcept { return _elements_.end(); }
	typename Container::const_iterator end() const noexcept { return _elements_.end(); }
	typename Container::const_iterator cend() const noexcept { return _elements_.cend(); }
	
/*--- Capacity ---*/
	bool empty() const noexcept { return _elements_.empty(); }
	std::size_t size() const noexcept { return _elements_.size(); }
	std::size_t max_size() const noexcept { return _elements_.max_size(); }

/*--- Modifiers ---*/
	void clear_elements() noexcept { _elements_.clear(); }
	void clear () noexcept { clear_elements(); }
	
	std::pair<typename Container::iterator, bool> insert(const std::shared_ptr<T>& element) {
    	if (element != nullptr) {
        	return _elements_.insert(element);
    	}
    	// Handle the case where element is nullptr
    	return std::make_pair(_elements_.end(), false);
	}

	// TODO: emplace

	// TODO: emplace hint 

/*--- Erase Wrappers ---*/
	std::size_t remove(const std::shared_ptr<T>& element) {
		if (element != nullptr)
			return _elements_.erase(element);
		return 0;
	}
	std::size_t remove(const std::string& id) {
		auto it = std::find_if(_elements_.begin(), _elements_.end(), 
			[&id](const std::shared_ptr<T>& element) { return element->id() == id; });
		if (it != _elements_.end())
			return _elements_.erase(*it);
		return 0;
	}

	// TODO: swap

	// TODO: extract

	// TODO: merge

/*--- Lookup ---*/
	/*--- Contained-object-based lookup ---*/
	std::size_t count(const std::shared_ptr<T>& element) const {
		if (element != nullptr)
			return _elements_.count(element);
		return 0;
	}

	typename Container::iterator find(const std::shared_ptr<T>& element) {
		if (element != nullptr)
			return _elements_.find(element);
		return _elements_.end();
	}

	typename Container::const_iterator find(const std::shared_ptr<T>& element) const {
		if (element != nullptr)
			return _elements_.find(element);
		return _elements_.end();
	}

	bool contains(const std::shared_ptr<T>& element) const {
		if (element != nullptr)
			return _elements_.find(element) != _elements_.end();
		return false;
	}
	
	/*--- ID-based lookups ---*/
	typename Container::iterator find(const std::string& id) {
		auto it = std::find_if(_elements_.begin(), _elements_.end(), 
			[&id](const std::shared_ptr<T>& element) { return element->id() == id; });
		return it;
		// If the element is not find it is returning the end iterator anyway.
	}
	typename Container::const_iterator find(const std::string& id) const {
		auto it = std::find_if(_elements_.begin(), _elements_.end(), 
			[&id](const std::shared_ptr<T>& element) { return element->id() == id; });
		return it;
		// If the element is not find it is returning the end iterator anyway.
	}

	std::size_t count(const std::string& id) const {
		auto it = find(id);
		if (it != _elements_.end())
			return count(*it);
		return 0;
	}
	
	bool contains(const std::string& id) const {
		auto it = find(id);
		if (it != _elements_.end())
			return contains(*it);
		return false;
	}

	T& element(const std::string& id) {
		auto it = find(id);
		if (it != _elements_.end())
			return **it;
		else
			throw std::runtime_error("Element with id " + id + " not found.");
	}

	const T& element(const std::string& id) const {
		auto it = find(id);
		if (it != _elements_.end())
			return **it;
		else
			throw std::runtime_error("Element with id " + id + " not found.");
	}

	// TODO: equal_range

	// Bucket interface ??

/*--- Hash policy ---*/
	// TODO: load factors and rehash

	void reserve(std::size_t n) { _elements_.reserve(n); }

}; // class ElementsGroup

int _is_en_object_type_valid(const std::string& en_object_type);

std::tuple<int, std::vector<std::string>, std::string> __load_egroup_data_from_stream(std::istream& is);

using Subnetwork = ElementsGroup<NetworkElement>;
using Nodes = ElementsGroup<Node>;
using Links = ElementsGroup<Link>;
using Junctions = ElementsGroup<Junction>;
using Sources = ElementsGroup<Source>;
using Tanks = ElementsGroup<Tank>;
using Reservoirs = ElementsGroup<Reservoir>;
using DimensionedLinks = ElementsGroup<DimensionedLink>;
using Pipes = ElementsGroup<Pipe>;
using Pumps = ElementsGroup<Pump>;
// using Valves = ElementsGroup<Valve>;
using Patterns = ElementsGroup<Pattern>;
using Curves = ElementsGroup<Curve>;
// using Controls = ElementsGroup<Control>;
// using Rules = ElementsGroup<Rule>;

} // namespace wds
} // namespace bevarmejo

// TODO: create subnetwork from file

#endif // BEVARMEJOLIB__WDS__ELEMENTS_GROUP_HPP