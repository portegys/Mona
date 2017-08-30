// For conditions of distribution and use, see copyright notice in minc.hpp

#include "Tmaze.hpp"

// Constructor.
Tmaze::Tmaze()
{
}


// Destructor.
Tmaze::~Tmaze()
{
   path.clear();
}


// Is given maze a duplicate?
bool Tmaze::isDuplicate(Tmaze *maze)
{
   if ((int)path.size() != (int)maze->path.size())
   {
      return(false);
   }
   for (int i = 0; i < (int)path.size(); i++)
   {
      if (path[i].mark != maze->path[i].mark)
      {
         return(false);
      }

      if (path[i].direction != maze->path[i].direction)
      {
         return(false);
      }
   }
   return(true);
}
