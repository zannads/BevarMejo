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

/// WDS tank
/*******************************************************************************
 * The wds::junction class represents a tank in the network. It is a dynamic element.
 ******************************************************************************/

static const std::string LNAME_TANK= "Tank";
static const std::string LVOLUME = "Volume";

class tank : public source {

public:
    using inherited= source;

protected:

public:
    tank() = delete;
    tank(const std::string& id) : inherited(id) { }

    // Copy constructor
    tank(const tank& other) : inherited(other) { }

    // Move constructor
    tank(tank&& rhs) noexcept : inherited(std::move(rhs)) { }

    // Copy assignment operator
    tank& operator=(const tank& rhs) {
        if (this != &rhs) {
            inherited::operator=(rhs);
        }
        return *this;
    }

    // Move assignment operator
    tank& operator=(tank&& rhs) noexcept {
        if (this != &rhs) {
            inherited::operator=(std::move(rhs));
        }
        return *this;
    }

    // Destructor
    ~tank() override { }

    // ----- override inherited pure virtual methods ----- // 
    const std::string& element_name() const override { return LNAME_TANK; }
    const unsigned int& element_type() const override { return ELEMENT_TANK; }

};

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__TANK_HPP
