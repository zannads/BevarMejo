#ifndef BEVARMEJOLIB__WDS_ELEMENTS__PIPE_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__PIPE_HPP

#include <string>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/variable.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/link.hpp"
#include "bevarmejo/wds/elements/dimensioned_link.hpp"

namespace bevarmejo {
namespace wds {

/// WDS pipe
/*******************************************************************************
 * The wds::pipe class represents a pipe in the network.
 ******************************************************************************/

static const std::string LNAME_PIPE= "Pipe";
static const std::string L_LENGTH= "Length";

class pipe : public dimensioned_link {

public:
    using inherited= dimensioned_link;

 /*--- Attributes ---*/
    protected:
    /*--- Properties ---*/
    vars::var_real* _length_;

    /*---  Results   ---*/

protected:
    void _add_properties() override;
    void _update_pointers() override;

/*--- Constructors ---*/
public:
    pipe() = delete;

    pipe(const std::string& id);

    // Copy constructor
    pipe(const pipe& other);

    // Move constructor
    pipe(pipe&& rhs) noexcept;

    // Copy assignment operator
    pipe& operator=(const pipe& rhs);

    // Move assignment operator
    pipe& operator=(pipe&& rhs) noexcept;

    // Destructor
    virtual ~pipe();

/*--- Getters and setters ---*/
public:
    /*--- Properties ---*/
    vars::var_real& length() const { return *_length_; }

    /*---  Results   ---*/

/*--- Pure virtual methods ---*/
public:
    /*--- Properties ---*/
    const std::string& element_name() const override {return LNAME_PIPE;}
    const unsigned int element_type() const override {return ELEMENT_PIPE;}

/*-- EPANET-dependent PVMs --*/
public:
    /*--- Properties ---*/
    void retrieve_properties(EN_Project ph) override;

};

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__PIPE_HPP
