
#include <string>
#include <random>

//class to provide random numbers from a given distribution via the () operator
//initiated with a random number generator and a distribution and distribution parameters
template <typename generator>
template <typename distribution>


class RandomGenerator
{
public:
    RandomGenerator(std::string gen_name, std::string distribution_name, std::vector<double> distribution_params );
    operator() const;
    std::string get_distribution_name() const;
    std::string get_generator_name() const;
    
    ~RandomGenerator();
private:
    std::string m_dist_name;
    std::string m_gen_name;
    distribution dist;
    generator rand_gen;
    
};
