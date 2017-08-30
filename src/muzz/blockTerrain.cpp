// For conditions of distribution and use, see copyright notice in muzz.hpp

// Block terrain implementation.

#include "blockTerrain.hpp"

// Graphical block size.
const GLfloat BlockTerrain::DEFAULT_BLOCK_SIZE = 0.05f;

// Block identifier dispenser.
int BlockTerrain::Block::idDispenser = 0;

// Initialize terrain.
void BlockTerrain::init(RANDOM randomSeed, int width, int height,
                        int maxElevation, int minPlatformSize, int maxPlatformSize,
                        int maxPlatformGenerations, int extraRamps, GLfloat blockSize)
{
   assert(width > 0 && height > 0);
   assert(minPlatformSize > 0 && maxPlatformSize >= minPlatformSize);
   assert(minPlatformSize <= width && minPlatformSize <= height);
   assert(maxPlatformSize <= width && maxPlatformSize <= height);
   assert(extraRamps >= 0);
   this->RANDOM_SEED              = randomSeed;
   this->WIDTH                    = width;
   this->HEIGHT                   = height;
   this->MAX_PLATFORM_ELEVATION   = maxElevation;
   this->MIN_PLATFORM_SIZE        = minPlatformSize;
   this->MAX_PLATFORM_SIZE        = maxPlatformSize;
   this->MAX_PLATFORM_GENERATIONS = maxPlatformGenerations;
   this->EXTRA_RAMPS              = extraRamps;
   this->BLOCK_SIZE               = blockSize;
   randomizer = new Random(this->RANDOM_SEED);
   assert(randomizer != NULL);
   displaysCreated = false;
   heightmap       = NULL;
   texturesLoaded  = false;

   // Generate the terrain.
   generate();
}


// Generate terrain.
void BlockTerrain::generate()
{
   int i, j, k;

   vector<struct ConnectablePlatforms> connectablePlatforms;
   vector<struct ConnectableBlocks>    connectableBlocks;

   // Create blocks.
   blocks = new Block *[WIDTH];
   assert(blocks != NULL);
   for (i = 0; i < WIDTH; i++)
   {
      blocks[i] = new Block[HEIGHT];
      assert(blocks[i] != NULL);
   }
   markPlatforms();
   saveBlocks = new Block *[WIDTH];
   assert(saveBlocks != NULL);
   for (i = 0; i < WIDTH; i++)
   {
      saveBlocks[i] = new Block[HEIGHT];
      assert(saveBlocks[i] != NULL);
   }

   // Create and connect block platforms.
   for (k = 0; k < MAX_PLATFORM_GENERATIONS; k++)
   {
      // create a platform.
      createPlatform();

      // Try to connect platforms with ramps.
      if (!connectPlatforms())
      {
         for (i = 0; i < WIDTH; i++)
         {
            for (j = 0; j < HEIGHT; j++)
            {
               blocks[i][j] = saveBlocks[i][j];
            }
         }
      }
   }

   // Add extra ramps.
   for (k = 0; k < EXTRA_RAMPS; k++)
   {
      // Get connectable platforms.
      getConnectablePlatforms(connectablePlatforms, true);

      // All possible connections made?
      if (connectablePlatforms.size() == 0)
      {
         break;
      }

      // Select a pair of platforms to build a ramp between.
      i = randomizer->RAND_CHOICE((int)connectablePlatforms.size());
      getConnectableBlocks(connectablePlatforms[i].platforms[0],
                           connectablePlatforms[i].platforms[1], connectableBlocks);

      // Select blocks and build ramp.
      assert(connectableBlocks.size() > 0);
      i = randomizer->RAND_CHOICE((int)connectableBlocks.size());
      connectBlocks(connectableBlocks[i].x[0], connectableBlocks[i].y[0],
                    connectableBlocks[i].x[1], connectableBlocks[i].y[1]);

      // Re-mark platforms since new ramp may have partitioned them.
      markPlatforms();
   }

   // Build the terrain.
   build();
}


// Destructor.
BlockTerrain::~BlockTerrain()
{
   for (int i = 0; i < WIDTH; i++)
   {
      delete [] blocks[i];
      delete [] saveBlocks[i];
   }
   delete [] blocks;
   delete [] saveBlocks;
   delete randomizer;
   delete heightmap;
   glDeleteTextures(NUM_BLOCK_TEXTURES, blockTextures);
   glDeleteLists(blockDisplay, 4);
}


// Create and mark a platform.
void BlockTerrain::createPlatform()
{
   int i, j, x, y, w, h, e;

   // Determine platform dimensions and position.
   w = randomizer->RAND_CHOICE(MAX_PLATFORM_SIZE - MIN_PLATFORM_SIZE + 1) + MIN_PLATFORM_SIZE;
   h = randomizer->RAND_CHOICE(MAX_PLATFORM_SIZE - MIN_PLATFORM_SIZE + 1) + MIN_PLATFORM_SIZE;
   x = randomizer->RAND_CHOICE(WIDTH - w + 1);
   y = randomizer->RAND_CHOICE(HEIGHT - h + 1);
   e = randomizer->RAND_CHOICE(MAX_PLATFORM_ELEVATION + 1);

   for (i = 0; i < WIDTH; i++)
   {
      for (j = 0; j < HEIGHT; j++)
      {
         saveBlocks[i][j]  = blocks[i][j];
         blocks[i][j].type = Block::PLATFORM;
      }
   }
   for (i = x; i < (x + w); i++)
   {
      for (j = y; j < (y + h); j++)
      {
         blocks[i][j].elevation = e;
      }
   }

   // Mark platforms.
   markPlatforms();
}


// Mark platforms.
void BlockTerrain::markPlatforms()
{
   int i, j, mark;

   for (i = 0; i < WIDTH; i++)
   {
      for (j = 0; j < HEIGHT; j++)
      {
         blocks[i][j].platform = -1;
      }
   }

   // Mark all platforms.
   mark = 0;
   for (i = 0; i < WIDTH; i++)
   {
      for (j = 0; j < HEIGHT; j++)
      {
         if ((blocks[i][j].platform == -1) &&
             ((blocks[i][j].type == Block::PLATFORM) || (blocks[i][j].type == Block::LANDING)))
         {
            // Mark continguous blocks on platform.
            markPlatform(mark, i, j);
            mark++;
         }
      }
   }
}


// Recursively mark a platform.
void BlockTerrain::markPlatform(int mark, int x, int y)
{
   int lx, ly, crx, cry, clx, cly;

   blocks[x][y].platform = mark;
   if (blocks[x][y].type == Block::RAMP)
   {
      return;
   }
   if (x > 0)
   {
      if ((blocks[x - 1][y].elevation == blocks[x][y].elevation) &&
          (blocks[x - 1][y].platform == -1))
      {
         if (blocks[x][y].type == Block::PLATFORM)
         {
            if (blocks[x - 1][y].type != Block::RAMP)
            {
               markPlatform(mark, x - 1, y);
            }
         }
         else                                     // landing.
         {
            if (blocks[x - 1][y].type == Block::RAMP)
            {
               getRampInfo(x - 1, y, lx, ly, crx, cry, clx, cly);
               if ((lx == x) && (ly == y))
               {
                  markPlatform(mark, x - 1, y);
               }
            }
            else
            {
               markPlatform(mark, x - 1, y);
            }
         }
      }
   }
   if (y > 0)
   {
      if ((blocks[x][y - 1].elevation == blocks[x][y].elevation) &&
          (blocks[x][y - 1].platform == -1))
      {
         if (blocks[x][y].type == Block::PLATFORM)
         {
            if (blocks[x][y - 1].type != Block::RAMP)
            {
               markPlatform(mark, x, y - 1);
            }
         }
         else                                     // landing.
         {
            if (blocks[x][y - 1].type == Block::RAMP)
            {
               getRampInfo(x, y - 1, lx, ly, crx, cry, clx, cly);
               if ((lx == x) && (ly == y))
               {
                  markPlatform(mark, x, y - 1);
               }
            }
            else
            {
               markPlatform(mark, x, y - 1);
            }
         }
      }
   }
   if (x < WIDTH - 1)
   {
      if ((blocks[x + 1][y].elevation == blocks[x][y].elevation) &&
          (blocks[x + 1][y].platform == -1))
      {
         if (blocks[x][y].type == Block::PLATFORM)
         {
            if (blocks[x + 1][y].type != Block::RAMP)
            {
               markPlatform(mark, x + 1, y);
            }
         }
         else                                     // landing.
         {
            if (blocks[x + 1][y].type == Block::RAMP)
            {
               getRampInfo(x + 1, y, lx, ly, crx, cry, clx, cly);
               if ((lx == x) && (ly == y))
               {
                  markPlatform(mark, x + 1, y);
               }
            }
            else
            {
               markPlatform(mark, x + 1, y);
            }
         }
      }
   }
   if (y < HEIGHT - 1)
   {
      if ((blocks[x][y + 1].elevation == blocks[x][y].elevation) &&
          (blocks[x][y + 1].platform == -1))
      {
         if (blocks[x][y].type == Block::PLATFORM)
         {
            if (blocks[x][y + 1].type != Block::RAMP)
            {
               markPlatform(mark, x, y + 1);
            }
         }
         else                                     // landing.
         {
            if (blocks[x][y + 1].type == Block::RAMP)
            {
               getRampInfo(x, y + 1, lx, ly, crx, cry, clx, cly);
               if ((lx == x) && (ly == y))
               {
                  markPlatform(mark, x, y + 1);
               }
            }
            else
            {
               markPlatform(mark, x, y + 1);
            }
         }
      }
   }
}


// Connect platforms with ramps into a fully connected terrain.
// Return true if full connection achieved.
bool BlockTerrain::connectPlatforms()
{
   int  i, j, mark;
   bool connected;

   vector<struct ConnectablePlatforms> connectablePlatforms;
   vector<struct ConnectableBlocks>    connectableBlocks;

   // Connect all platforms with ramps.
   while (true)
   {
      // Get connectable platforms.
      getConnectablePlatforms(connectablePlatforms);

      // All possible connections made?
      if (connectablePlatforms.size() == 0)
      {
         // Check for a single group that signifies connected terrain.
         markGroups();
         mark = blocks[0][0].group;
         for (i = 0; i < WIDTH; i++)
         {
            for (j = 0; j < HEIGHT; j++)
            {
               if (blocks[i][j].group != mark)
               {
                  return(false);
               }
            }
         }
         return(true);
      }

      // Select a pair of platforms to build a ramp between.
      i = randomizer->RAND_CHOICE((int)connectablePlatforms.size());
      getConnectableBlocks(connectablePlatforms[i].platforms[0],
                           connectablePlatforms[i].platforms[1], connectableBlocks);

      // Select blocks and build ramp.
      assert(connectableBlocks.size() > 0);
      i = randomizer->RAND_CHOICE((int)connectableBlocks.size());
      connectBlocks(connectableBlocks[i].x[0], connectableBlocks[i].y[0],
                    connectableBlocks[i].x[1], connectableBlocks[i].y[1]);

      // Re-mark platforms since new ramp may have partitioned them.
      markPlatforms();

      // Check for a single group that signifies connected terrain.
      markGroups();
      mark      = blocks[0][0].group;
      connected = true;
      for (i = 0; i < WIDTH && connected; i++)
      {
         for (j = 0; j < HEIGHT && connected; j++)
         {
            if (blocks[i][j].group != mark)
            {
               connected = false;
            }
         }
      }
      if (connected)
      {
         return(true);
      }
   }
}


// Mark groups.
// Connected platforms belong to the same group.
void BlockTerrain::markGroups()
{
   int i, j;

   for (i = 0; i < WIDTH; i++)
   {
      for (j = 0; j < HEIGHT; j++)
      {
         blocks[i][j].group = -1;
      }
   }

   // Mark all groups.
   for (i = 0; i < WIDTH; i++)
   {
      for (j = 0; j < HEIGHT; j++)
      {
         if ((blocks[i][j].group == -1) &&
             ((blocks[i][j].type == Block::PLATFORM) || (blocks[i][j].type == Block::LANDING)))
         {
            // Mark connected platforms with group.
            markGroup(blocks[i][j].platform, i, j);
         }
      }
   }
}


// Recursively mark a group.
void BlockTerrain::markGroup(int mark, int x, int y)
{
   int lx, ly, crx, cry, clx, cly;

   blocks[x][y].group = mark;

   // Extend group marking across ramps to other platforms.
   if (blocks[x][y].type == Block::RAMP)
   {
      // Find connected ramp and mark the landing.
      getRampInfo(x, y, lx, ly, crx, cry, clx, cly);
      if (blocks[clx][cly].group == -1)
      {
         markGroup(mark, clx, cly);
      }
      return;
   }

   // Mark the platform.
   if (x > 0)
   {
      if ((blocks[x - 1][y].platform == blocks[x][y].platform) &&
          (blocks[x - 1][y].group == -1))
      {
         markGroup(mark, x - 1, y);
      }
   }

   if (y > 0)
   {
      if ((blocks[x][y - 1].platform == blocks[x][y].platform) &&
          (blocks[x][y - 1].group == -1))
      {
         markGroup(mark, x, y - 1);
      }
   }

   if (x < WIDTH - 1)
   {
      if ((blocks[x + 1][y].platform == blocks[x][y].platform) &&
          (blocks[x + 1][y].group == -1))
      {
         markGroup(mark, x + 1, y);
      }
   }

   if (y < HEIGHT - 1)
   {
      if ((blocks[x][y + 1].platform == blocks[x][y].platform) &&
          (blocks[x][y + 1].group == -1))
      {
         markGroup(mark, x, y + 1);
      }
   }
}


// Get connectable platforms.
// Can specify platforms that are not already connected.
void BlockTerrain::getConnectablePlatforms(
   vector<struct ConnectablePlatforms>& connectablePlatforms,
   bool                                 alreadyConnected)
{
   int i, j, k;

   vector<int> platforms;
   vector<struct ConnectableBlocks> connectableBlocks;
   struct ConnectablePlatforms      platformConnection;

   // List the platforms.
   for (i = 0; i < WIDTH; i++)
   {
      for (j = 0; j < HEIGHT; j++)
      {
         for (k = 0; k < (int)platforms.size(); k++)
         {
            if (platforms[k] == blocks[i][j].platform)
            {
               break;
            }
         }
         if (k == platforms.size())
         {
            platforms.push_back(blocks[i][j].platform);
         }
      }
   }

   // Sort by platform number.
   for (i = 0; i < (int)platforms.size(); i++)
   {
      for (j = i + 1; j < (int)platforms.size(); j++)
      {
         if (platforms[j] < platforms[i])
         {
            k            = platforms[i];
            platforms[i] = platforms[j];
            platforms[j] = k;
         }
      }
   }

   // Find platforms having a connectable border.
   connectablePlatforms.clear();
   for (i = 0; i < (int)platforms.size(); i++)
   {
      for (j = i + 1; j < (int)platforms.size(); j++)
      {
         if (getConnectableBlocks(platforms[i], platforms[j], connectableBlocks) &&
             !alreadyConnected)
         {
            continue;
         }
         if (connectableBlocks.size() > 0)
         {
            platformConnection.platforms[0] = platforms[i];
            platformConnection.platforms[1] = platforms[j];
            connectablePlatforms.push_back(platformConnection);
         }
      }
   }
}


// Get connectable blocks for given platforms.
// Return true if already connected.
bool BlockTerrain::getConnectableBlocks(int platform1, int platform2,
                                        vector<struct ConnectableBlocks>& connectableBlocks)
{
   int x, y, lx, ly, crx, cry, clx, cly, e;
   struct ConnectableBlocks blockConnection;
   bool alreadyConnected;

   connectableBlocks.clear();
   alreadyConnected = false;
   for (x = 0; x < WIDTH; x++)
   {
      for (y = 0; y < HEIGHT; y++)
      {
         if (blocks[x][y].platform != platform1)
         {
            continue;
         }

         // Cannot convert a landing into a ramp.
         if (blocks[x][y].type == Block::LANDING)
         {
            continue;
         }

         // Already connected?
         if (blocks[x][y].type == Block::RAMP)
         {
            getRampInfo(x, y, lx, ly, crx, cry, clx, cly);
            if (blocks[crx][cry].platform == platform2)
            {
               alreadyConnected = true;
            }
            continue;
         }

         // Check bordering blocks.
         if ((x > 1) && (x < WIDTH - 1))
         {
            if ((blocks[x - 1][y].platform == platform2) &&
                (blocks[x - 1][y].type == Block::PLATFORM) &&
                (blocks[x - 2][y].platform == platform2) &&
                (blocks[x - 2][y].type != Block::RAMP) &&
                (blocks[x + 1][y].platform == platform1) &&
                (blocks[x + 1][y].type != Block::RAMP))
            {
               // Check elevation difference.
               e = blocks[x][y].elevation - blocks[x - 1][y].elevation;
               if (e < 0)
               {
                  e = -e;
               }
               if (e == 1)
               {
                  blockConnection.x[0] = x;
                  blockConnection.y[0] = y;
                  blockConnection.x[1] = x - 1;
                  blockConnection.y[1] = y;
                  connectableBlocks.push_back(blockConnection);
               }
            }
         }

         if ((y > 1) && (y < HEIGHT - 1))
         {
            if ((blocks[x][y - 1].platform == platform2) &&
                (blocks[x][y - 1].type == Block::PLATFORM) &&
                (blocks[x][y - 2].platform == platform2) &&
                (blocks[x][y - 2].type != Block::RAMP) &&
                (blocks[x][y + 1].platform == platform1) &&
                (blocks[x][y + 1].type != Block::RAMP))
            {
               e = blocks[x][y].elevation - blocks[x][y - 1].elevation;
               if (e < 0)
               {
                  e = -e;
               }
               if (e == 1)
               {
                  blockConnection.x[0] = x;
                  blockConnection.y[0] = y;
                  blockConnection.x[1] = x;
                  blockConnection.y[1] = y - 1;
                  connectableBlocks.push_back(blockConnection);
               }
            }
         }

         if ((x < WIDTH - 2) && (x > 0))
         {
            if ((blocks[x + 1][y].platform == platform2) &&
                (blocks[x + 1][y].type == Block::PLATFORM) &&
                (blocks[x + 2][y].platform == platform2) &&
                (blocks[x + 2][y].type != Block::RAMP) &&
                (blocks[x - 1][y].platform == platform1) &&
                (blocks[x - 1][y].type != Block::RAMP))
            {
               e = blocks[x][y].elevation - blocks[x + 1][y].elevation;
               if (e < 0)
               {
                  e = -e;
               }
               if (e == 1)
               {
                  blockConnection.x[0] = x;
                  blockConnection.y[0] = y;
                  blockConnection.x[1] = x + 1;
                  blockConnection.y[1] = y;
                  connectableBlocks.push_back(blockConnection);
               }
            }
         }

         if ((y < HEIGHT - 2) && (y > 0))
         {
            if ((blocks[x][y + 1].platform == platform2) &&
                (blocks[x][y + 1].type == Block::PLATFORM) &&
                (blocks[x][y + 2].platform == platform2) &&
                (blocks[x][y + 2].type != Block::RAMP) &&
                (blocks[x][y - 1].platform == platform1) &&
                (blocks[x][y - 1].type != Block::RAMP))
            {
               e = blocks[x][y].elevation - blocks[x][y + 1].elevation;
               if (e < 0)
               {
                  e = -e;
               }
               if (e == 1)
               {
                  blockConnection.x[0] = x;
                  blockConnection.y[0] = y;
                  blockConnection.x[1] = x;
                  blockConnection.y[1] = y + 1;
                  connectableBlocks.push_back(blockConnection);
               }
            }
         }
      }
   }
   return(alreadyConnected);
}


// Connect blocks with a ramp.
void BlockTerrain::connectBlocks(int block1x, int block1y, int block2x, int block2y)
{
   if (block1x < block2x)
   {
      blocks[block1x - 1][block1y].type = Block::LANDING;
      blocks[block1x][block1y].type     = Block::RAMP;
      blocks[block2x + 1][block2y].type = Block::LANDING;
      blocks[block2x][block2y].type     = Block::RAMP;
      if (blocks[block1x][block1y].elevation > blocks[block2x][block2y].elevation)
      {
         blocks[block1x][block1y].rampDir = Block::WEST;
         blocks[block2x][block2y].rampDir = Block::WEST;
      }
      else
      {
         blocks[block1x][block1y].rampDir = Block::EAST;
         blocks[block2x][block2y].rampDir = Block::EAST;
      }
      return;
   }
   if (block1y < block2y)
   {
      blocks[block1x][block1y - 1].type = Block::LANDING;
      blocks[block1x][block1y].type     = Block::RAMP;
      blocks[block2x][block2y + 1].type = Block::LANDING;
      blocks[block2x][block2y].type     = Block::RAMP;
      if (blocks[block1x][block1y].elevation > blocks[block2x][block2y].elevation)
      {
         blocks[block1x][block1y].rampDir = Block::NORTH;
         blocks[block2x][block2y].rampDir = Block::NORTH;
      }
      else
      {
         blocks[block1x][block1y].rampDir = Block::SOUTH;
         blocks[block2x][block2y].rampDir = Block::SOUTH;
      }
      return;
   }
   if (block1x > block2x)
   {
      blocks[block1x + 1][block1y].type = Block::LANDING;
      blocks[block1x][block1y].type     = Block::RAMP;
      blocks[block2x - 1][block2y].type = Block::LANDING;
      blocks[block2x][block2y].type     = Block::RAMP;
      if (blocks[block1x][block1y].elevation > blocks[block2x][block2y].elevation)
      {
         blocks[block1x][block1y].rampDir = Block::EAST;
         blocks[block2x][block2y].rampDir = Block::EAST;
      }
      else
      {
         blocks[block1x][block1y].rampDir = Block::WEST;
         blocks[block2x][block2y].rampDir = Block::WEST;
      }
      return;
   }
   if (block1y > block2y)
   {
      blocks[block1x][block1y + 1].type = Block::LANDING;
      blocks[block1x][block1y].type     = Block::RAMP;
      blocks[block2x][block2y - 1].type = Block::LANDING;
      blocks[block2x][block2y].type     = Block::RAMP;
      if (blocks[block1x][block1y].elevation > blocks[block2x][block2y].elevation)
      {
         blocks[block1x][block1y].rampDir = Block::SOUTH;
         blocks[block2x][block2y].rampDir = Block::SOUTH;
      }
      else
      {
         blocks[block1x][block1y].rampDir = Block::NORTH;
         blocks[block2x][block2y].rampDir = Block::NORTH;
      }
      return;
   }
}


// Given a half-ramp location, get remaining ramp coordinates.
void BlockTerrain::getRampInfo(int rampX, int rampY,
                               int& landingX, int& landingY,
                               int& connectedRampX, int& connectedRampY,
                               int& connectedLandingX, int& connectedLandingY)
{
   assert(blocks[rampX][rampY].type == Block::RAMP);
   switch (blocks[rampX][rampY].rampDir)
   {
   case Block::NORTH:
      if (blocks[rampX][rampY - 1].type == Block::RAMP)
      {
         landingX          = rampX;
         landingY          = rampY + 1;
         connectedRampX    = rampX;
         connectedRampY    = rampY - 1;
         connectedLandingX = rampX;
         connectedLandingY = rampY - 2;
      }
      else
      {
         landingX          = rampX;
         landingY          = rampY - 1;
         connectedRampX    = rampX;
         connectedRampY    = rampY + 1;
         connectedLandingX = rampX;
         connectedLandingY = rampY + 2;
      }
      break;

   case Block::SOUTH:
      if (blocks[rampX][rampY + 1].type == Block::RAMP)
      {
         landingX          = rampX;
         landingY          = rampY - 1;
         connectedRampX    = rampX;
         connectedRampY    = rampY + 1;
         connectedLandingX = rampX;
         connectedLandingY = rampY + 2;
      }
      else
      {
         landingX          = rampX;
         landingY          = rampY + 1;
         connectedRampX    = rampX;
         connectedRampY    = rampY - 1;
         connectedLandingX = rampX;
         connectedLandingY = rampY - 2;
      }
      break;

   case Block::EAST:
      if (blocks[rampX + 1][rampY].type == Block::RAMP)
      {
         landingX          = rampX - 1;
         landingY          = rampY;
         connectedRampX    = rampX + 1;
         connectedRampY    = rampY;
         connectedLandingX = rampX + 2;
         connectedLandingY = rampY;
      }
      else
      {
         landingX          = rampX + 1;
         landingY          = rampY;
         connectedRampX    = rampX - 1;
         connectedRampY    = rampY;
         connectedLandingX = rampX - 2;
         connectedLandingY = rampY;
      }
      break;

   case Block::WEST:
      if (blocks[rampX - 1][rampY].type == Block::RAMP)
      {
         landingX          = rampX + 1;
         landingY          = rampY;
         connectedRampX    = rampX - 1;
         connectedRampY    = rampY;
         connectedLandingX = rampX - 2;
         connectedLandingY = rampY;
      }
      else
      {
         landingX          = rampX - 1;
         landingY          = rampY;
         connectedRampX    = rampX + 1;
         connectedRampY    = rampY;
         connectedLandingX = rampX + 2;
         connectedLandingY = rampY;
      }
      break;
   }
}


// Is block an upper half-ramp?
bool BlockTerrain::isUpperRamp(int rampX, int rampY)
{
   int x, y;

   if (blocks[rampX][rampY].type != Block::RAMP)
   {
      return(false);
   }
   switch (blocks[rampX][rampY].rampDir)
   {
   case Block::NORTH:
      x = rampX;
      y = rampY - 1;
      break;

   case Block::SOUTH:
      x = rampX;
      y = rampY + 1;
      break;

   case Block::EAST:
      x = rampX + 1;
      y = rampY;
      break;

   case Block::WEST:
      x = rampX - 1;
      y = rampY;
      break;
   }
   if (blocks[x][y].type == Block::RAMP)
   {
      return(false);
   }
   else
   {
      return(true);
   }
}


// Print block terrain.
void BlockTerrain::print(FILE *out)
{
   int  i, j;
   char c;

   for (i = 0; i < HEIGHT; i++)
   {
      for (j = 0; j < WIDTH; j++)
      {
         switch (blocks[j][i].type)
         {
         case Block::PLATFORM:
            c = 'p';
            break;

         case Block::LANDING:
            c = 'l';
            break;

         case Block::RAMP:
            c = 'r';
            break;
         }
         fprintf(out, "%c/%d/%d ", c, blocks[j][i].platform, blocks[j][i].elevation);
      }
      fprintf(out, "\n");
   }
}


// Build the drawable blocks and a heightmap where
// height is defined as the Y dimension on the XZ plane.
void BlockTerrain::build()
{
   int    i, j, k, e;
   Vector vmin, vmax, vertex;
   Bounds bounds;

   vector<Vector> vertices;
   Poly           *polygon;

   // Build displays.
   if (!displaysCreated)
   {
      blockDisplay = glGenLists(4);
      glNewList(blockDisplay, GL_COMPILE);
      drawBlock();
      glEndList();
      rampSurfaceDisplay = blockDisplay + 1;
      glNewList(rampSurfaceDisplay, GL_COMPILE);
      drawRampSurface();
      glEndList();
      leftRampDisplay = rampSurfaceDisplay + 1;
      glNewList(leftRampDisplay, GL_COMPILE);
      drawLeftRamp();
      glEndList();
      rightRampDisplay = leftRampDisplay + 1;
      glNewList(rightRampDisplay, GL_COMPILE);
      drawRightRamp();
      glEndList();
      displaysCreated = true;
   }

   // Assign textures to blocks.
   for (i = 0; i < WIDTH; i++)
   {
      for (j = 0; j < HEIGHT; j++)
      {
         blocks[i][j].textureIndexes.clear();
         e = blocks[i][j].elevation;
         if (isUpperRamp(i, j))
         {
            e--;
         }
         for (k = 0; k <= e; k++)
         {
            blocks[i][j].textureIndexes.push_back(randomizer->RAND_CHOICE(NUM_BLOCK_TEXTURES - 1));
         }
      }
   }

   // Get the terrain bounds and create the heightmap.
   vmin.x = -(BLOCK_SIZE * 0.1f);
   vmin.y = 0.0f;
   vmin.z = -(BLOCK_SIZE * 0.1f);
   vmax.x = BLOCK_SIZE * WIDTH * 1.1f;
   vmax.y = 0.0f;
   vmax.z = BLOCK_SIZE * HEIGHT * 1.1f;
   bounds = Bounds(vmin, vmax);
   if (heightmap != NULL)
   {
      delete heightmap;
   }
   heightmap = new QuadTree(bounds);
   assert(heightmap != NULL);

   // Create the terrain surface polygons and insert into heightmap.
   for (i = 0; i < WIDTH; i++)
   {
      for (j = 0; j < HEIGHT; j++)
      {
         // Ramp?
         if (blocks[i][j].type == Block::RAMP)
         {
            if (isUpperRamp(i, j))
            {
               vertices.clear();
               switch (blocks[i][j].rampDir)
               {
               case Block::NORTH:
                  vertex.x = (GLfloat)i * BLOCK_SIZE;
                  vertex.y = ((GLfloat)blocks[i][j].elevation * BLOCK_SIZE) + BLOCK_SIZE;
                  vertex.z = (GLfloat)j * BLOCK_SIZE;
                  vertices.push_back(vertex);
                  vertex.y -= BLOCK_SIZE;
                  vertex.z += BLOCK_SIZE * 2.0f;
                  vertices.push_back(vertex);
                  vertex.x += BLOCK_SIZE;
                  vertices.push_back(vertex);
                  vertex.y += BLOCK_SIZE;
                  vertex.z -= BLOCK_SIZE * 2.0f;
                  vertices.push_back(vertex);
                  break;

               case Block::SOUTH:
                  vertex.x = ((GLfloat)i * BLOCK_SIZE) + BLOCK_SIZE;
                  vertex.y = ((GLfloat)blocks[i][j].elevation * BLOCK_SIZE) + BLOCK_SIZE;
                  vertex.z = ((GLfloat)j * BLOCK_SIZE) + BLOCK_SIZE;
                  vertices.push_back(vertex);
                  vertex.y -= BLOCK_SIZE;
                  vertex.z -= BLOCK_SIZE * 2.0f;
                  vertices.push_back(vertex);
                  vertex.x -= BLOCK_SIZE;
                  vertices.push_back(vertex);
                  vertex.y += BLOCK_SIZE;
                  vertex.z += BLOCK_SIZE * 2.0f;
                  vertices.push_back(vertex);
                  break;

               case Block::EAST:
                  vertex.x = ((GLfloat)i * BLOCK_SIZE) + BLOCK_SIZE;
                  vertex.y = ((GLfloat)blocks[i][j].elevation * BLOCK_SIZE) + BLOCK_SIZE;
                  vertex.z = (GLfloat)j * BLOCK_SIZE;
                  vertices.push_back(vertex);
                  vertex.y -= BLOCK_SIZE;
                  vertex.x -= BLOCK_SIZE * 2.0f;
                  vertices.push_back(vertex);
                  vertex.z += BLOCK_SIZE;
                  vertices.push_back(vertex);
                  vertex.y += BLOCK_SIZE;
                  vertex.x += BLOCK_SIZE * 2.0f;
                  vertices.push_back(vertex);
                  break;

               case Block::WEST:
                  vertex.x = (GLfloat)i * BLOCK_SIZE;
                  vertex.y = ((GLfloat)blocks[i][j].elevation * BLOCK_SIZE) + BLOCK_SIZE;
                  vertex.z = ((GLfloat)j * BLOCK_SIZE) + BLOCK_SIZE;
                  vertices.push_back(vertex);
                  vertex.y -= BLOCK_SIZE;
                  vertex.x += BLOCK_SIZE * 2.0f;
                  vertices.push_back(vertex);
                  vertex.z -= BLOCK_SIZE;
                  vertices.push_back(vertex);
                  vertex.y += BLOCK_SIZE;
                  vertex.x -= BLOCK_SIZE * 2.0f;
                  vertices.push_back(vertex);
                  break;
               }
               polygon = new Poly(vertices);
               assert(polygon != NULL);
               heightmap->insert(polygon);
            }
         }
         else
         {
            // Insert block surface.
            vertices.clear();
            vertex.x = (GLfloat)i * BLOCK_SIZE;
            vertex.y = ((GLfloat)blocks[i][j].elevation * BLOCK_SIZE) + BLOCK_SIZE;
            vertex.z = (GLfloat)j * BLOCK_SIZE;
            vertices.push_back(vertex);
            vertex.z += BLOCK_SIZE;
            vertices.push_back(vertex);
            vertex.x += BLOCK_SIZE;
            vertices.push_back(vertex);
            vertex.z -= BLOCK_SIZE;
            vertices.push_back(vertex);
            polygon = new Poly(vertices);
            assert(polygon != NULL);
            heightmap->insert(polygon);
         }
      }
   }

   // Load the textures.
   if (!texturesLoaded)
   {
      loadTextures();
      texturesLoaded = true;
   }
}


// Load the textures.
void BlockTerrain::loadTextures()
{
   char *path, *image;

   // Load the block textures.
   image = (char *)"images/A.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 0))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/B.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 1))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/C.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 2))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/D.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 3))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/E.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 4))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/F.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 5))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/G.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 6))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/H.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 7))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/I.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 8))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/J.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 9))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/K.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 10))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/L.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 11))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/M.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 12))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/N.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 13))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/O.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 14))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/P.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 15))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/Q.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 16))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/R.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 17))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/S.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 18))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/T.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 19))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/U.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 20))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/V.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 21))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/W.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 22))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/X.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 23))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/Y.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 24))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/Z.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 25))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
   image = (char *)"images/gray.bmp";
   path  = getResourcePath(image);
   if (path == NULL)
   {
      fprintf(stderr, "Cannot get texture path for %s\n", image);
      exit(1);
   }
   if (!CreateTexture(path, blockTextures, 26))
   {
      fprintf(stderr, "Cannot load texture for %s\n", path);
      exit(1);
   }
   free(path);
}


// Get terrain height (Y) and facet normal vector at given XZ coordinates.
void BlockTerrain::getGeometry(GLfloat x, GLfloat z, GLfloat& y, Vector& normal)
{
   vector<Poly *> polygons;

   // Search for facet polygons at x, y coordinates.
   heightmap->search(x, z, polygons);
   if (polygons.size() == 0)
   {
      y        = 0.0f;
      normal.x = 0.0f;
      normal.y = 1.0f;
      normal.z = 0.0f;
      return;
   }

   // Assume single polygon match.
   normal = polygons[0]->plane.normal;
   if (normal.y > 0.0f)
   {
      // Solve plane equation for height.
      y = (-(normal.x * x) - (normal.z * z) - polygons[0]->plane.d) / normal.y;
   }
   else
   {
      y = 0.0f;
   }
}


// Draw a block.
void BlockTerrain::drawBlock()
{
   BaseObject::drawBlock(0.0f, BLOCK_SIZE, 0.0f, BLOCK_SIZE, 0.0f, BLOCK_SIZE);
}


// Draw ramp surface.
void BlockTerrain::drawRampSurface()
{
   int     i;
   GLfloat d, h, s, w, x, y, z;

   // Draw a chevron pattern on the ramp surface.
   h = BLOCK_SIZE / 2.0f;
   w = (BLOCK_SIZE * 2.236f * 0.25f) / (GLfloat)NUM_RAMP_STRIPES;
   d = ((BLOCK_SIZE * 2.236f) - w) / ((GLfloat)NUM_RAMP_STRIPES + 1.0f);
   s = (d * 2.0f) / h;
   for (i = 0; i < NUM_RAMP_STRIPES; i++)
   {
      // Draw black chevron.
      glColor3f(0.0f, 0.0f, 0.0f);
      glBegin(GL_QUADS);
      // Left half of chevron.
      glNormal3f(0.0f, 0.894f, 0.447f);
      x = 0.0f;
      y = d * (GLfloat)i * 0.447f;
      z = d * (GLfloat)i * 0.894f;
      z = (BLOCK_SIZE * 2.0f) - z;
      glVertex3f(x, y, z);
      glNormal3f(0.0f, 0.894f, 0.447f);
      x  = h;
      y += s * x * 0.447f;
      z -= s * x * 0.894f;
      glVertex3f(x, y, z);
      glNormal3f(0.0f, 0.894f, 0.447f);
      y += w * 0.447f;
      z -= w * 0.894f;
      glVertex3f(x, y, z);
      glNormal3f(0.0f, 0.894f, 0.447f);
      x = 0.0f;
      y = ((d * (GLfloat)i) + w) * 0.447f;
      z = ((d * (GLfloat)i) + w) * 0.894f;
      z = (BLOCK_SIZE * 2.0f) - z;
      glVertex3f(x, y, z);
      // Right half of chevron.
      glNormal3f(0.0f, 0.894f, 0.447f);
      x  = h;
      y  = d * (GLfloat)i * 0.447f;
      z  = d * (GLfloat)i * 0.894f;
      z  = (BLOCK_SIZE * 2.0f) - z;
      y += s * x * 0.447f;
      z -= s * x * 0.894f;
      glVertex3f(x, y, z);
      glNormal3f(0.0f, 0.894f, 0.447f);
      y -= s * x * 0.447f;
      z += s * x * 0.894f;
      x  = BLOCK_SIZE;
      glVertex3f(x, y, z);
      glNormal3f(0.0f, 1.0f, 0.0f);
      y += w * 0.447f;
      z -= w * 0.894f;
      glVertex3f(x, y, z);
      glNormal3f(0.0f, 0.894f, 0.447f);
      x  = h;
      y += s * x * 0.447f;
      z -= s * x * 0.894f;
      glVertex3f(x, y, z);
      glEnd();

      // Draw white background.
      glColor3f(1.0f, 1.0f, 1.0f);
      if (i == 0)
      {
         // Draw bottom white triangle.
         glBegin(GL_TRIANGLES);
         glNormal3f(0.0f, 0.894f, 0.447f);
         x = 0.0f;
         y = d * (GLfloat)i * 0.447f;
         z = d * (GLfloat)i * 0.894f;
         z = (BLOCK_SIZE * 2.0f) - z;
         glVertex3f(x, y, z);
         glNormal3f(0.0f, 0.894f, 0.447f);
         x = BLOCK_SIZE;
         glVertex3f(x, y, z);
         glNormal3f(0.0f, 0.894f, 0.447f);
         x  = h;
         y += s * x * 0.447f;
         z -= s * x * 0.894f;
         glVertex3f(x, y, z);
         glEnd();
      }
      else
      {
         // Draw white chevron.
         glBegin(GL_QUADS);
         // Left half of chevron.
         glNormal3f(0.0f, 0.894f, 0.447f);
         x = 0.0f;
         y = d * (GLfloat)i * 0.447f;
         z = d * (GLfloat)i * 0.894f;
         z = (BLOCK_SIZE * 2.0f) - z;
         glVertex3f(x, y, z);
         glNormal3f(0.0f, 0.894f, 0.447f);
         x = 0.0f;
         y = ((d * (GLfloat)(i - 1)) + w) * 0.447f;
         z = (d * (GLfloat)(i - 1) + w) * 0.894f;
         z = (BLOCK_SIZE * 2.0f) - z;
         glVertex3f(x, y, z);
         glNormal3f(0.0f, 0.894f, 0.447f);
         x  = h;
         y += s * x * 0.447f;
         z -= s * x * 0.894f;
         glVertex3f(x, y, z);
         glNormal3f(0.0f, 0.894f, 0.447f);
         x  = h;
         y  = d * (GLfloat)i * 0.447f;
         z  = d * (GLfloat)i * 0.894f;
         z  = (BLOCK_SIZE * 2.0f) - z;
         y += s * x * 0.447f;
         z -= s * x * 0.894f;
         glVertex3f(x, y, z);
         // Right half of chevron.
         glNormal3f(0.0f, 0.894f, 0.447f);
         x  = h;
         y  = d * (GLfloat)i * 0.447f;
         z  = d * (GLfloat)i * 0.894f;
         z  = (BLOCK_SIZE * 2.0f) - z;
         y += s * x * 0.447f;
         z -= s * x * 0.894f;
         glVertex3f(x, y, z);
         glNormal3f(0.0f, 0.894f, 0.447f);
         x  = h;
         y  = ((d * (GLfloat)(i - 1)) + w) * 0.447f;
         z  = ((d * (GLfloat)(i - 1)) + w) * 0.894f;
         z  = (BLOCK_SIZE * 2.0f) - z;
         y += s * x * 0.447f;
         z -= s * x * 0.894f;
         glVertex3f(x, y, z);
         glNormal3f(0.0f, 1.0f, 0.0f);
         y -= s * x * 0.447f;
         z += s * x * 0.894f;
         x  = BLOCK_SIZE;
         glVertex3f(x, y, z);
         glNormal3f(0.0f, 0.894f, 0.447f);
         x = BLOCK_SIZE;
         y = d * (GLfloat)i * 0.447f;
         z = d * (GLfloat)i * 0.894f;
         z = (BLOCK_SIZE * 2.0f) - z;
         glVertex3f(x, y, z);
         glEnd();
      }
      if (i == NUM_RAMP_STRIPES - 1)
      {
         // Draw top triangles.
         glBegin(GL_TRIANGLES);
         glNormal3f(0.0f, 0.894f, 0.447f);
         x = 0.0f;
         y = ((d * (GLfloat)i) + w) * 0.447f;
         z = ((d * (GLfloat)i) + w) * 0.894f;
         z = (BLOCK_SIZE * 2.0f) - z;
         glVertex3f(x, y, z);
         glNormal3f(0.0f, 0.894f, 0.447f);
         x  = h;
         y += s * x * 0.447f;
         z -= s * x * 0.894f;
         glVertex3f(x, y, z);
         glNormal3f(0.0f, 0.894f, 0.447f);
         x = 0.0f;
         glVertex3f(x, y, z);
         glNormal3f(0.0f, 0.894f, 0.447f);
         x  = h;
         y  = ((d * (GLfloat)i) + w) * 0.447f;
         z  = ((d * (GLfloat)i) + w) * 0.894f;
         z  = (BLOCK_SIZE * 2.0f) - z;
         y += s * x * 0.447f;
         z -= s * x * 0.894f;
         glVertex3f(x, y, z);
         glNormal3f(0.0f, 0.894f, 0.447f);
         y -= s * x * 0.447f;
         z += s * x * 0.894f;
         x  = BLOCK_SIZE;
         glVertex3f(x, y, z);
         glNormal3f(0.0f, 0.894f, 0.447f);
         x  = BLOCK_SIZE;
         y  = ((d * (GLfloat)i) + w) * 0.447f;
         z  = ((d * (GLfloat)i) + w) * 0.894f;
         z  = (BLOCK_SIZE * 2.0f) - z;
         y += s * h * 0.447f;
         z -= s * h * 0.894f;
         glVertex3f(x, y, z);
         glEnd();
      }
   }
   glColor3f(1.0f, 1.0f, 1.0f);
}


// Draw ramp left side.
void BlockTerrain::drawLeftRamp()
{
   glColor3f(0.5f, 0.5f, 0.5f);
   glBegin(GL_TRIANGLES);
   glNormal3f(-1.0f, 0.0f, 0.0f);
   glVertex3f(0.0f, 0.0f, 0.0f);
   glNormal3f(-1.0f, 0.0f, 0.0f);
   glVertex3f(0.0f, 0.0f, BLOCK_SIZE * 2.0f);
   glNormal3f(-1.0f, 0.0f, 0.0f);
   glVertex3f(0.0f, BLOCK_SIZE, 0.0f);
   glEnd();
   glColor3f(1.0f, 1.0f, 1.0f);
}


// Draw ramp right side.
void BlockTerrain::drawRightRamp()
{
   glColor3f(0.5f, 0.5f, 0.5f);
   glBegin(GL_TRIANGLES);
   glNormal3f(1.0f, 0.0f, 0.0f);
   glVertex3f(BLOCK_SIZE, 0.0f, 0.0f);
   glNormal3f(1.0f, 0.0f, 0.0f);
   glVertex3f(BLOCK_SIZE, BLOCK_SIZE, 0.0f);
   glNormal3f(1.0f, 0.0f, 0.0f);
   glVertex3f(BLOCK_SIZE, 0.0f, BLOCK_SIZE * 2.0f);
   glEnd();
   glColor3f(1.0f, 1.0f, 1.0f);
}


// Draw block terrain.
void BlockTerrain::draw()
{
   int     i, j, k;
   GLfloat x, y, z;

   glMatrixMode(GL_MODELVIEW);
   glEnable(GL_TEXTURE_2D);
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

   // Draw blocks.
   for (i = 0; i < WIDTH; i++)
   {
      for (j = 0; j < HEIGHT; j++)
      {
         // Draw a ramp?
         if ((blocks[i][j].type == Block::RAMP) && isUpperRamp(i, j))
         {
            glPushMatrix();
            switch (blocks[i][j].rampDir)
            {
            case Block::NORTH:
               x = (GLfloat)i * BLOCK_SIZE;
               y = (GLfloat)blocks[i][j].elevation * BLOCK_SIZE;
               z = (GLfloat)j * BLOCK_SIZE;
               glTranslatef(x, y, z);
               break;

            case Block::SOUTH:
               x = ((GLfloat)i * BLOCK_SIZE) + BLOCK_SIZE;
               y = (GLfloat)blocks[i][j].elevation * BLOCK_SIZE;
               z = ((GLfloat)j * BLOCK_SIZE) + BLOCK_SIZE;
               glTranslatef(x, y, z);
               glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
               break;

            case Block::EAST:
               x = ((GLfloat)i * BLOCK_SIZE) + BLOCK_SIZE;
               y = (GLfloat)blocks[i][j].elevation * BLOCK_SIZE;
               z = (GLfloat)j * BLOCK_SIZE;
               glTranslatef(x, y, z);
               glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
               break;

            case Block::WEST:
               x = (GLfloat)i * BLOCK_SIZE;
               y = (GLfloat)blocks[i][j].elevation * BLOCK_SIZE;
               z = ((GLfloat)j * BLOCK_SIZE) + BLOCK_SIZE;
               glTranslatef(x, y, z);
               glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
               break;
            }
            glCallList(rampSurfaceDisplay);
            glCallList(leftRampDisplay);
            glCallList(rightRampDisplay);
            glPopMatrix();
         }

         // Draw block stack.
         x = (GLfloat)i * BLOCK_SIZE;
         z = (GLfloat)j * BLOCK_SIZE;
         for (k = 0; k < (int)blocks[i][j].textureIndexes.size(); k++)
         {
            y = (GLfloat)k * BLOCK_SIZE;
            glPushMatrix();
            glTranslatef(x, y, z);
            glBindTexture(GL_TEXTURE_2D, blockTextures[blocks[i][j].textureIndexes[k]]);
            glCallList(blockDisplay);
            glPopMatrix();
         }
      }
   }
}
