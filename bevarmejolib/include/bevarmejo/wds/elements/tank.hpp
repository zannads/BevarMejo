#ifndef BEVARMEJOLIB__WDS_ELEMENTS__TANK_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__TANK_HPP

#include <string>
#include <variant>

#include "bevarmejo/wds/elements/variable.hpp"
#include "bevarmejo/wds/elements/source.hpp"
#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/element.hpp"

namespace bevarmejo {
namespace wds {

/// WDS Tank
/*******************************************************************************
 * The wds::junction class represents a Tank in the network. It is a dynamic element.
 ******************************************************************************/

static const std::string LNAME_TANK= "Tank";
static const std::string LVOLUME = "Volume";

class Tank : public Source {

public:
    using inherited= Source;

protected:

public:
    Tank() = delete;
    Tank(const std::string& id) : inherited(id) { }

    // Copy constructor
    Tank(const Tank& other) : inherited(other) { }

    // Move constructor
    Tank(Tank&& rhs) noexcept : inherited(std::move(rhs)) { }

    // Copy assignment operator
    Tank& operator=(const Tank& rhs) {
        if (this != &rhs) {
            inherited::operator=(rhs);
        }
        return *this;
    }

    // Move assignment operator
    Tank& operator=(Tank&& rhs) noexcept {
        if (this != &rhs) {
            inherited::operator=(std::move(rhs));
        }
        return *this;
    }

    // Destructor
    ~Tank() override { }

    // ----- override inherited pure virtual methods ----- // 
    const std::string& element_name() const override { return LNAME_TANK; }
    const unsigned int element_type() const override { return ELEMENT_TANK; }

};

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__TANK_HPP
