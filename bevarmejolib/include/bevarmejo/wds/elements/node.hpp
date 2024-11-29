//
// node.hpp
//
// Created by Dennis Zanutto on 20/10/23.
//

#ifndef BEVARMEJOLIB__WDS_ELEMENTS__NODE_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__NODE_HPP

#include <memory>
#include <string>
#include <unordered_set>

#include "epanet2_2.h"

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/network_element.hpp"


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

        std::unordered_set<const Link*> m__links;

        // TODO: transform into variable of some type
        double _elevation_; // or z coordinate

        // TODO: add a parameter(option) that when true also forces the deletion
        // of the links connected to the node.

        /*---  Results   ---*/
        aux::QuantitySeries<double> m__head;
        aux::QuantitySeries<double> m__pressure;
        // TODO: water quality 

    /*--- Constructors ---*/
    public:
        /// @brief Default constructor
        Node() = delete;

        Node(const std::string& id, const WaterDistributionSystem& wds);

        // Copy constructor
        Node(const Node& other);

        // Move constructor
        Node(Node&& rhs) noexcept;

        // Copy assignment operator
        Node& operator=(const Node& rhs);

        // Move assignment operator
        Node& operator=(Node&& rhs) noexcept;

        /// @brief Destructor
        virtual ~Node() = default;

    /*--- Getters and setters ---*/
    public:
        /*--- Properties ---*/
        const double x_coord() const {return _x_coord_;}
        void x_coord(const double x_coord) {_x_coord_ = x_coord;}

        const double y_coord() const {return _y_coord_;}
        void y_coord(const double y_coord) {_y_coord_ = y_coord;}

        // TODO: See Issue #32
        std::unordered_set<const Link*>& connected_links() {return m__links;}
        const std::unordered_set<const Link*>& connected_links() const {return m__links;}
        void add_link(const Link* a_link);
        void remove_link(const Link* a_link);

        const double z_coord() const {return _elevation_;}
        const double elevation() const {return _elevation_;}
        void elevation(const double elevation) {_elevation_ = elevation;}

        /*--- Results ---*/
        const aux::QuantitySeries<double>& head() const {return m__head;}
        const aux::QuantitySeries<double>& pressure() const {return m__pressure;}

    /*--- Methods ---*/
    public:
        virtual const bool has_demand() const {return false;}

    /*--- Pure virtual methods override---*/
        virtual void clear_results() override;

    /*--- EPANET-dependent PVMs override ---*/
    public:
        void retrieve_index(EN_Project ph) override;
        void retrieve_results(EN_Project ph, long t) override;
    protected:
        void __retrieve_EN_properties(EN_Project ph) override;

}; // class Node

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__NODE_HPP
