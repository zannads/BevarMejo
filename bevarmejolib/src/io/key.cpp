#include <initializer_list>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using json_o = nlohmann::json;

#include "bevarmejo/utility/bemexcept.hpp"
#include "bevarmejo/utility/string_manip.hpp"

#include "key.hpp" 

namespace bevarmejo::io::log::cname {
static const std::string key = "Key";
}
namespace bevarmejo::io::log::fname {
static const std::string generic_access = "[[access_operator]]";
}

namespace bevarmejo::io::key {

Key::Key() : Key (std::initializer_list<std::string>{""}) {}

Key::Key(std::initializer_list<std::string> values) : m__values(values) {}

std::string Key::operator()() const { 
    
    switch (m__out_style)
    {
    case style::Original:
        return m__values[0];
        break;

    case style::Camel:
        return bevarmejo::to_camel_case(m__values[0]);
        break;

    case style::Kebab:
        return bevarmejo::to_kebab_case(m__values[0]);
        break;

    case style::Snake:
        return bevarmejo::to_snake_case(m__values[0]);
        break;
    
    default:
        break;
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

bool Key::exists_in(const json_o &j) const {
    
    for (const auto &key : *this) 
        if (j.contains(key))
            return true;

    return false;
}

std::size_t Key::n_versions() { return n_styles; }

std::size_t Key::n_alternatives() const { return m__values.size()*n_versions(); }

std::size_t Key::size() const { return m__values.size(); }

Key::const_iterator Key::begin() const { return const_iterator(*this, 0); }

Key::const_iterator Key::end() const { return const_iterator(*this, n_alternatives()); }

style Key::m__out_style = style::Kebab;

const style& Key::get_out_style() { return m__out_style; }

void Key::set_out_style(style s) { m__out_style = s; }

void Key::set_out_style(const std::string &s) { m__out_style = Style(s); }

style Style(const std::string &s) {
    if (s == "Original" || s == "original")
        return style::Original;
    
    if (s == "Camel" || s == "camel")
        return style::Camel;
    
    if (s == "Kebab" || s == "kebab")
       return style::Kebab;
    
    if (s == "Snake" || s == "snake")
        return style::Snake;
   
    beme_throw(std::invalid_argument,
        "Impossible to create a style from the provided string.",
        "Invalid string.",
        "Style string: ", s);
}

} // namespace bevarmejo::io::key

namespace bevarmejo::io::json::detail {

json_o& hjm::from(json_o &j) const
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

const json_o& hjm::from(const json_o &j) const
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

bevarmejo::io::json::detail::hjm bevarmejo::io::json::extract(const key::Key &k)
{
    return bevarmejo::io::json::detail::hjm(k);
}
