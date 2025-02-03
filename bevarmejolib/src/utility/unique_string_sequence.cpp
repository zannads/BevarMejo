#include <algorithm>
#include <initializer_list>
#include <string>
#include <vector>

#include "bevarmejo/utility/except.hpp"

#include "unique_string_sequence.hpp"

namespace bevarmejo
{
// I define a class that is a ordered (in the order of insertion) sequence of unique strings.
// This is why I redefine it, instead of using a std::set<std::string> (would not be ordered).
// or instead of using a std::vector<std::string> (would not be unique).
// Everything is like using a simple std::vector<std::string> but there is the extra guarantee that
// the strings are unique.

/*------- Member functions -------*/
// (constructor)
UniqueStringSequence::UniqueStringSequence(const Container &elements) : 
m__elements()
{ 
    for (const auto &el : elements)
        push_back(el);
}

// operator=
UniqueStringSequence &UniqueStringSequence::operator=(const Container &elements)
{
    clear();
    for (const auto &el : elements)
        push_back(el);
    return *this;
}

void UniqueStringSequence::assign(const UniqueStringSequence::Container &elements) { this->operator=(elements); }

/*--- Iterators ---*/
auto UniqueStringSequence::begin() noexcept -> iterator { return m__elements.begin(); }
auto UniqueStringSequence::begin() const noexcept -> iterator { return m__elements.begin(); }
auto UniqueStringSequence::cbegin() const noexcept -> const_iterator { return m__elements.cbegin(); }

auto UniqueStringSequence::end() noexcept -> iterator { return m__elements.end(); }
auto UniqueStringSequence::end() const noexcept -> iterator { return m__elements.end(); }
auto UniqueStringSequence::cend() const noexcept -> const_iterator { return m__elements.cend(); }

auto UniqueStringSequence::rbegin() noexcept -> reverse_iterator { return m__elements.rbegin(); }
auto UniqueStringSequence::rbegin() const noexcept -> reverse_iterator { return m__elements.rbegin(); }
auto UniqueStringSequence::crbegin() const noexcept -> const_reverse_iterator { return m__elements.crbegin(); }

auto UniqueStringSequence::rend() noexcept -> reverse_iterator { return m__elements.rend(); }
auto UniqueStringSequence::rend() const noexcept -> reverse_iterator { return m__elements.rend(); }
auto UniqueStringSequence::crend() const noexcept -> const_reverse_iterator { return m__elements.crend(); }

/*--- Capacity ---*/
auto UniqueStringSequence::empty() const noexcept -> bool { return m__elements.empty(); }

auto UniqueStringSequence::size() const noexcept -> size_type { return m__elements.size(); }

auto UniqueStringSequence::max_size() const noexcept -> size_type { return m__elements.max_size(); }

void UniqueStringSequence::reserve(size_type new_cap) { m__elements.reserve(new_cap); }

auto UniqueStringSequence::capacity() const noexcept -> size_type { return m__elements.capacity(); }

void UniqueStringSequence::shrink_to_fit() { m__elements.shrink_to_fit(); }

/*--- Element access ---*/
// If we return a reference, the user can modify the elements of the sequence.
// Meaning that the sequence is not unique anymore.
auto UniqueStringSequence::at(size_type pos) -> const_reference { return m__elements.at(pos); }
auto UniqueStringSequence::at(size_type pos) const -> const_reference { return m__elements.at(pos); }

auto UniqueStringSequence::operator[](size_type pos) -> const_reference { return m__elements.at(pos); }
auto UniqueStringSequence::operator[](size_type pos) const -> const_reference { return m__elements.at(pos); }

auto UniqueStringSequence::front() -> const_reference { return m__elements.front(); }
auto UniqueStringSequence::front() const -> const_reference { return m__elements.front(); }

auto UniqueStringSequence::back() -> const_reference { return m__elements.back(); }
auto UniqueStringSequence::back() const -> const_reference { return m__elements.back(); }

/*--- Modifiers ---*/
void UniqueStringSequence::clear() noexcept { m__elements.clear(); }

auto UniqueStringSequence::insert(const_iterator pos, const value_type &id) -> insert_return_type
{
    auto it = std::find(m__elements.begin(), m__elements.end(), id);
    if (it != m__elements.end())
        return {it, false};

    size_type idx = pos - cbegin();
    m__elements.insert(m__elements.begin() + idx, id);
    return {m__elements.begin() + idx, true};
}
auto UniqueStringSequence::insert(const_iterator pos, value_type &&id) -> insert_return_type
{
    auto it = std::find(m__elements.begin(), m__elements.end(), id);
    if (it != m__elements.end())
        return {it, false};

    size_type idx = pos - cbegin();
    m__elements.insert(m__elements.begin() + idx, std::move(id));
    return {m__elements.begin() + idx, true};
}
auto UniqueStringSequence::insert(const_iterator pos, const_iterator first, const_iterator last) -> insert_iter_return_type
{
    auto res = insert_iter_return_type{end(), 0, 0};
    for (auto it=first; it!=last; ++it)
    {
        auto [iter, inserted] = insert(pos, *it);
        if (inserted)
        {
            res.last = iter;
            ++res.inserted;
        }
        ++res.total;
    }
    return res;
}
auto UniqueStringSequence::insert(const_iterator pos, std::initializer_list<std::string> ilist) -> insert_iter_return_type
{
    auto res = insert_iter_return_type{end(), 0, 0};
    for (auto it=ilist.begin(); it!=ilist.end(); ++it)
    {
        auto [iter, inserted] = insert(pos, *it);
        if (inserted)
        {
            res.last = iter;
            ++res.inserted;
        }
        ++res.total;
    }
    return res;
}

auto UniqueStringSequence::erase(const_iterator pos) -> iterator
{
    return m__elements.erase(pos);
}
auto UniqueStringSequence::erase(const_iterator first, const_iterator last) -> iterator
{
    return m__elements.erase(first, last);
}
auto UniqueStringSequence::erase(const value_type &id) -> iterator
{
    return erase(const_cast<const UniqueStringSequence *>(this)->find(id));
}

auto UniqueStringSequence::push_back(const value_type &id) -> iterator
{
    auto it = std::find(m__elements.begin(), m__elements.end(), id);
    if (it != m__elements.end())
        return it;

    m__elements.push_back(id);
    return m__elements.end() - 1;
}
auto UniqueStringSequence::push_back(value_type &&id) -> iterator
{
    auto it = std::find(m__elements.begin(), m__elements.end(), id);
    if (it != m__elements.end())
        return it;

    m__elements.push_back(std::move(id));
    return m__elements.end() - 1;
}

void UniqueStringSequence::pop_back() { m__elements.pop_back(); }

// No resize method

void UniqueStringSequence::swap(UniqueStringSequence &other) noexcept { m__elements.swap(other.m__elements); }

/*--- Lookup ---*/
auto UniqueStringSequence::count(const value_type &id) const -> size_type
{
    return find(id) != cend() ? 1 : 0;
}

auto UniqueStringSequence::find(const value_type &id) -> iterator
{
    return std::find_if(m__elements.begin(), m__elements.end(), [&id](const value_type &inst) { return inst == id; });
}
auto UniqueStringSequence::find(const value_type &id) const -> const_iterator
{
    return std::find_if(m__elements.begin(), m__elements.end(), [&id](const value_type &inst) { return inst == id; });
}

bool UniqueStringSequence::contains(const value_type &id) const
{
    return find(id) != cend();
}

} // namespace bevarmejo
