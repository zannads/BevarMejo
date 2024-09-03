#pragma once 

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <sstream>
#include <tuple>

namespace bevarmejo {

namespace detail {

constexpr unsigned int version_year = 2024;
constexpr unsigned int version_month = 9;
constexpr unsigned int version_release = 0;

class Version {

private:
    unsigned int m_year;
    unsigned int m_month;
    unsigned int m_release;

public:
    Version() : m_year{version_year}, m_month{version_month}, m_release{version_release} {}
    Version(unsigned int year, unsigned int month, unsigned int release) :
        m_year{year}, m_month{month}, m_release{release}
    {
        if (year < 2023 || year > version_year)
            throw std::invalid_argument("Year must be between 2023 and " + std::to_string(version_year));
        
        if (month < 1 || month > 12 || (year == version_year && month > version_month) || (year == 2023 && month < 6))
            throw std::invalid_argument("Month must be between 1 and 12");

        if (release < 0 || release > 99 || (year == version_year && month == version_month && release > version_release))
            throw std::invalid_argument("Release must be between 0 and 99");
    }

    bool operator<(const Version& other) const {
        return std::tie(m_year, m_month, m_release) < std::tie(other.m_year, other.m_month, other.m_release);
    }

    bool operator==(const Version& other) const {
        return std::tie(m_year, m_month, m_release) == std::tie(other.m_year, other.m_month, other.m_release);
    }

    bool operator>(const Version& other) const {
        return std::tie(m_year, m_month, m_release) > std::tie(other.m_year, other.m_month, other.m_release);
    }

    bool operator<=(const Version& other) const {
        return std::tie(m_year, m_month, m_release) <= std::tie(other.m_year, other.m_month, other.m_release);
    }

    bool operator>=(const Version& other) const {
        return std::tie(m_year, m_month, m_release) >= std::tie(other.m_year, other.m_month, other.m_release);
    }

    bool operator!=(const Version& other) const {
        return std::tie(m_year, m_month, m_release) != std::tie(other.m_year, other.m_month, other.m_release);
    }

    bool operator<(const std::string& other) const {
        return *this < parse(other);
    }

    bool operator==(const std::string& other) const {
        return *this == parse(other);
    }

    bool operator>(const std::string& other) const {
        return *this > parse(other);
    }

    bool operator<=(const std::string& other) const {
        return *this <= parse(other);
    }

    bool operator>=(const std::string& other) const {
        return *this >= parse(other);
    }

    bool operator!=(const std::string& other) const {
        return *this != parse(other);
    }

    static Version parse( const std::string &version_str ) {
        std::stringstream ss(version_str);
        std::string token;

        // Skip the 'v' at the beginning
        std::getline(ss, token, 'v');

        std::getline(ss, token, '.');
        auto year = std::stoi(token) +2000;
        std::getline(ss, token, '.');
        auto month = std::stoi(token);
        std::getline(ss, token, '.');
        auto release = std::stoi(token);

        return Version(year, month, release);
    }

    static constexpr unsigned int numeric( const unsigned int year, const unsigned int month, const unsigned int release ) {
        return (year-2000) * 10000 + month * 100 + release;
    }

    static std::string str( const unsigned int year, const unsigned int month, const unsigned int release ) {
        std::stringstream ss;
        ss << "v" << year-2000 << '.' 
            << std::setw(2) <<std::setfill('0') << month << '.' 
            << release;
        return ss.str();
    }

    std::string str() const {
        return str(m_year, m_month, m_release);
    }

}; // struct Version

constexpr unsigned int version = Version::numeric(version_year, version_month, version_release);
// constexpr const char* version_str = "v24.09.0";
static const std::string version_str = Version::str(version_year, version_month, version_release);

} // namespace detail

class VersionManager {
public:

    static VersionManager& user() {
        static VersionManager user_requested_version;
        return user_requested_version;
    }

    static const VersionManager& library() {
        static const VersionManager library_version;
        return library_version;
    }

    void set( const std::string &version ) {
        version_ = detail::Version::parse(version);
    }

    detail::Version version() const {
        return version_;
    }

    static detail::Version v(const unsigned int year, const unsigned int month, const unsigned int release) {
        return detail::Version{year, month, release};
    }

    static detail::Version v(const std::string &version_str) {
        return detail::Version::parse(version_str);
    }

private:
    detail::Version version_; // Just hold the version, either for the static library or the static user requested version

    VersionManager() : version_{detail::version_year, detail::version_month, detail::version_release} {}

    // Disable copy and move constructor and assignment operator
    VersionManager(const VersionManager&) = delete;
    VersionManager& operator=(const VersionManager&) = delete;
    VersionManager(VersionManager&&) = delete;
    VersionManager& operator=(VersionManager&&) = delete;

}; // class VersionManager

} // namespace bevarmejo
