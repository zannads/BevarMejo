#ifndef BEVARMEJOLIB__WDS_ELEMENTS__SOURCE_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__SOURCE_HPP

#include <string>
#include <vector>

#include "epanet2_2.h"

#include "bevarmejo/wds/data_structures/temporal.hpp"
#include "bevarmejo/wds/data_structures/variable.hpp"

#include "bevarmejo/wds/epanet_helpers/en_time_options.hpp"
#include "bevarmejo/wds/auxiliary/time_series.hpp"
#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/element.hpp"

#include "bevarmejo/wds/elements_group.hpp"
#include "bevarmejo/wds/user_defined_elements_group.hpp"

#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"

namespace bevarmejo {
namespace wds {

/// WDS Source
/*******************************************************************************
 * The wds::Source class represents a fix pressure node in the network. 
 * It can either be a tank or a reservoir.
 ******************************************************************************/

static const std::string LSOURCE_ELEV= "Source Elevation";
static const std::string LINFLOW= "Inflow";

class Source : public Node {

public:
    using inherited= Node;

/*--- Attributes ---*/
protected:
    /*--- Properties ---*/

    /*---  Results   ---*/
    vars::var_tseries_real* _inflow_;
    vars::var_tseries_real* _source_elevation_;

protected:
    void _add_results() override;
    void _update_pointers() override;

/*--- Constructors ---*/
public:
    Source() = delete;
    Source(const std::string& id, const WaterDistributionSystem& wds);

    // Copy constructor
    Source(const Source& other);

    // Move constructor
    Source(Source&& rhs) noexcept;

    // Copy assignment operator
    Source& operator=(const Source& rhs);

    // Move assignment operator
    Source& operator=(Source&& rhs) noexcept;

    // Destructor
    virtual ~Source();

/*--- Getters and setters ---*/
public:
    /*--- Properties ---*/

    /*---  Results   ---*/
    vars::var_tseries_real& inflow() const { return *_inflow_; }
    vars::var_tseries_real& source_elevation() const { return *_source_elevation_; }

/*--- Pure virtual methods override---*/

/*--- EPANET-dependent PVMs override ---*/
public:
    /*--- Properties ---*/

    /*--- Results ---*/
    void retrieve_results(EN_Project ph, long t) override;

};
    
} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__SOURCE_HPP
