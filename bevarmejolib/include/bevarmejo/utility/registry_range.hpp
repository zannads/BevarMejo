#pragma once

#include <algorithm>
#include <memory>
#include <initializer_list>
#include <string>
#include <vector>

#include "bevarmejo/bemexcept.hpp"
#include "bevarmejo/utility/safe_member_ptr.hpp"
#include "bevarmejo/utility/registry.hpp"
#include "bevarmejo/utility/unique_string_sequence.hpp"


namespace bevarmejo::wds
{

// Now that I have defined a class the keeps track of the IDs of the elements in the network
// that I want to see (or not to see). I can define a class that is a range of the registry
// and by inherting from the UniqueStringSequence I know I can already modify the 
// sequence of IDs that I want to see.
// There are 3 types of ranges: Exclude Ids, Include Only (Registry order), Include Only (View order).

struct Exclude { };
struct Include { };
struct OrderedInclude { };

template <typename R, typename Style,
            typename = std::enable_if_t<
                std::is_same_v<Style, Exclude> || std::is_same_v<Style, Include> || std::is_same_v<Style, OrderedInclude>
            >>
class RegistryRange final : public UniqueStringSequence
{
/*------- Member types -------*/
private:
    using inherited = UniqueStringSequence;
    using Reg = Registry<R>;
    using const_Reg = const Registry<R>;
    using style_type = Style;
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
    using iterator = Iterator<RegistryRange<R, Style>>;
    using const_iterator = Iterator<const RegistryRange<R, Style>>;
    using reverse_iterator = ReverseIterator<RegistryRange<R, Style>>;
    using const_reverse_iterator = ReverseIterator<const RegistryRange<R, Style>>;

private:
    friend iterator;
    friend const_iterator;

/*------- Member objects -------*/
private:
#ifdef ENABLE_SAFETY_CHECKS
    SafeMemberPtr<Reg> m__registry;
#else
    Reg* m__registry;
#endif

/*------- Member functions -------*/
/*--- (constructor) ---*/
public:
    RegistryRange() :
        inherited(),
        m__registry(nullptr)
    { }
    template <typename... Args>
    RegistryRange(Args&&... args) :
        inherited(std::forward<Args>(args)...),
        m__registry(nullptr)
    { }
    template <typename... Args>
    RegistryRange(const Registry* registry, Args&&... args) :
        inherited(std::forward<Args>(args)...),
        m__registry(registry)
    { }
    RegistryRange(const RegistryRange &other) = default;
    RegistryRange(RegistryRange &&other) noexcept = default;

/*--- (destructor) ---*/
public:
    ~RegistryRange() = default;

/*--- operator= ---*/
public:
    RegistryRange &operator=(const RegistryRange &rhs) = default;
    RegistryRange &operator=(RegistryRange &&rhs) noexcept = default;

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
        if (!m__registry)
            __format_and_throw<std::runtime_error>("RegistryRange", "at", "The registry is not set.");

        // If the style is Exclude, asking for an element that is in the list of excluded elements, it is like asking for a non-existing element.
        // If the style is Include or OrderedInclude, asking for an element that is not in the list of included elements, it is like asking for a non-existing element.
        if constexpr (std::is_same_v<style_type, Exclude>)
        {
            if (inherited::contains(id))
                __format_and_throw<std::out_of_range>("RegistryRange", "at", "The element is excluded.");
        }
        else // if constexpr (std::is_same_v<style_type, Include> || std::is_same_v<style_type, OrderedInclude>)
        {
            if (!inherited::contains(id))
                __format_and_throw<std::out_of_range>("RegistryRange", "at", "The element is not included.");
        }
        
        return m__registry->at(id);
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

    iterator end() noexcept { return iterator(this, m__registry->size()); }
    const_iterator end() const noexcept { return const_iterator(this, m__registry->size()); }
    const_iterator cend() const noexcept { return const_iterator(this, m__registry->size()); }

    reverse_iterator rbegin() noexcept { return reverse_iterator(this, m__registry->size()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(this ,m__registry->size()); }
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(this, m__registry->size()); }

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
        if (m__registry == nullptr)
            return 0;

        if constexpr (std::is_same_v<Style, Exclude>)
        {
            size_type count = m__registry->size();

            // Remove the elements to exclude but only if they actually exist
            for (const auto& id : m__elements)
            {
                if (m__registry->contains(id))
                    --count;
            }

            return count;
        }

        // if constrexpr (std::is_same_v<Style, Include> || std::is_same_v<Style, OrderedInclude>)
        size_type count = 0;

        // Count the elements to include but only if they actually exist
        for (const auto& id : m__elements)
        {
            if (m__registry->contains(id))
                ++count;
        }

        return count;
    }

// These are not needed for the RegistryRange. They are already implemented in the UniqueStringSequence.
    // size_type max_size() const noexcept;

    // void reserve(size_type new_cap);

    // size_type capacity() const;

    // void shrink_to_fit();

/*--- Modifiers ---*/
public:
    // Modifiers are not needed for the RegistryRange. They are already implemented in the UniqueStringSequence.
    
}; // class RegistryRange

} // namespace bevarmejo::wds
