// For conditions of distribution and use, see copyright notice in muzz.hpp

// Evolve muzzes.

#ifndef EVOLVE_MUZZES
#define EVOLVE_MUZZES

#ifdef WIN32
#include <windows.h>
#include <io.h>
#else
#include <errno.h>
#endif
#include <signal.h>
#include "muzzWorld.hpp"
#include "../common/log.hpp"

// Version (SCCS "what" format).
#define MUZZ_EVOLVE_VERSION            "@(#)Muzz evolve version 1.1"

// Evolution parameters.
#define FIT_POPULATION_SIZE            20
#define NUM_MUTANTS                    10
#define NUM_OFFSPRING                  10
#define POPULATION_SIZE                (FIT_POPULATION_SIZE + NUM_MUTANTS + NUM_OFFSPRING)
#define DEFAULT_MIN_TRAINING_TRIALS    5
#define DEFAULT_MAX_TRAINING_TRIALS    10
#define DEFAULT_MUTATION_RATE          0.25
#define DEFAULT_MUZZ_CYCLES            500
#define SAVE_FREQUENCY                 10
#endif
