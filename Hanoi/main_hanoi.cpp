/*----------------------
Author: DennisZ
descr: Main file for the optimization of the Hanoi problem.
----------------------*/

#include "main_hanoi.h"

using namespace pagmo;

int main()
{   

    model_hanoi mH = model_hanoi();
    std::vector<double> prova {4.};
    std::cout <<"Fitness: " <<mH.fitness(prova)[0] <<std::endl;
    
    try {
        //Construct a pagmo::problem for Hanoi model 
        //problem p{ problem_NET1{} };
        //unsigned int seed = 3;

        // Instantiate Optimization Algorithm 
        //algorithm algo{ nsga2(1000) };
        //algo.set_seed(seed);

        // Instantiate population 
        //population pop{ p, 24, seed };
    }
    catch( const std::exception& e ){
        std::cout <<"Error setting up the problem." <<std::endl;
        std::cout << e.what() <<std::endl;
        std::cout <<"ABORT" <<std::endl;

        return 0;
    }

    try {
        // Evolve 
        //pop = algo.evolve(pop);
        
    }
    catch (const std::exception& e)
    {
        std::cout <<"Error during optimization." <<std::endl;
        std::cout << e.what() <<std::endl;
        std::cout <<"ABORT" <<std::endl;

        return 0;
    }
    
    return 0;
}
