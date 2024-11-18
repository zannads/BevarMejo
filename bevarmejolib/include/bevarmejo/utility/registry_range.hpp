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
public:
    using iterator = Iterator<self_type>;
    using const_iterator = Iterator<const self_type>;

private:
    friend iterator;

/*------- Member objects -------*/
private:
#ifdef ENABLE_SAFETY_CHECKS
    SafeMemberPtr<Reg> mp__registry; // My Pointer to Registry
    std::weak_ptr<USS> mp__u_ids; // My Pointer to Unique ID Sequence
#else
    Reg* mp__registry; // My Pointer to Registry
    USS* mp__u_ids; // My Pointer to Unique ID Sequence
#endif

/*------- Member functions -------*/
/*--- (constructor) ---*/
public:
    RegistryView() noexcept = default;
    RegistryView(SafeMemberPtr<Reg> registry) noexcept : 
        mp__registry(registry), 
        mp__u_ids(nullptr)
    { }
    RegistryView(std::weak_ptr<USS> elements) noexcept : 
        mp__registry(nullptr), 
        mp__u_ids(elements)
    { }
    RegistryView(SafeMemberPtr<Reg> registry, std::weak_ptr<USS> elements) noexcept : 
        mp__registry(registry), 
        mp__u_ids(elements)
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
        if (!valid(mp__registry))
            __format_and_throw<std::out_of_range>("RegistryView", "at", "Element not found.",
                "The registry is not available.");

        // If the style is Exclude, asking for an element that is in the list of excluded elements, it is like asking for a non-existing element.
        // If the style is Include or OrderedInclude, asking for an element that is not in the list of included elements, it is like asking for a non-existing element.
        if constexpr (std::is_same_v<behaviour_type, ViewBehaviour::Exclude>)
        {
            if (valid(mp__u_ids) && mp__u_ids->contains(id))
                __format_and_throw<std::out_of_range>("RegistryView", "at", "Element not found.",
                    "The element is excluded.");
        }
        else // if constexpr (std::is_same_v<behaviour_type, ViewBehaviour::Include> || std::is_same_v<behaviour_type, ViewBehaviour::OrderedInclude>)
        {
            if (valid(mp__u_ids) && !mp__u_ids->contains(id))
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
    iterator begin() noexcept { return iterator(this); }
    const_iterator begin()  const noexcept { return const_iterator(this); }
    const_iterator cbegin() const noexcept { return const_iterator(this); }

    iterator end() noexcept { return iterator(this, /*end = */ true); }
    const_iterator end()  const noexcept { return const_iterator(this, /*end = */ true); }
    const_iterator cend() const noexcept { return const_iterator(this, /*end = */ true); }

/*--- Capacity ---*/
public:
    // Can not mark the methods noexcept because they use the find method that
    // can throw (because of string comparison).
    bool empty() const { return size() > 0; }

    size_type size() const 
    {
        if (!valid(mp__registry))
            return 0;

        if constexpr (std::is_same_v<B, ViewBehaviour::Exclude>)
        {
            size_type count = mp__registry->size();

            // Remove the elements to exclude but only if they actually exist
            for (const auto& id : value_or_empty(mp__u_ids))
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
            for (const auto& id : value_or_empty(mp__u_ids))
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
        using iterator_category = std::forward_iterator_tag;
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
        bool f__end;

    /*--- Member functions ---*/
    /*--- (constructor) ---*/
    public:
        Iterator() noexcept :
            m__range(nullptr), 
            m__iter(),
            f__end(true)
        { }
        // For begin operator
        Iterator(RV* range) noexcept :
            Iterator(range, false)
        { }
        // For end operator
        Iterator(RV* range, bool end) noexcept :
            m__range(range), 
            m__iter(),
            f__end()
        {
            // If end iterator, nothing to be done. Otherwise, I need to find the first valid element.
            // It's an end iterator when: I ask, or ur view doesn't point to a registry, or the registry is empty.
            if (end || !valid(m__range) || !valid(m__range->mp__registry) || m__range->mp__registry->empty())
            {
                f__end = true;
                return;
            }
            
            // Based on the B, I need to find the first valid element in the Registry. 
            // It could easily go all the way to the end iterator again.
            // Use try catch to guarantee noexcept in case ENABLE_SAFETY_CHECKS is on.
            try
            {
                if constexpr (std::is_same_v<typename RV::behaviour_type, ViewBehaviour::Exclude>)
                {
                    m__iter = m__range->mp__registry->begin();
                    while (m__iter != m__range->mp__registry->end() && m__range->mp__u_ids->contains(m__iter->id))
                        ++m__iter;
                }

                if constexpr (std::is_same_v<typename RV::behaviour_type, ViewBehaviour::Include>)
                {
                    m__iter = m__range->mp__registry->begin();
                    while (m__iter != m__range->mp__registry->end() && !m__range->mp__u_ids->contains(m__iter->id))
                        ++m__iter;
                }

                if constexpr (std::is_same_v<typename RV::behaviour_type, ViewBehaviour::OrderedInclude>)
                {
                    auto curr_id = m__range->mp__u_ids->begin();
                    do
                    {
                        m__iter = m__range->mp__registry->find(*curr_id);
                        ++curr_id;
                    }
                    while (m__iter == m__range->mp__registry->end() && curr_id != m__range->mp__u_ids->end());
                }

                f__end = m__iter == m__range->mp__registry->end();

            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
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
        base_iter& base() { return m__iter; }
        const base_iter& base() const { return m__iter; }

    /*--- access operators ---*/
    public:
        reference operator*() const {
#ifdef ENABLE_SAFETY_CHECKS
            if (f__end)
                __format_and_throw<std::out_of_range>("RegistryView::Iterator::operator*()", "Impossible to dereference the iterator.",
                    "The iterator is at the end.");
#endif
            return *m__iter;
        }

        pointer operator->() const { 
#ifdef ENABLE_SAFETY_CHECKS
            if (f__end)
                __format_and_throw<std::out_of_range>("RegistryView::Iterator::operator->()", "Impossible to dereference the iterator.",
                    "The iterator is at the end.");
#endif
            return m__iter.operator->();
        }

    /*--- increment/decrement operators ---*/
    public:
        iterator_type &operator++()
        {
#ifdef ENABLE_SAFETY_CHECKS
            if (f__end)
                __format_and_throw<std::out_of_range>("RegistryView::Iterator::operator++()", "Impossible to increment the iterator.",
                    "The iterator is at the end.");
            if (!valid(m__range) || !valid(m__range->mp__registry))
                __format_and_throw<std::out_of_range>("RegistryView::Iterator::operator++()", "Impossible to increment the iterator.",
                    "The registry is not available.");
            if (m__iter == m__range->mp__registry->end())
                __format_and_throw<std::out_of_range>("RegistryView::Iterator::operator++()", "Impossible to increment the iterator.",
                    "The iterator is at the end.");
#endif
            // Based on the style, I need to find the next valid element in the Registry.
            // If it is exclude, I move forward more than once, if my next element is in the list of excluded elements.
            // If it is include, I move forward more than once, if my next element is not in the list of included elements.
            // If it is ordered include, I simply find the next one

            if constexpr (std::is_same_v<typename RV::behaviour_type, ViewBehaviour::Exclude>)
            {
                do
                {
                    ++m__iter;
                } while (m__iter != m__range->mp__registry->end() && m__range->mp__u_ids->contains(m__iter->id));
            }
            
            if constexpr (std::is_same_v<typename RV::behaviour_type, ViewBehaviour::Include>)
            {
                do
                {
                    ++m__iter;
                } while (m__iter != mp__registry->end() && !m__range->mp__u_ids->contains(m__iter->id));
            }
            
            if constexpr (std::is_same_v<typename RV::behaviour_type, ViewBehaviour::OrderedInclude>)
            {
                // I am pointing at ID x, find it in the USS, get the next, find it in the registry, get the iterator.
                auto curr_id = m__range->mp__u_ids->find(m__iter->id);
                auto next_id = std::next(curr_id);

                // If I can't find it (should not happen at all) or I am at the end of the list, I am done.
                if (curr_id == m__ids.end() || next_id == m__ids.end())
                {
                    m__iter = m__range->mp__registry->end();
                    f__end = true;
                    return *this;
                }
                
                // else, I have a valid next element, find it in the registry.
                m__iter = m__range->mp__registry->find(*next_id);
            }

            f__end = m__iter == m__range->mp__registry->end();  

            return *this;
        }
        iterator_type operator++(int) {auto tmp= *this; ++(*this); return tmp;}

        iterator_type &operator+=(size_type n)
        {
            while (n)
                ++(*this);
                    --n;

            return *this;
        }
        iterator_type operator+(size_type n) const {auto tmp= *this; return tmp += n;}

        difference_type operator-(const iterator_type &other) const { return m__iter - other.m__iter; }

    /*--- comparison operators ---*/
    public:
        bool operator==(const iterator_type &other) const
        {
#ifdef ENABLE_SAFETY_CHECKS
            check_comparable(other);
#endif
            if (f__end && other.f__end)
                return true;

            return f__end == other.f__end && m__iter == other.m__iter;
        }

        bool operator!=(const iterator_type &other) const
        {
#ifdef ENABLE_SAFETY_CHECKS
            check_comparable(other);
#endif
            if (f__end && other.f__end)
                return false;
            
            return f__end != other.f__end || m__iter != other.m__iter;
        }

        bool operator<(const iterator_type &other) const
        {
#ifdef ENABLE_SAFETY_CHECKS
            check_comparable(other);
#endif
            if (f__end && other.f__end)
                return false;

            if (other.f__end)
                return true;

            return !f__end && m__iter < other.m__iter;
        }

        bool operator>(const iterator_type &other) const
        {
#ifdef ENABLE_SAFETY_CHECKS
            check_comparable(other);
#endif
            if (f__end && other.f__end)
                return false;

            if (f__end)
                return true;

            return !other.f__end && m__iter > other.m__iter;
        }

        bool operator<=(const iterator_type &other) const
        {
#ifdef ENABLE_SAFETY_CHECKS
            check_comparable(other);
#endif    
            if (f__end && other.f__end)
                return true;

            if (other.f__end)
                    return false;

            return !f__end && m__iter <= other.m__iter;
        }

        bool operator>=(const iterator_type &other) const {
#ifdef ENABLE_SAFETY_CHECKS
            check_comparable(other);
#endif
            if (f__end && other.f__end)
                return true;

            if (f__end)
                return false;

            return !other.f__end && m__iter >= other.m__iter;
        }

// Helper for check if the iterator is valid.
#ifdef ENABLE_SAFETY_CHECKS
    private:    
        void check_comparable(const iterator_type &other) const
        {
            if (!valid(m__range) || !valid(m__range->mp__registry))
                __format_and_throw<std::out_of_range>("RegistryView::Iterator", "operator==", "Impossible to compare the iterators.",
                    "The registry is not available.");
            if (m__range != other.m__range)
                __format_and_throw<std::out_of_range>("RegistryView::Iterator", "operator==", "Impossible to compare the iterators.",
                    "The iterators are not from the same registry.");
        }
#endif    
    }; // class RegistryView::Iterator

}; // class RegistryView

} // namespace bevarmejo
