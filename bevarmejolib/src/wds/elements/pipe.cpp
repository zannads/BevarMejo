#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/wds/epanet_helpers/en_help.hpp"

#include "bevarmejo/wds/epanet_helpers/en_time_options.hpp"
#include "bevarmejo/wds/auxiliary/time_series.hpp"
#include "bevarmejo/wds/auxiliary/quantity_series.hpp"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/link.hpp"
#include "bevarmejo/wds/elements/dimensioned_link.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"

#include "pipe.hpp"

namespace bevarmejo {
namespace wds {

Pipe::Pipe(const std::string& id, const WaterDistributionSystem& wds) : 
    inherited(id, wds),
    m__length(wds.time_series(label::__CONSTANT_TS)) { }

// Copy constructor
Pipe::Pipe(const Pipe& other) : 
    inherited(other),
    m__length(other.m__length) { }

// Move constructor
Pipe::Pipe(Pipe&& rhs) noexcept : 
    inherited(std::move(rhs)),
    m__length(std::move(rhs.m__length)) { }

// Copy assignment operator
Pipe& Pipe::operator=(const Pipe& rhs) {
    if (this != &rhs) {
        inherited::operator=(rhs);
        m__length = rhs.m__length;
    }
    return *this;
}

// Move assignment operator
Pipe& Pipe::operator=(Pipe&& rhs) noexcept {
    if (this != &rhs) {
        inherited::operator=(std::move(rhs));
        m__length = std::move(rhs.m__length);
    }
    return *this;
}

std::unique_ptr<Pipe> Pipe::clone() const {
    std::unique_ptr<Pipe> p_pipe = std::make_unique<Pipe>(*this);
    // Now I have a copy of myself, but the pointers still point to the old object
    // If they are unique I can clone them, otherwise I need to invalidate them! 

    // A new cloned pipe doesn't point to any node. This is the only
    // pointer to the outside of the class in this class.
    p_pipe->start_node(nullptr);
    p_pipe->end_node(nullptr);

    return p_pipe;
}

std::unique_ptr<Pipe> Pipe::duplicate() const {
    std::string new_id = "D"+ this->id();
    return this->duplicate(new_id);
}

std::unique_ptr<Pipe> Pipe::duplicate(const std::string& id) const {
    std::unique_ptr<Pipe> p_pipe = std::make_unique<Pipe>(*this);
    p_pipe->id(id);
    p_pipe->index(0);
    // TODO: the results should be "cleared". Time series should be empty, and
    // the other variables should be set to 0.0 (or whatever the default value).
    return p_pipe;
}

/*--- Notes on copy vs clone vs duplicate ---*/
// The copy constructor is used when you want to create a "shallow" copy of an object
// this means that you copy the object, but the pointers inside the object still point
// to the same memory location. This is the default behaviour of the copy constructor.
//
// The clone method is used when you want to create a "deep" copy of an object. This means
// that you copy the object, and the pointers inside the object point to a new memory location.
// This is the default behaviour of the clone method. 
// In this specific case, the only pointers the pipe object has are the pointers
// to the start and end nodes. Since I expect the cloned object to be used in a 
// DIFFERENT network, I don't want the pipe to point to the same nodes as the 
// original pipe. Therefore, I set the pointers to null.
//
// The duplicate method is used when you want to create a new object, identical 
// to the original one, but with a different id, and possibly different 
// properties, to be used in the SAME network.
// In this specific case, I want to create a new pipe, identical to the original
// one, but with a different id (default or no). This pipe is not registered
// in the network, but it needs to know to which nodes it is connected. Hence,
// I don't set it to nullptr. The final "connection" between links and nodes
// (i.e., registering which links are connected to which nodes) is done by the
// network class.
// 
// Finally, use the copy when you want to have an object that is identical to the
// original one, but you don't want to change the original one (mainly for 
// passing it in the functions etc.).
// Use the clone constructor when you need to copy the object to a NEW network.
// Use the duplicate method when you need to copy the object to the SAME network.

void Pipe::__retrieve_EN_properties(EN_Project ph) {
    inherited::__retrieve_EN_properties(ph);
    assert(index()!= 0);

    double val = 0.0;
    int errorode = EN_getlinkvalue(ph, index(), EN_LENGTH, &val);
    if (errorode > 100)
        throw std::runtime_error("Error retrieving pipe length");

    if(ph->parser.Unitsflag == US)
        val *= MperFT; // from ft to m
    m__length= val;
}

} // namespace wds
} // namespace bevarmejo
