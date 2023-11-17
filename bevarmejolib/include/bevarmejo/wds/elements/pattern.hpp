

#ifndef BEVARMEJOLIB__WDS_ELEMENTS__PATTERN_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__PATTERN_HPP

#include <string>
#include <vector>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/element.hpp"

namespace bevarmejo {
namespace wds {

static const std::string LNAME_PATTERN= "Pattern";

class pattern : public element {
    public:
        using inherited= element;
        using container= std::vector<double>;

    /*--- Attributes ---*/
    protected:
        /*--- Properties ---*/
        container _multipliers_;

    protected:
        
     /*--- Constructors ---*/
    public: 
        // Default constructor
        pattern() = delete;

        pattern(const std::string& id) : inherited(id), _multipliers_() { }

        // TODO: could add a constructor with a vector of values

        // Copy constructor
        pattern(const pattern& other) : inherited(other), 
                                        _multipliers_(other._multipliers_) { }

        // Move constructor
        pattern(pattern&& rhs) noexcept : 
            inherited(std::move(rhs)),
            _multipliers_(std::move(rhs._multipliers_)) { }

        // Copy assignment operator
        pattern& operator=(const pattern& rhs);

        // Move assignment operator
        pattern& operator=(pattern&& rhs) noexcept;

        virtual ~pattern() { _multipliers_.clear(); }

    /*--- Getters and setters ---*/
    public:
        /*--- Properties ---*/
        const container& multipliers() const { return _multipliers_; }

    /*--- Pure virtual methods override---*/
    public:
        /*--- Properties ---*/
        const std::string& element_name() const override {return LNAME_PATTERN;}
        const unsigned int element_type() const override {return ELEMENT_PATTERN;}


    /*--- EPANET-dependent PVMs ---*/
    public:
        /*--- Properties ---*/
        void retrieve_index(EN_Project ph) override;
        void retrieve_properties(EN_Project ph) override;

}; // class pattern

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__PATTERN_HPP
