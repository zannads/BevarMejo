#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "bevarmejo/bemexcept.hpp"

namespace bevarmejo::wds
{

// Forward declaration because it is a friend class (only one that can and knows how to add elements).
class WaterDistributionSystem;

template <typename T>
class Registry final
{

/*--- Member types ---*/
public:
    using key_type = std::string;
    using mapped_type = T; // We hide the fact that is a shared_ptr
private:
    struct instance
    {
        const key_type id;
        std::shared_ptr<T> p_entry;
    };
    struct return_type 
    {
        const key_type id;
        mapped_type entry;
    };
public:
    using value_type = return_type;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
private:
    // Forward declaration of the iterator.
    template <class C>
    class Iterator;
public:
    using iterator = Iterator<Registry<T>>;
    using const_iterator = Iterator<const Registry<T>>;
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
    using Container = std::vector<instance>;
    using DuplicatePtrChecker = std::unordered_set<T*>;
    using DuplicateIdChecker = std::unordered_set<key_type>;

private:
    friend class WaterDistributionSystem;
    friend iterator;
    friend const_iterator;

/*------- Member objects -------*/
private:
    Container m__elements;
    DuplicatePtrChecker duplicate_ptr_checker;
    DuplicateIdChecker duplicate_id_checker;

/*------- Member functions -------*/
/*--- (constructor) ---*/
private:
    Registry() = default;

    explicit Registry(size_type n) : 
        m__elements(n), 
        duplicate_ptr_checker(n),
        duplicate_id_checker(n)
    { }

    Registry(const Registry &other) = default;

    Registry(Registry &&other) noexcept = default;

/*--- (destructor) ---*/
public:
    virtual ~Registry() = default;

/*--- operator= ---*/
private: 
    Registry &operator=(const Registry &rhs) = default;

    Registry &operator=(Registry &&rhs) noexcept = default;

/*--- Iterators ---*/
private:
    template <typename R>
    class Iterator
    {
    /*--- Member types ---*/
    public:
        using iterator_type = Iterator<R>;
        // Determine if C is a const type and choose the appropriate base type
        using base_iter = typename std::conditional<
            std::is_const<R>::entry,
            typename R::Container::const_iterator,
            typename R::Container::iterator
        >::type;
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = typename R::value_type;
        using difference_type = typename R::difference_type;
        using pointer = typename R::pointer;
        using reference = typename R::reference;

    /*--- Member objects ---*/
    private:
        R* reg;
        size_type idx;
    
    /*--- Member functions ---*/
    /*--- (constructor) ---*/
    public:
        Iterator() = delete;
        Iterator(R* container, size_type index) : 
            reg(container), 
            idx(index)
        { 
            if (index > reg->size())
                __format_and_throw<std::out_of_range>("Registry::Iterator::Iterator()", "Impossible to construct the iterator.",
                "The index is out of range.",
                "Index: ", index, "Size: ", reg->size());
        }
        Iterator(const Iterator &other) = default;
        Iterator(Iterator &&other) noexcept = default;

    /*--- (destructor) ---*/
    public:
        ~Iterator() = default;
    
    /*--- operator= ---*/
    public:
        Iterator &operator=(const Iterator &rhs) = default;
        Iterator &operator=(Iterator &&rhs) noexcept = default;

    /*--- base ---*/
    // Access the underlying iterator
    protected:
        base_iter base() const
        {
            return reg->m__elements.begin() + idx;
        }
    /*--- access operators ---*/
    // Access the pointed-to element
    public:
        reference operator*() const
        {
            return {reg->m__elements[idx].id, *reg->m__elements[idx].p_entry};
        }
        // TODO: operator->() const

        reference operator[](difference_type n) const
        {
            return *(*this + n);
        }

    /*--- increment/decrement operators ---*/
    public:
        iterator_type &operator++()
        {
            if (idx >= reg->size())
                __format_and_throw<std::out_of_range>("Registry::Iterator::operator++()", "Impossible to increment the iterator.",
                "The index is out of range.",
                "Index: ", idx, "Size: ", reg->size());

            ++idx;
            return *this;
        }
        iterator_type operator++(int) {auto tmp= *this; ++(*this); return tmp;}

        iterator_type &operator--()
        {
            if (idx == 0)
                __format_and_throw<std::out_of_range>("Registry::Iterator::operator--()", "Impossible to decrement the iterator.",
                "The index is out of range.",
                "Index: ", idx, "Size: ", reg->size());

            --idx;
            return *this; 
        }
        iterator_type operator--(int) {auto tmp= *this; --(*this); return tmp;}

        iterator_type &operator+=(difference_type n)
        {
            if (n < 0)
                return (*this += (-n));

            if (n == 0)
                return *this;

            // ======== Actual Implementation (hyp: n>0) ========

            if (idx + n <= reg->size())
                idx += n;
            else
                idx = reg->size();

            return *this;
        }
        iterator_type operator+(difference_type n) const {auto tmp= *this; return tmp += n;}
        iterator_type &operator-=(difference_type n) {return (*this += (-n));}
        iterator_type operator-(difference_type n) const {return (*this + (-n));}
        difference_type operator-(const iterator_type &other) const {return other.idx - idx;}

    /*--- comparison operators ---*/
    public:
        bool operator==(const iterator_type &other) const {return idx == other.idx;}
        bool operator!=(const iterator_type &other) const {return idx != other.idx;}
        bool operator<(const iterator_type &other) const {return idx < other.idx;}
        bool operator>(const iterator_type &other) const {return idx > other.idx;}
        bool operator<=(const iterator_type &other) const {return idx <= other.idx;}
        bool operator>=(const iterator_type &other) const {return idx >= other.idx;}
    };

public:
    iterator begin() noexcept { return iterator(this, 0); }
	const_iterator begin() const noexcept { return const_iterator(this, 0); }
	const_iterator cbegin() const noexcept { return const_iterator(this, 0); }

	iterator end() noexcept { return iterator(this, size()); }
	const_iterator end() const noexcept { return const_iterator(this, size()); }
	const_iterator cend() const noexcept { return const_iterator(this, size()); }

/*--- Capacity ---*/
public:
    bool empty() const noexcept { return m__elements.empty(); }

    size_type size() const noexcept { return m__elements.size(); }

    size_type max_size() const noexcept { return m__elements.max_size(); }

    void reserve(size_type new_cap)
    {
        m__elements.reserve(new_cap);
        duplicate_ptr_checker.reserve(new_cap);
        duplicate_id_checker.reserve(new_cap);
    }

    // No capacity()

    // No shrink_to_fit()    

/*--- Modifiers ---*/
private:
    void clear() noexcept
    {
        m__elements.clear();
        duplicate_ptr_checker.clear();
        duplicate_id_checker.clear();
    }

    insert_return_type insert(const instance &value)
    {
        if (duplicate_ptr_checker.find(value.entry.get()) != duplicate_ptr_checker.end())
            return {end(), false};

        if (duplicate_id_checker.find(value.id) != duplicate_id_checker.end())
            return {end(), false};

        m__elements.push_back(value);
        duplicate_ptr_checker.insert(value.entry.get());
        duplicate_id_checker.insert(value.id);

        return {iterator(this, m__elements.size() - 1), true};
    }

    insert_return_type insert(instance &&value)
    {
        if (duplicate_ptr_checker.find(value.entry.get()) != duplicate_ptr_checker.end())
            return {end(), false};

        if (duplicate_id_checker.find(value.id) != duplicate_id_checker.end())
            return {end(), false};

        m__elements.push_back(std::move(value));
        duplicate_ptr_checker.insert(value.entry.get());
        duplicate_id_checker.insert(value.id);

        return {iterator(this, m__elements.size() - 1), true};
    }

    insert_return_type insert(const_iterator hint, const instance &value)
    {
        if (duplicate_ptr_checker.find(value.entry.get()) != duplicate_ptr_checker.end())
            return {end(), false};

        if (duplicate_id_checker.find(value.id) != duplicate_id_checker.end())
            return {end(), false};

        size_type idx = hint - cbegin();
        m__elements.insert(m__elements.begin() + idx, value);
        duplicate_ptr_checker.insert(value.entry.get());
        duplicate_id_checker.insert(value.id);

        return {iterator(this, idx), true};
    }

    insert_return_type insert(const_iterator hint, instance &&value)
    {
        if (duplicate_ptr_checker.find(value.entry.get()) != duplicate_ptr_checker.end())
            return {end(), false};

        if (duplicate_id_checker.find(value.id) != duplicate_id_checker.end())
            return {end(), false};

        size_type idx = hint - cbegin();
        m__elements.insert(m__elements.begin() + idx, std::move(value));
        duplicate_ptr_checker.insert(value.entry.get());
        duplicate_id_checker.insert(value.id);

        return {iterator(this, idx), true};
    }

    template <class InputIt>
    insert_iter_return_type insert(InputIt first, InputIt last)
    {
        auto res = insert_iter_return_type{end(), 0, 0};
        for (auto& it=first; it!=last; ++it)
        {
            auto [iter, inserted] = insert(*it);
            if (inserted)
            {
                res.last = iter;
                ++res.inserted;
            }
            ++res.total;
        }
        return res;
    }

    iterator erase(const_iterator pos)
    {
        if (pos == cend())
            return end();
        
        size_type idx = pos - cbegin();

        duplicate_ptr_checker.erase(m__elements[idx].p_entry.get());
        duplicate_id_checker.erase(m__elements[idx].id);
        m__elements.erase(m__elements.begin() + idx);
        
        return iterator(this, idx < size() ? idx : size());
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        auto ret_iter = end();

        for (auto it = first; it != last; ++it)
        {
            ret_iter = erase(it);
        }

        return ret_iter;
    }

/*--- Lookup/Element access ---*/
public:
    iterator find(const key_type &id)
    {
        auto it = std::find_if(m__elements.begin(), m__elements.end(), [&id](const instance &inst) { return inst.id == id; });
        return it != m__elements.end() ? iterator(this, it - m__elements.begin()) : end();
    }

    const_iterator find(const key_type &id) const
    {
        auto it = std::find_if(m__elements.begin(), m__elements.end(), [&id](const instance &inst) { return inst.id == id; });
        return it != m__elements.end() ? const_iterator(this, it - m__elements.begin()) : cend();
    }

    mapped_type &at(const key_type &id)
    {
        auto it = find(id);
        if (it == end())
            __format_and_throw<std::out_of_range>("Registry::at()", "Impossible to access the element.",
            "The element with the specified id does not exist.",
            "Id: ", id);
        return *it->entry;
    }

    const mapped_type &at(const key_type &id) const
    {
        auto it = find(id);
        if (it == cend())
            __format_and_throw<std::out_of_range>("Registry::at()", "Impossible to access the element.",
            "The element with the specified id does not exist.",
            "Id: ", id);
        return *it->entry;
    }

    // No operator[]

    size_type count(const key_type &id) const
    {
        return find(id) != cend() ? 1 : 0;
    }

}; // class Registry

} // namespace bevarmejo::wds