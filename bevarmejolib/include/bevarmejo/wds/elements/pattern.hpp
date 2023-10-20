

#ifndef BEVARMEJOLIB__WDS_ELEMENTS__PATTERN_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__PATTERN_HPP

#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/element.hpp"

namespace bevarmejo {
namespace wds {

static const std::string LNAME_PATTERN= "Pattern";

class pattern : public element, public vars::temporal<double> {
    public:
        using inherited= element;
        using container= vars::temporal<double>;

    // should not have any property in my opinion

    public: 
        // Default constructor
        pattern() = delete;

        pattern(const std::string& id);

        // TODO: could add a constructor with a vector of values

        // Copy constructor
        pattern(const pattern& other);

        // Move constructor
        pattern(pattern&& rhs) noexcept;

        // Copy assignment operator
        pattern& operator=(const pattern& rhs);

        // Move assignment operator
        pattern& operator=(pattern&& rhs) noexcept;

        virtual ~pattern() { container::clear(); }

        // Actually all this function could be handled in the temporal class 
        //const unsigned int start_time() const { return (*this).begin()->first; }

        //std::vector<double> multipliers() const;

        void _update_pointers() override { inherited::_update_pointers(); }
        void _add_results() override { inherited::_add_results(); }

        // ----- override inherited pure virtual methods ----- // 
        const std::string& element_name() const override {return LNAME_PATTERN;}
        const unsigned int& element_type() const override {return ELEMENT_PATTERN;}

}; // class pattern

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__PATTERN_HPP
