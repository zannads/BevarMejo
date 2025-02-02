#pragma once

#include <initializer_list>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using json_o = nlohmann::json;

#include "bevarmejo/utility/string_manip.hpp"

namespace bevarmejo::io
{

class Key final
{
public:
    enum class style
    {
        CamelCase,
        KebabCase,
        PascalCase,
        SentenceCase,
        SnakeCase
    };

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
    // With the operator() I can access the Sentence value as const references.
    // Same with the operator[] method but with the possibility to access the alternatives.
    // While with the get method I can access the values in the various formats
    // using the template parameter.

    // Main value of the key in the output style.
    std::string operator()() const;

    // Alternative value of the key in Sentence style.
    const std::string& operator[](std::size_t alt) const;

    // Get any alternative value of the key in the desired style.
    template <style S= style::SentenceCase>
    std::string get(std::size_t alt = 0ul) const
    {
        if constexpr ( S == style::CamelCase )
            return bevarmejo::sentence_case_to_camel_case(get<style::SentenceCase>(alt));
        
        if constexpr ( S == style::KebabCase )
            return bevarmejo::sentence_case_to_kebab_case(get<style::SentenceCase>(alt));

        if constexpr ( S == style::PascalCase )
            return bevarmejo::sentence_case_to_pascal_case(get<style::SentenceCase>(alt));
        
        if constexpr ( S == style::SnakeCase )
            return bevarmejo::sentence_case_to_snake_case(get<style::SentenceCase>(alt));
    
         // ======== Actual Implementation (hyp: S == style::SentenceCase) ========
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

    // Returns the number of versions that a key can have. (Sentence, Camel, Snake, Kebab, Pascal) 
    static std::size_t n_versions(); 
    
    // Counts the number of alternatives, in all case of the key (Sentence, Camel, Snake, Kebab, Pascal).
    // It's the number of steps of the iterators and it is equal to size()*n_versions().
    std::size_t n_alternatives() const;
    
    // Actual size of the key (vector of string underlying).
    std::size_t size() const;

    // I also need the iterators to iterate across all possible versions of the key.
    // However, only const iterator is needed and only in the forward direction.
    // For each value of the key (e.g., long name, short name, etc), I check all possible styles.
    // So the index of the iterator modulo n_versions() gives me the style and the integer division
    // gives me the value of the key.

public:
    class const_iterator
    {
        private:
            const Key* m__key;
            std::size_t m__index;

        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = std::string;
            using reference = std::string&;
            using const_reference = const std::string&;
        private:
            friend class Key;
        
        private:
            // Normal constructors are private so that only keys can create iterators.
            const_iterator(const Key* key, std::size_t index);
        public:
            // Copy and move constructor and assignment operators are default.
            const_iterator(const const_iterator&) = default;
            const_iterator(const_iterator&&) = default;
            const_iterator& operator=(const const_iterator&) = default;
            const_iterator& operator=(const_iterator&&) = default;
            ~const_iterator() = default;

            value_type operator*() const;

            const_iterator& operator++();
            const_iterator operator++(int);

            bool operator==(const const_iterator& other) const;
            bool operator!=(const const_iterator& other) const;
    };

public:

    const_iterator begin() const;

    const_iterator end() const;

public:
    // Read what is the current output style of the keys.
    static style get_out_style();
};

}  // namespace bevarmejo::io

namespace bevarmejo::io::json {
namespace detail {
class hjm;
} // namespace detail

detail::hjm extract(const Key &k);

namespace detail {
class hjm final {
private:
    const io::Key &m__key;

    friend hjm bevarmejo::io::json::extract(const Key &k);

    hjm() = delete;
    hjm(const io::Key &k) : m__key{k} {}
    hjm(const hjm&) = delete;
    hjm(hjm&&) = delete;
    hjm& operator=(const hjm&) = delete;
    hjm& operator=(hjm&&) = delete;
public:
    ~hjm() = default;

    json_o& from(json_o &j) const;

    const json_o& from(const json_o &j) const;
}; // Hidden Json Methods
} // namespace detail

}

// Overload nlohmann::json methods to use the Key class as a key in the json object.


