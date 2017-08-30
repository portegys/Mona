// For conditions of distribution and use, see copyright notice in minc.hpp

// T-maze.

#ifndef __TMAZE__
#define __TMAZE__

#include "../common/common.h"

class Tmaze
{
public:

   // Special maze symbols.
   enum { START=0, GOAL=1, LEFT=0, RIGHT=1 };

   // Junction.
   class Junction
   {
public:
      int    mark;
      int    direction;
      double probability;
   };

   // Maze path.
   vector<Junction> path;

   // Constructor.
   Tmaze();

   // Destructor.
   ~Tmaze();

   // Is given maze a duplicate?
   bool isDuplicate(Tmaze *maze);
};
#endif
