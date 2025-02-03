#pragma once

#include <initializer_list>
#include <vector>

#include "bevarmejo/io/json.hpp"

#include "bevarmejo/utility/except.hpp"
#include "bevarmejo/utility/string.hpp"

namespace bevarmejo::io
{

// The AliasedKey object is meant to be a static const global object and defined
// in the various translation units where they are needed.
// With the operator() I can access the Sentence value as const references.
// Same with the operator[] method but with the possibility to access the alternatives.
// While with the get method I can access the values in the various formats
// using the template parameter.
class AliasedKey final
{
/*----------------------------------------------------------------------------*/
/*---------------------------- Member types ----------------------------------*/
/*----------------------------------------------------------------------------*/
public:
    enum class style
    {
        CamelCase,
        KebabCase,
        PascalCase,
        SentenceCase,
        SnakeCase
    };

/*----------------------------------------------------------------------------*/
/*---------------------------- Member objects --------------------------------*/
/*----------------------------------------------------------------------------*/
private:
    std::vector<std::string> m__values;

/*----------------------------------------------------------------------------*/
/*--------------------------- Member functions -------------------------------*/
/*----------------------------------------------------------------------------*/
// (constructor)
public:
    AliasedKey() = delete;
    AliasedKey(AliasedKey&&) = default;
    AliasedKey(const AliasedKey&) = default;
    // Alternative constructors (AliasedKeys are not meant to be modified, they should only exist as static constants)
    AliasedKey(std::initializer_list<std::string> values) :
        m__values(values)
    { }

// (destructor)
public:
    ~AliasedKey() = default;

// operator=
public:
    AliasedKey& operator=(AliasedKey&&) = default;
    AliasedKey& operator=(const AliasedKey&) = default;

// Information methods ---------------------------------------------------------
public:
// Read what is the current output style of the AliasedKeys.
    static constexpr style get_out_style()
    {
#if defined(OUT_STYLE_0)
        return style::CamelCase;
#elif defined(OUT_STYLE_1)
        return style::KebabCase;
#elif defined(OUT_STYLE_2)
        return style::PascalCase;
#elif defined(OUT_STYLE_3)
        return style::SentenceCase;
#elif defined(OUT_STYLE_4)
        return style::SnakeCase;
#else
    #error "The output style selected is invalid."
#endif
    }

    // Returns the number of versions that a AliasedKey can have. (Sentence, Camel, Snake, Kebab, Pascal) 
    static constexpr std::size_t n_styles()
    {
        return 5;
    }
    
// Element access --------------------------------------------------------------
public:
    // Main value of the AliasedKey in the DEFAULT output style.
    std::string operator()() const
    {
        if constexpr (get_out_style() == style::CamelCase)
        {
            return bevarmejo::sentence_case_to_camel_case(m__values[0]);
        }
        else if constexpr (get_out_style() == style::KebabCase)
        {
            return bevarmejo::sentence_case_to_kebab_case(m__values[0]);
        }
        else if constexpr (get_out_style() == style::PascalCase)
        {
            return bevarmejo::sentence_case_to_pascal_case(m__values[0]);
        }
        else if constexpr (get_out_style() == style::SentenceCase)
        {
            return m__values[0];
        }
        else // if constexpr (get_out_style() == style::SnakeCase)
        {
            return bevarmejo::sentence_case_to_snake_case(m__values[0]);
        }
    }

    // Alternative value of the AliasedKey in Sentence style.
    const std::string& operator[](std::size_t alt) const
    {
        beme_throw_if(alt >= m__values.size(), std::out_of_range,
            "Impossible to provide the requested key.",
            "Index out of range.",
            "Index : ", alt, 
            "Size : ", m__values.size()
        );

        return m__values[alt];
    }

    // Get any alternative value of the AliasedKey in the camel style.
    std::string as_camel_case(std::size_t alt = 0ul) const
    {
        return bevarmejo::sentence_case_to_camel_case(m__values[alt]);
    }

    // Get any alternative value of the AliasedKey in the kebab style.
    std::string as_kebab_case(std::size_t alt = 0ul) const
    {
        return bevarmejo::sentence_case_to_kebab_case(m__values[alt]);
    }

    // Get any alternative value of the AliasedKey in the pascal style.
    std::string as_pascal_case(std::size_t alt = 0ul) const
    {
        return bevarmejo::sentence_case_to_pascal_case(m__values[alt]);
    }

    // Get any alternative value of the AliasedKey in the sentence style.
    std::string as_sentence_case(std::size_t alt = 0ul) const
    {
        return m__values[alt];
    }

    // Get any alternative value of the AliasedKey in the snake style.
    std::string as_snake_case(std::size_t alt = 0ul) const
    {
        return bevarmejo::sentence_case_to_snake_case(m__values[alt]);
    }

// Capacity --------------------------------------------------------------------
public:
    // Counts the number of alternatives of the AliasedKey.
    // How many different versions in the Sentence style.
    std::size_t n_alternatives() const
    {
        return m__values.size();
    }
    
    // Apparent size of the AliasedKey (the number of steps you count iterating 
    // it or the number of versions that you can access).
    std::size_t size() const
    {
        return n_styles() * n_alternatives();
    }

// Operations ------------------------------------------------------------------
// No specific operation to do.

// Iterators -------------------------------------------------------------------

 // I also need the iterators to iterate across all possible versions of the AliasedKey.
// However, only const iterator is needed and only in the forward direction.
// For each value of the AliasedKey (e.g., long name, short name, etc), I check all possible styles.
// So the index of the iterator modulo n_styles() gives me the style and the integer division
// gives me the value of the AliasedKey.
public:
    class const_iterator
    {
        private:
            const AliasedKey* m__key;
            std::size_t m__index;

        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = std::string;
            using reference = const std::string&;
            using const_reference = const std::string&;
        private:
            friend class AliasedKey;
        
        private:
            // Normal constructors are private so that only AliasedKeys can create iterators.
            const_iterator(const AliasedKey* a_key, std::size_t index) :
                m__key{a_key}, m__index{index}
            { }

        public:
            // Copy and move constructor and assignment operators are default.
            const_iterator(const const_iterator&) = default;
            const_iterator(const_iterator&&) = default;
            const_iterator& operator=(const const_iterator&) = default;
            const_iterator& operator=(const_iterator&&) = default;
            ~const_iterator() = default;

            value_type operator*() const
            {
                // m__index is the index of the alternative and version.
                // First check all styles for the main key, then proceed to the alternatives.
                std::size_t alt = m__index / AliasedKey:: n_styles();
                std::size_t ver = m__index % AliasedKey::n_styles();

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

            const_iterator& operator++()
            {
                assertm(m__index < m__key->size(), "Impossible to increment the iterator. The index is out of range.");
                ++m__index;
                return *this;
            }
            const_iterator operator++(int)
            {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            bool operator==(const const_iterator& other) const
            {
                assertm(m__key == other.m__key, "Impossible to compare the iterators. The iterators are not from the same key.");
                return m__index == other.m__index;
            }
            bool operator!=(const const_iterator& other) const
            {
                assertm(m__key == other.m__key, "Impossible to compare the iterators. The iterators are not from the same key.");
                return m__index != other.m__index;
            }
    };

public:

    const_iterator begin() const
    {
        return const_iterator(this, 0);
    }
    const_iterator cbegin() const
    {
        return const_iterator(this, 0);
    }

    const_iterator end() const
    {
        return const_iterator(this, size());
    }
    const_iterator cend() const
    {
        return const_iterator(this, size());
    }

// JSON methods ----------------------------------------------------------------
public:
    // Check if the AliasedKey exists in the json object.
    bool exists_in(const Json &j) const
    {
        for (const auto &alias : *this)
        {
            if (j.contains(alias))
            {
                return true;
            }
        }
        return false;
    }

    // Extract the value of the AliasedKey from the json object.
    std::string as_in(const Json &j) const
    {
        for (const auto &alias : *this)
        {
            if (j.contains(alias))
            {
                return alias;
            }
        }
        // If the key is not found, let's simply return the default value and let
        // the JSON class handle the missing key.
        return operator()();
    }
};

}  // namespace bevarmejo::io
