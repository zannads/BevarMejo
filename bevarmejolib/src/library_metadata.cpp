#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <sstream>
#include <tuple>

#include "bevarmejo/bemexcept.hpp"

#include "library_metadata.hpp"

namespace bevarmejo::detail {

constexpr unsigned int version_year = 2024;
constexpr unsigned int version_month = 10;
constexpr unsigned int version_release = 1;

namespace io::log::cname {
static const std::string version = "Version"; // "Version"
} // namespace io::log::cname
namespace io::log::fname {
static const std::string constructor = "Version<std::vector<unsigned int>>"; // "Version<std::vector<unsigned int>>"
} // namespace io::log::fname
namespace io::log::mex {
static const std::string failed_to_create = "Failed to create a Version object."; // "Failed to create a Version object."
} // namespace io::log::mex

// Main constructor to create a version. All the others use this one.
Version::Version(std::vector<unsigned int> il) : 
    m_year(version_year), m_month(version_month), m_release(version_release)
{
    if (il.size() != 3)
        bevarmejo::__format_and_throw<std::invalid_argument, FunctionError>(
            io::log::cname::version, io::log::fname::constructor, io::log::mex::failed_to_create,
            "The initializer list must have 3 elements: year, month, release.",
            "The initializer list has " + std::to_string(il.size()) + " elements."
            );

    const unsigned int year = *(il.begin());
    const unsigned int month = *(il.begin()+1);
    const unsigned int release = *(il.begin()+2);

    if (year < 2000 || year > version_year)
        bevarmejo::__format_and_throw<std::invalid_argument, FunctionError>(
            io::log::cname::version, io::log::fname::constructor, io::log::mex::failed_to_create,
            "Year must be between 2000 and " + std::to_string(version_year),
            "Year = " + std::to_string(year)
            );

    if (month < 1 || month > 12)
        bevarmejo::__format_and_throw<std::invalid_argument, FunctionError>(
            io::log::cname::version, io::log::fname::constructor, io::log::mex::failed_to_create,
            "Invalid month number (must be between 1 and 12).",
            "Month = " + std::to_string(month)
            );
    
    if (release > 99)
        bevarmejo::__format_and_throw<std::invalid_argument, FunctionError>(
            io::log::cname::version, io::log::fname::constructor, io::log::mex::failed_to_create,
            "Invalid release number (must be between 0 and 99).",
            "Release = " + std::to_string(release)
            );

    if (year == version_year && month > version_month ||
        year == version_year && month == version_month && release > version_release)
        bevarmejo::__format_and_throw<std::invalid_argument, FunctionError>(
            io::log::cname::version, io::log::fname::constructor, io::log::mex::failed_to_create,
            "The version is greater than the current version.",
            "Current version = ", Version().str(),
            "Year = " + std::to_string(year) + ", Month = " + std::to_string(month) + ", Release = " + std::to_string(release)
            );

    m_year = year;
    m_month = month;
    m_release = release;
}

Version::Version() : 
    Version(std::vector{version_year, version_month, version_release}) {}

Version::Version(unsigned int year, unsigned int month, unsigned int release) :
    Version(std::vector{year, month, release}) {}

Version::Version(unsigned int numeric) : 
    Version(Version::parse(numeric)) {}

Version::Version(const std::string &version_str) : 
    Version(Version::parse(version_str)) {}

// Accessors

unsigned int Version::year() const { return m_year; }

unsigned int Version::month() const { return m_month; }

unsigned int Version::release() const { return m_release; }

unsigned int Version::numeric() const { return numeric(m_year, m_month, m_release); }

std::string Version::str() const { return str(m_year, m_month, m_release); }

// Comparison operators

bool Version::operator<(const Version& other) const {
    return std::tie(m_year, m_month, m_release) < std::tie(other.m_year, other.m_month, other.m_release);
}

bool Version::operator==(const Version& other) const {
    return std::tie(m_year, m_month, m_release) == std::tie(other.m_year, other.m_month, other.m_release);
}

bool Version::operator>(const Version& other) const {
    return std::tie(m_year, m_month, m_release) > std::tie(other.m_year, other.m_month, other.m_release);
}

bool Version::operator<=(const Version& other) const {
    return std::tie(m_year, m_month, m_release) <= std::tie(other.m_year, other.m_month, other.m_release);
}

bool Version::operator>=(const Version& other) const {
    return std::tie(m_year, m_month, m_release) >= std::tie(other.m_year, other.m_month, other.m_release);
}

bool Version::operator!=(const Version& other) const {
    return std::tie(m_year, m_month, m_release) != std::tie(other.m_year, other.m_month, other.m_release);
}

bool Version::operator<(const std::string& other) const { return *this < Version(parse(other)); }
bool Version::operator==(const std::string& other) const { return *this == Version(parse(other)); }
bool Version::operator>(const std::string& other) const { return *this > Version(parse(other)); }
bool Version::operator<=(const std::string& other) const { return *this <= Version(parse(other)); }
bool Version::operator>=(const std::string& other) const { return *this >= Version(parse(other)); }
bool Version::operator!=(const std::string& other) const { return *this != Version(parse(other)); }

bool Version::operator<(const unsigned int other) const { return *this < Version(parse(other)); }
bool Version::operator==(const unsigned int other) const { return *this == Version(parse(other)); }
bool Version::operator>(const unsigned int other) const { return *this > Version(parse(other)); }
bool Version::operator<=(const unsigned int other) const { return *this <= Version(parse(other)); }
bool Version::operator>=(const unsigned int other) const { return *this >= Version(parse(other)); }
bool Version::operator!=(const unsigned int other) const { return *this != Version(parse(other)); }

// Conversions from other formats

std::vector<unsigned int> Version::parse( const unsigned int numeric ) {
    
    unsigned int year = numeric / 10000 + 2000;
    
    unsigned int month = (numeric % 10000) / 100;

    unsigned int release = numeric % 100;

    return std::vector<unsigned int>{year, month, release};
}

std::vector<unsigned int> Version::parse( const std::string &version_str ) {
    std::stringstream ss(version_str);
    std::string token;

    // Skip the 'v' at the beginning
    std::getline(ss, token, 'v');

    std::getline(ss, token, '.');
    unsigned int year = std::stoi(token) +2000u;
    std::getline(ss, token, '.');
    unsigned int month = std::stoi(token);
    std::getline(ss, token, '.');
    unsigned int release = std::stoi(token);

    return std::vector<unsigned int>{year, month, release};
}

// Conversions to other formats

constexpr unsigned int Version::numeric( const unsigned int year, const unsigned int month, const unsigned int release ) {
    return (year-2000) * 10000 + month * 100 + release;
}

std::string Version::str( const unsigned int year, const unsigned int month, const unsigned int release ) {
    std::stringstream ss;
    ss << "v" << year-2000 << '.' 
        << std::setw(2) <<std::setfill('0') << month << '.' 
        << release;
    return ss.str();
}
    
} // namespace bevarmejo::detail    

namespace bevarmejo {

VersionManager& VersionManager::user() {
    static VersionManager user_requested_version;
    return user_requested_version;
}

const VersionManager& VersionManager::library() {
    static const VersionManager library_version;
    return library_version;
}

void VersionManager::set( const std::string &version ) {
    version_ = detail::Version(version);
}

const detail::Version& VersionManager::version() const { return version_; }

const detail::Version& VersionManager::operator()() const { return version_; }

detail::Version VersionManager::v() { return detail::Version(); }

VersionManager::VersionManager() : 
    version_{detail::version_year, detail::version_month, detail::version_release} 
{ }

} // namespace bevarmejo
