//
// element.hpp
// 
// Created by Dennis Zanutto on 20/10/23.

#ifndef BEVARMEJOLIB__WDS_ELEMENTS__ELEMENT_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__ELEMENT_HPP

#include <string>

#include "bevarmejo/wds/elements/results.hpp"

namespace bevarmejo {
namespace wds {

class element {
    // WDS ancestor object
    /************************************************************************
     * The bevarmejo::wds::element class is the ancestor of all the elements
     * of the WDS. It is a pure virtual class, so it cannot be instantiated.
    */

    private:
        std::string _id_; // Human readable id (EPANET ID too)

        results _results_; // Results of the last simulation, will be filled in the derived class at construction.

    protected:
        virtual void _add_results(); 

        /* should be called every time you add a variable to the results, so that, if you have a property
         * in the derived class that is a pointer to a variable in the results, you can update it. 
         */   
        virtual void _update_pointers(); 
        
    public:
        /// @brief Default constructor
        element();

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

        virtual const std::string& element_name() const = 0;
        virtual const std::string& element_type() const = 0;

        results& results() {return _results_;}
};

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__ELEMENT_HPP
