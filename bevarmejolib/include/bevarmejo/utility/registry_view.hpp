#pragma once

#include <algorithm>
#include <memory>
#include <initializer_list>
#include <string>
#include <vector>

#include "bevarmejo/bemexcept.hpp"
#include "bevarmejo/utility/registry.hpp"

namespace bevarmejo::wds
{

// I define a class that is a ordered (in the order of insertion) sequence of unique strings.
// This is why I redefine it, instead of using a std::set<std::string> (would not be ordered).
// or instead of using a std::vector<std::string> (would not be unique).
// Everything is like using a simple std::vector<std::string> but there is the extra guarantee that
// the strings are unique.
class UniqueStringSequence
{
/*------- Member types -------*/
private:
    using Container = std::vector<std::string>;
public:
    using value_type = Container::value_type;
    using allocator_type = Container::allocator_type;
    using size_type = Container::size_type;
    using difference_type = Container::difference_type;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    using iterator = typename Container::const_iterator; // So that I can't modify the elements
    using const_iterator = typename Container::const_iterator;
    using reverse_iterator = typename Container::const_reverse_iterator;
    using const_reverse_iterator = typename Container::const_reverse_iterator;

    struct insert_return_type
    {
        iterator iterator;
        bool inserted;
    };
    struct insert_iter_return_type
    {
        iterator last;
        size_type inserted;
        size_type total;
    };

/*------- Member objects -------*/
private:
    Container m__elements;

/*------- Member functions -------*/
/*--- (constructor) ---*/
protected:
    UniqueStringSequence() noexcept = default;
    UniqueStringSequence(const Container &elements) : 
        m__elements()
    { 
        for (const auto &el : elements)
            push_back(el);
    }
    UniqueStringSequence(UniqueStringSequence &&elements) = default;
    UniqueStringSequence(const UniqueStringSequence &other) = default;

/*--- (destructor) ---*/
public:
    ~UniqueStringSequence() = default;
    
/*--- operator= ---*/
public:
    UniqueStringSequence &operator=(const UniqueStringSequence &rhs) = default;
    UniqueStringSequence &operator=(UniqueStringSequence &&rhs) noexcept = default;
    UniqueStringSequence &operator=(const Container &elements)
    {
        clear();
        for (const auto &el : elements)
            push_back(el);
        return *this;
    }

    void assign(const Container &elements) { this->operator=(elements); }

/*--- Iterators ---*/
public:
    iterator begin() { return m__elements.begin(); }
    const_iterator begin() const { return m__elements.begin(); }
    const_iterator cbegin() const { return m__elements.cbegin(); }

    iterator end() { return m__elements.end(); }
    const_iterator end() const { return m__elements.end(); }
    const_iterator cend() const { return m__elements.cend(); }

    reverse_iterator rbegin() { return m__elements.rbegin(); }
    const_reverse_iterator rbegin() const { return m__elements.rbegin(); }
    const_reverse_iterator crbegin() const { return m__elements.crbegin(); }

    reverse_iterator rend() { return m__elements.rend(); }
    const_reverse_iterator rend() const { return m__elements.rend(); }
    const_reverse_iterator crend() const { return m__elements.crend(); }

/*--- Capacity ---*/
public:
    bool empty() const noexcept { return m__elements.empty(); }

    size_type size() const noexcept { return m__elements.size(); }

    size_type max_size() const noexcept { return m__elements.max_size(); }

    void reserve(size_type new_cap) { m__elements.reserve(new_cap); }

    size_type capacity() const noexcept { return m__elements.capacity(); }

    void shrink_to_fit() { m__elements.shrink_to_fit(); }

/*--- Element access ---*/
public:
// If we return a reference, the user can modify the elements of the sequence.
// Meaning that the sequence is not unique anymore.
    const_reference at(size_type pos) { return m__elements.at(pos); }
    const_reference at(size_type pos) const { return m__elements.at(pos); }

    const_reference operator[](size_type pos) { return m__elements.at(pos); }
    const_reference operator[](size_type pos) const { return m__elements.at(pos); }

    const_reference front() { return m__elements.front(); }
    const_reference front() const { return m__elements.front(); }

    const_reference back() { return m__elements.back(); }
    const_reference back() const { return m__elements.back(); }

/*--- Modifiers ---*/
public:
    void clear() noexcept { m__elements.clear(); }

    insert_return_type insert(const_iterator pos, const value_type &id)
    {
        auto it = std::find(m__elements.begin(), m__elements.end(), id);
        if (it != m__elements.end())
            return {it, false};

        size_type idx = pos - cbegin();
        m__elements.insert(m__elements.begin() + idx, id);
        return {m__elements.begin() + idx, true};
    }
    insert_return_type insert(const_iterator pos, value_type &&id)
    {
        auto it = std::find(m__elements.begin(), m__elements.end(), id);
        if (it != m__elements.end())
            return {it, false};

        size_type idx = pos - cbegin();
        m__elements.insert(m__elements.begin() + idx, std::move(id));
        return {m__elements.begin() + idx, true};
    }
    insert_iter_return_type insert(const_iterator pos, const_iterator first, const_iterator last)
    {
        auto res = insert_iter_return_type{end(), 0, 0};
        for (auto it=first; it!=last; ++it)
        {
            auto [iter, inserted] = insert(pos, *it);
            if (inserted)
            {
                res.last = iter;
                ++res.inserted;
            }
            ++res.total;
        }
        return res;
    }
    insert_iter_return_type insert(const_iterator pos, std::initializer_list<std::string> ilist)
    {
        auto res = insert_iter_return_type{end(), 0, 0};
        for (auto it=ilist.begin(); it!=ilist.end(); ++it)
        {
            auto [iter, inserted] = insert(pos, *it);
            if (inserted)
            {
                res.last = iter;
                ++res.inserted;
            }
            ++res.total;
        }
        return res;
    }

    iterator erase(const_iterator pos) { return m__elements.erase(pos); }
    iterator erase(const_iterator first, const_iterator last) { return m__elements.erase(first, last); }

    iterator push_back(const value_type &id)
    {
        auto it = std::find(m__elements.begin(), m__elements.end(), id);
        if (it != m__elements.end())
            return it;

        m__elements.push_back(id);
        return m__elements.end() - 1;
    }
    iterator push_back(value_type &&id)
    {
        auto it = std::find(m__elements.begin(), m__elements.end(), id);
        if (it != m__elements.end())
            return it;

        m__elements.push_back(std::move(id));
        return m__elements.end() - 1;
    }

    void pop_back() { m__elements.pop_back(); }

    // No resize method

    void swap(UniqueStringSequence &other) noexcept { m__elements.swap(other.m__elements); }

}; // class UniqueStringSequence

// Now that I have defined a class the keep track of the IDs of the elements in the network,
// thta I want to see (or not to see). I can define a class that is a view of the registry
// and by inherting from the UniqueStringSequence I know I can already modify the 
// sequence of IDs that I want to see.
// There are 3 types of views: Exclude Ids, Include Only (Registry order), Include Only (View order).

struct Exclude { };
struct Only { };
struct Ordered { };

template <typename R, typename T>
class RegistryView : public UniqueStringSequence
{
/*------- Member types -------*/
private:
    using inherited = UniqueStringSequence;
    using Reg = Registry<R>;
    using style_type = T;
public:
    using key_type = typename Reg::key_type;
    using mapped_type = typename Reg::mapped_type;
    using value_type = typename Reg::value_type;
    using size_type = typename Reg::size_type;
    using difference_type = typename Reg::difference_type;
    using reference = typename Reg::reference;
    using const_reference = typename Reg::const_reference;
    using pointer = typename Reg::pointer;
    using const_pointer = typename Reg::const_pointer;
private:
    // Forward declaration of the iterator.
    template <class RV>
    class Iterator;
public:
    using iterator = Iterator<RegistryView<R, T>>;
    using const_iterator = Iterator<const RegistryView<R, T>>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    struct insert_return_type
    {
        iterator iterator;
        bool inserted;
    };
    struct insert_iter_return_type
    {
        iterator last;
        size_type inserted;
        size_type total;
    };
private:
    friend iterator;
    friend const_iterator;

/*------- Member objects -------*/
private:
    const Reg* m__registry;

/*------- Member functions -------*/
/*--- (constructor) ---*/
public:
    RegistryView() noexcept :
        inherited(),
        m__registry(nullptr)
    { }
    RegistryView(const typename inherited::Container &elements) :
        inherited(elements),
        m__registry(nullptr)
    { }
    RegistryView(Reg &registry) : 
        inherited(), 
        m__registry(&registry)
    { }
    RegistryView(Reg &registry, const typename inherited::Container &elements) : 
        inherited(elements), 
        m__registry(&registry)
    { }
    RegistryView(Reg &registry, const RegistryView &other) : 
        inherited(other), 
        m__registry(&registry)
    { }
    RegistryView(Reg &registry, RegistryView &&elements) : 
        inherited(std::move(elements)), 
        m__registry(&registry)
    { }
    
    RegistryView(const RegistryView &other) = default;
    RegistryView(RegistryView &&other) noexcept = default;

/*--- (destructor) ---*/
public:
    ~RegistryView() = default;

/*--- operator= ---*/
public:
    RegistryView &operator=(const RegistryView &rhs) = default;
    RegistryView &operator=(RegistryView &&rhs) noexcept = default;
    RegistryView &operator=(const typename inherited::Container &elements)
    {
        inherited::operator=(elements);
        return *this;
    }
    RegistryView &operator=(Reg &registry)
    {
        m__registry = &registry;
        return *this;
    }

    void assign(const typename inherited::Container &elements) { this->operator=(elements); }
    
/*--- Iterators ---*/
private:
    template <class RV>
    class Iterator
    {
    /*--- Member types ---*/
    private:
        using base_iter = typename Reg::const_iterator;
        using value_type = typename Reg::value_type;
        using difference_type = typename Reg::difference_type;
        using pointer = typename Reg::pointer;
        using reference = typename Reg::reference;

    /*--- Member objects ---*/
    private:
        const RV* reg;
        base_iter it;



} // namespace bevarmejo::wds
