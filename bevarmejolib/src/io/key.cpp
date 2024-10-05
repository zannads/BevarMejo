#include <initializer_list>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using json_o = nlohmann::json;

#include "bevarmejo/bemexcept.hpp"
#include "bevarmejo/utils/string_manip.hpp"

#include "key.hpp" 

namespace bevarmejo::io::log::cname {
static const std::string key = "Key";
}
namespace bevarmejo::io::log::fname {
static const std::string generic_access = "[[access_operator]]";
}

namespace bevarmejo::io::key {

Key::Key() : m__values(std::vector(std::initializer_list<std::string>{""})) {}

Key::Key(std::initializer_list<std::string> values) : m__values(values) {}

const std::string& Key::operator()() const { return m__values[0]; }

const std::string& Key::operator[](std::size_t alt) const {
    
    if (alt < m__values.size())
        return m__values[alt];

    __format_and_throw<std::out_of_range, ClassError>(log::cname::key, log::fname::generic_access,
        "Impossible to provide the requested key.",
        "Index out of range.\n\tIndex : ", alt, 
        "\n\tValid index range : [0, ", m__values.size()-1, "]"
    );
}

bool Key::exists_in(const json_o &j) const {
    
    for (const auto &key : *this) 
        if (j.contains(key))
            return true;

    return false;
}

std::size_t Key::n_versions() { return style::n_styles; }

std::size_t Key::n_alternatives() const { return m__values.size()*n_versions(); }

std::size_t Key::size() const { return m__values.size(); }

}   // namespace bevarmejo::io::key

namespace bevarmejo::io::json::detail {

json_o& __hjm__::from(json_o &j) const {

    for (const auto &key : m__key) 
        if (j.contains(key))
            return j[key];

    __format_and_throw<std::out_of_range, FunctionError>("<extract><from>", 
        "Impossible to extract the requested key from the json object.",
        "No version of the key has been found.",
        "\tKey : ", m__key()
    );
}

const json_o& __hjm__::from(const json_o &j) const {
    
    for (const auto &key : m__key) 
        if (j.contains(key))
            return j[key];

    __format_and_throw<std::out_of_range, FunctionError>("<extract><from>", 
        "Impossible to extract the requested key from the json object.",
        "No version of the key has been found.",
        "\tKey : ", m__key()
    );
}

}  // namespace bevarmejo::io::json::detail

namespace bevarmejo::io::json {

detail::__hjm__ extract(const key::Key &k) { return detail::__hjm__{k}; }

}  // namespace bevarmejo::io::json
