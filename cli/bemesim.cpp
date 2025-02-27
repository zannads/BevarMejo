#include <iostream>

#include "bevarmejo/utility/io.hpp"
#include "bevarmejo/simulator.hpp"

int main(int argc, char* argv[])
{
    // 1. Parse the inputs, ideally I could change anything and should perform checks.
    // argv[1] the problem settings file (it also implicitly defines the experiment folder unless copy flag is active)
    // argv[2] the decision variables file

    bevarmejo::Simulator simulator;
    try {
        simulator = bevarmejo::Simulator::parse(argc, argv);

    } catch (const std::exception& e) {
        bevarmejo::io::stream_out(std::cerr, "An error happend while parsing the CLI inputs:\n", e.what(), "\n" );
        return 1;
    }

    try {
        simulator.pre_run_tasks();
    }
    catch (const std::exception& e) {
        bevarmejo::io::stream_out(std::cerr, "An error happend while doing the tasks in preparation of the simulation run:\n", e.what(), "\n" );
        return 2;
    }

    try {
        simulator.run();
    }
    catch (const std::exception& e) {
        bevarmejo::io::stream_out(std::cerr, "An error happend while running the simulation:\n", e.what(), "\n" );
        return 3;
    }

    try {
        simulator.post_run_tasks();
    }
    catch( const std::exception& e) {
        bevarmejo::io::stream_out(std::cerr, "An error happend while doing the tasks after the simulation run:\n", e.what(), "\n" );
        return 4;
    }

    return 0;
}
