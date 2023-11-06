//
// element.hpp
// 
// Created by Dennis Zanutto on 20/10/23.

#ifndef BEVARMEJOLIB__WDS_ELEMENTS__ELEMENT_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__ELEMENT_HPP

#include <string>
#include <unordered_map>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/variable.hpp"
#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/results.hpp"

namespace bevarmejo {
namespace wds {

// Registry of element types...
static const unsigned int ELEMENT_ELEMENT = 0; // can't be instantiated but can be used for filtering
static const unsigned int ELEMENT_NETWORK = 1;
static const unsigned int ELEMENT_GROUP= 2;
static const unsigned int ELEMENT_PATTERN= 3;

static const unsigned int ELEMENT_DISTRIBUTION= 4;
static const unsigned int ELEMENT_DISTRIBUTIONS= 5;

static const unsigned int ELEMENT_NODES= 8;
static const unsigned int ELEMENT_LINKS= 9;

static const unsigned int ELEMENT_JUNCTION= 10;
static const unsigned int ELEMENT_RESERVOIR= 11;
static const unsigned int ELEMENT_TANK = 12;
static const unsigned int ELEMENT_SOURCE = 13;

static const unsigned int ELEMENT_NODE = 14;

static const unsigned int ELEMENT_LINK = 15;
static const unsigned int ELEMENT_DIMENSIONED_LINK= 16;

static const unsigned int ELEMENT_PIPE = 20;
static const unsigned int ELEMENT_PUMP= 21;

static const unsigned int ELEMENT_VALVE= 30;
static const unsigned int ELEMENT_PRV= 31;
static const unsigned int ELEMENT_PSV= 32;
static const unsigned int ELEMENT_PBV= 33;
static const unsigned int ELEMENT_FCV= 34;
static const unsigned int ELEMENT_TCV= 35;
static const unsigned int ELEMENT_GPV= 36;

class element {
    // WDS ancestor object
    /************************************************************************
     * The bevarmejo::wds::element class is the ancestor of all the elements
     * of the WDS. It is a pure virtual class, so it cannot be instantiated.
    */

    private:
        std::string _id_; // Human readable id (EPANET ID too)
        int _index_; // Index in the EPANET project for cache purposes

        //using PropertiesTypes = std::variant<std::string, vars::var_int, vars::var_real, vars::var_tseries_int, vars::var_tseries_real>;
        // don't use strings for now
        using PropertiesTypes = std::variant<vars::var_int, vars::var_real, vars::var_tseries_int, vars::var_tseries_real>;
        using PropertiesMap = std::unordered_map<std::string, PropertiesTypes>;
        
        PropertiesMap _properties_; // Properties of the element
        results _results_; // Results of the last simulation, will be filled in the derived class at construction.


    protected:
        virtual void _add_properties();
        virtual void _add_results(); 

        /* should be called every time you add a variable to the results, so that, if you have a property
         * in the derived class that is a pointer to a variable in the results, you can update it. 
         */   
        virtual void _update_pointers(); 
        
    public:
        /// @brief Default constructor
        element();

        element(const std::string& id);

        // Copy constructor
        element(const element& other);

        // Move constructor
        element(element&& rhs) noexcept;

        // Copy assignment operator
        element& operator=(const element& rhs);

        // Move assignment operator
        element& operator=(element&& rhs) noexcept;

        /// @brief Destructor
        virtual ~element();

        bool operator==(const element& rhs) const;
        
        // getters
        const std::string& id() const {return _id_;}
        void id(const std::string& id) {_id_ = id;}
        int index() const {return _index_;}
        void index(const int index) {_index_ = index;}

        virtual const std::string& element_name() const = 0;
        virtual const unsigned int& element_type() const = 0;

        // Functions to get information from EPANET project
        // Results are not loaded thorugh these functions, but rather from the WDS object
        virtual void retrieve_index(EN_Project ph) = 0;
        virtual void retrieve_properties(EN_Project ph) = 0;

        PropertiesMap& properties() {return _properties_;}
        results& results() {return _results_;}
};

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__ELEMENT_HPP
