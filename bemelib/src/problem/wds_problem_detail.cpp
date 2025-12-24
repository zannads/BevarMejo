#include <cctype>
#include <cstddef>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

#include "wds_problem.hpp"

namespace bevarmejo::io {

detail::ProblemName split_problem_name(std::string_view problem_name) {
    // A valid problem name is suite::problem::formulation or suite::problem, since
    // name and problem are namespaces, while formulations are (enum) classes, they
    // can be called only with names according to the C++ rules to declare them:
    // i.e., only "_" letters and numbers are allowed but not starting with a number.
    
    // 1. Check if the problem name is valid (i.e., only contains letters, numbers and "_" and the "::" separator)
    
    // 2. Check that there are enough "::" separators and they are correctly placed
    // (i.e., not at the beginning or at the end, exactly one or two)
    
    // 3. Split the problem name into suite, problem and formulation

    // ======== Actual Implementation ========
    // Instead of looping 3 times, do it once.
    std::vector<std::size_t> starts(3, 0ul);
    std::vector<std::size_t> durations(3, 0ul);
    std::size_t separator_counter = 0ul;
    std::size_t valid_chars_counter = 0ul;
    for (auto it = problem_name.begin(); it != problem_name.end(); ++it) {
        auto c = *it;
        if (!std::isalnum(c) && c != '_' && c != ':') {
            // TODO: throw an exception
        }
        if ((it == problem_name.begin() && c == ':') || (it == (problem_name.end()-1) && c == ':')) {
            // TODO: throw an exception
        }
        if (c == ':') {
            // Here I know I am not at the beginning or at the end, so I can safely check the previous and next character
            // I found a separator when the previous character is a ":" and the next one is not
            // This means that I can actually have suite::::problem or anythign with at least two colons
            if (*(it-1) == ':' && *(it+1) != ':') {
                // Found
                
                durations[separator_counter++] = valid_chars_counter;
                if (separator_counter == 2) { // We already found all of our separators, you can put more but they will be ignored 
                    starts[separator_counter] = std::distance(problem_name.begin(), it + 1);
                    durations[separator_counter] = std::distance(it + 1, problem_name.end());
                    break;
                }
                else { // Reset the valid_chars_counter and prepare for the next one
                    valid_chars_counter = 0;
                    starts[separator_counter] = std::distance(problem_name.begin(), it + 1);
                }
            }
        }
        else {
            // Not at a colon, thus increment
            ++valid_chars_counter;
        }
    }
    
    return detail::ProblemName{std::string_view(problem_name).substr(starts[0], durations[0]),
                               std::string_view(problem_name).substr(starts[1], durations[1]),
                               std::string_view(problem_name).substr(starts[2], durations[2])};
}

} // namespace bevarmejo::io