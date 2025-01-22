#include <chrono>
#include <filesystem>
namespace fsys = std::filesystem;
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "bevarmejo/io/streams.hpp"
#include "bevarmejo/parsers.hpp"
#include "bevarmejo/simulation.hpp"

// temp inclusion until I figure out how to expose the save_soluton method
#include "Anytown/prob_anytown.hpp"

int main(int argc, char* argv[]) {

    // 1. Parse the inputs, ideally I could change anything and should perform checks.
    // argv[1] the problem settings file (it also implicitly defines the experiment folder unless copy flag is active)
    // argv[2] the decision variables file

    bevarmejo::Simulation simu;
    try {
        simu = bevarmejo::sim::parse(argc, argv);

    } catch (const std::exception& e) {
        bevarmejo::io::stream_out(std::cerr, "An error happend while parsing the CLI inputs:\n", e.what(), "\n" );
        return 1;
    }
    
    std::vector<double> res(simu.p.get_nf());
    try
    {   
        simu.start = std::chrono::high_resolution_clock::now();
        res = simu.p.fitness(simu.dvs);
    }
    catch(const std::exception& e)
    {
        simu.end = std::chrono::high_resolution_clock::now();
        bevarmejo::io::stream_out(std::cerr, "An error happend while evaluating the decision vector:\n", e.what(), "\n" );
        return 1;
    }
    
    simu.end = std::chrono::high_resolution_clock::now();

    bevarmejo::io::stream_out(std::cout, 
        "Element with ID ", simu.id,
        " evaluated with Fitness vector : ", res,
        " in ", std::chrono::duration_cast<std::chrono::milliseconds>(simu.end - simu.start).count(), " ms\n");

    if (!simu.extra_message.empty())
        bevarmejo::io::stream_out(std::cout, simu.extra_message, "\n");

    bool success = true;
    if (!simu.fvs.empty() && simu.fvs.size() == res.size())
    {
       
        for (size_t i = 0; i < simu.fvs.size(); ++i)
        {
            if ( std::abs(simu.fvs[i] - res[i]) > 1e-8 )
            {
                bevarmejo::io::stream_out(std::cerr, "Mismatch between the fitness vector provided and the one returned by the problem simulation.\n",
                    std::setprecision(16),
                    "Index: ", i, "\n",
                    "Expected: ", simu.fvs[i], "\n",
                    "Returned: ", res[i], "\n");

                success = false;
            }
        }
    }

    // If I pass the --saveinp flag than I should save the inp file
    if (simu.save_inp)
    {
        bevarmejo::io::stream_out(std::cout, "Thanks for using BeMe-Sim, saving the inp file...\n");
        try
        {
            if ( simu.p.is<bevarmejo::anytown::Problem>() )
            {
                simu.p.extract<bevarmejo::anytown::Problem>()->save_solution(simu.dvs, std::to_string(simu.id) + ".inp");
            }
            else 
            {
                bevarmejo::io::stream_out(std::cerr, "The problem is not supported for saving the inp file.\n");
                success = false;
            }
        } catch (const std::exception& e)
        {
            bevarmejo::io::stream_out(std::cerr, "An error happend while saving the inp file:\n", e.what(), "\n" );
            success = false;
        }
    }

    return success ? 0 : 1;
}
