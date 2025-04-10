#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace bevarmejo
{
    
class PagmoDecisionVectorAdapter
{

/*---------------------------- Member types ----------------------------------*/
private:
    using DecisionVector = std::vector<double>;
    using DecisionVectorMask = std::vector<bool>;
    using Indexes = std::vector<DecisionVector::size_type>;

/*---------------------------- Member objects --------------------------------*/
private:
    Indexes m__b2p_idxs; // Indexes for the transformation from beme to pagmo
    Indexes m__p2b_idxs; // Indexes for the transformation from pagmo to beme

/*--------------------------- Member functions -------------------------------*/
// (constructor)
public:
    PagmoDecisionVectorAdapter() = default;
    PagmoDecisionVectorAdapter(const DecisionVectorMask& continuous_dvs);
    PagmoDecisionVectorAdapter(const PagmoDecisionVectorAdapter& other) = default;
    PagmoDecisionVectorAdapter(PagmoDecisionVectorAdapter&& other) noexcept = default;

// (destructor)
public:
    ~PagmoDecisionVectorAdapter() = default;

// operator= 
public:
    PagmoDecisionVectorAdapter& operator=(const PagmoDecisionVectorAdapter& other) = default;
    PagmoDecisionVectorAdapter& operator=(PagmoDecisionVectorAdapter&& other) noexcept = default;

// reconfigure
public:
    void reconfigure(const DecisionVectorMask& continuous_dvs);

// transformation
private:
    auto reorder(const DecisionVector& original, const Indexes& idxs, std::string_view from_format_str, std::string_view to_format_str) const -> DecisionVector;

public:
    auto from_beme_to_pagmo(const DecisionVector& original) const -> DecisionVector;

    auto from_pagmo_to_beme(const DecisionVector& original) const -> DecisionVector;

};

} // namespace bevarmejo
