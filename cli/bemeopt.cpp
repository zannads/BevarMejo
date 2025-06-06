#include <iostream>

#include "bevarmejo/utility/io.hpp"
#include "bevarmejo/experiment.hpp"

int main(int argc, char* argv[])
{
    // 1. Parse the inputs, ideally I could change anything and should perform checks.
    // argv[1] the settings file (it also implicitly defines the experiment folder unless copy flag is active)
    bevarmejo::Experiment experiment;
    try {
        experiment = bevarmejo::Experiment::parse(argc, argv);
    }
    catch (const std::exception& e) {
        bevarmejo::io::stream_out(std::cerr, "An error happend while parsing the CLI inputs:\n", e.what(), "\n" );
        return 1;
    }

    try {
        experiment.pre_run_tasks();
    }
    catch (const std::exception& e) {
        bevarmejo::io::stream_out(std::cerr, "An error happend while doing the tasks in preparation of the experiment run:\n", e.what(), "\n" );
        return 2;
    }

    try {
        experiment.run();
    }
    catch (const std::exception& e) {
        bevarmejo::io::stream_out(std::cerr, "An error happend while running the experiment:\n", e.what(), "\n" );
        return 3;
    }

    try {
        experiment.post_run_tasks();
    }
    catch( const std::exception& e) {
        bevarmejo::io::stream_out(std::cerr, "An error happend while doing the tasks after the experiment run:\n", e.what(), "\n" );
        return 4;
    }
    
    return 0;
}
