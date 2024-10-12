#pragma once 

#include <string>
#include <utility>
#include <vector>

#include "bevarmejo/bemexcept.hpp"

namespace bevarmejo {

class VersionManager;

namespace detail {

class Version {

private:
    unsigned int m_year;
    unsigned int m_month;
    unsigned int m_release;

    friend class bevarmejo::VersionManager; // Allow only the VersionManager to access the private constructor.

private:
    Version();
    Version(unsigned int year, unsigned int month, unsigned int release);
    Version(std::vector<unsigned int> il);

    Version(unsigned int numeric);
    Version(const std::string &version_str);

public:
    Version(const Version&) = default;
    Version& operator=(const Version&) = default;
    Version(Version&&) = default;
    Version& operator=(Version&&) = default;

    ~Version() = default;

    unsigned int year() const;
    unsigned int month() const;
    unsigned int release() const;

    unsigned int numeric() const;
    std::string str() const;

    bool operator<(const Version& other) const;
    bool operator==(const Version& other) const;
    bool operator>(const Version& other) const;
    bool operator<=(const Version& other) const;
    bool operator>=(const Version& other) const;
    bool operator!=(const Version& other) const;

    bool operator<(const std::string& other) const;
    bool operator==(const std::string& other) const;
    bool operator>(const std::string& other) const;
    bool operator<=(const std::string& other) const;
    bool operator>=(const std::string& other) const;
    bool operator!=(const std::string& other) const;

    bool operator<(const unsigned int other) const;
    bool operator==(const unsigned int other) const;
    bool operator>(const unsigned int other) const;
    bool operator<=(const unsigned int other) const;
    bool operator>=(const unsigned int other) const;
    bool operator!=(const unsigned int other) const;

private:
    static std::vector<unsigned int> parse( const unsigned int numeric );
    static std::vector<unsigned int> parse( const std::string &version_str );

    static constexpr unsigned int numeric( const unsigned int year, const unsigned int month, const unsigned int release );
    static std::string str( const unsigned int year, const unsigned int month, const unsigned int release );

}; // class Version

} // namespace detail

class VersionManager {

private:
    detail::Version version_; // Just hold the version, either for the static library or the static user requested version

    VersionManager();

    // Disable copy and move constructor and assignment operator
    VersionManager(const VersionManager&) = delete;
    VersionManager& operator=(const VersionManager&) = delete;
    VersionManager(VersionManager&&) = delete;
    VersionManager& operator=(VersionManager&&) = delete;
public:
    ~VersionManager() = default;

public:

    static VersionManager& user();

    static const VersionManager& library();

    template <typename... Args>
    static void set_user_v(Args... args) {
        user().set(std::forward<Args>(args)...);
    }

    const detail::Version& version() const;
    const detail::Version& operator()() const;

// You should not be able to create a version object actually. // I will leave it for testing purposes.
    static detail::Version v();

    template <typename... Args>
    static detail::Version v(Args... args) {
        return detail::Version(std::forward<Args>(args)...);
    }

private:
    static detail::Version first_v();

    template <typename... Args>
    void set(Args... args) {
        auto v = detail::Version(std::forward<Args>(args)...);

        if (v < first_v())
            __format_and_throw<std::invalid_argument, bevarmejo::ClassError>(
                "VersionManager", "set", "Impossible to set the requested version.",
                "The requested version is before the first version."
                "Version = " + v.str()
            );
    
        else
            version_ = v;

    }

}; // class VersionManager

} // namespace bevarmejo
