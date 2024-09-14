#pragma once 

#include <stdexcept>
#include <string>
#include <utility>

#include "bevarmejo/io.hpp"

namespace bevarmejo {

namespace log {
static const std::string class_postfix= "::";
static const std::string function_postfix= "()";
static const std::string wide_colon= " : ";

// Common function names shared between many classes.
namespace fname {
static const std::string at= "at";
} // namespace fname

} // namespace log

namespace detail {

// Specialisation for class errors.
inline std::string __format_c(  const std::string& class_name,
                                const std::string& function_name,
                                const std::string& problem_message)
{
    std::ostringstream oss;
    io::stream_out(oss, class_name, log::class_postfix, 
                        function_name, log::function_postfix,
                        log::wide_colon, problem_message, "\n" );
    return oss.str();
}

// Specialisation for class errors.
inline std::string __format_f( const std::string& function_name,
                        const std::string& problem_message)
{
    std::ostringstream oss;
    io::stream_out(oss, function_name, log::function_postfix,
                        log::wide_colon, problem_message, "\n" );
    return oss.str();
}

// For when the user wants to pass extra arguments after the problem message:
template <typename... Args>
inline std::string __format_c(  const std::string& class_name,
                                const std::string& function_name,
                                const std::string& problem_message,
                                Args&&... args)
{
    std::ostringstream oss;
    io::stream_out(oss, __format_c(class_name, function_name, problem_message), 
                        std::forward<Args>(args)...);
    io::stream_out(oss, "\n");
    return oss.str();
}

template <typename... Args>
inline std::string __format_f(  const std::string& function_name,
                                const std::string& problem_message,
                                Args&&... args)
{
    std::ostringstream oss;
    io::stream_out(oss, __format_f(function_name, problem_message), 
                        std::forward<Args>(args)...);
    io::stream_out(oss, "\n");
    return oss.str();
}
    
} // namespace detail

struct ClassError { };
struct FunctionError { };
// Generic function template with a variadic input and switch using the template
template <typename E = std::runtime_error, typename T = ClassError, typename... Args>
[[noreturn]] inline void __format_and_throw(Args&&... args) {
    if constexpr (std::is_same_v<T, ClassError>)
        throw E(detail::__format_c(std::forward<Args>(args)...));
    else if constexpr (std::is_same_v<T, FunctionError>)
        throw E(detail::__format_f(std::forward<Args>(args)...));
    else
        static_assert(!std::is_same_v<T, T>, "Invalid type passed to __format_and_throw. Expected ClassError or FunctionError.");
}

} // namespace bevarmejo
