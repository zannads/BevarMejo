/*----------------------
Author: DennisZ
descr: Main file for the optimization of the Hanoi problem.
----------------------*/

#include "main_hanoi.h"

using namespace pagmo;
using namespace boost::filesystem;

struct nsga2p{
    int a;
    int b;
    const char* rootDataFolder;
};

int main(int argc, char* argv[])
{
    // Let's parse the input of the program
    if (argc < 2){
        std::cout << "You need to pass a SettingsFile" <<std::endl;
        return 1;
    }
    path settingsFile(argv[1]);
    
    if (!exists(settingsFile) || !is_regular_file(settingsFile)){
        std::cout << "SettingsFile not existing or not a file (e.g., a directory)!" <<std::endl;
        return 1;
    }
    
    //auto progSettings = bevarmejo::from_XML_to_settingsStruct(settingsFile.string(), nsga2p{});
    
    //Construct a pagmo::problem for Hanoi model
    problem p{ model_hanoi{} };
    
    // Get a pointer to the internal copy of the UDP from the Pagmo::problem.
    model_hanoi* mh_ptr = p.extract<model_hanoi>();
    
    // Initialize the internal copy of the UDP.
    mh_ptr->upload_settings(settingsFile.string());
    
    // example: load seed number from settingsfile
    unsigned int seed = 3u;
    unsigned int nfe = 100u;
    unsigned int report_nfe = 100u;
    pop_size_t pop_size = 100u;
    
    // check I don't do mistakes
    if (report_nfe == 0){
        report_nfe = nfe;
    }
    
    // optional stuff retrived from settingsfile but that I will keep constant
    double cr{0.9}, eta_c{15.}, m{1./34.}, eta_m{7.};
    
    // Instantiate Optimization Algorithm
    algorithm algo{ nsga2(report_nfe, cr, eta_c, m, eta_m, seed) };

    // Instantiate population
    population pop{ p, pop_size, seed };
    
    // Evolve
    for(unsigned int i = 0; i*report_nfe<nfe; ++i){
        // Pop
        pop = algo.evolve(pop);
        // Save pop
    }
    
    std::fstream fs;
    
    fs.open("final.txt", std::fstream::in );
    
    std::cout << fs.is_open() <<std::endl;
   
    fs << pop;
    
    fs.close();
    
    // save pop
    mh_ptr->clear();
    
    return 0;
}
