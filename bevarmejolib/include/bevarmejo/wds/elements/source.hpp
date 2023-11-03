#ifndef BEVARMEJOLIB__WDS_ELEMENTS__SOURCE_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__SOURCE_HPP

#include <string>
#include <variant>

#include "bevarmejo/wds/elements/variable.hpp"
#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/element.hpp"

namespace bevarmejo {
namespace wds {

/// WDS source
/*******************************************************************************
 * The wds::source class represents a fix pressure node in the network. 
 * It can either be a tank or a reservoir.
 ******************************************************************************/

static const std::string LSOURCE_ELEV= "Source Elevation";
static const std::string LINFLOW= "Inflow";

class source : public node {

public:
    using inherited= node;

protected:
    // no pointers to properties

    // results
    vars::var_tseries_real* _inflow_;
    vars::var_tseries_real* _source_elevation_;

    void _add_results() override;
    void _update_pointers() override;

public:
    source() = delete;
    source(const std::string& id);

    // Copy constructor
    source(const source& other);

    // Move constructor
    source(source&& rhs) noexcept;

    // Copy assignment operator
    source& operator=(const source& rhs);

    // Move assignment operator
    source& operator=(source&& rhs) noexcept;

    // Destructor
    virtual ~source();

    vars::var_tseries_real& inflow() const { return *_inflow_; }
    vars::var_tseries_real& source_elevation() const { return *_source_elevation_; }

};
    
} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__SOURCE_HPP
