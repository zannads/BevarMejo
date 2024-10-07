#ifndef BEVARMEJOLIB__CLI_PARSER_HPP
#define BEVARMEJOLIB__CLI_PARSER_HPP

#include <iostream>

namespace bevarmejo {

// Forward declarations
struct ExperimentSettings;
struct Simulation;

namespace opt {
ExperimentSettings parse(int argc, char* argv[]);

} // namespace opt

namespace sim {
Simulation parse(int argc, char* argv[]);
} // namespace sim

} // namespace bevarmejo

#endif // BEVARMEJOLIB__CLI_PARSER_HPP
