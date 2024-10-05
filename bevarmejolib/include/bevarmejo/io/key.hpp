#pragma once

#include <initializer_list>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using json_o = nlohmann::json;

#include "bevarmejo/utils/string_manip.hpp"

namespace bevarmejo::io::key {

namespace style {
struct Original {};
struct Camel {};
struct Kebab {};
struct Snake {};

constexpr std::size_t n_styles = 4ul;
} // namespace key::style

class Key final{
private:
    std::vector<std::string> m__values;

public:
    Key();
    Key(Key&&) = default;
    Key(const Key&) = default;
    Key& operator=(Key&&) = default;
    Key& operator=(const Key&) = default;
    ~Key() = default;

    // Alternative constructors (keys are not meant to be modified, they should only exist as static constants)
    Key(std::initializer_list<std::string> values);

    // The key object is meant to be a static const global object and defined
    // in the various translation units where they are needed.
    // With the operator() I can access the original value as const references.
    // Same with the operator[] method but with the possibility to access the alternatives.
    // While with the get method I can access the values in the various formats
    // using the template parameter.

    // Main value of the key in original style.
    const std::string& operator()() const;

    // Alternative value of the key in original style.
    const std::string& operator[](std::size_t alt) const;

    // Get any alternative value of the key in the desired style.
    template <typename S= style::Original>
    std::string get(std::size_t alt = 0ul) const {
        if constexpr (std::is_same_v<S, style::Camel>)
            return bevarmejo::to_camel_case(get<style::Original>(alt));
        
        if constexpr (std::is_same_v<S, style::Kebab>)
            return bevarmejo::to_kebab_case(get<style::Original>(alt));
        
        if constexpr (std::is_same_v<S, style::Snake>)
            return bevarmejo::to_snake_case(get<style::Original>(alt));
    
         // ======== Actual Implementation (hyp: S == style::Original) ========
         return operator[](alt);
            
    }

    // Get any alternative value of the key in the camel style.
    std::string camel(std::size_t alt = 0ul) const;

    // Get any alternative value of the key in the kebab style.
    std::string kebab(std::size_t alt = 0ul) const;

    // Get any alternative value of the key in the snake style.
    std::string snake(std::size_t alt = 0ul) const;

    // Check if the key exists in the json object.
    bool exists_in(const json_o &j) const; 

    // Returns the number of versions that a key can have. (Original, Camel, Snake, Kebab) 
    static std::size_t n_versions(); 
    
    // Counts the number of alternatives, in all case of the key (Original, Camel, Snake, Kebab).
    // It's the number of steps of the iterators and it is equal to size()*n_versions().
    std::size_t n_alternatives() const;
    
    // Actual size of the key (vector of string underlying).
    std::size_t size() const;

    // I also need the iterators to iterate across all possible versions of the key.
    // However, only const iterator is needed and only in the forward direction.
    // For each value of the key (e.g., long name, short name, etc), I check all possible styles.
    // So the index of the iterator modulo n_versions() gives me the style and the integer division
    // gives me the value of the key.

private:

    template <typename T>
    class Iterator {
        private:
            T& m__key;
            std::size_t m__index;

        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = std::string;
            using reference = std::string&;
            using const_reference = const std::string&;

            Iterator(T& key, std::size_t index) : m__key(key), m__index(index) {}
            
            value_type operator*() const {
                assert(m__index < m__key.n_alternatives());

                std::size_t alt = m__index / style::n_styles;
                std::size_t style = m__index % style::n_styles;

                if (style == 0)
                    return m__key.template get<style::Original>(alt);
                else if (style == 1)
                    return m__key.template get<style::Camel>(alt);
                else if (style == 2)
                    return m__key.template get<style::Kebab>(alt);
                else
                    return m__key.template get<style::Snake>(alt);

            }

            Iterator& operator++() {
                if (m__index < m__key.n_alternatives())
                    ++m__index;
                return *this;
            }
            Iterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }

            bool operator==(const Iterator& other) const { return m__index == other.m__index; }
            bool operator!=(const Iterator& other) const { return m__index != other.m__index; }
    };

public:
    using const_iterator = Iterator<const Key>;

    const_iterator begin() const { return const_iterator(*this, 0); }
    const_iterator end() const { return const_iterator(*this, n_alternatives()); }
};



}  // namespace bevarmejo::io::key

namespace bevarmejo::io::json {
namespace detail {
struct __hjm__ {
    const io::key::Key &m__key;

    json_o& from(json_o &j) const;
    const json_o& from(const json_o &j) const;
}; // Hidden Json Methods
} // namespace detail

detail::__hjm__ extract(const key::Key &k);
}

// Overload nlohmann::json methods to use the Key class as a key in the json object.


