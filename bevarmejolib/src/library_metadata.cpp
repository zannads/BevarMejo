#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "bevarmejo/utility/bemexcept.hpp"
#include "bevarmejo/io/streams.hpp"

#include "library_metadata.hpp"

namespace bevarmejo::detail {

constexpr unsigned int version_year = 2024;
constexpr unsigned int version_month = 12;
constexpr unsigned int version_release = 0;

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
    beme_throw_if(il.size() != 3, std::invalid_argument,
        "Failed to create a Version object.",
        "The initializer list must have 3 elements: year, month, release.",
        "The initializer list has ", il.size(), " elements."
        );

    const unsigned int year = *(il.begin());
    const unsigned int month = *(il.begin()+1);
    const unsigned int release = *(il.begin()+2);

    beme_throw_if(year < 2000 || year > version_year, std::invalid_argument,
        "Failed to create a Version object.",
        "Invalid year number (must be between 2000 and ", version_year, ").",
        "Year = ", year
        );

    beme_throw_if(month < 1 || month > 12, std::invalid_argument,
        "Failed to create a Version object.",
        "Invalid month number (must be between 1 and 12).",
        "Month = ", month
        );

    beme_throw_if(release > 99, std::invalid_argument,
        "Failed to create a Version object.",
        "Invalid release number (must be between 0 and 99).",
        "Release = ", release
        );

    beme_throw_if(
        year == version_year && month > version_month ||
        year == version_year && month == version_month && release > version_release,
        std::invalid_argument,
        "Failed to create a Version object.",
        "The version is greater than the current version.",
        "Current version = ", Version().str(),
        "Year = ", year, ", Month = ", month, ", Release = ", release
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

std::vector<unsigned int> Version::parse( const std::string &version_str )
{
    beme_throw_if(version_str.empty() || *version_str.begin() != 'v', std::invalid_argument,
        "Could not parse the version string.",
        "The version string is empty or does not start with 'v'.",
        "Version string = ", version_str
        );

    std::istringstream iss(std::string(version_str.begin()+1, version_str.end()));

    std::vector<unsigned int> version_numbers(3, 0);
    auto get_next = [&iss, &version_numbers](std::size_t i) {
        std::string token;
        std::getline(iss, token, '.');
        
        try
        {
            version_numbers[i] = std::stoi(token);
        }
        catch (const std::invalid_argument& e)
        {
            beme_throw(std::invalid_argument,
                "Could not parse the version string.",
                "The version string has an invalid element.",
                "Element = ", token,
                "Error = ", e.what()
                );
        }
    };
    
    get_next(0);
    version_numbers[0] += 2000;
    get_next(1);
    get_next(2);
    
    return std::move(version_numbers);
}

// Conversions to other formats

constexpr unsigned int Version::numeric( const unsigned int year, const unsigned int month, const unsigned int release ) {
    return (year-2000) * 10000 + month * 100 + release;
}

std::string Version::str( const unsigned int year, const unsigned int month, const unsigned int release ) {
    std::ostringstream ss;
    bevarmejo::io::stream_out(ss, "v", year-2000, '.');
    ss << std::setw(2) <<std::setfill('0');
    bevarmejo::io::stream_out(ss, month, '.', release);
    return ss.str();
}
    
} // namespace bevarmejo::detail    

namespace bevarmejo {

VersionManager::VersionManager() : 
    version_{detail::version_year, detail::version_month, detail::version_release} 
{ }

VersionManager& VersionManager::user() {
    static VersionManager user_requested_version;
    return user_requested_version;
}

const VersionManager& VersionManager::library() {
    static const VersionManager library_version;
    return library_version;
}

const detail::Version& VersionManager::version() const { return version_; }

const detail::Version& VersionManager::operator()() const { return version_; }

detail::Version VersionManager::v() { return detail::Version(); }

detail::Version VersionManager::first_v() { return detail::Version(2023, 6, 0); }

} // namespace bevarmejo
