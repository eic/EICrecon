#include "randomGenerator.h"

// Virtual destructor implementation to pin vtable and typeinfo to this
// translation unit
~RandomGenerator::RandomGenerator() {};


RandomGenerator::RandomGenerator(std::string gen_name,std::string distribution_name, std::vector <double> distribution_params )
{
    m_dist_name = distribution_name;
    m_gen_name = gen_name;

    if (m_dist_name == "uniform")
    {
        if(distribution_params.size() != 2)
        {
            std::cout << "Error: uniform distribution requires 2 parameters" << std::endl;
            exit(1);
        }
        dist = std::uniform_real_distribution<double>(distribution_params[0], distribution_params[1]);
    }
    else if (m_dist_name == "normal")
    {
        if(distribution_params.size() != 2)
        {
            std::cout << "Error: normal distribution requires 2 parameters" << std::endl;
            exit(1);
        }
        dist = std::normal_distribution<double>(distribution_params[0], distribution_params[1]);
    }
    else
    {
        throw std::runtime_error("Unknown distribution");
    }

    switch (m_gen_name)
    {
        case "default_random_engine":
            rand_gen = std::default_random_engine();
            break;
        case "minstd_rand":
            rand_gen = std::minstd_rand();
            break;
        case "minstd_rand0":
            rand_gen = std::minstd_rand0();
            break;
        case "mt19937":
            rand_gen = std::mt19937();
            break;
        case "mt19937_64":
            rand_gen = std::mt19937_64();
            break;
        case "ranlux24_base":
            rand_gen = std::ranlux24_base();
            break;
        case "ranlux48_base":
            rand_gen = std::ranlux48_base();
            break;
        case "ranlux24":
            rand_gen = std::ranlux24();
            break;
        case "ranlux48":
            rand_gen = std::ranlux48();
            break;
        case "ranlux24_base":
            rand_gen = std::ranlux24_base();
            break;
        default:
            throw std::runtime_error("Unknown generator");
    }

    RandomGenerator::operator()(double rand) const
    {
        return dist(rand_gen);
    }

    RandomGenerator::get_distribution_name() const
    {
        return m_dist_name;
    }

    RandomGenerator::get_generator_name() const
    {
        return m_gen_name;
    }


}
