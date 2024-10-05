#pragma once 

#include <string>

namespace bevarmejo {

class WDSProblem {

protected:
    std::string m__name;
    std::string m__extra_info;

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

} // namespace bevarmejo
