#ifndef BEVARMEJOLIB__WDS_ELEMENTS__SOURCE_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__SOURCE_HPP

#include <string>

#include "epanet2_2.h"

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

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
    aux::QuantitySeries<double> m__inflow;
    aux::QuantitySeries<double> m__source_elevation;

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
    virtual ~Source() = default;

/*--- Getters and setters ---*/
public:
    /*--- Properties ---*/

    /*---  Results   ---*/
    const aux::QuantitySeries<double>& inflow() const { return m__inflow; }
    const aux::QuantitySeries<double>& source_elevation() const { return m__source_elevation; }

/*--- Pure virtual methods override---*/
    virtual void clear_results() override;

/*--- EPANET-dependent PVMs override ---*/
public:
    /*--- Properties ---*/

    /*--- Results ---*/
    void retrieve_results(EN_Project ph, long t) override;

};
    
} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__SOURCE_HPP
