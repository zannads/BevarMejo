#ifndef BEVARMEJOLIB__PAGMO_HELPERS_HPP
#define BEVARMEJOLIB__PAGMO_HELPERS_HPP

#include <string>

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
} // namespace bevarmejo

#endif // BEVARMEJOLIB__PAGMO_HELPERS_HPP
