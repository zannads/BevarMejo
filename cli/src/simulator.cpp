#include <cmath>
#include <iostream>
#include <filesystem>
namespace fsys = std::filesystem;
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <pagmo/problem.hpp>

#include "bevarmejo/io/fsys.hpp"
#include "bevarmejo/io/json.hpp"
#include "bevarmejo/io/keys/beme.hpp"
#include "bevarmejo/io/keys/bemeexp.hpp"
#include "bevarmejo/io/keys/bemeopt.hpp"
#include "bevarmejo/io/keys/bemesim.hpp"

#include "bevarmejo/io/streams.hpp"

#include "bevarmejo/utility/exceptions.hpp"
#include "bevarmejo/utility/string.hpp"
#include "bevarmejo/utility/metadata.hpp"
#include "bevarmejo/utility/pagmo/serializers/json/containers.hpp"

#include "simulator.hpp"

namespace bevarmejo {

using Paths = std::vector<fsys::path>;

namespace io {
namespace log {
namespace nname {
static const std::string opt = "optimisation::"; // "optimisation::"
static const std::string sim = "simulation::"; // "simulation::"
}
namespace fname {
static const std::string parse = "parse"; // "parse"
}
namespace mex {
static const std::string parse_error = "Error parsing the settings file."; // "Error parsing the settings file."

static const std::string nearg = "Not enough arguments."; // "Not enough arguments."
static const std::string usage_start = "Usage: "; // "Usage: "
static const std::string usage_end = " <settings_file> [flags]"; // " <settings_file> [flags]"
}
} // namespace log

namespace other {
static const std::string settings_file = "Settings file : "; // "Settings file : "
}
} // namespace io

Simulator::Simulator(const fsys::path& settings_file) :
    m__settings_file(settings_file),
    m__root_folder(settings_file.parent_path()),
    m__lookup_paths({settings_file.parent_path(), fsys::current_path()}),
    m__dvs({}),
    m__p(),
    m__fvs({}),
    m__id(0ull),
    m__extra_message(""),
    m__version(version_str),
    m__res({}),
    m__start_time(),
    m__end_time()
{
    // Check the extension, and based on that open the file, parse it based on
    // the file structure (JSON, YAML, XML, etc). 
    // Apply the key value pairs passed from command line
    // Once you have the final object call the build function.
    
    std::ifstream file(settings_file);
    beme_throw_if(!file.is_open(), std::runtime_error,
        "Failed to create the simulator.",
        "Failed to open the settings file.",
        io::other::settings_file + settings_file.string());

    if (file.peek() == std::ifstream::traits_type::eof())
    {
        file.close();
        beme_throw(std::runtime_error,
            "Failed to create the simulator.",
            "The settings file is empty.",
            io::other::settings_file + settings_file.string());
    }

    std::string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    Json jinput;
    if (settings_file.extension() == io::other::ext__json) 
    {   
        try
        {
            jinput = Json::parse(file_content);
        }
        catch (const std::exception& e)
        {
            beme_throw(std::runtime_error,
                "Failed to create the simulator.",
                "Failed to parse the settings file as JSON.",
                e.what(),
                io::other::settings_file + settings_file.string());
        }
    }
    else
    {
        beme_throw(std::runtime_error,
            "Impossible to create the simulator.",
            "The file format of the settings file is not supported.",
            "Format: ", settings_file.extension(),
            io::other::settings_file + settings_file.string());
    }

    try
    {
        // 1.3.1 Optional keys that may change the behavior of the simulation
        auto paths = jinput.value(io::key::lookup_paths.as_in(jinput), Json{});
        
        if (paths != nullptr && paths.is_string())
        {
            Json jpath = Json::array();
            jpath.push_back(paths.get<std::string>());
            paths = std::move(jpath);
        }

        for (const auto& path : paths)
        {
			auto p = path.get<fsys::path>();

            if (fsys::exists(p) && fsys::is_directory(p))
            {
                m__lookup_paths.push_back(p);
            }
            else
            {
                std::cerr << "Path in the simulation settings file is not a valid directory: " << p.string() << std::endl;
            }
        }

        if(io::key::beme_version.exists_in(jinput))
        {
            auto user_v_str = jinput.at(io::key::beme_version.as_in(jinput)).get<std::string>();
            if (!is_valid_version(user_v_str))
            {
                io::stream_out(std::cerr,
                    "The requested version is not valid. To use this version, recompile the library with the -DPROJECT_VERSION=YY.MM.PP flag.\n",
                    "Requested version: ", user_v_str, "\n",
                    "Valid versions for this executable: [", min_version_str, ", ", version_str, "].\n");
                std::abort();
            }
        }

        // 1.3.2 mandatory keys first: dv, udp
        check_mandatory_field(io::key::dv, jinput);
        check_mandatory_field(io::key::problem, jinput);

        m__dvs = jinput.at(io::key::dv.as_in(jinput)).get<std::vector<double>>();;

        // 1.5 build the problem
        Json jproblem = jinput.at(io::key::problem.as_in(jinput));
        jproblem[io::key::lookup_paths()] = m__lookup_paths;
        m__p = jproblem.get<pagmo::problem>();

        // 1.6 optional keys that don't change the behavior of the simulation
        m__fvs = jinput.value(io::key::fv.as_in(jinput), std::vector<double>{});

        m__id = jinput.value(io::key::id.as_in(jinput), 0ull);

        m__extra_message = jinput.value(io::key::print.as_in(jinput), std::string{});
    }
    catch (const std::exception& e)
    {
        beme_throw(std::runtime_error,
            "Failed to parse the simulation settings file.",
            e.what(),
            "File: ", m__settings_file.string());
    }
}

/*----------------------------------------------------------------------------*/
/*-------------------------------- Tasks -------------------------------------*/
/*----------------------------------------------------------------------------*/
void print_hello_msg(Simulator & simr);

void print_results_msg(Simulator & simr);

void check_correctness(Simulator & simr);

void save_results(Simulator & simr);

void save_inp(Simulator & simr);

/*----------------------------------------------------------------------------*/
/*--------------------------- Member functions -------------------------------*/
/*----------------------------------------------------------------------------*/

// Element access
const std::vector<double>& Simulator::decision_variables() const
{
    return m__dvs;
}

const pagmo::problem& Simulator::problem() const
{
    return m__p;
}

const std::vector<double>& Simulator::expected_fitness_vector() const
{
    return m__fvs;
}

unsigned long long Simulator::id() const
{
    return m__id;
}

const std::string& Simulator::extra_message() const
{
    return m__extra_message;
}

const std::vector<double>& Simulator::resulting_fitness_vector() const
{
    return m__res;
}

const std::chrono::high_resolution_clock::time_point& Simulator::start_time() const
{
    return m__start_time;
}

const std::chrono::high_resolution_clock::time_point& Simulator::end_time() const
{
    return m__end_time;
}

Simulator Simulator::parse(int argc, char *argv[])
{
    beme_throw_if(argc < 2, std::invalid_argument,
        "Error parsing the command line arguments.",
        "Not enough arguments.",
        "Usage: beme-sim <settings_file> [flags]");

    // Add the cwd to the lookup path for the settings file as it may be a rel path
    std::vector<fsys::path> lookup_paths;
    lookup_paths.push_back(fsys::current_path());

    auto settings_file = bevarmejo::io::locate_file(fsys::path{argv[1]}, lookup_paths);

    auto simulator = Simulator(settings_file);

    // Add default tasks
    simulator.m__pre_run_tasks.emplace_back("Print hello message", print_hello_msg, "");

    simulator.m__post_run_tasks.emplace_back("Print results message", print_results_msg, "");
    
    if (!simulator.expected_fitness_vector().empty())
    {
        simulator.m__post_run_tasks.emplace_back("Check correctness", check_correctness, "");
    }

    // 3. Check the flags
    // 3.1 save inp file
    for (int i = 2; i < argc; ++i)
    {
        auto arg = std::string(argv[i]);
        if (arg == "--saveinp")
        {
            simulator.m__post_run_tasks.emplace_back("Save inp file", save_inp, "");
        }
        else if (arg == "--savefv")
        {
            simulator.m__post_run_tasks.emplace_back("Save results", save_results, "");
        }
    }

    return std::move(simulator);
}

void Simulator::pre_run_tasks()
{
    for (const auto& task : m__pre_run_tasks)
    {
        try
        {
            std::get<1>(task)(*this);
        }
        catch(const std::exception& e)
        {
            bevarmejo::io::stream_out(std::cerr,
                "An error happend while executing a pre-run task:\n",
                "Task: ", std::get<0>(task), "\n",
                e.what(), "\n");
        }
    }
}

void Simulator::run()
{
    try
    {   
        m__start_time = std::chrono::high_resolution_clock::now();
        m__res = m__p.fitness(m__dvs);
    }
    catch(const std::exception& e)
    {
        m__end_time = std::chrono::high_resolution_clock::now();
        bevarmejo::io::stream_out(std::cerr, "An error happend while evaluating the decision vector:\n", e.what(), "\n" );
        return;
    }
    
    m__end_time = std::chrono::high_resolution_clock::now();
}

void Simulator::post_run_tasks()
{
    for (const auto& task : m__post_run_tasks)
    {
        try
        {
            std::get<1>(task)(*this);
        }
        catch(const std::exception& e)
        {
            bevarmejo::io::stream_out(std::cerr,
                "An error happend while executing a post-run task:\n",
                "Task: ", std::get<0>(task), "\n",
                e.what(), "\n");
        }
    }
}

/*----------------------------------------------------------------------------*/
/*-------------------------------- Tasks -------------------------------------*/
/*----------------------------------------------------------------------------*/
void print_hello_msg(Simulator & simr)
{
    bevarmejo::io::stream_out(std::cout,
        "\nThanks for using BeMe-Sim!\n\n");
}

void print_results_msg(Simulator & simr)
{
    if ( simr.id() != 0 )
    {
        bevarmejo::io::stream_out(std::cout,
            "Element with ID: ", simr.id(), " evaluated.\n");
    }
    else
    {
        bevarmejo::io::stream_out(std::cout,
            "Unnamed element evaluated.\n");
    }

    bevarmejo::io::stream_out(std::cout,
        "\tFitness vector: ", simr.resulting_fitness_vector(), "\n",
        "\tElapsed time: ", std::chrono::duration_cast<std::chrono::milliseconds>(simr.end_time() - simr.start_time()).count(), " ms\n");

    if (!simr.extra_message().empty())
    {
        bevarmejo::io::stream_out(std::cout,
            "\n\t", simr.extra_message(), "\n");
    }
}

void check_correctness(Simulator & simr)
{
    assert(!simr.expected_fitness_vector().empty()); // This function should be added and used only if a fitness vector is provided.

    // lambda to pretty print the fitness vectors.
    // Expected output:
    // Expected   |  Result
    // 1.2345     |  1.2345
    // 1.2345     |        

    auto pretty_print_header = []() -> std::string
    {
        std::ostringstream msg;
        std::string expected_header = "Expected";
        std::string result_header = "Result";
        size_t expected_padding = (17 - expected_header.length()) / 2;
        size_t result_padding = (17 - result_header.length()) / 2;

        msg << std::setw(expected_padding + expected_header.length()) << std::right << expected_header;
        msg << std::setw(17 - expected_header.length() - expected_padding) << " ";
        msg << " | ";
        msg << std::setw(result_padding + result_header.length()) << std::right << result_header;
        msg << std::setw(17 - result_header.length() - result_padding) << " ";
        msg << "\n";
        return msg.str();
    };

    auto pretty_print_fvs = [pretty_print_header](const std::vector<double>& expected, const std::vector<double>& result) -> std::string
    {
        size_t max_size = std::max(expected.size(), result.size());
    
        std::ostringstream msg;
        bevarmejo::io::stream_out(msg, pretty_print_header());
        for (size_t i = 0; i < max_size; ++i) {
            msg << std::fixed << std::setprecision(16);
            if (i < expected.size()) {
                msg << std::setw(17) << expected[i];
            } else {
                msg << std::setw(17) << " "; // 16 digits + space
            }
            msg << " | ";
            if (i < result.size()) {
                msg << std::setw(17) << result[i];
            } else {
                msg << std::setw(17) << " "; // 16 digits + space
            }
            msg << "\n";
        }
        return msg.str();
    };

    // Check that the sizes match, otherwise it means that the results are not for the same problem.
    beme_throw_if(simr.expected_fitness_vector().size() != simr.resulting_fitness_vector().size(), std::runtime_error,
        "Simulations results don't match the expected fitness vector.",
        "The size of the fitness vector provided and the one returned by the simulation are different.",
        "Expected size: ", simr.expected_fitness_vector().size(),
        "Returned size: ", simr.resulting_fitness_vector().size(),
        "\nContent:\n", pretty_print_fvs(simr.expected_fitness_vector(), simr.resulting_fitness_vector())
    );
    
    double tolerance = 1e-7;
    std::vector<std::size_t> mismatch_status(simr.expected_fitness_vector().size(), 0);
    for (std::size_t i = 0; i < simr.expected_fitness_vector().size(); ++i)
    {
        // Return 0, 1, or 2 to represent the extent of the mismatch.
        // 0: no mismatch
        // 1: weak mismatch
        // 2: strong mismatch
        auto get_status_code = [tolerance](double expected, double result) -> std::size_t
        {
            if (expected == result)
                return 0;
            
            if (expected == 0.0)
                return (std::abs(result) < tolerance) ? 1 : 2;

            return std::abs(expected - result) < std::abs(expected) * tolerance ? 1 : 2;
        };

        mismatch_status[i] = get_status_code(simr.expected_fitness_vector()[i], simr.resulting_fitness_vector()[i]);
    }

    // If there is at least a non zero value in the mismatch_status vector, it means that there are differences and we will print out something.
    // The error is thrown at the end of the function only if there is a strong mismatch (condition 2).
    if ( std::any_of(mismatch_status.begin(), mismatch_status.end(), [](std::size_t s) { return s==1 || s==2;} ) )
    {
        bool need_throw = false;
        // Prepare a cool message to show the differences.
        // Style:
        // Expected   |  Result
        // * 1.2345   |  1.2456
        //   1.2345   |  1.2345
        std::ostringstream msg;
        bevarmejo::io::stream_out(msg, "  ", pretty_print_header());

        for (std::size_t i = 0; i < simr.expected_fitness_vector().size(); ++i)
        {
            msg << std::fixed << std::setprecision(16);
            if (mismatch_status[i] == 0)
            {
                msg << "  ";
            }
            else if (mismatch_status[i] == 1)
            {
                msg << "~ ";
            }
            else
            {
                msg << "* ";
                need_throw = true;
            }

            msg << std::setw(17) << simr.expected_fitness_vector()[i] << " | " << std::setw(17) << simr.resulting_fitness_vector()[i] << "\n";
        }

        beme_throw_if(need_throw, std::runtime_error,
            "Simulations results don't match the expected fitness vector.",
            "One or more fitness values are different.",
            "Content:\n", msg.str());
    }
}

void save_results(Simulator &simr)
{
    Json jres = simr.resulting_fitness_vector();
    std::ofstream res_file(std::to_string(simr.id())+".fv.json");

    beme_throw_if(!res_file.is_open(), std::runtime_error,
        "Failed to open the result file for writing.",
        "The file could not be opened.",
        "File: ", std::to_string(simr.id()) + ".fv.json");

    res_file << jres.dump();
    res_file.close();

    bevarmejo::io::stream_out(std::cout,
        "Results saved in: ", (fsys::current_path()/fsys::path(std::to_string(simr.id()) + ".fv.json\n")).string());
}

void save_inp(Simulator &simr)
{
    if (simr.problem().is<bevarmejo::anytown::Problem>())
    {
        simr.problem().extract<bevarmejo::anytown::Problem>()->save_solution(simr.decision_variables(), std::to_string(simr.id()) + ".inp");
    }
    else if (simr.problem().is<bevarmejo::anytown_systol25::Problem>())
    {
        simr.problem().extract<bevarmejo::anytown_systol25::Problem>()->save_solution(simr.decision_variables(), std::to_string(simr.id()) + ".inp");
    }
    else
    {
        beme_throw_if( !simr.problem().is<bevarmejo::anytown::Problem>(), std::runtime_error,
        "Impossible to save the inp file.",
        "The problem is not of the type anytown::Problem.",
        "Problem type: ", simr.problem().get_name());

    }
    
    bevarmejo::io::stream_out(std::cout,
        "EPANET '.inp' file saved in: ", (fsys::current_path()/fsys::path(std::to_string(simr.id()) + ".inp\n")).string());
}

} // namespace bevarmejo
