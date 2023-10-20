

#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/element.hpp"

#include "pattern.hpp"

namespace bevarmejo {
namespace wds {

pattern::pattern() : inherited() 
                    {
                        _add_results();
                        _update_pointers();
                    }

pattern::pattern(const std::string& id) : inherited(id) 
                                            {
                                                _add_results();
                                                _update_pointers();
                                            }

// Copy constructor
pattern::pattern(const pattern& other) : inherited(other) 
                                            {
                                                _update_pointers();
                                            }

// Move constructor
pattern::pattern(pattern&& rhs) noexcept : inherited(std::move(rhs))
                                            {
                                                _update_pointers();
                                            }

// Copy assignment operator
pattern& pattern::operator=(const pattern& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        _update_pointers();
    }
    return *this;
}

// Move assignment operator
pattern& pattern::operator=(pattern&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        _update_pointers();
    }
    return *this;
}

} // namespace wds
} // namespace bevarmejo
