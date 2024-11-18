#pragma once

#include <memory>
#include <string>

#include "bevarmejo/wds/water_distribution_system.hpp"

namespace bevarmejo::wds {

namespace ApplyToElements {

template <typename T>
class Subset final {

/*--- Attributes ---*/
private:
    std::string m__subset_name;

/*--- Construction ---*/
public:
    Subset() = default;
    Subset(const Subset&) = default;
    Subset(Subset&&) noexcept = default;
    ~Subset() = default;

    Subset& operator=(const Subset&) = default;
    Subset& operator=(Subset&&) noexcept = default;

/*--- Access ---*/
public:
    const std::string& name() const noexcept { return m__subset_name; }

    void name(const std::string& name) { m__subset_name = name; }

/*--- Operations ---*/
public:
    static auto get(const WDS &a_wds) -> std::shared_ptr<UserDefinedElementsGroup<T>>
    {
        return a_wds.typename subset_ptr<T>(m__subset_name);
    }

};

}

} // namespace bevarmejo::wds