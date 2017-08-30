// For conditions of distribution and use, see copyright notice in mona.hpp

// Evolve maze-learning mouse.

#ifndef EVOLVE_MOUSE
#define EVOLVE_MOUSE

#ifdef WIN32
#include <windows.h>
#include <io.h>
#else
#include <errno.h>
#endif
#include <signal.h>
#include "../mona/mona.hpp"
#include "../common/log.hpp"

// Version (SCCS "what" format).
#define MOUSE_EVOLVE_VERSION                "@(#)Mouse evolve version 1.0"

// Evolution parameters.
#define FIT_POPULATION_SIZE                 20
#define NUM_MUTANTS                         10
#define NUM_OFFSPRING                       10
#define POPULATION_SIZE                     (FIT_POPULATION_SIZE + NUM_MUTANTS + NUM_OFFSPRING)
#define DEFAULT_NUM_MAZE_TESTS              1
#define DEFAULT_NUM_DOOR_TRAINING_TRIALS    50
#define DEFAULT_NUM_MAZE_TRAINING_TRIALS    20
#define DEFAULT_MUTATION_RATE               0.25
#define SAVE_FREQUENCY                      10
#endif
