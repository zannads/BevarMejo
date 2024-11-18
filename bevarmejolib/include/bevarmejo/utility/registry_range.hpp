#pragma once

#include <algorithm>
#include <initializer_list>
#include <string>
#include <vector>

#include "bevarmejo/bemexcept.hpp"
#include "bevarmejo/utility/bemememory.hpp"
#include "bevarmejo/utility/registry.hpp"
#include "bevarmejo/utility/unique_string_sequence.hpp"

namespace bevarmejo
{

// A View of a Registry is a sub-range of the registry with a specific behaviour.
// The behaviour can be:
// - Exclude: The elements in the list are excluded from the view.
// - Include: The elements in the list are the only ones in the view.
// - OrderedInclude: The elements in the list are the only ones in the view and 
//      they are in the order of the list and not of the registry.
namespace ViewBehaviour {
    struct Exclude {};
    struct Include {};
    struct OrderedInclude {};
};

template <typename RegistryElement, typename B,
            typename = std::enable_if_t<
                std::is_same_v<B, ViewBehaviour::Exclude> || std::is_same_v<B, ViewBehaviour::Include> || std::is_same_v<B, ViewBehaviour::OrderedInclude>
            >>
class RegistryView final 
{
/*------- Member types -------*/
private:
    using Reg = Registry<RegistryElement>;
    using behaviour_type = B;
    using self_type = RegistryView<RegistryElement, B>;
    using USS = UniqueStringSequence;
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
    template <class RV>
    class ReverseIterator;
public:
    using iterator = Iterator<self_type>;
    using const_iterator = Iterator<const self_type>;
    using reverse_iterator = ReverseIterator<self_type>;
    using const_reverse_iterator = ReverseIterator<const self_type>;

private:
    friend iterator;
    friend const_iterator;

/*------- Member objects -------*/
private:
#ifdef ENABLE_SAFETY_CHECKS
    SafeMemberPtr<Reg> mp__registry;
    std::weak_ptr<USS> mp__elements;
#else
    Reg* mp__registry;
    USS* mp__elements;
#endif

/*------- Member functions -------*/
/*--- (constructor) ---*/
public:
    RegistryView() noexcept = default;
    RegistryView(SafeMemberPtr<Reg> registry) noexcept : 
        mp__registry(registry), 
        mp__elements(nullptr)
    { }
    RegistryView(std::weak_ptr<USS> elements) noexcept : 
        mp__registry(nullptr), 
        mp__elements(elements)
    { }
    RegistryView(SafeMemberPtr<Reg> registry, std::weak_ptr<USS> elements) noexcept : 
        mp__registry(registry), 
        mp__elements(elements)
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

/*--- assign ---*/
public:
    
/*------- Element access -------*/
public:
    mapped_type& at(const key_type& id)
    {
        return const_cast<mapped_type&>(std::as_const(*this).at(id));
    }
    const mapped_type& at(const key_type& id) const
    {
        if (!mp__registry)
            __format_and_throw<std::out_of_range>("RegistryView", "at", "Element not found.",
                "The registry is not available.");

        // If the style is Exclude, asking for an element that is in the list of excluded elements, it is like asking for a non-existing element.
        // If the style is Include or OrderedInclude, asking for an element that is not in the list of included elements, it is like asking for a non-existing element.
        if constexpr (std::is_same_v<behaviour_type, ViewBehaviour::Exclude>)
        {
            if (mp__elements!= nullptr && mp__elements->contains(id))
                __format_and_throw<std::out_of_range>("RegistryView", "at", "Element not found.",
                    "The element is excluded.");
        }
        else // if constexpr (std::is_same_v<behaviour_type, ViewBehaviour::Include> || std::is_same_v<behaviour_type, ViewBehaviour::OrderedInclude>)
        {
            if (mp__elements!= nullptr && !mp__elements->contains(id))
                __format_and_throw<std::out_of_range>("RegistryView", "at", "The element is not included.",
                    "The element is not in the list of included elements.");
        }
        
        return mp__registry->at(id);
    }
    reference at(size_type pos) { return *(begin()+pos); }
    const_reference at(size_type pos) const { return *(cbegin()+pos); }

    reference front() { return *begin(); }
    const_reference front() const { return *begin(); }

    reference back() { return *(end() - 1); }
    const_reference back() const { return *(end() - 1); }

/*--- Iterators ---*/
public:
    iterator begin() noexcept { return iterator(this, 0); }
    const_iterator begin() const noexcept { return const_iterator(this, 0); }
    const_iterator cbegin() const noexcept { return const_iterator(this, 0); }

    iterator end() noexcept { return iterator(this, mp__registry->size()); }
    const_iterator end() const noexcept { return const_iterator(this, mp__registry->size()); }
    const_iterator cend() const noexcept { return const_iterator(this, mp__registry->size()); }

    reverse_iterator rbegin() noexcept { return reverse_iterator(this, mp__registry->size()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(this ,mp__registry->size()); }
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(this, mp__registry->size()); }

    reverse_iterator rend() noexcept { return reverse_iterator(this, 0); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(this, 0); }
    const_reverse_iterator crend() const noexcept { return const_reverse_iterator(this, 0); }

/*--- Capacity ---*/
public:
    // Can not mark the methods noexcept because they use the find method that
    // can throw (because of string comparison).
    bool empty() const { return size() > 0; }

    size_type size() const 
    {
        if (mp__registry == nullptr)
            return 0;

        if constexpr (std::is_same_v<B, ViewBehaviour::Exclude>)
        {
            size_type count = mp__registry->size();

            // Remove the elements to exclude but only if they actually exist
            for (const auto& id : value_or_empty(mp__elements))
            {
                if (mp__registry->contains(id))
                    --count;
            }

            return count;
        }

        if constrexpr (std::is_same_v<B, ViewBehaviour::Include> || std::is_same_v<B, ViewBehaviour::OrderedInclude>)
        {
            size_type count = 0;

            // Count the elements to include but only if they actually exist
            for (const auto& id : value_or_empty(mp__elements))
            {
                if (mp__registry->contains(id))
                    ++count;
            }

            return count;
        }
    }

/*--- Modifiers ---*/
public:
    // Modifiers are not needed for the RegistryView.

/*--- Lookup ---*/
public:
    // TODO: Implement the find, count and contains method.
    
/*--- Iterators ---*/
    template <class RV>
    class Iterator
    {
    /*--- Member types ---*/
    public:
        using iterator_type = Iterator<RV>;
        using base_iter = typename std::conditional<
            std::is_const<RV>::value,
            typename RV::Reg::const_iterator,
            typename RV::Reg::iterator
        >::type;
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = typename base_iter::value_type;
        using difference_type = typename base_iter::difference_type;
        using pointer = typename base_iter::pointer;
        using reference = typename base_iter::reference;

    /*--- Member objects ---*/
    private:
#ifdef ENABLE_SAFETY_CHECKS
        SafeMemberPtr<RV> m__range;
#else
        RV* m__range;
#endif
        base_iter m__iter;
        UniqueStringSequence& m__ids;

    /*--- Member functions ---*/
    /*--- (constructor) ---*/
    public:
        Iterator() = delete;
        Iterator(RV* range, size_type index) noexcept :
            m__range(range), 
            m__iter(range->mp__registry->begin()),
            m__ids(*range)
        {
            // Based on the B, I need to find valid element in the Registry. 
            // It could easily go all the way to the end iterator.
            // Use try catch in case ENABLE_SAFETY_CHECKS is on.
            try
            {
                m__iter += index;
            }
            catch (...)
            {
                m__iter = range->mp__registry->end();
            }
        }
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
    protected:
        base_iter base() const { return m__iter; }

    /*--- access operators ---*/
    public:
        reference operator*() const { return *m__iter; }

        pointer operator->() const { return m__iter.operator->(); }

    /*--- increment/decrement operators ---*/
    public:
        iterator_type &operator++()
        {

            return *this;
        }
        iterator_type operator++(int) {auto tmp= *this; ++(*this); return tmp;}

        iterator_type &operator--()
        {

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
        difference_type operator-(const iterator_type &other) const { return m__iter - other.m__iter; }

    /*--- comparison operators ---*/
    public:
        bool operator==(const iterator_type &other) const { return m__iter == other.m__iter; }
        bool operator!=(const iterator_type &other) const { return m__iter != other.m__iter; }
        bool operator<(const iterator_type &other) const { return m__iter < other.m__iter; }
        bool operator>(const iterator_type &other) const { return m__iter > other.m__iter; }
        bool operator<=(const iterator_type &other) const { return m__iter <= other.m__iter; }
        bool operator>=(const iterator_type &other) const { return m__iter >= other.m__iter; }

    }; // class RegistryView::Iterator

}; // class RegistryView

} // namespace bevarmejo
