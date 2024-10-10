// 

#ifndef BEVARMEJOLIB__WDS_ELEMENTS__NETWORK_ELEMENT_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__NETWORK_ELEMENT_HPP

#include <string>

#include "epanet2_2.h"

#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/element.hpp"


namespace bevarmejo {
namespace wds {

class WaterDistributionSystem;

class NetworkElement : public Element {
    // WDS ancestor object
    /************************************************************************
     * The bevarmejo::wds::NetworkElement class is the ancestor of networks 
     * and pipes elements of the WDS. It is a pure virtual class.
    */

    public:
        using inherited= Element;

        using ResultsMap= aux::QuantitiesMap;

    /*--- Attributes ---*/
    protected:
        /*--- Properties ---*/
        const WaterDistributionSystem& m__wds;

        /*---  Results   ---*/
    private:
        ResultsMap m__ud_results;

    /*--- Constructors ---*/
    public:
        NetworkElement() = delete;

        NetworkElement(const std::string& id, const WaterDistributionSystem& wds);

        // Copy constructor
        NetworkElement(const NetworkElement& other);

        // Move constructor
        NetworkElement(NetworkElement&& other) noexcept;

        // Copy assignment operator
        NetworkElement& operator=(const NetworkElement& rhs);

        // Move assignment operator
        NetworkElement& operator=(NetworkElement&& rhs) noexcept;

        // Destructor
        virtual ~NetworkElement() = default;

    /*--- Getters and setters ---*/
    public:
        /*--- Properties ---*/

        /*---  Results   ---*/
        const ResultsMap& results() const { return m__ud_results; }
        virtual void clear_results();

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
