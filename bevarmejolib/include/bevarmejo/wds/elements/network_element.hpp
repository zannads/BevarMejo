// 

#ifndef BEVARMEJOLIB__WDS_ELEMENTS__NETWORK_ELEMENT_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__NETWORK_ELEMENT_HPP

#include <string>
#include <unordered_map>
#include <variant>

#include "epanet2_2.h"

#include "bevarmejo/wds/data_structures/variable.hpp"
#include "bevarmejo/wds/data_structures/temporal.hpp"

#include "bevarmejo/wds/epanet_helpers/en_time_options.hpp"
#include "bevarmejo/wds/auxiliary/time_series.hpp"
#include "bevarmejo/wds/auxiliary/quantity_series.hpp"
#include "bevarmejo/wds/elements/element.hpp"

#include "bevarmejo/wds/elements_group.hpp"
#include "bevarmejo/wds/user_defined_elements_group.hpp"

namespace bevarmejo {
namespace wds {

class NetworkElement : public Element {
    // WDS ancestor object
    /************************************************************************
     * The bevarmejo::wds::NetworkElement class is the ancestor of networks 
     * and pipes elements of the WDS. It is a pure virtual class.
    */

    public:
        using inherited= Element;

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
        NetworkElement() = delete;

        NetworkElement(const std::string& id);

        // Copy constructor
        NetworkElement(const NetworkElement& other);

        // Move constructor
        NetworkElement(NetworkElement&& other) noexcept;

        // Copy assignment operator
        NetworkElement& operator=(const NetworkElement& rhs);

        // Move assignment operator
        NetworkElement& operator=(NetworkElement&& rhs) noexcept;

        // Destructor
        virtual ~NetworkElement();

    /*--- Getters and setters ---*/
    public:
        /*--- Properties ---*/

        /*---  Results   ---*/
        ResultsMap& results() { return _results_; }
        void clear_results();

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

using Subnetwork = UserDefinedElementsGroup<NetworkElement>;

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__NETWORK_ELEMENT_HPP
