#pragma once 

#include <array>
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>

namespace bevarmejo
{

namespace detail
{

constexpr unsigned int version = BEME_VERSION;
constexpr unsigned int version_year = BEME_MAJOR_VERSION;
constexpr unsigned int version_month = BEME_MINOR_VERSION;
constexpr unsigned int version_release = BEME_PATCH_VERSION;

constexpr std::array<char, 10> version_arr = {
        'v',
        static_cast<char>('0' + version_year/10),
        static_cast<char>('0' + version_year%10),
        '.',
        static_cast<char>('0' + version_month/10),
        static_cast<char>('0' + version_month%10),
        '.',
        static_cast<char>('0' + version_release/10),
        static_cast<char>('0' + version_release%10),
        '\0'
    };

#if BEME_VERSION < 240401
constexpr unsigned int min_version = 230600;
#elif BEME_VERSION < 240601
constexpr unsigned int min_version = 240401;
#elif BEME_VERSION < 241100
constexpr unsigned int min_version = 240601;
#elif BEME_VERSION < 241200
constexpr unsigned int min_version = 241100;
#elif BEME_VERSION < 250200
constexpr unsigned int min_version = 241200;
#else // BEME_VERSION >= 250200
constexpr unsigned int min_version = 250200;
#endif

constexpr std::array<char, 10> min_version_arr = {
        'v',
        static_cast<char>('0' + (min_version%1000000)/100000),
        static_cast<char>('0' + (min_version%100000)/10000),
        '.',
        static_cast<char>('0' + (min_version%10000)/1000),
        static_cast<char>('0' + (min_version%1000)/100),
        '.',
        static_cast<char>('0' + (min_version%100)/10),
        static_cast<char>('0' + (min_version%10)/1),
        '\0'
    };

// From a string, parse major, minor and patch versions
inline std::tuple<unsigned int, unsigned int, unsigned int> parse(const std::string& v_str)
{
    // Let's intialize the version numbers with invalid values so that if at least one is not found, the return will be invalid.
    unsigned int major = 0;
    unsigned int minor = 0;
    unsigned int patch = 100;

    if (v_str.empty() || *v_str.begin() != 'v')
    {
        return std::make_tuple(major, minor, patch);
    }

    std::istringstream iss(std::string(v_str.begin()+1, v_str.end()));

    auto parse_value = [&iss](unsigned int& v) -> void {
        try
        {
            std::string token;
            std::getline(iss, token, '.');
            v = std::stoi(token);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    };
    
    parse_value(major);
    parse_value(minor);
    parse_value(patch);

    return std::make_tuple(major, minor, patch);
}

// From major, minor and patch versions, create a numeric version
inline unsigned int as_numeric(const std::tuple<unsigned int, unsigned int, unsigned int>& v)
{
    // A version tuple is valid if:
    //  - the patch version is less than 100
    //  - the minor version is a month
    //  - the major version is the last two digits of a year between min_version and version

    if (std::get<2>(v) > 99 || std::get<1>(v) > 12 || std::get<1>(v) < 1 || std::get<0>(v) < min_version/10000 || std::get<0>(v) > version/10000)
    {
        return 0;
    }
    
    return std::get<0>(v) * 10000 + std::get<1>(v) * 100 + std::get<2>(v);
}

} // namespace detail

inline bool is_valid_version(const std::string& version_str)
{
    auto v_numeric =  detail::as_numeric(detail::parse(version_str));

    return v_numeric != 0 && v_numeric >= detail::min_version && v_numeric <= detail::version;
}

constexpr const char* version_str = detail::version_arr.data();

constexpr const char* min_version_str = detail::min_version_arr.data();

} // namespace bevarmejo
