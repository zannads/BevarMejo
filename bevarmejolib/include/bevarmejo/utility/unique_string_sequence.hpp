#pragma once

#include <algorithm>
#include <initializer_list>
#include <string>
#include <vector>

namespace bevarmejo
{
// I define a class that is a ordered (in the order of insertion) sequence of unique strings.
// This is why I redefine it, instead of using a std::set<std::string> (would not be ordered).
// or instead of using a std::vector<std::string> (would not be unique).
// Conceptually, it is like a simple std::vector<std::string>, but there are extra guarantees that
// the strings are unique. As it should also behave like a set (so no free access to the elements),
// I will provide only const access to the elements.
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
        iterator it;
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
public:
    UniqueStringSequence() noexcept = default;
    UniqueStringSequence(const Container &elements);
    UniqueStringSequence(UniqueStringSequence &&elements) = default;
    UniqueStringSequence(const UniqueStringSequence &other) = default;

/*--- (destructor) ---*/
public:
    ~UniqueStringSequence() = default;
    
/*--- operator= ---*/
public:
    UniqueStringSequence &operator=(const UniqueStringSequence &rhs) = default;
    UniqueStringSequence &operator=(UniqueStringSequence &&rhs) noexcept = default;
    UniqueStringSequence &operator=(const Container &elements);

    void assign(const Container &elements);

/*--- Iterators ---*/
public:
    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    const_iterator cbegin() const noexcept;

    iterator end() noexcept;
    const_iterator end() const noexcept;
    const_iterator cend() const noexcept;

    reverse_iterator rbegin() noexcept;
    const_reverse_iterator rbegin() const noexcept;
    const_reverse_iterator crbegin() const noexcept;

    reverse_iterator rend() noexcept;
    const_reverse_iterator rend() const noexcept;
    const_reverse_iterator crend() const noexcept;

/*--- Capacity ---*/
public:
    bool empty() const noexcept;

    size_type size() const noexcept;

    size_type max_size() const noexcept;

    void reserve(size_type new_cap);

    size_type capacity() const noexcept;

    void shrink_to_fit();

/*--- Element access ---*/
public:
// If we return a reference, the user can modify the elements of the sequence.
// Meaning that the sequence is not unique anymore.
    const_reference at(size_type pos);
    const_reference at(size_type pos) const;

    const_reference operator[](size_type pos);
    const_reference operator[](size_type pos) const;

    const_reference front();
    const_reference front() const;

    const_reference back();
    const_reference back() const;

/*--- Modifiers ---*/
public:
    void clear() noexcept;

    insert_return_type insert(const_iterator pos, const value_type &id);
    insert_return_type insert(const_iterator pos, value_type &&id);
    insert_iter_return_type insert(const_iterator pos, const_iterator first, const_iterator last);
    insert_iter_return_type insert(const_iterator pos, std::initializer_list<std::string> ilist);

    iterator erase(const_iterator pos);
    iterator erase(const_iterator first, const_iterator last);
    iterator erase(const value_type &id);

    iterator push_back(const value_type &id);
    iterator push_back(value_type &&id);

    void pop_back();

    // No resize method

    void swap(UniqueStringSequence &other) noexcept;

/*--- Lookup ---*/
public:
    size_type count(const value_type &id) const;

    iterator find(const value_type &id);
    const_iterator find(const value_type &id) const;

    bool contains(const value_type &id) const;

}; // class UniqueStringSequence

} // namespace bevarmejo
