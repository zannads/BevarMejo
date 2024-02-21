//
// node.hpp
//
// Created by Dennis Zanutto on 20/10/23.
//

#ifndef BEVARMEJOLIB__WDS_ELEMENTS__NODE_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__NODE_HPP

#include <string>
#include <unordered_set>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/variable.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/link.hpp"

namespace bevarmejo {
namespace wds {

// Node object
/************************************************************************
 * The bevarmejo::wds::Node class is the ancestor of all the nodes
 * of the WDS. It should be a pure virtual class, so it cannot be instantiated.
 */
const std::string LABEL_PRESSURE=       "Pressure";
const std::string LABEL_PRESSURE_UNITS= "m";
const std::string LABEL_HEAD=           "Head";

class Link;

class Node : public NetworkElement {
    
    public:
        using inherited= NetworkElement;

    /*--- Attributes ---*/
    protected:
        /*--- Properties ---*/
        // TODO: use boost:geometry to store coordinates
        double _x_coord_;
        double _y_coord_;

        std::unordered_set<Link*> _links_;

        // TODO: transform into variable of some type
        double _elevation_; // or z coordinate

        // TODO: add a parameter(option) that when true also forces the deletion
        // of the links connected to the node.

        /*---  Results   ---*/ 
        // pointer to the result property in the results object
        vars::var_tseries_real* _head_;
        vars::var_tseries_real* _pressure_;
        // TODO: water quality 

    protected:
        void _add_properties() override;
        void _add_results() override;
        void _update_pointers() override;

    /*--- Constructors ---*/
    public:
        /// @brief Default constructor
        Node() = delete;

        Node(const std::string& id);

        // Copy constructor
        Node(const Node& other);

        // Move constructor
        Node(Node&& rhs) noexcept;

        // Copy assignment operator
        Node& operator=(const Node& rhs);

        // Move assignment operator
        Node& operator=(Node&& rhs) noexcept;

        /// @brief Destructor
        virtual ~Node();

    /*--- Getters and setters ---*/
    public:
        /*--- Properties ---*/
        const double x_coord() const {return _x_coord_;}
        void x_coord(const double x_coord) {_x_coord_ = x_coord;}

        const double y_coord() const {return _y_coord_;}
        void y_coord(const double y_coord) {_y_coord_ = y_coord;}

        // TODO: See Issue #32
        std::unordered_set<Link*>& connected_links() {return _links_;}
        void add_link(Link* a_link);
        void remove_link(Link* a_link);

        const double z_coord() const {return _elevation_;}
        const double elevation() const {return _elevation_;}
        void elevation(const double elevation) {_elevation_ = elevation;}

        /*--- Results ---*/
        const vars::var_tseries_real& head() const {return *_head_;}
        const vars::var_tseries_real& pressure() const {return *_pressure_;}

    /*--- Methods ---*/
    public:
        virtual const bool has_demand() const {return false;}

    /*--- Pure virtual methods override---*/

    /*--- EPANET-dependent PVMs override ---*/
    public:
        void retrieve_index(EN_Project ph) override;
        void retrieve_properties(EN_Project ph) override;
        void retrieve_results(EN_Project ph, long t) override;

}; // class Node

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__NODE_HPP
