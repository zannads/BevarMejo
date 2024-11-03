#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "bevarmejo/bemexcept.hpp"
#include "bevarmejo/utility/safe_member.hpp"
#include "bevarmejo/utility/safe_member_ptr.hpp"

namespace bevarmejo
{

// Forward declaration of friend classes.
// Only these classes are allowed to create them, insert and remove elements.
// The only thing you can do when you get a refernece to these objects is iterate them,
// and access and modify (if non-const) the elements. 
// you are always getting a complete set of the elements inside the registry when iterating
// and you can't add or remove elements, if you want to see less, you have to use a registry_view.

// WDS uses the registry to store the elements.
class WaterDistributionSystem;

template <typename T>
class Registry final 

#ifdef ENABLE_SAFETY_CHECKS
    : public SafeMember
#endif

{

/*--- Member types ---*/
public:
    using key_type = std::string;
    using mapped_type = T; // We hide the fact that is a shared_ptr
private:
    struct __instance__
    {
        const key_type id;
        std::shared_ptr<mapped_type> p_entry;
    };
    struct instance 
    {
        const key_type id;
        mapped_type entry;
    };
    struct instance_ref
    {
        const key_type& id;
        mapped_type& entry;
    };
public:
    using value_type = instance;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using key_compare = std::less<key_type>;
    using allocator_type = std::allocator<__instance__>;
    using reference = instance_ref;
    using const_reference = const instance_ref;
    using pointer = instance_ref*;
    using const_pointer = const instance_ref*;
private:
    // Forward declaration of the iterator.
    template <class C>
    class Iterator;
public:
    using iterator = Iterator<Registry<T>>;
    using const_iterator = Iterator<const Registry<T>>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
private:
    friend iterator;
    friend const_iterator;
public:
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

///////////////////////////////////////////////////////////////////////////
// What the user can see from or do to a registry.
///////////////////////////////////////////////////////////////////////////
public:
/*------- Member objects -------*/

/*------- Member functions -------*/
// Constructors and assignement operators will be private because the user is not allowed to create a registry, only
// the friend classes can do it.
// The user can only get a reference to registry from the friend classes.
public:
    ~Registry() = default;

/*------- Element access -------*/
// at (by key and index) const and non const versions always checking for bounds.
// operator [] will be private because it is not safe to use it, it can create a new element.
// front and back const and non const versions checking for bound if enabled.
// data will be private because it is not safe to give the user the pointer to the data.
public:
    mapped_type& at(const key_type& id) 
    {
        auto it = find(id);
        if (it == end())
            __format_and_throw<std::out_of_range>("Registry::at()", "Impossible to access the element.",
                "The element with the given id does not exist.",
                "Id: ", id);
        return it->entry;
    }
    const mapped_type& at(const key_type& id) const
    {
        auto it = find(id);
        if (it == cend())
            __format_and_throw<std::out_of_range>("Registry::at()", "Impossible to access the element.",
                "The element with the given id does not exist.",
                "Id: ", id);
        return it->entry;
    }
    reference at( size_type pos )
    {
        if (pos >= size())
            __format_and_throw<std::out_of_range>("Registry::at()", "Impossible to access the element.",
                "The index is out of range.",
                "Index: ", pos, "Size: ", size());
        return {m__elements[pos].id, *m__elements[pos].p_entry};
    }
    const_reference at( size_type pos ) const
    {
        if (pos >= size())
            __format_and_throw<std::out_of_range>("Registry::at()", "Impossible to access the element.",
                "The index is out of range.",
                "Index: ", pos, "Size: ", size());
        return {m__elements[pos].id, *m__elements[pos].p_entry};
    }

    reference front()
    {
#ifdef ENABLE_SAFETY_CHECKS
        return this->at(0);
#else
        return {m__elements[0].id, *m__elements[0].p_entry};
#endif
    }
    const_reference front() const
    {
#ifdef ENABLE_SAFETY_CHECKS
        return this->at(0);
#else
        return {m__elements[0].id, *m__elements[0].p_entry};
#endif
    }

    reference back()
    {
        auto last = m__elements.size() - 1;
#ifdef ENABLE_SAFETY_CHECKS
        return this->at(last);
#else
        return {m__elements[last].id, *m__elements[last].p_entry};
#endif
    }
    const_reference back() const
    {
        auto last = m__elements.size() - 1;
#ifdef ENABLE_SAFETY_CHECKS
        return this->at(last);
#else
        return {m__elements[last].id, *m__elements[last].p_entry};
#endif
    }

/*------- Iterators -------*/
public:
    iterator begin() noexcept { return iterator(this, 0); }
	const_iterator begin() const noexcept { return const_iterator(this, 0); }
	const_iterator cbegin() const noexcept { return const_iterator(this, 0); }

	iterator end() noexcept { return iterator(this, size()); }
	const_iterator end() const noexcept { return const_iterator(this, size()); }
	const_iterator cend() const noexcept { return const_iterator(this, size()); }

    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }

    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

/*------- Capacity -------*/
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

    size_type capacity() const noexcept { return m__elements.capacity(); }

    // No shrink_to_fit()

/*------- Modifiers -------*/
// The user is not allowed to insert or remove elements.

/*------- Lookup -------*/
public:
    size_type count(const key_type &id) const
    {
        return find(id) != cend() ? 1 : 0;
    }

    iterator find(const key_type &id)
    {
        auto it = std::find_if(m__elements.begin(), m__elements.end(), [&id](const __instance__ &inst) { return inst.id == id; });
        return it != m__elements.end() ? iterator(this, it - m__elements.begin()) : end();
    }
    const_iterator find(const key_type &id) const
    {
        auto it = std::find_if(m__elements.begin(), m__elements.end(), [&id](const __instance__ &inst) { return inst.id == id; });
        return it != m__elements.end() ? const_iterator(this, it - m__elements.begin()) : cend();
    }

    bool contains(const key_type &id) const
    {
        return find(id) != cend();
    }

///////////////////////////////////////////////////////////////////////////
// What the friend classes can see from or do to a registry.
// Everuthing will be private, because this is final class and there is no point to 
// have protected stuff.
///////////////////////////////////////////////////////////////////////////
private:
    using Container = std::vector<__instance__, allocator_type>;
    using DuplicatePtrChecker = std::unordered_set<mapped_type*>;
    using DuplicateIdChecker = std::unordered_set<key_type>;

private:
    // Here make friend the classes that will use the registry.
    friend class WaterDistributionSystem;

/*------- Member objects -------*/
private:
    Container m__elements;
    DuplicatePtrChecker duplicate_ptr_checker;
    DuplicateIdChecker duplicate_id_checker;

/*------- Member functions -------*/
// (constructor)
private:
    Registry() = default;

    Registry(const Registry &other) = default;

    Registry(Registry &&other) noexcept = default;

    template <class InputIt>
    Registry(InputIt first, InputIt last) : 
        Registry()
    {
        auto n_elements = std::distance(first, last);
        
        m__elements.reserve(n_elements);
        duplicate_ptr_checker.reserve(n_elements);
        duplicate_id_checker.reserve(n_elements);

        for (; first != last; ++first)
        {
            insert(first->id, first->p_entry);
        }
    }

    template <class InputContainer>
    Registry( const InputContainer& container ) :
        Registry(container.begin(), container.end())
    { }

// operator=
private: 
    Registry &operator=(const Registry &rhs) = default;

    Registry &operator=(Registry &&rhs) noexcept = default;

    Registry& operator=( std::initializer_list<__instance__> ilist )
    {
        clear();

        auto n_elements = ilist.size();
        
        m__elements.reserve(n_elements);
        duplicate_ptr_checker.reserve(n_elements);
        duplicate_id_checker.reserve(n_elements);

        for (const auto& inst : ilist)
        {
            insert(inst.id, inst.p_entry);
        }

        return *this;
    }

/*------- Element access -------*/
// only operator[] (only by idx as it is not exposed).
private:
    reference operator[](size_type pos)
    {
        return {m__elements[pos].id, *m__elements[pos].p_entry};
    }
    const_reference operator[](size_type pos) const
    {
        return {m__elements[pos].id, *m__elements[pos].p_entry};
    }

    // To access the data from friend classes simply acccess the member.

/*------- Modifiers -------*/
// We need to clear, insert, emplace, erase, extract, merge, and swap (push_back, pop_back as special cases).
// No need to have resize.
private:
    void clear() noexcept
    {
        m__elements.clear();
        duplicate_ptr_checker.clear();
        duplicate_id_checker.clear();
    }

    insert_return_type insert(const __instance__& value)
    {
        return insert(this->end(), value);
    }
    insert_return_type insert(__instance__ &&value)
    {
        return insert(this->end(), std::move(value));
    }
    insert_return_type insert(const_iterator pos, const __instance__ &value)
    {
        return insert(pos, __instance__{value.id, value.p_entry});
    }
    insert_return_type insert(const_iterator pos, __instance__ &&value)
    {
        // If the element is already in the registry and you try to insert it twice it is a failure.
        // While if you simply try to insert a new element with the same id, it's a soft error and 
        // you get back the pointer to the already existing element and the bool to false.

        // Instead of checking if the element is already in the checking sets, 
        // we can simply try to insert it and check the result of the insertion.
        if (value.p_entry == nullptr)
        {
            __format_and_throw<std::invalid_argument>("Registry::insert()", "Impossible to insert the element.",
                "The element with name "+value.id+" is a pointer to null.");
        }

        auto ins_res_dup_ptr = duplicate_ptr_checker.insert(value.p_entry.get());

        if (ins_res_dup_ptr.second == false)
        {
            __format_and_throw<std::invalid_argument>("Registry::insert()", "Impossible to insert the element.",
                "The element is already in the registry with a different id.",
                "Id (new) : ", value.id,
                "Id (old) : ", 
                    std::find_if(
                        m__elements.begin(), m__elements.end(), 
                        [&value](const __instance__ &inst) { return inst.p_entry.get() == value.p_entry.get(); }
                    )->id,
                "Address: ", value.p_entry.get());
        }
        
        auto ins_res_dup_id = duplicate_id_checker.insert(value.id);

        if (ins_res_dup_id.second == false)
        {
            // Remove the pointer from the duplicate_ptr_checker
            duplicate_ptr_checker.erase(value.p_entry.get());
            return {find(value.id), false};
        }

        // Finally include the element in the registry.
        size_type idx = pos - cbegin();
        m__elements.insert(m__elements.begin() + idx, std::move(value));

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
    insert_iter_return_type insert(std::initializer_list<__instance__> ilist)
    {
        return insert(ilist.begin(), ilist.end());
    }

    template <class... Args>
    insert_return_type emplace(Args&&... args)
    {
        return insert(this->cend() ,__instance__{std::forward<Args>(args)...});
    }
    template <class... Args>
    insert_return_type emplace(const_iterator pos, Args&&... args)
    {
        return insert(pos, __instance__{std::forward<Args>(args)...});
    }

    iterator erase(const_iterator pos)
    {
        if(pos == cend())
            return end();

        duplicate_ptr_checker.erase(pos->p_entry.get());
        duplicate_id_checker.erase(pos->id);

        auto idx = pos - cbegin();
        auto it = m__elements.erase(m__elements.begin() + idx);
        
        return iterator(this, it - m__elements.begin());
    }
    iterator erase(const_iterator first, const_iterator last)
    {
        auto idx_first = first - cbegin();
        auto idx_last = last - cbegin();

        for (auto it=first; it!=last; ++it)
        {
            duplicate_ptr_checker.erase(it->p_entry.get());
            duplicate_id_checker.erase(it->id);
        }

        auto it = m__elements.erase(m__elements.begin() + idx_first, m__elements.begin() + idx_last);

        return iterator(this, it - m__elements.begin());
    }
    size_type erase(const key_type &id)
    {
        auto it = find(id);
        if (it == end())
            return 0;

        erase(it);
        return 1;
    }

    __instance__ extract(const_iterator pos)
    {
        if(pos == cend())
            return __instance__{"", nullptr};

        duplicate_ptr_checker.erase(pos->p_entry.get());
        duplicate_id_checker.erase(pos->id);

        auto idx = pos - cbegin();
        auto inst = std::move(m__elements[idx]);
        m__elements.erase(m__elements.begin() + idx);

        return inst;
    }
    __instance__ extract(const key_type &id)
    {
        return extract(find(id));
    }

    // TODO: merge (allowing to merge with another registry of derived types)
    size_type merge(Registry &source);
    size_type merge(Registry &&source);

    void swap(Registry &other) noexcept
    {
        m__elements.swap(other.m__elements);
        duplicate_ptr_checker.swap(other.duplicate_ptr_checker);
        duplicate_id_checker.swap(other.duplicate_id_checker);
    }

}; // class Registry

/*--- Iterators ---*/

template <typename T>
template <typename R>
class Registry<T>::Iterator
{
/*--- Member types ---*/
public:
    using iterator_type = Iterator<R>;
    // Determine if C is a const type and choose the appropriate base type
    using base_iter = typename std::conditional<
        std::is_const<R>::value,
        typename R::Container::const_iterator,
        typename R::Container::iterator
    >::type;
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = typename R::value_type;
    using difference_type = typename R::difference_type;
    using pointer = typename std::conditional<
        std::is_const<R>::value,
        typename R::const_pointer,
        typename R::pointer
    >::type;
    using reference = typename std::conditional<
        std::is_const<R>::value,
        typename R::const_reference,
        typename R::reference
    >::type;

/*--- Member objects ---*/
private:
#ifdef ENABLE_SAFETY_CHECKS
    SafeMemberPtr<R> reg;
#else
    R* reg;
#endif
    size_type idx;
    mutable reference temp_ref;

/*--- Member functions ---*/
/*--- (constructor) ---*/
public:
    Iterator() = delete;
    Iterator(R* container, size_type index) : 
        reg(container), 
        idx(index)
    { 
#ifdef ENABLE_SAFETY_CHECKS
        if (index > reg->size())
            __format_and_throw<std::out_of_range>("Registry::Iterator::Iterator()", "Impossible to construct the iterator.",
            "The index is out of range.",
            "Index: ", index, "Size: ", reg->size());
#endif
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
#ifdef ENABLE_SAFETY_CHECKS
        return reg->at(idx);
#else
        reg->operator[](idx);
#endif
    }
    
    pointer operator->() const
    {
        temp_ref = operator*();
        return &temp_ref;
    }

    reference operator[](difference_type n) const
    {
        return *(*this + n);
    }

/*--- increment/decrement operators ---*/
public:
    iterator_type &operator++()
    {
#ifdef ENABLE_SAFETY_CHECKS
        if (idx >= reg->size())
            __format_and_throw<std::out_of_range>("Registry::Iterator::operator++()", "Impossible to increment the iterator.",
            "The index is out of range.",
            "Index: ", idx, "Size: ", reg->size());
#endif

        ++idx;
        return *this;
    }
    iterator_type operator++(int) {auto tmp= *this; ++(*this); return tmp;}

    iterator_type &operator--()
    {
#ifdef ENABLE_SAFETY_CHECKS
        if (idx == 0)
            __format_and_throw<std::out_of_range>("Registry::Iterator::operator--()", "Impossible to decrement the iterator.",
            "The index is out of range.",
            "Index: ", idx, "Size: ", reg->size());
#endif

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
#ifdef ENABLE_SAFETY_CHECKS
        if (idx + n <= reg->size())
            idx += n;
        else
            idx = reg->size();
#else
        idx += n;
#endif

        return *this;
    }
    iterator_type operator+(difference_type n) const {auto tmp= *this; return tmp += n;}
    iterator_type &operator-=(difference_type n) {return (*this += (-n));}
    iterator_type operator-(difference_type n) const {return (*this + (-n));}
    difference_type operator-(const iterator_type &other) const
    {
#ifdef ENABLE_SAFETY_CHECKS
        check_same_registry(other);
#endif
        return other.idx - idx;
    }

/*--- comparison operators ---*/
public:
    bool operator==(const iterator_type &other) const 
    {
#ifdef ENABLE_SAFETY_CHECKS
        check_same_registry(other);
#endif
        return idx == other.idx;
    }
    bool operator!=(const iterator_type &other) const
    {
#ifdef ENABLE_SAFETY_CHECKS
        check_same_registry(other);
#endif
        return idx != other.idx;
    }
    bool operator<(const iterator_type &other) const
    {
#ifdef ENABLE_SAFETY_CHECKS
        check_same_registry(other);
#endif
        return idx < other.idx;
    }
    bool operator>(const iterator_type &other) const
    {
#ifdef ENABLE_SAFETY_CHECKS
        check_same_registry(other);
#endif
        return idx > other.idx;
    }
    bool operator<=(const iterator_type &other) const
    {
#ifdef ENABLE_SAFETY_CHECKS
        check_same_registry(other);
#endif
        return idx <= other.idx;
    }
    bool operator>=(const iterator_type &other) const
    {
#ifdef ENABLE_SAFETY_CHECKS
        check_same_registry(other);
#endif
        return idx >= other.idx;
    }

// Helper for check if the iterator is valid.
#ifdef ENABLE_SAFETY_CHECKS
private:
    void check_same_registry(const iterator_type &other) const
    {
        if (reg != other.reg)
            __format_and_throw<std::invalid_argument>("Registry::Iterator::check_same_registry()", "Impossible to compare the iterators.",
                "The iterators are from different registries.");
    }
#endif
};

} // namespace bevarmejo