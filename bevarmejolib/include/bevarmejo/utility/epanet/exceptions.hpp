#pragma once

#include <stdexcept>

#include "epanet2_2.h"

namespace bevarmejo::epanet
{
class EN_error : public std::runtime_error
{
public:
    template <typename Str>
    explicit EN_error(int errorcode, Str&& pre_what_msg) :
        std::runtime_error(std::string(std::forward<Str>(pre_what_msg))+EN_message(errorcode)),
        m__errorcode(errorcode)
    {
        assert(errorcode > 100);
    }

    int code() const noexcept
    {
        return m__errorcode;
    }

private:
    int m__errorcode;

    static std::string EN_message(int errorcode)
    {
        char msg[EN_MAXMSG+1 +7]; // +7 for the "EPANET " prefix
        std::strcpy(msg, "EPANET ");
        EN_geterror(errorcode, msg+7, EN_MAXMSG+1);
        std::cout << msg << std::endl;
        return std::string(msg);
    }

}; // class EN_error

} // namespace bevarmejo::epanet
