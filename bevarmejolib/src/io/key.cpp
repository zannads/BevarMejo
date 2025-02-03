#include <initializer_list>
#include <string>
#include <vector>

#include "bevarmejo/utility/except.hpp"
#include "bevarmejo/utility/string.hpp"

#include "bevarmejo/io/json.hpp"

#include "key.hpp" 

namespace bevarmejo::io::log::cname {
static const std::string key = "Key";
}
namespace bevarmejo::io::log::fname {
static const std::string generic_access = "[[access_operator]]";
}

namespace bevarmejo::io
{

#if defined(OUT_STYLE_0)
    constexpr Key::style default_style = Key::style::CamelCase;
#elif defined(OUT_STYLE_1)
    constexpr Key::style default_style = Key::style::KebabCase;
#elif defined(OUT_STYLE_2)
    constexpr Key::style default_style = Key::style::PascalCase;
#elif defined(OUT_STYLE_3)
    constexpr Key::style default_style = Key::style::SentenceCase;
#elif defined(OUT_STYLE_4)
    constexpr Key::style default_style = Key::style::SnakeCase;
#else
    #error "The output style selected is invalid."
#endif
constexpr std::size_t n_styles = 5ul;

Key::Key() : Key (std::initializer_list<std::string>{""}) {}

Key::Key(std::initializer_list<std::string> values) : m__values(values) {}

std::string Key::operator()() const
{
    if constexpr (default_style == style::CamelCase)
    {
        return bevarmejo::sentence_case_to_camel_case(m__values[0]);
    }
    else if constexpr (default_style == style::KebabCase)
    {
        return bevarmejo::sentence_case_to_kebab_case(m__values[0]);
    }
    else if constexpr (default_style == style::PascalCase)
    {
        return bevarmejo::sentence_case_to_pascal_case(m__values[0]);
    }
    else if constexpr (default_style == style::SentenceCase)
    {
        return m__values[0];
    }
    else if constexpr (default_style == style::SnakeCase)
    {
        return bevarmejo::sentence_case_to_snake_case(m__values[0]);
    }
    else
    {
        beme_throw(std::logic_error,
        "Throw in unreachable code.",
        "The default style is not valid.",
        "Style : ", static_cast<int>(default_style)
        );
    }
}

const std::string& Key::operator[](std::size_t alt) const
{
    beme_throw_if(alt >= m__values.size(), std::out_of_range,
        "Impossible to provide the requested key.",
        "Index out of range.",
        "Index : ", alt, 
        "Size : ", m__values.size()
    );

    return m__values[alt];
}

std::string Key::as_camel_case(std::size_t alt) const
{
    return bevarmejo::sentence_case_to_camel_case(this->operator[](alt));
}

std::string Key::as_kebab_case(std::size_t alt) const
{
    return bevarmejo::sentence_case_to_kebab_case(this->operator[](alt));
}

std::string Key::as_pascal_case(std::size_t alt) const
{
    return bevarmejo::sentence_case_to_pascal_case(this->operator[](alt));
}

std::string Key::as_sentence_case(std::size_t alt) const
{
    return this->operator[](alt);
}

std::string Key::as_snake_case(std::size_t alt) const
{
    return bevarmejo::sentence_case_to_snake_case(this->operator[](alt));
}

bool Key::exists_in(const Json &j) const {
    
    for (const auto &key : *this) 
        if (j.contains(key))
            return true;

    return false;
}

std::size_t Key::n_versions() { return n_styles; }

std::size_t Key::n_alternatives() const { return m__values.size()*n_versions(); }

std::size_t Key::size() const { return m__values.size(); }

Key::const_iterator Key::begin() const { return const_iterator(this, 0); }

Key::const_iterator Key::end() const { return const_iterator(this, n_alternatives()); }

Key::style Key::get_out_style() { return default_style; }

bool Key::operator==(const std::string &s) const
{
    for (const auto &key : *this)
    {
        if (key == s)
        {
            return true;
        }
    }
    return false;
}

bool Key::operator==(const std::string_view &s) const
{
    for (const auto &key : *this)
    {
        if (key == s)
        {
            return true;
        }
    }
    return false;
}

bool Key::operator==(const char *s) const
{
    for (const auto &key : *this)
    {
        if (key == s)
        {
            return true;
        }
    }
    return false;
}

bool Key::operator<(const std::string &s) const
{
    for (const auto &key : *this)
    {
        if (key < s)
        {
            return true;
        }
    }
    return false;
}

bool Key::operator<(const std::string_view &s) const
{
    for (const auto &key : *this)
    {
        if (key < s)
        {
            return true;
        }
    }
    return false;
}

bool Key::operator<(const char *s) const
{
    for (const auto &key : *this)
    {
        if (key < s)
        {
            return true;
        }
    }
    return false;
}



// Iterators
Key::const_iterator::const_iterator(const Key* key, std::size_t index) : 
    m__key{key}, m__index{index}
{ }

Key::const_iterator::value_type Key::const_iterator::operator*() const
{
    // m__index is the index of the alternative and version.
    // First check all styles for the main key, then proceed to the alternatives.
    std::size_t alt = m__index / n_styles;
    std::size_t ver = m__index % n_styles;

    if (ver == 0)
    {
        return m__key->as_camel_case(alt);
    }
    else if (ver == 1)
    {
        return m__key->as_kebab_case(alt);
    }
    else if (ver == 2)
    {
        return m__key->as_pascal_case(alt);
    }
    else if (ver == 3)
    {
        return m__key->as_sentence_case(alt);
    }
    else if (ver == 4)
    {
        return m__key->as_snake_case(alt);
    }
    else
    {
        beme_throw(std::logic_error,
            "Throw in unreachable code.",
            "The version is not valid.",
            "Version : ", ver
        );
    }

}

Key::const_iterator& Key::const_iterator::operator++()
{
    assertm(m__index < m__key->n_alternatives(), "Impossible to increment the iterator. The index is out of range.");
    ++m__index;
    return *this;
}

Key::const_iterator Key::const_iterator::operator++(int)
{
    auto tmp = *this;
    ++(*this);
    return tmp;
}

bool Key::const_iterator::operator==(const Key::const_iterator& other) const
{
    assertm(m__key == other.m__key, "Impossible to compare the iterators. The iterators are not from the same key.");
    return m__index == other.m__index;
}

bool Key::const_iterator::operator!=(const Key::const_iterator& other) const
{
    assertm(m__key == other.m__key, "Impossible to compare the iterators. The iterators are not from the same key.");
    return m__index != other.m__index;
}

} // namespace bevarmejo::io

namespace bevarmejo::io::json::detail {

Json& hjm::from(Json &j) const
{
    for (const auto &key : m__key)
    {
        if (j.contains(key))
        {
            return j[key];
        }
    }

    beme_throw(std::out_of_range,
        "Impossible to extract the requested key from the json object.",
        "No version of the key has been found.",
        "Key : ", m__key()
    );
}

const Json& hjm::from(const Json &j) const
{    
    for (const auto &key : m__key)
    {
        if (j.contains(key))
        {
            return j[key];
        }
    }
    
    beme_throw(std::out_of_range,
        "Impossible to extract the requested key from the json object.",
        "No version of the key has been found.",
        "Key : ", m__key()
    );
}

} // namespace bevarmejo::io::json::detail

bevarmejo::io::json::detail::hjm bevarmejo::io::json::extract(const Key &k)
{
    return bevarmejo::io::json::detail::hjm(k);
}
