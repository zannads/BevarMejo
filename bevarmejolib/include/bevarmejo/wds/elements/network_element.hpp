// 

#ifndef BEVARMEJOLIB__WDS_ELEMENTS__NETWORK_ELEMENT_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__NETWORK_ELEMENT_HPP

#include <string>
#include <unordered_map>
#include <variant>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/variable.hpp"
#include "bevarmejo/wds/elements/temporal.hpp"
#include "bevarmejo/wds/elements/element.hpp"

namespace bevarmejo {
namespace wds {

class network_element : public element {
    // WDS ancestor object
    /************************************************************************
     * The bevarmejo::wds::network_element class is the ancestor of networks 
     * and pipes elements of the WDS. It is a pure virtual class.
    */

    public:
        using inherited= element;

    /*--- Attributes ---*/
    private:
        /*--- Properties ---*/

        /*---  Results   ---*/
        using ResultsTypes = std::variant<
            vars::var_int,
            vars::var_real,
            vars::var_tseries_int,
            vars::var_tseries_real> ;
        using ResultsMap = std::unordered_map<std::string, ResultsTypes>;

        ResultsMap _results_;

    protected:
        void _add_properties() override;
        virtual void _add_results();
        void _update_pointers() override;

    /*--- Constructors ---*/
    public:
        network_element() = delete;

        network_element(const std::string& id);

        // Copy constructor
        network_element(const network_element& other);

        // Move constructor
        network_element(network_element&& other) noexcept;

        // Copy assignment operator
        network_element& operator=(const network_element& rhs);

        // Move assignment operator
        network_element& operator=(network_element&& rhs) noexcept;

        // Destructor
        virtual ~network_element();

    /*--- Getters and setters ---*/
    public:
        /*--- Properties ---*/

        /*---  Results   ---*/
        ResultsMap& results() { return _results_; }

    /*--- Pure virtual methods ---*/
    public:
        /*--- Properties ---*/

        /*---  Results   ---*/

    /*--- EPANET-dependent PVMs ---*/
    public:
        /*--- Properties ---*/

        /*---  Results   ---*/
        virtual void retrieve_results(EN_Project ph, long t) = 0;

};

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__NETWORK_ELEMENT_HPP
