#include "rng.h"

#define USE_SPLITMIX

#define LCG_A   1103515245
#define LCG_C   12345

struct random_t {
    uint64_t seed;
    uint64_t x;
};
struct random_t rand_container;

uint64_t splitmix_state;

void rng_srand(uint64_t seed)
{
#ifdef USE_SPLITMIX
    splitmix_state = seed;
#else
    rand_container.seed = seed;
    rand_container.x = seed;
#endif /* ifdef USE_SPLITMIX */
}

uint64_t linear_congruential_generator()
{
    rand_container.x = ((rand_container.x * LCG_A) + LCG_C) & 0x7fffffff;
    return rand_container.x;
}

/*
 * SplitMix64 generator.
 * See: https://prng.di.unimi.it/splitmix.c
 */
uint64_t splitmix_next()
{
    uint64_t z = (splitmix_state += UINT64_C(0x9e3779b97f4a7c15));

    z = (z ^ (z >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94d049bb133111eb);

    return z ^ (z >> 31);
}

int rng_rand()
{
#ifdef USE_SPLITMIX
    return (int) (splitmix_next() & 0x7fffffff);
#else
    return (int) linear_congruential_generator();
#endif /* ifdef USE_SPLITMIX */
}

