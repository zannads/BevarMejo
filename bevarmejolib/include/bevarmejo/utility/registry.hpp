#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#include "bevarmejo/bemexcept.hpp"
#include "bevarmejo/utility/bememory.hpp"

#include "bevarmejo/utility/registry_users.hpp"

// Forward declaration of friend classes.
// Only these classes are allowed to create them, insert and remove elements.
// The only thing you can do when you get a refernece to these objects is iterate them,
// and access and modify (if non-const) the elements. 
// you are always getting a complete set of the elements inside the registry when iterating
// and you can't add or remove elements, if you want to see less, you have to use a registry_range.
// Define them in the file registry_users.hpp
FORWARD_DEFINITIONS_REGISTRY_FRIEND_CLASSES

namespace bevarmejo
{
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
    using pointer = std::shared_ptr<mapped_type>;
    using const_pointer = std::shared_ptr<const mapped_type>;
private:
    struct __entry__
    {
        key_type name;
        pointer ptr;

        __entry__() = default;
        __entry__(const __entry__&) = default;
        __entry__(__entry__&&) = default;
        template <typename NameT, typename PtrT>
        __entry__(NameT&& a_name, PtrT&& a_ptr) :  // Constructor with perfect forwarding for both arguments.
            name(std::forward<NameT>(a_name)),
            ptr(std::forward<PtrT>(a_ptr))
        {
            static_assert(
                !std::is_pointer_v<std::decay_t<PtrT>>,
                "Raw pointers are not allowed. Use std::shared_ptr instead."
            );
        }

        __entry__& operator=(const __entry__&) = default;
        __entry__& operator=(__entry__&&) = default;
    };
    struct entry_ref
    {
        const key_type& name;
        mapped_type& value;
    };
    struct const_entry_ref
    {
        const key_type& name;
        const mapped_type& value;
    };
public:
    using value_type = __entry__;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using key_compare = std::less<key_type>;
    using allocator_type = std::allocator<__entry__>;
    using reference = entry_ref;
    using const_reference = const_entry_ref;
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
    mapped_type& at(const key_type& name) 
    {
        auto it = find_index(name);
        if (it == -1)
            __format_and_throw<std::out_of_range>("Registry::at()", "Impossible to access the element.",
                "The element with the given name does not exist.",
                "Id: ", name);
        return *m__elements[it].ptr;
    }
    const mapped_type& at(const key_type& name) const
    {
        auto it = find_index(name);
        if (it == -1)
            __format_and_throw<std::out_of_range>("Registry::at()", "Impossible to access the element.",
                "The element with the given name does not exist.",
                "Id: ", name);
        return *m__elements[it].ptr;
    }
    reference at( size_type pos )
    {
        if (pos >= size())
            __format_and_throw<std::out_of_range>("Registry::at()", "Impossible to access the element.",
                "The index is out of range.",
                "Index: ", pos, "Size: ", size());
        return {m__elements[pos].name, *m__elements[pos].ptr};
    }
    const_reference at( size_type pos ) const
    {
        if (pos >= size())
            __format_and_throw<std::out_of_range>("Registry::at()", "Impossible to access the element.",
                "The index is out of range.",
                "Index: ", pos, "Size: ", size());
        return {m__elements[pos].name, *m__elements[pos].ptr};
    }

    reference front() { return *begin(); }
    const_reference front() const { return *begin(); }

    reference back() { return *(end() - 1); }
    const_reference back() const { return *(end() - 1); }

    template <class OutputT = mapped_type>
    std::shared_ptr<OutputT> get(const key_type& name)
    {
        static_assert(
            std::is_base_of_v<mapped_type, OutputT> || std::is_base_of_v<OutputT, mapped_type>,
            "The output type must inherit from or be a parent of the mapped type of the registry."
        );

        auto idx = find_index(name);
        if (idx == -1)
            __format_and_throw<std::out_of_range>("Registry::get()", "Impossible to access the element.",
                "The element with the given name does not exist.",
                "Id: ", name);

        return std::dynamic_pointer_cast<OutputT>(m__elements[idx].ptr);
    }
    template <class OutputT = mapped_type>
    std::shared_ptr<const OutputT> get(const key_type& name) const
    {
        static_assert(
            std::is_base_of_v<mapped_type, OutputT> || std::is_base_of_v<OutputT, mapped_type>,
            "The output type must inherit from or be a parent of the mapped type of the registry."
        );

        auto idx = find_index(name);
        if (idx == -1)
            __format_and_throw<std::out_of_range>("Registry::get()", "Impossible to access the element.",
                "The element with the given name does not exist.",
                "Id: ", name);

        return std::dynamic_pointer_cast<const OutputT>(m__elements[idx].ptr);
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

    // Reserve is private because the user is not allowed to change the capacity.

    size_type capacity() const noexcept { return m__elements.capacity(); }

    // No shrink_to_fit()

/*------- Modifiers -------*/
// The user is not allowed to insert or remove elements.

/*------- Lookup -------*/
public:
    size_type count(const key_type &name) const
    {
        return find_index(name) != -1 ? 1 : 0;
    }

    difference_type find_index(const key_type &name) const
    {
        auto it = std::find_if(m__elements.begin(), m__elements.end(), [&name](const __entry__ &inst) { return inst.name == name; });
        return it != m__elements.end() ? it - m__elements.begin() : -1;
    }
    iterator find(const key_type &name)
    {
        auto idx = find_index(name);
        return idx != -1 ? iterator(this, idx) : end();
    }
    const_iterator find(const key_type &name) const
    {
        auto idx = find_index(name);
        return idx != -1 ? const_iterator(this, idx) : cend();
    }

    bool contains(const key_type &name) const
    {
        return find_index(name) != -1;
    }

///////////////////////////////////////////////////////////////////////////
// What the friend classes can see from or do to a registry.
// Everuthing will be private, because this is final class and there is no point to 
// have protected stuff.
///////////////////////////////////////////////////////////////////////////
private:
    using Container = std::vector<__entry__, allocator_type>;
    using DuplicatePtrChecker = std::unordered_set<mapped_type*>;
    using DuplicateIdChecker = std::unordered_set<key_type>;

private:
    FRIEND_RELATIONSHIPS_FOR_REGISTRY

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
            insert(first->name, first->ptr);
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

    Registry& operator=( std::initializer_list<__entry__> ilist )
    {
        clear();

        auto n_elements = ilist.size();
        
        m__elements.reserve(n_elements);
        duplicate_ptr_checker.reserve(n_elements);
        duplicate_id_checker.reserve(n_elements);

        for (const auto& inst : ilist)
        {
            insert(inst.name, inst.ptr);
        }

        return *this;
    }

/*------- Element access -------*/
// only operator[] (only by idx as it is not exposed).
private:
    reference operator[](size_type pos)
    {
        return {m__elements[pos].name, *m__elements[pos].ptr};
    }
    const_reference operator[](size_type pos) const
    {
        return {m__elements[pos].name, *m__elements[pos].ptr};
    }

    // To access the data from friend classes simply acccess the member.

/*------- Capacity ---------*/
// The onlye capacity method that is not const because it can modify the elemnts.
private:
    void reserve(size_type new_cap)
    {
        m__elements.reserve(new_cap);
        duplicate_ptr_checker.reserve(new_cap);
        duplicate_id_checker.reserve(new_cap);
    }

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

    insert_return_type insert(const_iterator pos, __entry__ &&value)
    {
        // If the element is already in the registry and you try to insert it twice it is a failure.
        // While if you simply try to insert a new element with the same name, it's a soft error and 
        // you get back the pointer to the already existing element and the bool to false.

        // Instead of checking if the element is already in the checking sets, 
        // we can simply try to insert it and check the result of the insertion.
        if (value.ptr == nullptr)
        {
            __format_and_throw<std::invalid_argument>("Registry::insert()", "Impossible to insert the element.",
                "The element with name "+value.name+" is a pointer to null.");
        }

        auto ins_res_dup_ptr = duplicate_ptr_checker.insert(value.ptr.get());

        if (ins_res_dup_ptr.second == false)
        {
            __format_and_throw<std::invalid_argument>("Registry::insert()", "Impossible to insert the element.",
                "The element is already in the registry with a different name.",
                "Id (new) : ", value.name,
                "Id (old) : ", 
                    std::find_if(
                        m__elements.begin(), m__elements.end(), 
                        [&value](const __entry__ &inst) { return inst.ptr.get() == value.ptr.get(); }
                    )->name,
                "Address: ", value.ptr.get());
        }
        
        auto ins_res_dup_id = duplicate_id_checker.insert(value.name);

        if (ins_res_dup_id.second == false)
        {
            // Remove the pointer from the duplicate_ptr_checker
            duplicate_ptr_checker.erase(value.ptr.get());
            return {find(value.name), false};
        }

        // Finally include the element in the registry.
        size_type idx = pos - cbegin();
        m__elements.insert(m__elements.begin() + idx, std::move(value));

        return {iterator(this, idx), true};
    }
    insert_return_type insert(const_iterator pos, const __entry__ &value)
    {
        return insert(pos, __entry__(value));
    }
    template <typename E>
    insert_return_type insert(E&& value)
    {
        return insert(this->cend(), std::forward<E>(value));
    }
    template <typename String, typename Ptr>
    insert_return_type insert(String&& name, Ptr&& ptr)
    {
        return insert(this->cend(), __entry__(std::forward<String>(name), std::forward<Ptr>(ptr)));
    }
    template <typename String, typename Ptr>
    insert_return_type insert(const_iterator pos, String&& name, Ptr&& ptr)
    {
        return insert(pos, __entry__(std::forward<String>(name), std::forward<Ptr>(ptr)));
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
    insert_iter_return_type insert(std::initializer_list<__entry__> ilist)
    {
        return insert(ilist.begin(), ilist.end());
    }

    template <class String, typename... Args,
                typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
    insert_return_type emplace(String&& name, Args&&... args)
    {
        return insert(this->cend() ,
            __entry__(std::forward<String>(name), std::make_shared<T>(std::forward<Args>(args)...)));
    }
    template <typename U, class String, typename... Args,
                typename = std::enable_if_t<!std::is_constructible_v<T, Args...>>>
    insert_return_type emplace(String&& name, Args&&... args)
    {
        static_assert(
            std::is_constructible_v<U, Args...>,
            "The type of element to be emplaced must be constructible with the given arguments."
        );
        static_assert(
            std::is_base_of_v<T, U>,
            "The type of element to be emplaced must be a derived type of the registry mapped type."
        );
        return insert(this->cend() ,
            __entry__(std::forward<String>(name), std::make_shared<U>(std::forward<Args>(args)...)));
    }
    template <class String, typename... Args,
                typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
    insert_return_type emplace(const_iterator pos, String&& name, Args&&... args)
    {
        return insert(pos,
            __entry__(std::forward<String>(name), std::make_shared<T>(std::forward<Args>(args)...)));
    }
    template <typename U, class String, typename... Args,
                typename = std::enable_if_t<!std::is_constructible_v<T, Args...>>>
    insert_return_type emplace(const_iterator pos, String&& name, Args&&... args)
    {
        static_assert(
            std::is_constructible_v<U, Args...>,
            "The type of element to be emplaced must be constructible with the given arguments."
        );
        static_assert(
            std::is_base_of_v<T, U>,
            "The type of element to be emplaced must be a derived type of the registry mapped type."
        );
        return insert(pos,
            __entry__(std::forward<String>(name), std::make_shared<U>(std::forward<Args>(args)...)));
    }

    iterator erase(const_iterator pos)
    {
        if(pos == cend())
            return end();

        duplicate_ptr_checker.erase(pos->ptr.get());
        duplicate_id_checker.erase(pos->name);

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
            duplicate_ptr_checker.erase(it->ptr.get());
            duplicate_id_checker.erase(it->name);
        }

        auto it = m__elements.erase(m__elements.begin() + idx_first, m__elements.begin() + idx_last);

        return iterator(this, it - m__elements.begin());
    }
    size_type erase(const key_type &name)
    {
        auto it = find(name);
        if (it == end())
            return 0;

        erase(it);
        return 1;
    }

    __entry__ extract(const_iterator pos)
    {
        if(pos == cend())
            return __entry__{"", nullptr};

        duplicate_ptr_checker.erase(pos->ptr.get());
        duplicate_id_checker.erase(pos->name);

        auto idx = pos - cbegin();
        auto inst = std::move(m__elements[idx]);
        m__elements.erase(m__elements.begin() + idx);

        return inst;
    }
    __entry__ extract(const key_type &name)
    {
        return extract(find(name));
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

/*--- Member functions ---*/
/*--- (constructor) ---*/
public:
    Iterator() noexcept : 
        reg(), 
        idx(0)
    { }
    Iterator(R* container, size_type index) noexcept : 
        reg(container), 
        idx(index)
    { }
    Iterator(const Iterator &other) noexcept = default;
    Iterator(Iterator &&other) noexcept = default;

/*--- (destructor) ---*/
public:
    ~Iterator() = default;

/*--- operator= ---*/
public:
    Iterator &operator=(const Iterator &rhs) noexcept = default;
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
        if (!valid(reg))
            __format_and_throw<std::out_of_range>("Registry::Iterator::operator*()", "Impossible to dereference the iterator.",
                "The registry is not available.");

        if (idx >= reg->size())
            __format_and_throw<std::out_of_range>("Registry::Iterator::operator*()", "Impossible to dereference the iterator.",
                "The index is out of range.",
                "Index: ", idx, "Size: ", reg->size());
#endif
        return reg->operator[](idx);
    }
    
    pointer operator->() const
    {
#ifdef ENABLE_SAFETY_CHECKS
        if (!valid(reg))
            __format_and_throw<std::out_of_range>("Registry::Iterator::operator->()", "Impossible to dereference the iterator.",
                "The registry is not available.");

        if (idx >= reg->size())
            __format_and_throw<std::out_of_range>("Registry::Iterator::operator->()", "Impossible to dereference the iterator.",
                "The index is out of range.",
                "Index: ", idx, "Size: ", reg->size());
#endif
        return reg->m__elements[idx].ptr; // It is casted to const pointer automatically if the iterator is const.
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
        if (!valid(reg))
            __format_and_throw<std::out_of_range>("Registry::Iterator::operator*()", "Impossible to dereference the iterator.",
                "The registry is not available.");
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
        if (!valid(reg))
            __format_and_throw<std::out_of_range>("Registry::Iterator::operator*()", "Impossible to dereference the iterator.",
                "The registry is not available.");
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
        while (n)
        {
            if (n > 0)
            {
                ++(*this);
                --n;
            }
            else
            {
                --(*this);
                ++n;
            }
        }

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
        return idx - other.idx;
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
        if (!valid(reg))
            __format_and_throw<std::out_of_range>("Registry::Iterator::operator*()", "Impossible to dereference the iterator.",
                "The registry is not available.");
        if (reg != other.reg) // Implicity checking also that the other registry is valid.
            __format_and_throw<std::invalid_argument>("Registry::Iterator::check_same_registry()", "Impossible to compare the iterators.",
                "The iterators are from different registries.");
    }
#endif
};

} // namespace bevarmejo