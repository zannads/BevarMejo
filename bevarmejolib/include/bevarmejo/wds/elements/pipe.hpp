#ifndef BEVARMEJOLIB__WDS_ELEMENTS__PIPE_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__PIPE_HPP

#include <string>
#include <variant>

#include "bevarmejo/wds/elements/variable.hpp"
#include "bevarmejo/wds/elements/dimensioned_link.hpp"
#include "bevarmejo/wds/elements/link.hpp"
#include "bevarmejo/wds/elements/element.hpp"

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

protected:
    // pointers to variables
    vars::var_real* _length_;

    // no extra results

    void _add_properties() override;
    void _update_pointers() override;

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

    // ----- override inherited pure virtual methods ----- // 
    const std::string& element_name() const override {return LNAME_PIPE;}
    const unsigned int& element_type() const override {return ELEMENT_PIPE;}

    vars::var_real& length() const { return *_length_; }

};

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__PIPE_HPP
