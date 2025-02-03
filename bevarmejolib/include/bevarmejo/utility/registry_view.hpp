#pragma once

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <string>
#include <type_traits>
#include <vector>

#include "bevarmejo/utility/exceptions.hpp"
#include "bevarmejo/utility/memory.hpp"
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
// With C++20 I could simply use filter() to select or exclude the elements and 
// then sort() to order them. But I am using C++17, so I need to define my own
// class to do this.

// Let's start by defining the behaviour of the view, to allow for template 
// specialization and the type traits.
namespace RVMode
{
    struct Exclude {};
    struct Include {};
    struct OrderedInclude {};
}   // namespace RVMode

template <typename T>
inline constexpr bool is_registry_view_behaviour_v = std::is_same_v<T, RVMode::Exclude> ||
                                                     std::is_same_v<T, RVMode::Include> ||
                                                     std::is_same_v<T, RVMode::OrderedInclude>;

// Tuple of the View Modes
using ViewModes = std::tuple<RVMode::Exclude, RVMode::Include, RVMode::OrderedInclude>;

// A view needs a reference to the registry (it doesn't make sense without referring to a registry)
// and a pointer to a UniqueStringSequence that contains the IDs of the elements to include or exclude
// (pointer because it can be null, for example, to not exclude any element).
// The view can be created on a const or non-const registry.
// In both cases, I can't modify the registry, but in the non-const case, I can 
// modify the elements of the registry.
// To simplify the implementation, I will expose only the template T and base on
// a const or not template option, I will define the return type of the methods.
template <typename T, bool IsMutable>
class RegistryViewCore
{
/*------- Member types -------*/
private:
    using self_type = RegistryViewCore<T, IsMutable>;
protected:
    using Registry_t = std::conditional_t<IsMutable, Registry<T>, const Registry<T>>;
    using Registry_pointer = Registry_t*;
    using const_Registry_pointer = const Registry_t*;
    using Registry_reference = std::conditional_t<IsMutable, Registry<T>&, const Registry<T>&>;
    using const_Registry_reference = const Registry_t&;
    using USS = UniqueStringSequence;
    using mutable_type = std::integral_constant<bool, IsMutable>;
    using USS_pointer = std::weak_ptr<const USS>;
public:
    using key_type = typename Registry_t::key_type;
    using mapped_type = typename Registry_t::mapped_type;
    using value_type = typename Registry_t::value_type;
    using size_type = typename Registry_t::size_type;
    using difference_type = typename Registry_t::difference_type;
    using reference = std::conditional_t<IsMutable, typename Registry_t::reference, typename Registry_t::const_reference>;
    using const_reference = typename Registry_t::const_reference;
    using pointer = std::conditional_t<IsMutable, typename Registry_t::pointer, typename Registry_t::const_pointer>;
    using const_pointer = typename Registry_t::const_pointer;
    using mapped_reference = std::conditional_t<IsMutable, typename Registry_t::mapped_type&, const typename Registry_t::mapped_type&>;
    using const_mapped_reference = const typename Registry_t::mapped_type&;

/*------- Member objects -------*/
protected:
    Registry_reference m__registry; // Reference to Registry
    USS_pointer p__uss; // Pointer to Unique ID Sequence

/*------- Member functions -------*/
/*--- (constructor) ---*/
public:
    RegistryViewCore() = delete; // I need to know the registry.
    explicit RegistryViewCore(Registry_reference registry) noexcept : 
        m__registry(registry),
        p__uss()
    { }
    template <typename U, typename = std::enable_if_t<std::is_constructible_v<USS_pointer, U>>>
    explicit RegistryViewCore(Registry_reference registry, U&& elements) noexcept : 
        m__registry(registry),
        p__uss(std::forward<U>(elements))
    { }
    RegistryViewCore(const self_type& other) noexcept = default;
    RegistryViewCore(self_type&& other) noexcept = default;

/*--- (destructor) ---*/
public:
    virtual ~RegistryViewCore() = default;

/*--- operator= ---*/
public:
    self_type& operator=(const self_type& rhs) = delete;
    self_type& operator=(self_type&& rhs) noexcept = delete;

}; // class RegistryViewCore

template <typename T, typename M,
          bool IsMutable, 
          typename = std::enable_if_t<is_registry_view_behaviour_v<M>>>
class RegistryView final : public RegistryViewCore<T, IsMutable>
{
/*------- Member types -------*/
private:
    using self_type = RegistryView<T, M, IsMutable>;
    using core_type = RegistryViewCore<T, IsMutable>;
    using mode_type = M;
    using mutable_type = std::integral_constant<bool, IsMutable>;
public:
    using key_type = typename core_type::key_type;
    using mapped_type = typename core_type::mapped_type;
    using value_type = typename core_type::value_type;
    using size_type = typename core_type::size_type;
    using difference_type = typename core_type::difference_type;
    using reference = typename core_type::reference;
    using const_reference = typename core_type::const_reference;
    using pointer = typename core_type::pointer;
    using const_pointer = typename core_type::const_pointer;
    using mapped_reference = typename core_type::mapped_reference;
    using const_mapped_reference = typename core_type::const_mapped_reference;
private:
    // Forward declaration of the iterators (I need two different types because the sorted one has a completely different behaviour).
    template <class RV>
    class FilterIterator; // For the Exclude and Include modes
    template <class RV>
    class OrderedFilterIterator; // For the OrderedInclude mode
public:
    using iterator = std::conditional_t<
        std::is_same_v<mode_type, RVMode::OrderedInclude>,
        OrderedFilterIterator<self_type>,
        FilterIterator<self_type>
    >;
    using const_iterator = std::conditional_t<
        std::is_same_v<mode_type, RVMode::OrderedInclude>,
        OrderedFilterIterator<const self_type>,
        FilterIterator<const self_type>
    >;
 
private:
    friend iterator;
    friend const_iterator;

/*------- Member objects -------*/
// Everything is inherited from the core class.

/*------- Member functions -------*/
/*--- (constructor) ---*/
public:
    RegistryView() = delete; // I need to know the registry.
    RegistryView(const self_type& other) noexcept = default;
    RegistryView(self_type&& other) noexcept = default;
    template <typename... Args>
    explicit RegistryView(Args&&... args) noexcept : 
        core_type(std::forward<Args>(args)...)
    { }

/*--- (destructor) ---*/
public:
    ~RegistryView() = default;

/*--- operator= ---*/
public:
    self_type& operator=(const self_type& rhs) = delete;
    self_type& operator=(self_type&& rhs) noexcept = delete;
    
/*------- Element access -------*/
/*
public:
    mapped_type& at(const key_type& id)
    { }
    const mapped_type& at(const key_type& id) const
    { }
    reference at(size_type pos) { return *(begin()+pos); }
    const_reference at(size_type pos) const { return *(cbegin()+pos); }

    reference front() { return *begin(); }
    const_reference front() const { return *begin(); }

    reference back() { return *(end() - 1); }
    const_reference back() const { return *(end() - 1); }
*/
/*--- Iterators ---*/
public:
    iterator begin() noexcept { return                    iterator(this, 0, 0); }
    const_iterator begin()  const noexcept { return const_iterator(this, 0, 0); }
    const_iterator cbegin() const noexcept { return const_iterator(this, 0, 0); }

    iterator end() noexcept { return                    iterator(this, this->m__registry.size(), this->p__uss.expired() ? 0 : this->p__uss.lock()->size()); }
    const_iterator end()  const noexcept { return const_iterator(this, this->m__registry.size(), this->p__uss.expired() ? 0 : this->p__uss.lock()->size()); }
    const_iterator cend() const noexcept { return const_iterator(this, this->m__registry.size(), this->p__uss.expired() ? 0 : this->p__uss.lock()->size()); }

/*--- Capacity ---*/
public:
    // The complexity of these methods is O(n) because I need to iterate over the registry.
    // I could make them noexcept when the SAFETY_CHECKS are off, but let's not go there.

    bool empty() const
    {
        return size() > 0;
    }

    size_type size() const
    {
        // Special cases for when the UniqueStringSequence is non-existent or 
        // empty. In these cases, it depends on the mode and the registry and 
        // I can tell with constant time complexity.
        // If we are excluding no element, the size is the registry's size.
        // On the other hand, if we are including no element, the size is 0.
        if (this->p__uss.expired() || this->p__uss.lock()->empty())
        {
            if constexpr (std::is_same_v<mode_type, RVMode::Exclude>)
            {
                return this->m__registry.size();
            }
            else if constexpr (std::is_same_v<mode_type, RVMode::Include>)
            {
                return 0;
            }
        }
        
        size_type count = 0;
        for (auto it = begin(); it != end(); ++it)
            ++count;

        return count;
    }

/*--- Modifiers ---*/
public:
    // Modifiers are not needed for the RegistryView.

/*--- Lookup ---*/
public:
    // TODO: Implement the find, count and contains method.
    
/*--- Iterators ---*/
private:
    template <class RV>
    class FilterIterator
    {
    /*--- Member types ---*/
    private:
        using self_type = FilterIterator<RV>;
        using mutable_type = std::integral_constant<bool, IsMutable>;
    public:
        using iterator_type = self_type;
        using base_iter = typename std::conditional<
            std::is_const_v<RV> || !IsMutable,
            typename RV::Registry_t::const_iterator,
            typename RV::Registry_t::iterator
        >::type;
        using iterator_category = std::forward_iterator_tag;
        using value_type = typename base_iter::value_type;
        using size_type = typename base_iter::size_type;
        using difference_type = typename base_iter::difference_type;
        using reference = typename base_iter::reference;
        using pointer = typename base_iter::pointer;

    /*--- Member objects ---*/
    private:
        RV* p__range;
        size_type i__reg; // Index of the element in the registry.
        size_type i__uss; // Index of the element in the UniqueStringSequence.

    /*--- Member functions ---*/
    /*--- (constructor) ---*/
    public:
        FilterIterator() noexcept = delete;
        FilterIterator(RV* range, size_type index_reg, size_type index_uss) noexcept :
            p__range(range),
            i__reg(0),
            i__uss(0)
        {
            static_assert(std::is_same_v<mode_type, RVMode::Exclude> || std::is_same_v<mode_type, RVMode::Include>,
                "The FilterIterator can only be used with Exclude or Include modes.");

            // If the UniqueStringSequence is non-existent or empty, I got to the end
            // iterator for the include mode, and the required iterator for the exclude mode.
            if (p__range->p__uss.expired() || p__range->p__uss.lock()->empty())
            {
                if constexpr(std::is_same_v<mode_type, RVMode::Exclude>)
                {
                    i__reg = index_reg;
                }
                else
                {
                    i__reg = p__range->m__registry.size();
                }
                return;
            }

            // Index_reg is as candidate_i__reg in the increment operator.
            // See the rationale there.

            auto p__uss = p__range->p__uss.lock();

            for (; index_reg < p__range->m__registry.size(); ++index_reg)
            {
                bool condition_target = false;
                if constexpr (std::is_same_v<mode_type, RVMode::Include>)
                {
                    condition_target = true;
                }

                auto candidate_it__uss = p__uss->find(p__range->m__registry.at(index_reg).name);
                if (condition_target == (candidate_it__uss != p__uss->end()))
                {
                    i__reg = index_reg;
                    return;
                }
            }
            i__reg = p__range->m__registry.size();
            return;
        }
        FilterIterator(const FilterIterator &other) noexcept = default;
        FilterIterator(FilterIterator &&other) noexcept = default;
        
    /*--- (destructor) ---*/
    public:
        ~FilterIterator() = default;
        
    /*--- operator= ---*/
    public:
        FilterIterator &operator=(const FilterIterator &rhs) noexcept = default;
        FilterIterator &operator=(FilterIterator &&rhs) noexcept = default;
        
    /*--- base ---*/
    protected:
        base_iter base() const
        {
            return p__range->m__registry.begin() + i__reg;
        }
       
    /*--- access operators ---*/
    public:
        reference operator*() const { return *base(); }

        pointer operator->() const { return base().operator->(); }
        
    /*--- increment/decrement operators ---*/
    public:
        iterator_type& operator++()
        {
            // We search for the next element in the registry that is (Include) 
            // or is not (Exclude) in the UniqueStringSequence.
            // We always only move the registry index and the uss index stays to 0.

            // Special cases for when the UniqueStringSequence is non-existent or
            // empty.
            if (p__range->p__uss.expired() || p__range->p__uss.lock()->empty())
            {
                if constexpr(std::is_same_v<mode_type, RVMode::Exclude>)
                {
                    assertm(i__reg < p__range->m__registry.size(), "Impossible to increment the iterator. The index is out of range.");
                    ++i__reg;
                }
                else
                {
                    i__reg = p__range->m__registry.size();
                }
                return *this;
            }

            // We can search for the next element in the registry that needs to 
            // be included or excluded, because we have a valid UniqueStringSequence.
            auto p__uss = p__range->p__uss.lock();

            // We need to find the next element in the registry, therefore we try
            // and see if more steps are needed.
            for (auto candidate_i__reg = i__reg + 1; candidate_i__reg < p__range->m__registry.size(); ++candidate_i__reg)
            {
                // By default it is in exclude mode.
                // So, we stop when we find an element that is not in the UniqueStringSequence.
                bool condition_target = false;
                // On the other hand, in include mode, we stop when we find the
                // element in the UniqueStringSequence.
                if constexpr(std::is_same_v<mode_type, RVMode::Include>)
                {
                    condition_target = true;
                }

                // Let's look for it, if not found it will return -1.
                auto candidate_it__uss = p__uss->find(p__range->m__registry.at(candidate_i__reg).name);
                if (condition_target == (candidate_it__uss != p__uss->end()))
                {
                    i__reg = candidate_i__reg;
                    return *this;
                }
            }
            // We got to the end of the registry.
            i__reg = p__range->m__registry.size();
            return *this;
        }
        iterator_type operator++(int)
        {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }
        
        iterator_type& operator+=(size_type n)
        {
            while (n--)
            {
                ++(*this);
            }
            return *this;
        }
        iterator_type operator+(size_type n) const
        {
            auto tmp = *this;
            return tmp += n;
        }

        difference_type operator-(const iterator_type &other) const
        {
            // I can't simply do the difference between the indexes because the
            // indexes are not contiguous. I need to iterate over the registry
            // to find the difference.
            assertm(p__range == other.p__range, "Impossible to compare the iterators. The iterators are not from the same registry.");
            
            difference_type diff = 0;
            if (*this < other)
            {
                auto it = *this;
                while (it++ != other)
                {
                    ++diff;
                }

                return diff;
            }
            else
            {
                auto it = other;
                while (it++ != *this)
                {
                    --diff;
                }

                return diff;
            }
        }

    /*--- comparison operators ---*/
    public:
        bool operator==(const iterator_type &other) const
        {
            assertm(p__range == other.p__range, "Impossible to compare the iterators. The iterators are not from the same registry.");
            return i__reg == other.i__reg;
        }

        bool operator!=(const iterator_type &other) const
        {
            assertm(p__range == other.p__range, "Impossible to compare the iterators. The iterators are not from the same registry.");
            return i__reg != other.i__reg;
        }

        bool operator<(const iterator_type &other) const
        {
            assertm(p__range == other.p__range, "Impossible to compare the iterators. The iterators are not from the same registry.");
            return i__reg < other.i__reg;
        }

        bool operator>(const iterator_type &other) const
        {
            assertm(p__range == other.p__range, "Impossible to compare the iterators. The iterators are not from the same registry.");
            return i__reg > other.i__reg;
        }

        bool operator<=(const iterator_type &other) const
        {
            assertm(p__range == other.p__range, "Impossible to compare the iterators. The iterators are not from the same registry.");
            return i__reg <= other.i__reg;
        }

        bool operator>=(const iterator_type &other) const
        {
            assertm(p__range == other.p__range, "Impossible to compare the iterators. The iterators are not from the same registry.");
            return i__reg >= other.i__reg;
        }
    }; // class RegistryView::FilterIterator

    template <class RV>
    class OrderedFilterIterator
    {
    /*--- Member types ---*/
    private:
        using self_type = OrderedFilterIterator<RV>;
        using mutable_type = std::integral_constant<bool, IsMutable>;
    public:
        using iterator_type = self_type;
        using base_iter = typename std::conditional<
            std::is_const_v<RV> || !IsMutable,
            typename RV::Registry_t::const_iterator,
            typename RV::Registry_t::iterator
        >::type;
        using iterator_category = std::forward_iterator_tag;
        using value_type = typename base_iter::value_type;
        using size_type = typename base_iter::size_type;
        using difference_type = typename base_iter::difference_type;
        using reference = typename base_iter::reference;
        using pointer = typename base_iter::pointer;

    /*--- Member objects ---*/
    private:
        RV* p__range;
        size_type i__reg; // Index of the element in the registry.
        size_type i__uss; // Index of the element in the UniqueStringSequence.

    /*--- Member functions ---*/
    /*--- (constructor) ---*/
    public:
        OrderedFilterIterator() noexcept = delete; // I need to know the range.
        OrderedFilterIterator(RV* range, size_type index_reg, size_type index_uss) noexcept :
            p__range(range),
            i__reg(0),
            i__uss(0)
        {
            static_assert(std::is_same_v<typename RV::mode_type, RVMode::OrderedInclude>,
                "This iterator is only for the OrderedInclude mode.");

            if(p__range->p__uss.expired() || p__range->p__uss.lock()->empty())
            {
                i__reg = p__range->m__registry.size();
                i__uss = 0;
                return;
            }

            auto p__uss = p__range->p__uss.lock();

            auto it_uss = p__uss->begin() + index_uss;
            while (it_uss != p__uss->end())
            {
                auto pos = p__range->m__registry.find_index(*it_uss);
                if (pos != -1)
                {
                    i__reg = pos;
                    i__uss = index_uss;
                    return;
                }

                // This element is not in the registry, move to the next one.
                ++it_uss;
            }

            // We could not find the element in the registry.
            i__reg = p__range->m__registry.size();
            i__uss = p__uss->size();
            return;
        }
        OrderedFilterIterator(const OrderedFilterIterator &other) noexcept = default;
        OrderedFilterIterator(OrderedFilterIterator &&other) noexcept = default;

    /*--- (destructor) ---*/
    public:
        ~OrderedFilterIterator() = default;
        
    /*--- operator= ---*/
    public:
        OrderedFilterIterator &operator=(const OrderedFilterIterator &rhs) noexcept = default;
        OrderedFilterIterator &operator=(OrderedFilterIterator &&rhs) noexcept = default;

    /*--- base ---*/
    protected:
        base_iter base() const
        {
            return p__range->m__registry.begin() + i__reg;
        }

    /*--- access operators ---*/
    public:
        reference operator*() const { return *base(); }

        pointer operator->() const { return base().operator->(); }

    /*--- increment/decrement operators ---*/
    public:
        iterator_type& operator++()
        {
            if(p__range->p__uss.expired() || p__range->p__uss.lock()->empty())
            {
                i__reg = p__range->m__registry.size();
                i__uss = 0;
                return *this;
            }

            auto p__uss = p__range->p__uss.lock();

            for (auto candidate_i__uss = i__uss + 1; candidate_i__uss < p__uss->size(); ++candidate_i__uss)
            {
                auto candidate_i__reg = p__range->m__registry.find_index(p__uss->at(candidate_i__uss));
                if (candidate_i__reg != -1)
                {
                    i__reg = candidate_i__reg;
                    i__uss = candidate_i__uss;
                    return *this;
                }
            }

            // We could not find any new element in the registry.
            i__reg = p__range->m__registry.size();
            i__uss = p__uss->size();
            return *this;
        }
        iterator_type operator++(int)
        {
            auto tmp = *this; 
            ++(*this);
            return tmp;
        }

        iterator_type& operator+=(size_type n)
        {
            while (n--)
            {
                ++(*this);
            }
            return *this;
        }
        iterator_type operator+(size_type n) const
        {
            auto tmp = *this; 
            return tmp += n;
        }

        difference_type operator-(const iterator_type &other) const
        {
            // I can't simply do the difference between the indexes because the
            // indexes are not contiguous. I need to iterate over the registry
            // to find the difference.
            assertm(p__range == other.p__range, "Impossible to compare the iterators. The iterators are not from the same registry.");
            
            difference_type diff = 0;
            if (*this < other)
            {
                auto it = *this;
                while (it++ != other)
                {
                    ++diff;
                }

                return diff;
            }
            else
            {
                auto it = other;
                while (it++ != *this)
                {
                    --diff;
                }

                return diff;
            }
        }

    /*--- comparison operators ---*/
    public:
        bool operator==(const iterator_type &other) const
        {
            assertm(p__range == other.p__range, "Impossible to compare the iterators. The iterators are not from the same registry.");
            return i__uss == other.i__uss;
        }

        bool operator!=(const iterator_type &other) const
        {
            assertm(p__range == other.p__range, "Impossible to compare the iterators. The iterators are not from the same registry.");
            return i__uss != other.i__uss;
        }

        bool operator<(const iterator_type &other) const
        {
            assertm(p__range == other.p__range, "Impossible to compare the iterators. The iterators are not from the same registry.");
            return i__uss < other.i__uss;
        }

        bool operator>(const iterator_type &other) const
        {
            assertm(p__range == other.p__range, "Impossible to compare the iterators. The iterators are not from the same registry.");
            return i__uss > other.i__uss;
        }

        bool operator<=(const iterator_type &other) const
        {
            assertm(p__range == other.p__range, "Impossible to compare the iterators. The iterators are not from the same registry.");
            return i__uss <= other.i__uss;
        }

        bool operator>=(const iterator_type &other) const
        {
            assertm(p__range == other.p__range, "Impossible to compare the iterators. The iterators are not from the same registry.");
            return i__uss >= other.i__uss;
        }
    }; // class RegistryView::OrderedFilterIterator


}; // class RegistryView

// Typedefs
template <typename T, bool IsMutable>
using ExcludingRegistryView = RegistryView<T, RVMode::Exclude, IsMutable>;
template <typename T>
using InputExcludingRegistryView = ExcludingRegistryView<T, false>;
template <typename T>
using OutputExcludingRegistryView = ExcludingRegistryView<T, true>;

template <typename T, bool IsMutable>
using IncludingRegistryView = RegistryView<T, RVMode::Include, IsMutable>;
template <typename T>
using InputIncludingRegistryView = IncludingRegistryView<T, false>;
template <typename T>
using OutputIncludingRegistryView = IncludingRegistryView<T, true>;

template <typename T, bool IsMutable>
using OrderedRegistryView = RegistryView<T, RVMode::OrderedInclude, IsMutable>;
template <typename T>
using InputOrderedRegistryView = OrderedRegistryView<T, false>;
template <typename T>
using OutputOrderedRegistryView = OrderedRegistryView<T, true>;

// Deduction guides
// By default, if I don't pass a UniqueStringSequence, it is of type exclude with a null pointer (so like accessing the whole registry).
template <typename T>
RegistryView(Registry<T>&) -> RegistryView<T, RVMode::Exclude, true>;

template <typename T>
RegistryView(const Registry<T>&) -> RegistryView<T, RVMode::Exclude, false>;

// Factory functions 
template <typename T, typename U>
auto make_exc_registry_view(Registry<T>& registry, U&& elements) -> RegistryView<T, RVMode::Exclude, true>
{
    return RegistryView<T, RVMode::Exclude, true>(registry, std::forward<U>(elements));
}
template <typename T, typename U>
auto make_exc_registry_view(const Registry<T>& registry, U&& elements) -> RegistryView<T, RVMode::Exclude, false>
{
    return RegistryView<T, RVMode::Exclude, false>(registry, std::forward<U>(elements));
}

template <typename T, typename U>
auto make_inc_registry_view(Registry<T>& registry, U&& elements) -> RegistryView<T, RVMode::Include, true>
{
    return RegistryView<T, RVMode::Include, true>(registry, std::forward<U>(elements));
}
template <typename T, typename U>
auto make_inc_registry_view(const Registry<T>& registry, U&& elements) -> RegistryView<T, RVMode::Include, false>
{
    return RegistryView<T, RVMode::Include, false>(registry, std::forward<U>(elements));
}

template <typename T, typename U>
auto make_ord_registry_view(Registry<T>& registry, U&& elements) -> RegistryView<T, RVMode::OrderedInclude, true>
{
    return RegistryView<T, RVMode::OrderedInclude, true>(registry, std::forward<U>(elements));
}
template <typename T, typename U>
auto make_ord_registry_view(const Registry<T>& registry, U&& elements) -> RegistryView<T, RVMode::OrderedInclude, false>
{
    return RegistryView<T, RVMode::OrderedInclude, false>(registry, std::forward<U>(elements));
}

} // namespace bevarmejo
