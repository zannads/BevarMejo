//
// node.hpp
//
// Created by Dennis Zanutto on 20/10/23.
//

#ifndef BEVARMEJOLIB__WDS_ELEMENTS__NODE_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__NODE_HPP

#include <string>
#include <unordered_set>

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/variable.hpp"

#include "bevarmejo/wds/elements/link.hpp"

namespace bevarmejo {
namespace wds {

// Node object
/************************************************************************
 * The bevarmejo::wds::node class is the ancestor of all the nodes
 * of the WDS. It should be a pure virtual class, so it cannot be instantiated.
 */
const std::string LABEL_PRESSURE=       "Pressure";
const std::string LABEL_PRESSURE_UNITS= "m";
const std::string LABEL_HEAD=           "Head";

class link;

class node : public element {
    
    public:
        using inherited= element;

    protected:
        // TODO: use boost:geometry to store coordinates
        double _x_coord_;
        double _y_coord_;

        std::unordered_set<link*> _links_;

        double _elevation_; // or z coordinate

        // results 
        // pointer to the result property in the results object
        vars::var_tseries_real* _head_;
        vars::var_tseries_real* _pressure_;
        // TODO: water quality 

        void _add_properties() override;
        void _add_results() override;

        void _update_pointers() override;

    public:
        /// @brief Default constructor
        node();

        node(const std::string& id);

        // Copy constructor
        node(const node& other);

        // Move constructor
        node(node&& rhs) noexcept;

        // Copy assignment operator
        node& operator=(const node& rhs);

        // Move assignment operator
        node& operator=(node&& rhs) noexcept;

        /// @brief Destructor
        virtual ~node();

        // getters
        const double& x_coord() const {return _x_coord_;}
        void x_coord(const double x_coord) {_x_coord_ = x_coord;}

        const double& y_coord() const {return _y_coord_;}
        void y_coord(const double y_coord) {_y_coord_ = y_coord;}

        const double& z_coord() const {return _elevation_;}
        const double& elevation() const {return _elevation_;}
        void elevation(const double elevation) {_elevation_ = elevation;}

        std::unordered_set<link*>& connected_links() {return _links_;}
        void add_link(link* l);
        void remove_link(link* l);

        // ----- load from EPANET ----- //
        void retrieve_index(EN_Project ph) override;
        void retrieve_properties(EN_Project ph) override;

        // results
        const vars::var_tseries_real& head() const {return *_head_;}
        void head(const vars::var_tseries_real& head) {*_head_ = head;}

        const vars::var_tseries_real& pressure() const {return *_pressure_;}
        void pressure(const vars::var_tseries_real& pressure) {*_pressure_ = pressure;}

}; // class node

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__NODE_HPP
