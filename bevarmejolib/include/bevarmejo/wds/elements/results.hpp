//
// results.hpp
// 
// Created by Dennis Zanutto on 19/10/23.

#ifndef BEVARMEJOLIB__WDS_ELEMENTS__RESULTS_HPP
#define BEVARMEJOLIB__WDS_ELEMENTS__RESULTS_HPP

#include <string>

#include "temporal.hpp"
#include "variable.hpp"
#include "variables.hpp"

namespace bevarmejo {
namespace wds {

class results {

private:
    vars::variables<vars::variable<std::string>> _strings_;
    vars::variables<vars::variable<int>> _integers_;
    vars::variables<vars::variable<double>> _reals_;   

    vars::variables<vars::variable<vars::temporal<int>>> _temporal_integers_;
    vars::variables<vars::variable<vars::temporal<double>>> _temporal_reals_;

public:
    /// @brief Default constructor
    results();

    // Copy constructor
    results(const results& other);

    // Move constructor
    results(results&& rhs) noexcept;

    // Copy assignment operator
    results& operator=(const results& rhs);

    // Move assignment operator
    results& operator=(results&& rhs) noexcept;

    /// @brief Destructor
    virtual ~results();

    // getters 
    vars::variables<vars::variable<std::string>>& strings() {return _strings_;}
    vars::variables<vars::variable<int>>& integers() {return _integers_;}
    vars::variables<vars::variable<double>>& reals() {return _reals_;}

    vars::variables<vars::variable<vars::temporal<int>>>& temporal_integers() {return _temporal_integers_;}
    vars::variables<vars::variable<vars::temporal<double>>>& temporal_reals() {return _temporal_reals_;}

    // to merge two results objects
    void add(const results& rhs);
    
    // count total number of result of any type
    const std::size_t size() const;

    // clear the results 
    void clear();


};

} // namespace wds
} // namespace bevarmejo

#endif // BEVARMEJOLIB__WDS_ELEMENTS__RESULTS_HPP
 