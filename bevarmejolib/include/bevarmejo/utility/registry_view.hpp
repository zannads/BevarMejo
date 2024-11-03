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
    using const_Reg = const Registry<R>;
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

private:
    friend iterator;
    friend const_iterator;

/*------- Member objects -------*/
private:
#ifdef ENABLE_SAFETY_CHECKS
    SafeMemberPtr<const_Reg> m__registry;
#else
    const_Reg* m__registry;
#endif

/*------- Member functions -------*/
/*--- (constructor) ---*/
public:
    RegistryView() :
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
    

} // namespace bevarmejo::wds
