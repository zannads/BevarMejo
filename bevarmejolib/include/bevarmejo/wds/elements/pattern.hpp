

#ifndef BEVARMEJOLIB__WDS_ELEMENTS__PATTERN_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__PATTERN_HPP

#include <vector>

#include "bevarmejo/wds/elements/element.hpp"

namespace bevarmejo {
namespace wds {

static const std::string LNAME_PATTERN= "Pattern";

class pattern : public element {
    public:
        using inherited= element;
        using container= std::vector<double>;

    protected:
        container _data_;

    public: 
        // Default constructor
        pattern();

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

        virtual ~pattern() { _data_.clear(); }

        // Actually all this function could be handled in the temporal class 
        //const unsigned int start_time() const { return (*this).begin()->first; }

        //std::vector<double> multipliers() const;

        void _update_pointers() override { inherited::_update_pointers(); }
        void _add_results() override { inherited::_add_results(); }

        // ----- override inherited pure virtual methods ----- // 
        const std::string& element_name() const override {return LNAME_PATTERN;}
        const unsigned int& element_type() const override {return ELEMENT_PATTERN;}

         // ----- load from EPANET ----- //
        void retrieve_index(EN_Project ph) override;
        void retrieve_properties(EN_Project ph) override;

}; // class pattern

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__PATTERN_HPP
