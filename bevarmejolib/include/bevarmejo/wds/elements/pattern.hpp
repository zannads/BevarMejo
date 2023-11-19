

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
        long _start_time_s_;
        long _step_s_;

    protected:
        
     /*--- Constructors ---*/
    public: 
        // Default constructor
        pattern() = delete;

        pattern(const std::string& id);

        pattern(const std::string& id, long a_start_time_s, long a_step_s);

        // Copy constructor
        pattern(const pattern& other);

        // Move constructor
        pattern(pattern&& rhs) noexcept;

        // Copy assignment operator
        pattern& operator=(const pattern& rhs);

        // Move assignment operator
        pattern& operator=(pattern&& rhs) noexcept;

        virtual ~pattern() { _multipliers_.clear(); }

    /*--- Getters and setters ---*/
    public:
        /*--- Properties ---*/
        const container& multipliers() const { return _multipliers_; }
        container& multipliers() { return _multipliers_; }
        const long start_time_s() const { return _start_time_s_; }
        void start_time_s(long a_start_time_s) { _start_time_s_ = a_start_time_s; }
        const long step_s() const { return _step_s_; }
        void step_s(long a_step_s) { _step_s_ = a_step_s; }

    /*--- Methods ---*/
    public: 
        size_t size() const { return _multipliers_.size(); }
        double& operator[](size_t index) { return _multipliers_[index]; }
        double& at(size_t index) { return _multipliers_.at(index); }

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
