#pragma once

#include <string>
#include <random>
#include <functional>

//class to provide random numbers from a given distribution via the () operator
//initiated with a random number generator and a distribution and distribution parameters


//base class for RandomGenerator (gen specific)
//abstracted class agnostic to


class RandomGenerator{
    protected:
        RandomGenerator()=default;

    public:
    static RandomGenerator* Make(std::string gen_name, std::string dist_type, std::vector<double> dist_params);

    virtual double operator()(){return 0.0;};
};


template<typename Generator, typename Distribution>
class RandomGeneratorT: public RandomGenerator
{

public:
    RandomGeneratorT(std::vector <double> distribution_params )
    {

        dist=Distribution(distribution_params[0], distribution_params[1]);
        rand_gen=Generator();

        rand_gen.seed(std::random_device()());

    }
    //return a random double from the distribution
    double operator()() override
    {
        return dist(rand_gen);
    }

    std::string get_distribution_name() const
    {
        return m_dist_name;
    }

    std::string get_generator_name() const
    {
        return m_gen_name;
    }

    ~RandomGeneratorT()=default;
private:
    std::string m_dist_name;
    std::string m_gen_name;
    Distribution dist;
    Generator rand_gen;

};

RandomGenerator* RandomGenerator::Make(std::string gen_name, std::string dist_type, std::vector<double> dist_params) {
    if (dist_type == "uniform") {

        if (gen_name == "default_random_engine") {
            return new RandomGeneratorT<std::default_random_engine, std::uniform_real_distribution<double>>(
                    dist_params);
        } else if (gen_name == "mt19937") {
            return new RandomGeneratorT<std::mt19937, std::uniform_real_distribution<double>>(dist_params);
        } else if (gen_name == "mt19937_64") {
            return new RandomGeneratorT<std::mt19937_64, std::uniform_real_distribution<double>>(dist_params);
        } else if (gen_name == "minstd_rand") {
            return new RandomGeneratorT<std::minstd_rand, std::uniform_real_distribution<double>>(dist_params);
        } else if (gen_name == "minstd_rand0") {
            return new RandomGeneratorT<std::minstd_rand0, std::uniform_real_distribution<double>>(dist_params);
        } else if (gen_name == "ranlux24_base") {
            return new RandomGeneratorT<std::ranlux24_base, std::uniform_real_distribution<double>>(dist_params);
        } else if (gen_name == "ranlux48_base") {
            return new RandomGeneratorT<std::ranlux48_base, std::uniform_real_distribution<double>>(dist_params);
        } else if (gen_name == "ranlux24") {
            return new RandomGeneratorT<std::ranlux24, std::uniform_real_distribution<double>>(dist_params);
        } else if (gen_name == "ranlux48") {
            return new RandomGeneratorT<std::ranlux48, std::uniform_real_distribution<double>>(dist_params);
        } else if (gen_name == "knuth_b") {
            return new RandomGeneratorT<std::knuth_b, std::uniform_real_distribution<double>>(dist_params);
        } else {
            std::cout << "Random Generator not found" << std::endl;
            return nullptr;
        }
    } else if (dist_type == "normal") {
        if (gen_name == "default_random_engine") {
            return new RandomGeneratorT<std::default_random_engine, std::normal_distribution<double>>(dist_params);
        } else if (gen_name == "mt19937") {
            return new RandomGeneratorT<std::mt19937, std::normal_distribution<double>>(dist_params);
        } else if (gen_name == "mt19937_64") {
            return new RandomGeneratorT<std::mt19937_64, std::normal_distribution<double>>(dist_params);
        } else if (gen_name == "minstd_rand") {
            return new RandomGeneratorT<std::minstd_rand, std::normal_distribution<double>>(dist_params);
        } else if (gen_name == "minstd_rand0") {
            return new RandomGeneratorT<std::minstd_rand0, std::normal_distribution<double>>(dist_params);
        } else if (gen_name == "ranlux24_base") {
            return new RandomGeneratorT<std::ranlux24_base, std::normal_distribution<double>>(dist_params);
        } else if (gen_name == "ranlux48_base") {
            return new RandomGeneratorT<std::ranlux48_base, std::normal_distribution<double>>(dist_params);
        } else if (gen_name == "ranlux24") {
            return new RandomGeneratorT<std::ranlux24, std::normal_distribution<double>>(dist_params);
        } else if (gen_name == "ranlux48") {
            return new RandomGeneratorT<std::ranlux48, std::normal_distribution<double>>(dist_params);
        } else if (gen_name == "knuth_b") {
            return new RandomGeneratorT<std::knuth_b, std::normal_distribution<double>>(dist_params);
        } else {
            std::cout << "Random Generator not found" << std::endl;
            return nullptr;
        }
        //dist=std::normal_distribution<double>(dist_params[0], dist_params[1]);std::normal_distribution<double>>
    } else {
        std::vector<double> distribution_params;
        distribution_params.push_back(0.0);
        distribution_params.push_back(1.0);
        //RandomGenerator<std::default_random
        return new RandomGeneratorT<std::default_random_engine, std::uniform_real_distribution<double>>(
                distribution_params);
    }

}
