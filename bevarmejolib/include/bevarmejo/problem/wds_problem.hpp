#pragma once 

#include <string>

#include "bevarmejo/problem/decision_variable.hpp"

namespace bevarmejo {

class WDSProblem {

protected:
    std::string m__name;
    std::string m__extra_info;

    // Decision Variables Adapter
    PagmoDecisionVectorAdapter m__dv_adapter;

public:
    WDSProblem();
    WDSProblem(const std::string& name, const std::string& extra_info="");

    WDSProblem(const WDSProblem&) = default;
    WDSProblem(WDSProblem&&) = default;
    WDSProblem& operator=(const WDSProblem&) = default;
    WDSProblem& operator=(WDSProblem&&) = default;

    virtual ~WDSProblem() = default;


/*----------------------*/
// PUBLIC functions for Pagmo Algorihtm 
/*----------------------*/
public:
    
    // Name of the problem
    std::string get_name() const;

    // Extra information about the problem
    std::string get_extra_info() const;

}; // class WDSProblem


// Given any problem, I can identify it as suite::problem. After that I could pass
// extra info to the problem, for example, the formulation or the version.
namespace io::detail {
struct ProblemName
{
    std::string_view suite;
    std::string_view problem;
    std::string_view formulation;
};
} // namespace io::detail
namespace io {
io::detail::ProblemName split_problem_name(std::string_view problem_name);
} // namespace io

} // namespace bevarmejo
