

#ifndef BEVARMEJOLIB__WDS_ELEMENTS__PATTERN_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__PATTERN_HPP

#include <string>
#include <vector>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/element.hpp"

namespace bevarmejo {
namespace wds {

static const std::string l__PATTERN= "Pattern";

class Pattern final : public Element {
public:
    using inherited= Element;
    using container= std::vector<double>;
    using value_type= double;
    using size_type= std::size_t;
    using difference_type= container::difference_type;
    using reference= double&;
    using const_reference= const double&;
    using pointer= double*;
    using const_pointer= const double*;
    using iterator= container::iterator;
    using const_iterator= container::const_iterator;
    using reverse_iterator= container::reverse_iterator;
    using const_reverse_iterator= container::const_reverse_iterator;

/*--- Attributes ---*/
protected:
    /*--- Properties ---*/
    container m__multipliers;

/*--- Member functions ---*/
/*--- Constructors ---*/
public: 
    // Default constructor
    Pattern() = delete;

    Pattern(const std::string& id) : 
        inherited(id),
        m__multipliers() {}

    Pattern(const Pattern& other) = default;

    Pattern(Pattern&& rhs) noexcept = default;

    Pattern& operator=(const Pattern& rhs) = default;

    Pattern& operator=(Pattern&& rhs) noexcept = default;

    virtual ~Pattern() { m__multipliers.clear(); }

/*--- Getters and setters ---*/
public:
    /*--- Properties ---*/
    container& multipliers() { return m__multipliers; }
    const container& multipliers() const { return m__multipliers; }

/*--- Element access ---*/
public:
    reference at(size_type pos) { return m__multipliers.at(pos); }
    const_reference at(size_type pos) const { return m__multipliers.at(pos); }

    // No operator[] because it is not safe to use it with this setup

    reference front() { return m__multipliers.front(); }
    const_reference front() const { return m__multipliers.front(); }

    reference back() { return m__multipliers.back(); }
    const_reference back() const { return m__multipliers.back(); }

    // No direct access to the time series! 

/*--- Iterators ---*/
public:
    iterator begin() noexcept { return m__multipliers.begin(); }
    const_iterator begin() const noexcept { return m__multipliers.begin(); }
    const_iterator cbegin() const noexcept { return m__multipliers.cbegin(); }

    iterator end() noexcept { return m__multipliers.end(); }
    const_iterator end() const noexcept { return m__multipliers.end(); }
    const_iterator cend() const noexcept { return m__multipliers.cend(); }

    reverse_iterator rbegin() noexcept { return m__multipliers.rbegin(); }
    const_reverse_iterator rbegin() const noexcept { return m__multipliers.rbegin(); }
    const_reverse_iterator crbegin() const noexcept { return m__multipliers.crbegin(); }

    reverse_iterator rend() noexcept { return m__multipliers.rend(); }
    const_reverse_iterator rend() const noexcept { return m__multipliers.rend(); }
    const_reverse_iterator crend() const noexcept { return m__multipliers.crend(); }

/*--- Capacity ---*/
public:
    bool empty() const noexcept { return m__multipliers.empty(); }

    size_type size() const noexcept { return m__multipliers.size(); }

    size_type max_size() const noexcept { return m__multipliers.max_size(); }

    void reserve(size_type new_cap) { m__multipliers.reserve(new_cap); }

    size_type capacity() const noexcept { return m__multipliers.capacity(); }

    // No shrink_to_fit
    
/*--- Modifiers ---*/
public:
    void clear() noexcept { m__multipliers.clear(); }

    iterator insert(const_iterator pos, const_reference value) { return m__multipliers.insert(pos, value); }
    iterator insert(const_iterator pos, double&& value) { return m__multipliers.insert(pos, value); }
    iterator insert(const_iterator pos, size_type count, const_reference value) { return m__multipliers.insert(pos, count, value); }
    template< class InputIt >
    iterator insert(const_iterator pos, InputIt first, InputIt last) { return m__multipliers.insert(pos, first, last); }
    iterator insert(const_iterator pos, std::initializer_list<double> ilist) { return m__multipliers.insert(pos, ilist); }

    template< class... Args >
    iterator emplace(const_iterator pos, Args&&... args) { return m__multipliers.emplace(pos, args...); }

    iterator erase(const_iterator pos) { return m__multipliers.erase(pos); }
    iterator erase(const_iterator first, const_iterator last) { return m__multipliers.erase(first, last); }

    void push_back(const_reference value) { m__multipliers.push_back(value); }
    void push_back(double&& value) { m__multipliers.push_back(value); }

    template< class... Args >
    reference emplace_back(Args&&... args) { return m__multipliers.emplace_back(args...); }

    void pop_back() { m__multipliers.pop_back(); }

    void resize(size_type count) { m__multipliers.resize(count); }
    void resize(size_type count, const_reference value) { m__multipliers.resize(count, value); }

    // TODO: swap void swap(container& other) { m__multipliers.swap(other); }

/*--- Pure virtual methods override---*/
public:
    /*--- Properties ---*/
    virtual const std::string& element_name() const override {return l__PATTERN;}
    virtual const unsigned int element_type() const override {return ELEMENT_PATTERN;}


/*--- EPANET-dependent PVMs ---*/
public:
    /*--- Properties ---*/
    void retrieve_index(EN_Project ph) override;
    void retrieve_properties(EN_Project ph) override;

}; // class Pattern

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__PATTERN_HPP
