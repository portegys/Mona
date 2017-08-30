// For conditions of distribution and use, see copyright notice in muzz.hpp

// T-maze terrain implementation.

#include "TmazeTerrain.hpp"

// Constructor.
TmazeTerrain::TmazeTerrain(RANDOM randomSeed, int width,
                           int height, GLfloat blockSize)
   : BlockTerrain(randomSeed, width, height, 1, 1, 1, 0, 0, blockSize)
{
   generateMaze();
}


// Destructor.
TmazeTerrain::~TmazeTerrain()
{
   TmazePath.clear();
}


// Generate T-maze terrain.
void TmazeTerrain::generateMaze()
{
   int i, j, xmin, xmax, ymin, ymax, xd, yd;

   vector<pair<int, int> > path;
   pair<int, int>          s(0, 0);

   // Sanity check.
   if ((WIDTH < 5) || (HEIGHT < 5))
   {
      fprintf(stderr, "Terrain dimensions too small for T-maze\n");
      exit(1);
   }

   // Create a long random maze path.
   TmazePath.clear();
   for (i = 0; i < MAZE_GENERATIONS; i++)
   {
      path.clear();
      path.push_back(s);
      mapMaze(path, Block::NORTH);
      if (path.size() > TmazePath.size())
      {
         TmazePath.clear();
         TmazePath = path;
      }
   }

   // Center path in terrain.
   getBounds(TmazePath, xmin, ymin, xmax, ymax);
   xd = 1 - xmin;
   yd = 1 - ymin;
   for (i = 0; i < (int)TmazePath.size(); i++)
   {
      TmazePath[i].first  += xd;
      TmazePath[i].second += yd;
   }

   // Build the terrain.
   for (i = 0; i < WIDTH; i++)
   {
      for (j = 0; j < HEIGHT; j++)
      {
         blocks[i][j].elevation = 1;
      }
   }
   for (i = 0; i < (int)TmazePath.size(); i++)
   {
      blocks[TmazePath[i].first][TmazePath[i].second].elevation = 0;
   }
   build();

   // Gray the floor.
   for (i = 0; i < WIDTH; i++)
   {
      for (j = 0; j < HEIGHT; j++)
      {
         if (blocks[i][j].elevation == 0)
         {
            blocks[i][j].textureIndexes[0] = NUM_BLOCK_TEXTURES - 1;
         }
      }
   }
}


// Map a maze.
bool TmazeTerrain::mapMaze(vector<pair<int, int> >& path,
                           Block::DIRECTION direction)
{
   int              s, c, b, t, e, x, y, x1, y1, x2, y2, xc, yc, xb, yb;
   Block::DIRECTION nextdir;

   pair<int, int> cell;

   // Check cell placements.
   s = (int)path.size() - 1;
   c = s + 1;
   b = c + 1;
   t = b + 1;
   e = t + 1;
   x = path[s].first;
   y = path[s].second;
   switch (direction)
   {
   case Block::NORTH:
      x1 = x - 1;
      y1 = y + 2;
      if (!checkCell(path, x1, y1))
      {
         return(false);
      }
      xc = x;
      yc = y + 1;
      if (!checkCell(path, xc, yc))
      {
         return(false);
      }
      xb = x;
      yb = y + 2;
      if (!checkCell(path, xb, yb))
      {
         return(false);
      }
      x2 = x + 1;
      y2 = y + 2;
      if (!checkCell(path, x2, y2))
      {
         return(false);
      }
      break;

   case Block::EAST:
      x1 = x + 2;
      y1 = y + 1;
      if (!checkCell(path, x1, y1))
      {
         return(false);
      }
      xc = x + 1;
      yc = y;
      if (!checkCell(path, xc, yc))
      {
         return(false);
      }
      xb = x + 2;
      yb = y;
      if (!checkCell(path, xb, yb))
      {
         return(false);
      }
      x2 = x + 2;
      y2 = y - 1;
      if (!checkCell(path, x2, y2))
      {
         return(false);
      }
      break;

   case Block::SOUTH:
      x1 = x - 1;
      y1 = y - 2;
      if (!checkCell(path, x1, y1))
      {
         return(false);
      }
      xc = x;
      yc = y - 1;
      if (!checkCell(path, xc, yc))
      {
         return(false);
      }
      xb = x;
      yb = y - 2;
      if (!checkCell(path, xb, yb))
      {
         return(false);
      }
      x2 = x + 1;
      y2 = y - 2;
      if (!checkCell(path, x2, y2))
      {
         return(false);
      }
      break;

   case Block::WEST:
      x1 = x - 2;
      y1 = y + 1;
      if (!checkCell(path, x1, y1))
      {
         return(false);
      }
      xc = x - 1;
      yc = y;
      if (!checkCell(path, xc, yc))
      {
         return(false);
      }
      xb = x - 2;
      yb = y;
      if (!checkCell(path, xb, yb))
      {
         return(false);
      }
      x2 = x - 2;
      y2 = y - 1;
      if (!checkCell(path, x2, y2))
      {
         return(false);
      }
      break;
   }
   cell.first  = xc;
   cell.second = yc;
   path.push_back(cell);
   cell.first  = xb;
   cell.second = yb;
   path.push_back(cell);
   cell.first  = 0;
   cell.second = 0;
   path.push_back(cell);
   path.push_back(cell);

   // Choose next direction.
   switch (direction)
   {
   case Block::NORTH:
   case Block::SOUTH:
      if (randomizer->RAND_BOOL())
      {
         nextdir        = Block::EAST;
         path[t].first  = x1;
         path[t].second = y1;
         path[e].first  = x2;
         path[e].second = y2;
      }
      else
      {
         nextdir        = Block::WEST;
         path[t].first  = x2;
         path[t].second = y2;
         path[e].first  = x1;
         path[e].second = y1;
      }
      break;

   case Block::EAST:
   case Block::WEST:
      if (randomizer->RAND_BOOL())
      {
         nextdir        = Block::SOUTH;
         path[t].first  = x1;
         path[t].second = y1;
         path[e].first  = x2;
         path[e].second = y2;
      }
      else
      {
         nextdir        = Block::NORTH;
         path[t].first  = x2;
         path[t].second = y2;
         path[e].first  = x1;
         path[e].second = y1;
      }
      break;
   }
   if (mapMaze(path, nextdir))
   {
      return(true);
   }

   // Try reverse direction.
   switch (nextdir)
   {
   case Block::NORTH:
      nextdir = Block::SOUTH;
      break;

   case Block::EAST:
      nextdir = Block::WEST;
      break;

   case Block::SOUTH:
      nextdir = Block::NORTH;
      break;

   case Block::WEST:
      nextdir = Block::EAST;
      break;
   }
   cell    = path[e];
   path[e] = path[t];
   path[t] = cell;
   mapMaze(path, nextdir);
   return(true);
}


// Is cell placement OK?
bool TmazeTerrain::checkCell(vector<pair<int, int> >& path,
                             int x, int y)
{
   int i, j, x2, y2, xmin, xmax, ymin, ymax;

   // Check maze bounds.
   getBounds(path, xmin, ymin, xmax, ymax);
   if ((x < xmin) && (WIDTH - (xmax - x) < 3))
   {
      return(false);
   }
   if ((x > xmax) && (WIDTH - (x - xmin) < 3))
   {
      return(false);
   }
   if ((y < ymin) && (HEIGHT - (ymax - y) < 3))
   {
      return(false);
   }
   if ((y > ymax) && (HEIGHT - (y - ymin) < 3))
   {
      return(false);
   }

   // Check adjacent cells.
   x2 = x - 1;
   y2 = y;
   for (i = 0, j = (int)path.size() - 1; i < j; i++)
   {
      if ((path[i].first == x2) && (path[i].second == y2))
      {
         return(false);
      }
   }
   x2 = x + 1;
   y2 = y;
   for (i = 0, j = (int)path.size() - 1; i < j; i++)
   {
      if ((path[i].first == x2) && (path[i].second == y2))
      {
         return(false);
      }
   }
   x2 = x;
   y2 = y - 1;
   for (i = 0, j = (int)path.size() - 1; i < j; i++)
   {
      if ((path[i].first == x2) && (path[i].second == y2))
      {
         return(false);
      }
   }
   x2 = x;
   y2 = y + 1;
   for (i = 0, j = (int)path.size() - 1; i < j; i++)
   {
      if ((path[i].first == x2) && (path[i].second == y2))
      {
         return(false);
      }
   }
   return(true);
}


// Get path bounds.
void TmazeTerrain::getBounds(vector<pair<int, int> >& path,
                             int& xmin, int& ymin, int& xmax, int& ymax)
{
   assert(path.size() > 0);
   for (int i = 0; i < (int)path.size(); i++)
   {
      if (i == 0)
      {
         xmin = xmax = path[i].first;
         ymin = ymax = path[i].second;
      }
      else
      {
         if (xmin > path[i].first)
         {
            xmin = path[i].first;
         }
         if (xmax < path[i].first)
         {
            xmax = path[i].first;
         }
         if (ymin > path[i].second)
         {
            ymin = path[i].second;
         }
         if (ymax < path[i].second)
         {
            ymax = path[i].second;
         }
      }
   }
}
