#pragma once 

#include <cassert>
#include <exception>
// #include <cerrno>
// #include <system_error>
// #include <stack_trace>
#include <stdexcept>

#include <string>
#include <sstream>

#include "bevarmejo/io/streams.hpp"

#include "bevarmejo/utility/epanet/exceptions.hpp"

#define assertm(exp, msg) assert(((void)msg, exp))

namespace bevarmejo::detail
{

template <typename Exception>
struct ExceptionThrower
{
    using line_type = std::decay_t<decltype(__LINE__)>;

    explicit ExceptionThrower(const char *file, line_type line, const char *func)
        : m__file(file), m__line(line), m__func(func)
    { }

    template <typename StrWhat, typename StrWhy = std::string, typename... Args>
    [[noreturn]] void operator()(StrWhat &&what, StrWhy &&why, Args&&... args) const
    {
        std::ostringstream oss;
        io::stream_out(oss,
            "function: ", m__func,
            "\n  where: ", m__file, ", ", m__line,
            "\n  what: ", std::forward<StrWhat>(what));

        auto why_str = std::string(std::forward<StrWhy>(why));
        if (!why_str.empty())
            io::stream_out(oss, "\n  why: ", why_str);

        if constexpr (sizeof...(Args) > 0)
            io::stream_out(oss, "\n  ", std::forward<Args>(args)...);

        io::stream_out(oss, "\n");

        throw Exception(oss.str());
    }

    // For the EPANET exceptions.
    template <typename StrWhat, typename StrWhy = std::string, typename... Args>
    [[noreturn]] void operator()(int errorcode, StrWhat &&what, StrWhy &&why, Args&&... args) const
    {
        std::ostringstream oss;
        io::stream_out(oss,
            "function: ", m__func,
            "\n  where: ", m__file, ", ", m__line,
            "\n  what: ", std::forward<StrWhat>(what));

        auto why_str = std::string(std::forward<StrWhy>(why));
        if (!why_str.empty())
            io::stream_out(oss, "\n  why: ", why_str);

        if constexpr (sizeof...(Args) > 0)
            io::stream_out(oss, "\n    ", std::forward<Args>(args)...);

        io::stream_out(oss, "\n    "); // 4 additional spaces to create more indentation on the EPANET error message.

        throw Exception(errorcode, oss.str());
    }

    const char *m__file;
    const line_type m__line;
    const char *m__func;
}; // struct ExceptionThrower

} // namespace bevarmejo::detail

/// Exception-throwing macros for the bevarmejo library.

// Throw an exception of type Exception with the message msg.
// It needs to pass a message, a reason and eventually some arguments for the reason.

#define beme_throw(exception_t, ...) \
    bevarmejo::detail::ExceptionThrower<exception_t>(__FILE__, __LINE__, __func__)(__VA_ARGS__);

#define beme_throw_if(condition, exception_t, ...) \
    if (condition) { beme_throw(exception_t, __VA_ARGS__) }

#define beme_throw_if_EN_error(errorcode, ...) \
    beme_throw_if(errorcode > 100, bevarmejo::epanet::EN_error, errorcode, __VA_ARGS__)

#define beme_throw_if_EN_warning(errorcode, ...) \
    beme_throw_if(errorcode > 0, bevarmejo::epanet::EN_error, errorcode, __VA_ARGS__)