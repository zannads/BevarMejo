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

    
}; // class RegistryRange

} // namespace bevarmejo::wds
