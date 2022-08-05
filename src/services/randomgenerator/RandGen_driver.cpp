
#include <iostream>
#include <vector>
#include "randomGenerator.h"

int main()
{
    std::vector<double> distribution_params;
    distribution_params.push_back(1.0);
    distribution_params.push_back(2.0);
    //RandomGenerator<std::default_random_engine, std::uniform_real_distribution<double>> rand_gen(distribution_params);

    RandomGenerator* rand_gen=RandomGenerator::Make("default_random_engine", "normal", distribution_params);

    for(int i = 0; i < 10; i++)
    {
        std::cout << (*rand_gen)() << std::endl;
    }
    return 0;
}