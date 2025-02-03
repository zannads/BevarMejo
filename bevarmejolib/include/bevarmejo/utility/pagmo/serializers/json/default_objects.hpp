#ifndef BEVARMEJOLIB__PAGMO__SERIALIZERS__JSON__DEFAULT_OBJECTS_HPP
#define BEVARMEJOLIB__PAGMO__SERIALIZERS__JSON__DEFAULT_OBJECTS_HPP

#include <algorithm>
#include <string>
#include <utility>

#include <pagmo/algorithms/null_algorithm.hpp>
#include <pagmo/problems/null_problem.hpp>
#include <pagmo/islands/thread_island.hpp>
#include <pagmo/r_policies/fair_replace.hpp>
#include <pagmo/s_policies/select_best.hpp>
#include <pagmo/topologies/unconnected.hpp>

#include "bevarmejo/io/json.hpp"
#include "bevarmejo/io/key.hpp"
#include "bevarmejo/io/keys/bemeopt.hpp"
#include "bevarmejo/utility/except.hpp"
#include "bevarmejo/utility/string.hpp"

namespace bevarmejo::io::key::detail
{
static const bevarmejo::io::Key pool_flag{"Using pool"}; // "Using pool"
static const bevarmejo::io::Key abs_mig_rate{"Absolute migration rate"}; // "Absolute migration rate"
static const bevarmejo::io::Key frac_mig_rate{"Fractional migration rate"}; // "Fractional migration rate"
} // namespace bevarmejo::io::key::detail

NLOHMANN_JSON_NAMESPACE_BEGIN

/*-------------------------------- Algorithm ---------------------------------*/
/*----------------------------- Null Algorithm -------------------------------*/

/*-------------------------------- Island ------------------------------------*/
/*----------------------------- Thread Island --------------------------------*/
template <>
struct adl_serializer<pagmo::thread_island>
{
    static void to_json(Json &j, const pagmo::thread_island &isl)
    {
        // Reset the json object, just in case.
        j = Json{};

        // I have to parse the extra info to get the pool flag
        // I know it returns "\tUsing pool: yes" or no
        std::string extra_info = isl.get_extra_info();
        extra_info.erase(std::remove(extra_info.begin(), extra_info.end(), '\t'), extra_info.end());
        auto tokens = bevarmejo::split(extra_info, ':');
        assert(tokens.size() == 2);
        assert(tokens[0] == bevarmejo::io::key::detail::pool_flag[0]);

        j[bevarmejo::io::key::detail::pool_flag()] = (tokens[1] == " yes") ? true : false; // space is there too
    }

    static void from_json(const Json &j, pagmo::thread_island &isl)
    {
        beme_throw(std::runtime_error,
            "Cannot build the pagmo::thread_island",
            "The thread island is not supported.");
    }

};

/*-------------------------------- Problem -----------------------------------*/
/*----------------------------- Null Problem ---------------------------------*/

/*---------------------------- Replacement policy ----------------------------*/
/*----------------------------- Fair Replace ---------------------------------*/
template <>
struct adl_serializer<pagmo::fair_replace>
{
    static void to_json(Json &j, const pagmo::fair_replace &rp)
    {
        // Reset, just in case.
        j = Json{};

        // I know the extra info returns "\tAbsolute migration rate: 1" and the 
        // number is an integer or a "\tFractional migration rate: 0.1" and the
        // number is a double.
        std::string extra_info = rp.get_extra_info(); 
        extra_info.erase(std::remove(extra_info.begin(), extra_info.end(), '\t'), extra_info.end());
        auto tokens = bevarmejo::split(extra_info, ':');
        assert(tokens.size() == 2);

        if (tokens[0] == bevarmejo::io::key::detail::abs_mig_rate[0])
        {
            j[bevarmejo::io::key::detail::abs_mig_rate()] = std::stoi(tokens[1]);
        } else /* if (tokens[0] == "Fractional migration rate") */
        {
            j[bevarmejo::io::key::detail::frac_mig_rate()] = std::stod(tokens[1]);
        }
    }

    static void from_json(const Json &j, pagmo::fair_replace &rp)
    {
        beme_throw(std::runtime_error,
            "Cannot build the pagmo::fair_replace",
            "The fair replace is not supported.");
    }

};

/*---------------------------- Selection policy ------------------------------*/
/*----------------------------- Select Best ---------------------------------*/
template <>
struct adl_serializer<pagmo::select_best>
{
    static void to_json(Json &j, const pagmo::select_best &sp)
    {
        // Reset, just in case.
        j = Json{};

        std::string extra_info = sp.get_extra_info(); 
        extra_info.erase(std::remove(extra_info.begin(), extra_info.end(), '\t'), extra_info.end());
        auto tokens = bevarmejo::split(extra_info, ':');
        assert(tokens.size() == 2);

        if (tokens[0] == bevarmejo::io::key::detail::abs_mig_rate[0])
        {
            j[bevarmejo::io::key::detail::abs_mig_rate()] = std::stoi(tokens[1]);
        } else /* if (tokens[0] == "Fractional migration rate") */
        {
            j[bevarmejo::io::key::detail::frac_mig_rate()] = std::stod(tokens[1]);
        }
    }

    static void from_json(const Json &j, pagmo::select_best &sp)
    {
        beme_throw(std::runtime_error,
            "Cannot build the pagmo::select_best",
            "The select best is not supported.");
    }
};

/*---------------------------- Topology -------------------------------------*/
/*----------------------------- Unconnected ---------------------------------*/
template <>
struct adl_serializer<pagmo::unconnected>
{
    static void to_json(Json &j, const pagmo::unconnected &tp)
    {
        // Reset, just in case.
        j = Json{};
        // Nothing to write
    }

    static void from_json(const Json &j, pagmo::unconnected &tp)
    {
        // Nothing to read, the unconnnected topology does not have any parameter.
    }

};

NLOHMANN_JSON_NAMESPACE_END

#endif // BEVARMEJOLIB__PAGMO__SERIALIZERS__JSON__DEFAULT_OBJECTS_HPP
