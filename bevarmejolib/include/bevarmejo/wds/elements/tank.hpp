#ifndef BEVARMEJOLIB__WDS_ELEMENTS__TANK_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__TANK_HPP

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

/// WDS Tank
/*******************************************************************************
 * The wds::junction class represents a Tank in the network. It is a dynamic element.
 ******************************************************************************/

static const std::string l__NAME_TANK= "Tank";

static const std::string l__MIN_VOLUME = "MinVolume";
static const std::string l__MIN_LEVEL = "MinLevel";
static const std::string l__MAX_LEVEL = "MaxLevel";
static const std::string l__INITIAL_LEVEL = "InitLevel";
static const std::string l__LEVEL = "Level";
static const std::string l__INITIAL_VOLUME = "InitVolume";
static const std::string l__VOLUME = "Volume";
static const std::string l__MAX_VOLUME = "MaxVolume";

class Tank : public Source {

public:
    using inherited= Source;

/*--- Attributes ---*/
protected:
    /*--- Properties ---*/
    //MIXMODEL
    //TANKDIAM
    vars::var_real* _min_volume_;
    //VOLCURVE
    vars::var_real* _min_level_;
    vars::var_real* _max_level_;
    //MIXFRACTION
    //TANK_KBULK
    //CANOVERFLOW
    vars::var_real* _initial_level_;

    /*---  Results   ---*/
    vars::var_tseries_real* _level_;
    vars::var_real* _initial_volume_;
    //MIXZONEVOL
    vars::var_tseries_real* _volume_;
    vars::var_real* _max_volume_;

protected:
    void _add_properties() override;
    void _add_results() override;
    void _update_pointers() override;

/*--- Constructors ---*/
public:
    Tank() = delete;
    Tank(const std::string& id);

    // Copy constructor
    Tank(const Tank& other);

    // Move constructor
    Tank(Tank&& rhs) noexcept;

    // Copy assignment operator
    Tank& operator=(const Tank& rhs);

    // Move assignment operator
    Tank& operator=(Tank&& rhs) noexcept;

    // Destructor
    ~Tank() override;

/*--- Getters and setters ---*/
public:
    /*--- Properties ---*/
    vars::var_real& min_volume() const { return *_min_volume_; }
    void min_volume(const double a_min_volume) { *_min_volume_ = a_min_volume; }
    vars::var_real& min_level() const { return *_min_level_; }
    void min_level(const double a_min_level) { *_min_level_ = a_min_level; }
    vars::var_real& max_level() const { return *_max_level_; }
    void max_level(const double a_max_level) { *_max_level_ = a_max_level; }
    vars::var_real& initial_level() const { return *_initial_level_; }
    void initial_level(const double a_initial_level) { *_initial_level_ = a_initial_level; }

    /*---  Results   ---*/
    const vars::var_tseries_real& level() const { return *_level_; }
    const vars::var_real& initial_volume() const { return *_initial_volume_; }
    const vars::var_tseries_real& volume() const { return *_volume_; }
    const vars::var_real& max_volume() const { return *_max_volume_; }

/*--- Pure virtual methods override---*/
public:
    /*--- Properties ---*/
    const std::string& element_name() const override { return l__NAME_TANK; }
    const unsigned int element_type() const override { return ELEMENT_TANK; }

    /*--- Results ---*/

/*--- EPANET-dependent PVMs override ---*/
public:
    /*--- Properties ---*/
    void retrieve_properties(EN_Project ph) override;

    /*--- Results ---*/
    void retrieve_results(EN_Project ph, long t) override;

}; // class Tank

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__TANK_HPP
