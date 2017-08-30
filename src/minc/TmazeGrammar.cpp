#include "TmazeGrammar.hpp"

const double TmazeGrammar::NEAR_ZERO = 0.0001;

// Constructor.
TmazeGrammar::TmazeGrammar(RANDOM randomSeed, int numMarks)
{
   assert(numMarks > 1);

   this->randomSeed = randomSeed;
   this->numMarks   = numMarks;
   mazeRandomizer   = new Random(randomSeed);
   assert(mazeRandomizer != NULL);
}


// Destructor.
TmazeGrammar::~TmazeGrammar()
{
   if (mazeRandomizer != NULL)
   {
      delete mazeRandomizer;
   }
}


// Generate maze from grammar.
// Output: vector of location mark/direction pairs.
Tmaze *TmazeGrammar::generateMaze()
{
   Tmaze  *maze;
   Random *randomizer;

   maze = new Tmaze();
   assert(maze != NULL);

   randomizer = new Random(randomSeed);
   assert(randomizer != NULL);

   while (extendMazePath(randomizer, maze) != Tmaze::GOAL)
   {
   }
   return(maze);
}


// Extend maze path with mark and door constant pair.
// Return mark constant.
int TmazeGrammar::extendMazePath(Random *randomizer, Tmaze *maze)
{
   Tmaze::Junction junction;

   // Select mark.
   if (maze->path.size() == 0)
   {
      junction.mark = Tmaze::START;
   }
   else
   {
      junction.mark = randomizer->RAND_CHOICE(numMarks - 1) + 1;
   }

   // Select direction.
   if (junction.mark == Tmaze::GOAL)
   {
      junction.direction   = Tmaze::LEFT;
      junction.probability = 1.0;
   }
   else
   {
      junction.probability = randomizer->RAND_PROB();
      if (mazeRandomizer->RAND_PROB() <= junction.probability)
      {
         junction.direction = Tmaze::LEFT;
      }
      else
      {
         junction.direction = Tmaze::RIGHT;
      }
   }
   maze->path.push_back(junction);

   // Incorporate direction choice into context.
   randomizer->SRAND(randomizer->RAND() ^ (RANDOM)(junction.direction + 1));
   return(junction.mark);
}
