#pragma once 

#include <iostream>
#include <string>
#include <sstream>
#include <tuple>

namespace bevarmejo {

namespace detail {
constexpr const char* version_str = "v24.08.0";

constexpr unsigned int version_year = 2024;
constexpr unsigned int version_month = 8;
constexpr unsigned int version_release = 0;

struct Version {
    unsigned int year;
    unsigned int month;
    unsigned int release;

    bool operator<(const Version& other) const {
        return std::tie(year, month, release) < std::tie(other.year, other.month, other.release);
    }

    bool operator==(const Version& other) const {
        return std::tie(year, month, release) == std::tie(other.year, other.month, other.release);
    }

    bool operator>(const Version& other) const {
        return std::tie(year, month, release) > std::tie(other.year, other.month, other.release);
    }

    bool operator<=(const Version& other) const {
        return std::tie(year, month, release) <= std::tie(other.year, other.month, other.release);
    }

    bool operator>=(const Version& other) const {
        return std::tie(year, month, release) >= std::tie(other.year, other.month, other.release);
    }

    bool operator!=(const Version& other) const {
        return std::tie(year, month, release) != std::tie(other.year, other.month, other.release);
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
        Version version;

        // Skip the 'v' at the beginning
        std::getline(ss, token, 'v');

        std::getline(ss, token, '.');
        version.year = std::stoi(token) +2000;
        std::getline(ss, token, '.');
        version.month = std::stoi(token);
        std::getline(ss, token, '.');
        version.release = std::stoi(token);

        return version;
    }

    static constexpr unsigned int numeric( const unsigned int year, const unsigned int month, const unsigned int release ) {
        return (year-2000) * 10000 + month * 100 + release;
    }
}; // struct Version

constexpr unsigned int version = Version::numeric(version_year, version_month, version_release);

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
