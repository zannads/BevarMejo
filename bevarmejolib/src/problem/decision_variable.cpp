#include "bevarmejo/utility/exceptions.hpp"
#include "bevarmejo/utility/string.hpp"

#include "decision_variable.hpp"

namespace bevarmejo
{

PagmoDecisionVectorAdapter::PagmoDecisionVectorAdapter(const DecisionVectorMask& continuous_dvs_mask) :
    PagmoDecisionVectorAdapter()
{
    reconfigure(continuous_dvs_mask);   
}

void PagmoDecisionVectorAdapter::reconfigure(const DecisionVectorMask& continuous_dvs_mask)
{
    // Create empy output one with the same values. This may fail if there is no memory, hence the missing noexcept
    m__b2p_idxs = Indexes(continuous_dvs_mask.size(), 0);
    m__p2b_idxs.clear();
    m__p2b_idxs.reserve(continuous_dvs_mask.size());

    // First look for the indexes of the continuous ones because in pagmo they go first.
    for (DecisionVector::size_type i = 0; i < continuous_dvs_mask.size(); ++i)
    {
        if (continuous_dvs_mask[i])
        {
            // This element is a continuous decision variable, so it will occupy
            // the next position in the vector of indexes for the transformation from pagmo to beme.
            // However, for the opposite direction, i.e. getting back to beme I just have to use the current idx
            // I first add in direct and using size because it has not been added there yet
            m__b2p_idxs[i] = m__p2b_idxs.size();
            m__p2b_idxs.push_back(i);
        }
    }

    // Now we do the same for the discrete ones
    for (DecisionVector::size_type i = 0; i < continuous_dvs_mask.size(); ++i)
    {
        if (!continuous_dvs_mask[i])
        {
            m__b2p_idxs[i] = m__p2b_idxs.size();
            m__p2b_idxs.push_back(i);
        }
    }
}

auto PagmoDecisionVectorAdapter::reorder(
    const DecisionVector& original,
    const Indexes& idxs,
    std::string_view from_format_str,
    std::string_view to_format_str
) const -> DecisionVector
{
    beme_throw_if(original.size() != idxs.size(),
        std::runtime_error,
        bevarmejo::stringify("Error converting the decision vector from ",from_format_str," to ",to_format_str),
        "The size of the decision vector doesn't match the one for which the adapter was configured.",
        "Decision vector size: ", original.size(),
        "\nAdapter configured for a dv with size: ", idxs.size()
    );

    auto ordered = original;

    for(Indexes::size_type i = 0; i < idxs.size(); ++i)
    {
        ordered[idxs[i]] = original[i];
    }

    return ordered;
}

auto PagmoDecisionVectorAdapter::from_beme_to_pagmo(
    const DecisionVector& original
) const ->  DecisionVector
{
    return reorder(original, m__b2p_idxs, "beme", "pagmo");
}

auto PagmoDecisionVectorAdapter::from_pagmo_to_beme(
    const DecisionVector& original
) const ->  DecisionVector
{
    return reorder(original, m__p2b_idxs, "pagmo", "beme");
}

}