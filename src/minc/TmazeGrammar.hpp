/*
 * T-maze grammar.
 *
 * A T-maze grammar is a stochastic context-sensitive Lindenmayer-like
 * grammar that generates a T-maze, which is a directed binary maze.
 *
 * A T-maze grammar can be specified by:
 * 1. A random number seed.
 * 2. A set of constants for maze markings, which includes a start
 *    and a goal.
 * 3. Two constants for left and right directions.
 * 4. Production rules of the form:
 *    C(p)->md
 *    where C is a context, p is a probability, m is a marking constant,
 *    and d is a direction constant. A context consists of all the
 *    symbols on the path to the next produced symbol. The random number
 *    generator uses these values to produce the relative probability
 *    of the production.
 *
 * A string produced by the grammar represents a maze consisting of a
 * sequence of marked locations and directions, terminating at a
 * location marked with the goal.
 */

#ifndef __TMAZE_GRAMMAR__
#define __TMAZE_GRAMMAR__

#include "../common/common.h"
#include "Tmaze.hpp"

using namespace std;

// T-maze grammar.
class TmazeGrammar
{
public:

   // Constructor.
   TmazeGrammar(RANDOM randomSeed, int numMarks);

   // Destructor.
   ~TmazeGrammar();

   // Generate a maze from the grammar.
   Tmaze *generateMaze();

private:

   RANDOM              randomSeed;
   int                 numMarks;
   Random              *mazeRandomizer;
   static const double NEAR_ZERO;
   int extendMazePath(Random *, Tmaze *);
};
#endif
