#ifndef BEVARMEJOLIB__WDS_ELEMENTS__PIPE_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__PIPE_HPP

#include <memory>
#include <string>

#include "epanet2_2.h"

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/dimensioned_link.hpp"

namespace bevarmejo {
namespace wds {

/// WDS Pipe
/*******************************************************************************
 * The wds::Pipe class represents a pipe in the network.
 ******************************************************************************/

static const std::string LNAME_PIPE= "Pipe";
static const std::string L_LENGTH= "Length";

class Pipe : public DimensionedLink {

public:
    using inherited= DimensionedLink;

 /*--- Attributes ---*/
protected:
    /*--- Properties ---*/
    aux::QuantitySeries<double> m__length; // Constant

    /*---  Results   ---*/

/*--- Constructors ---*/
public:
    Pipe() = delete;

    Pipe(const std::string& id, const WaterDistributionSystem& wds);

    // Copy constructor
    Pipe(const Pipe& other);

    // Move constructor
    Pipe(Pipe&& rhs) noexcept;

    // Copy assignment operator
    Pipe& operator=(const Pipe& rhs);

    // Move assignment operator
    Pipe& operator=(Pipe&& rhs) noexcept;

    // Destructor
    virtual ~Pipe() = default;

    std::unique_ptr<Pipe> clone() const;

/*--- Getters and setters ---*/
public:
    /*--- Properties ---*/
    aux::QuantitySeries<double>& length() {return m__length;}
    const aux::QuantitySeries<double>& length() const {return m__length;}
    void length(double a_length) {m__length.value(a_length);}

    /*---  Results   ---*/

/*--- Methods ---*/
public:
    /*--- Properties ---*/
    
    /*---  Results   ---*/

    /*---   Other    ---*/
    std::unique_ptr<Pipe> duplicate() const;
    std::unique_ptr<Pipe> duplicate(const std::string& id) const;

/*--- Pure virtual methods ---*/
public:
    /*--- Properties ---*/
    const std::string& element_name() const override {return LNAME_PIPE;}
    const unsigned int element_type() const override {return ELEMENT_PIPE;}

/*-- EPANET-dependent PVMs --*/
public:
    /*--- Properties ---*/
private:
    void __retrieve_EN_properties(EN_Project ph) override;

};

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__PIPE_HPP
