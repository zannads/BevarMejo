#pragma once 

#include <iostream>
#include <string>
#include <sstream>
#include <tuple>

namespace bevarmejo {

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

class VersionManager {
public:

    static VersionManager& instance() {
        static VersionManager instance;
        return instance;
    }

    void set( const std::string &version ) {
        version_ = Version::parse(version);
    }

    Version version() {
        return version_;
    }

private:
    Version version_; // User requested version

    VersionManager() : version_{version_year, version_month, version_release} {}

    // Disable copy and move constructor and assignment operator
    VersionManager(const VersionManager&) = delete;
    VersionManager& operator=(const VersionManager&) = delete;
    VersionManager(VersionManager&&) = delete;
    VersionManager& operator=(VersionManager&&) = delete;

}; // class VersionManager

} // namespace bevarmejo
