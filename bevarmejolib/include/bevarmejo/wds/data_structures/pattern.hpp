

#ifndef BEVARMEJOLIB__WDS_ELEMENTS__PATTERN_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__PATTERN_HPP

#include <string>
#include <vector>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/element.hpp"

namespace bevarmejo {
namespace wds {

static const std::string l__PATTERN= "Pattern";

class Pattern final : public Element {
    public:
        using inherited= Element;
        using container= std::vector<double>;

    /*--- Attributes ---*/
    protected:
        /*--- Properties ---*/
        container m__multipliers;
        long* m__start_time_s_;
        long* m__timestep__s_;
        
     /*--- Constructors ---*/
    public: 
        // Default constructor
        Pattern() = delete;

        Pattern(const std::string& id);

        Pattern(const std::string& id, long* ap__start_time_s, long* ap__timestep__s);

        // Copy constructor
        Pattern(const Pattern& other);

        // Move constructor
        Pattern(Pattern&& rhs) noexcept;

        // Copy assignment operator
        Pattern& operator=(const Pattern& rhs);

        // Move assignment operator
        Pattern& operator=(Pattern&& rhs) noexcept;

        virtual ~Pattern() { m__multipliers.clear(); }

    /*--- Getters and setters ---*/
    public:
        /*--- Properties ---*/
        const container& multipliers() const { return m__multipliers; }
        container& multipliers() { return m__multipliers; }
        const long start_time_s() const { return *m__start_time_s_; }
        void start_time_s(long* ap__start_time_s) { m__start_time_s_ = ap__start_time_s; }
        const long timestep__s() const { return *m__timestep__s_; }
        void timestep__s(long* ap__timestep__s) { m__timestep__s_ = ap__timestep__s; }

    /*--- Methods ---*/
    public: 
        size_t size() const { return m__multipliers.size(); }
        double& operator[](size_t index) { return m__multipliers[index]; }
        double& at(size_t index) { return m__multipliers.at(index); }

    /*--- Pure virtual methods override---*/
    public:
        /*--- Properties ---*/
        virtual const std::string& element_name() const override {return l__PATTERN;}
        virtual const unsigned int element_type() const override {return ELEMENT_PATTERN;}


    /*--- EPANET-dependent PVMs ---*/
    public:
        /*--- Properties ---*/
        void retrieve_index(EN_Project ph) override;
        void retrieve_properties(EN_Project ph) override;

}; // class Pattern

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__PATTERN_HPP
