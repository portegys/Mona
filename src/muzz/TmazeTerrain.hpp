// For conditions of distribution and use, see copyright notice in muzz.hpp

/*
 * T-maze terrain.
 *
 * A T-maze contains a path of adjacent T-shaped cell patterns.
 * TBE  or  EBT
 *  C        C
 *  S        S
 * where S = start cell, C = channel cell, B = branch cell,
 * T = terminal cell, E = end cell.
 * The end cell of one pattern abuts the start cell of the next
 * pattern such that the TBE cells of the first pattern are colinear with
 * the SCB cells of the second pattern. The cells comprising a pattern have
 * a block height of one, and are surrounded by cells of height two except
 * where connected to another pattern.
 * A T-maze contains no ramps or block heights other than one or two.
 */

#ifndef __TMAZE_TERRAIN__
#define __TMAZE_TERRAIN__

#include "blockTerrain.hpp"

// T-maze terrain.
class TmazeTerrain : public BlockTerrain
{
public:

   // Constructor.
   TmazeTerrain(RANDOM randomSeed, int width, int height, GLfloat blockSize);

   // Destructor.
   ~TmazeTerrain();

   // The path provides the coordinates of the start to end cells of
   // the maze in the form of repeated cell sets:
   // (start, channel, branch, terminal, end).
   vector<pair<int, int> > TmazePath;

private:

   // Tries for space-filling maze.
   enum { MAZE_GENERATIONS=10 };

   void generateMaze();
   bool mapMaze(vector<pair<int, int> >& path, Block::DIRECTION);
   bool checkCell(vector<pair<int, int> >& path, int x, int y);
   void getBounds(vector<pair<int, int> >& path,
                  int& xmin, int& ymin, int& xmax, int& ymax);
};
#endif
