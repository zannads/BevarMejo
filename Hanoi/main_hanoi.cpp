/*----------------------
Author: DennisZ
descr: Main file for the optimization of the Hanoi problem.
----------------------*/

#include "main_hanoi.h"

using namespace pagmo;

int main(int argc, char* argv[])
{
    // Let's parse the input of the program
    if (argc < 2){
        std::cout << "You need to pass a SettingsFile" <<std::endl;
        return 1;
    }
    // Check existance of the file
    // from XML to structs with the data
    
    //Construct a pagmo::problem for Hanoi model
    problem p{ model_hanoi{} };
    
    // Get a pointer to the internal copy of the UDP from the Pagmo::problem.
    model_hanoi* mh_ptr = p.extract<model_hanoi>();
    
    // Initialize the internal copy of the UDP.
    mh_ptr->upload_settings("FakeFilename.xml");
    
    // example: load seed number from settingsfile
    unsigned int seed = 3u;
    unsigned int nfe = 100u;
    unsigned int report_nfe = 1000u;
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
    
    std::cout <<pop <<std::endl;
    
    // save pop
    mh_ptr->clear();
    
    return 0;
}
