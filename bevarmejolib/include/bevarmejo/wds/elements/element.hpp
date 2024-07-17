//
// element.hpp
// 
// Created by Dennis Zanutto on 20/10/23.

#ifndef BEVARMEJOLIB__WDS_ELEMENTS__ELEMENT_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__ELEMENT_HPP

#include <string>
#include <unordered_map>
#include <variant>

#include "epanet2_2.h"

#include "bevarmejo/wds/data_structures/variable.hpp"
#include "bevarmejo/wds/data_structures/temporal.hpp"

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

class Element {
    // WDS ancestor object
    /************************************************************************
     * The bevarmejo::wds::Element class is the ancestor of all the elements
     * of the WDS. It is a pure virtual class, so it cannot be instantiated.
    */

   /*--- Attributes ---*/
    private:
        /*--- Properties ---*/
        std::string _id_; // Human readable id (EPANET ID too)
        int _index_; // Index in the EPANET project for cache purposes

        //using PropertiesTypes = std::variant<std::string, vars::var_int, vars::var_real, vars::var_tseries_int, vars::var_tseries_real>;
        // don't use strings for now
        using PropertiesTypes = std::variant<
            vars::var_int, 
            vars::var_real, 
            vars::var_tseries_int, 
            vars::var_tseries_real>;
        using PropertiesMap = std::unordered_map<std::string, PropertiesTypes>;
        
        PropertiesMap _properties_; // Properties of the element
        
    protected:
        virtual void _add_properties();

        /* should be called every time you add a variable to the results, so that, if you have a property
         * in the derived class that is a pointer to a variable in the results, you can update it. 
         */   
        virtual void _update_pointers(); 
        
    /*--- Constructors ---*/
    public:
        /// @brief Default constructor
        Element();

        Element(const std::string& id);

        // Copy constructor
        Element(const Element& other);

        // Move constructor
        Element(Element&& rhs) noexcept;

        // Copy assignment operator
        Element& operator=(const Element& rhs);

        // Move assignment operator
        Element& operator=(Element&& rhs) noexcept;

        /// @brief Destructor
        virtual ~Element();

    /*--- Operators ---*/
    public:
        bool operator==(const Element& rhs) const;
        
    /*--- Getters and setters ---*/
    public:
        /*--- Properties ---*/
        const std::string& id() const {return _id_;}
        void id(const std::string& id) {_id_ = id;}
        int index() const {return _index_;}
        void index(const int index) {_index_ = index;}

        PropertiesMap& properties() {return _properties_;}

    /*--- Pure virtual methods ---*/
    public:
        /*--- Properties ---*/
        virtual const std::string& element_name() const = 0;
        virtual const unsigned int element_type() const = 0;

    /*-- EPANET-dependent PVMs --*/
    public:
        /*--- Properties ---*/
        virtual void retrieve_index(EN_Project ph) = 0;
        virtual void retrieve_properties(EN_Project ph) = 0;
   
};

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__ELEMENT_HPP
