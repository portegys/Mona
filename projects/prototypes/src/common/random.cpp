 // A random number generator based on a linear-congruential method.

#include "random.hpp"

// The seed.
unsigned long Random::seed = time(NULL);

// Initialize the constants:
const unsigned long Random::A = 9301;
const unsigned long Random::C = 49297;
const unsigned long Random::M = 233280;



