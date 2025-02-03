#ifndef BEVARMEJOLIB__EPANET_HELPERS__WRAPPER_HPP
#define BEVARMEJOLIB__EPANET_HELPERS__WRAPPER_HPP

#include <cassert>
#include <sstream>
#include <string>

#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/utility/except.hpp"

namespace bevarmejo {
namespace epanet {


inline double convert_flow_to_L_per_s(EN_Project ph, double a_flow) {
    if (ph->parser.Unitsflag == US){
        switch (ph->parser.Flowflag) {
        case CFS:
            a_flow *= LPSperCFS; // from ft^3/S to L/s
            break;

        case GPM:
            a_flow *= LPSperCFS/GPMperCFS; // from GPM to L/s
            break;

        case MGD:
            a_flow *= LPSperCFS/MGDperCFS; // from MGD to L/s
            break;

        case IMGD:
            a_flow *= LPSperCFS/IMGDperCFS; // from IMGD to L/s
            break;

        case AFD:
            a_flow *= LPSperCFS/AFDperCFS; // from AFD to L/s
            break;

        default:
            break; // else result is like it is and it may be wrong.
        }
    }
    else { // SI
        switch (ph->parser.Flowflag) {
        case LPM:
            a_flow /= 60.0; // from L/min to L/s
            break;

        case MLD:
            a_flow *= 1000.*1000/(24*60*60); // from ML/day to L/s
            break;

        case CMH:
            a_flow *= 1000./60/60; // from m^3/h to L/s
            break;

        case CMD:
            a_flow *= 1000./24/60/60; // from m^3/day to L/s
            break;
        
        default:
            break; // already L/s or unknown type.
        }
    }

    return a_flow;
}

inline std::string get_node_id(EN_Project ph, int index) {
    char* node_id = new char[EN_MAXID+1];
    int errorcode = EN_getnodeid(ph, index, node_id);
    assert(errorcode <= 100);
    
    std::string node_id_str(node_id);
    delete[] node_id;

    return node_id_str;
}

inline int is_string_en_object_type(const std::string& en_obj_string)
{
    if (en_obj_string == "EN_NODE")
        return EN_NODE;

    if (en_obj_string == "EN_LINK")
        return EN_LINK;

    if (en_obj_string == "EN_TIMEPAT")
        return EN_TIMEPAT;

    if (en_obj_string == "EN_CURVE")
        return EN_CURVE;

    if (en_obj_string == "EN_CONTROL")
        return EN_CONTROL;

    if (en_obj_string == "EN_RULE")
        return EN_RULE;

    beme_throw(std::invalid_argument,
        "Can not convert string to EN object type.",
        "String is not a valid EN object type.",
        "String: ", en_obj_string);
}

} // namespace epanet
} // namespace bevarmejo

#endif // BEVARMEJOLIB__EPANET_HELPERS__WRAPPER_HPP
