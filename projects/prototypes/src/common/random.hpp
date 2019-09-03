// Random numbers.

#ifndef __RANDOM__
#define __RANDOM__

#include <time.h>

// Random number generators.
#ifdef UNIX
#define SRAND(seed) srand48(seed)
#define RAND lrand48()
#else
#ifdef NEVER
#define SRAND(seed) Random::srand(seed)
#define RAND Random::rand()
#endif
#define SRAND(seed) srand(seed)
#define RAND rand()
#endif

// Random probability >= 0.0 && <= 1.0
#ifdef UNIX
#define RAND_PROB drand48()
#else
#define RAND_PROB ((double)(RAND % 101) / 100.0)
#endif

// Random choice of 0 to n-1.
#define RAND_CHOICE(n) (RAND % n)

// Random boolean.
#define RAND_BOOL ((RAND % 2) == 0 ? false : true)

// A random number generator based on a linear-congruential method.
class Random
{
  public:

    // return random number and update seed:
    static unsigned long rand()
    {
      seed = (seed * A + C) % M;
      return seed;
    }

    // choose a new seed:
    static void srand(unsigned long Iseed) { seed = Iseed; }

  private:
    static const unsigned long A, C, M;
    static unsigned long seed;
};
#endif


