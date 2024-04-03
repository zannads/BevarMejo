#ifndef BEVARMEJOLIB__PAGMO_HELPERS__CONTAINERS_HELP_HPP
#define BEVARMEJOLIB__PAGMO_HELPERS__CONTAINERS_HELP_HPP

#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <pagmo/algorithm.hpp>
#include <pagmo/island.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/r_policy.hpp>
#include <pagmo/s_policy.hpp>
#include <pagmo/topology.hpp>

#include <nlohmann/json.hpp>
namespace nl = nlohmann;

#include "bevarmejo/io.hpp"
#include "bevarmejo/labels.hpp"

namespace bevarmejo {
namespace label {
// A set of strings to which we compare against to see if the given pagmo element
// is the default one. E.g., the default topology of an archipelago is
// "Unconnected", if it is so, we won't save it to the settings file.
namespace pagmodefault {

const std::string __problem     = "Null problem";
const std::string __algorithm   = "Null algorithm";
const std::string __island      = "Thread island";
const std::string __r_policy    = "Fair replace";
const std::string __s_policy    = "Select best";
const std::string __bfe         = "Default batch fitness evaluator";
const std::string __topology    = "Unconnected";

} // namespace pagmodefault
} // namespace label

// Given a pagmo UD class of any type, usually you call get_extra_info() to get
// the settings of the class. However, this function returns a string.
// I want to return a json object for saving, and actually, I would need one for 
// static parameters and one for dynamic parameters. For now only static.

namespace io {
namespace json {

/*-------------------- Algorithm --------------------*/
nl::json static_descr(const pagmo::algorithm& algo);
nl::json dynamic_descr(const pagmo::algorithm& algo);

/*-------------------- Problem --------------------*/
// TODO: add the problem and see how to deal with my user defined problems

nl::json static_descr(const pagmo::problem& prob);
nl::json dynamic_descr(const pagmo::problem& prob);
/*-------------------- Island --------------------*/
nl::json static_descr(const pagmo::island& isl);
// Islands can not have dynamic parameters, so it is a compile error to call this function
nl::json dynamic_descr(const pagmo::island& isl) = delete;

/*-------------------- Replacement policy --------------------*/
nl::json static_descr(const pagmo::r_policy& rp);
nl::json dynamic_descr(const pagmo::r_policy& rp) = delete;

/*-------------------- Selection policy --------------------*/
nl::json static_descr(const pagmo::s_policy& sp);
nl::json dynamic_descr(const pagmo::s_policy& sp) = delete;

/*-------------------- Topology --------------------*/
nl::json static_descr(const pagmo::topology& tp);
nl::json dynamic_descr(const pagmo::topology& tp) = delete;

} // namespace json
} // namespace io
} // namespace bevarmejo

#endif // BEVARMEJOLIB__PAGMO_HELPERS__CONTAINERS_HELP_HPP
