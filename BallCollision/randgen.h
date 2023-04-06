#pragma once
#include <iostream>
#include <random>
#include "constants.h"

class RandGen
{
public:
    // get copy of random generator, each copy produces same series
    static std::mt19937 get()  
    {
        static std::mt19937 rng = init_rand_gen();
        return rng;
    }

private:
    static std::mt19937 init_rand_gen() 
    {
        std::random_device dev;
        uint32_t seed = dev();

#ifdef FIXED_RANDOM_SEED
        seed = FIXED_RANDOM_SEED;
#endif
        
        std::cout << "Random seed: " << seed << std::endl;
        std::mt19937 rng(seed);

        return rng;
    }
};



