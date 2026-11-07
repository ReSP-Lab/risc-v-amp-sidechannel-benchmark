#ifndef __RNG_H
#define __RNG_H

#include <stdint.h>

void rng_srand(uint64_t seed);

int rng_rand();

#endif // !__RNG_H
