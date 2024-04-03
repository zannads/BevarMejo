#ifndef BEVARMEJOLIB__WDS_ELEMENTS__RESERVOIR_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__RESERVOIR_HPP

#include <string>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/variable.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/source.hpp"

namespace bevarmejo {
namespace wds {


/// WDS Reservoir
/*******************************************************************************
 * The wds::Reservoir class represents a Reservoir in the network. It is a static element.
 ******************************************************************************/

static const std::string l__NAME_RESERVOIR= "Reservoir";

class Reservoir : public Source {
public:
    using inherited= Source;

/*--- Attributes ---*/
protected:
    /*--- Properties ---*/

    /*---  Results   ---*/

protected:
    void _add_properties() override;
    void _add_results() override;
    void _update_pointers() override;

 /*--- Constructors ---*/
public:
    Reservoir() = delete;
    Reservoir(const std::string& id);
    Reservoir(const Reservoir& other);
    Reservoir(Reservoir&& rhs) noexcept;
    Reservoir& operator=(const Reservoir& other);
    Reservoir& operator=(Reservoir&& rhs) noexcept;
    ~Reservoir() override;

/*--- Getters and setters ---*/
public:
    /*--- Properties ---*/

    /*---  Results   ---*/

/*--- Methods ---*/
public:

/*--- Pure virtual methods override---*/
public:
    /*--- Properties ---*/
    const std::string& element_name() const override {return l__NAME_RESERVOIR;}
    const unsigned int element_type() const override {return ELEMENT_JUNCTION;}

/*--- EPANET-dependent PVMs ---*/
public:
    /*--- Properties ---*/
    void retrieve_properties(EN_Project ph) override;

    /*--- Results ---*/
    void retrieve_results(EN_Project ph, long t) override;

}; // class Reservoir

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__RESERVOIR_HPP
