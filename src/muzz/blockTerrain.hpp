// For conditions of distribution and use, see copyright notice in muzz.hpp

/*
 *
 * Block terrain: a grid of blocks piled to various elevations
 * that is connected by ramps such that the top of every block
 * is reachable.
 *
 * A platform is a horizontal rectangle of blocks impressed into
 * the terrain. Multiple platforms overlay to produce a terraced
 * effect.
 *
 * A ramp can connect platforms that have an elevation difference
 * of one. It is composed of a pair of inclined blocks, one rising from
 * the lower platform and the other sinking from the upper platform to
 * form a 30 degree incline.
 *
 * A landing is the level block that terminates a ramp. There is a
 * landing at the upper and lower ends to ensure continuity with
 * a platform.
 *
 * Constructors:
 *
 * BlockTerrain();
 * BlockTerrain(RANDOM randomSeed);
 * BlockTerrain(RANDOM randomSeed, int width, int height,
 * int maxElevation, int minPlatformSize, int maxPlatformSize,
 * int maxPlatformGenerations, int extraRamps, GLfloat blockSize);
 *
 * randomSeed - determines terrain topology.
 * width, height - dimensions of terrain.
 * maxElevation - maximum block elevation.
 * minPlatformSize, maxPlatformSize - dimensions platform.
 * maxPlatformGenerations - maximum platform creation attempts.
 * extraRamps - extra ramps to add more terrain routes.
 * blockSize - dimension size of a cubic block.
 *
 * The generated terrain topology:
 *
 * Block **blocks;
 *
 * To access terrain blocks:
 *
 * for (i = 0; i < terrain->width; i++)
 * {
 * for (j = 0; j < terrain->height; j++)
 * {
 * terrain->blocks[i][j].type;
 * terrain->blocks[i][j].elevation;
 * }
 * }
 *
 * Types: Block::PLATFORM, Block::LANDING, Block::RAMP
 *
 * To get terrain height (Y) and facet normal vector at given XZ coordinates:
 *
 * void getGeometry(GLfloat x, GLfloat z, GLfloat &y, Vector &normal);
 *
 * The draw() function draws the terrain using OpenGL graphics.
 *
 * The print() function prints out the terrain topology.
 *
 */

#ifndef __BLOCK_TERRAIN__
#define __BLOCK_TERRAIN__

#include "../graphics/graphics.h"

// Block terrain.
class BlockTerrain
{
public:

   // Block.
   class Block
   {
public:

      // Identifier dispenser.
      static int idDispenser;

      // Block type.
      typedef enum
      {
         PLATFORM,
         LANDING,
         RAMP
      } TYPE;

      // Direction.
      typedef enum
      {
         NORTH = 0,
         EAST  = 1,
         SOUTH = 2,
         WEST  = 3,
         NONE  = 4
      } DIRECTION;

      int         id;
      int         elevation;
      TYPE        type;
      DIRECTION   rampDir;
      int         platform;
      int         group;
      vector<int> textureIndexes;

      Block()
      {
         id = idDispenser;
         idDispenser++;
         elevation = 0;
         type      = PLATFORM;
         rampDir   = NONE;
         platform  = group = -1;
      }
   };

   // Parameters:

   // Random seed.
   enum { DEFAULT_RANDOM_SEED=4517 };
   RANDOM RANDOM_SEED;
   Random *randomizer;

   // Terrain width and height.
   enum { DEFAULT_WIDTH=4, DEFAULT_HEIGHT=4 };
   int WIDTH;
   int HEIGHT;

   // Maximum platform elevation.
   enum { DEFAULT_MAX_PLATFORM_ELEVATION=5 };
   int MAX_PLATFORM_ELEVATION;

   // Minimum/maximum platform sizes.
   enum { DEFAULT_MIN_PLATFORM_SIZE=2, DEFAULT_MAX_PLATFORM_SIZE=2 };
   int MIN_PLATFORM_SIZE;
   int MAX_PLATFORM_SIZE;

   // Maximum platform generations.
   enum { DEFAULT_MAX_PLATFORM_GENERATIONS=10 };
   int MAX_PLATFORM_GENERATIONS;

   // Extra ramps.
   enum { DEFAULT_EXTRA_RAMPS=0 };
   int EXTRA_RAMPS;

   // Block dimension size.
   static const GLfloat DEFAULT_BLOCK_SIZE;
   GLfloat              BLOCK_SIZE;

   // Ramp stripes.
   enum { NUM_RAMP_STRIPES=10 };

   // Terrain blocks
   Block **blocks;
   Block **saveBlocks;

   // Constructors.
   BlockTerrain()
   {
      init(DEFAULT_RANDOM_SEED, DEFAULT_WIDTH, DEFAULT_HEIGHT,
           DEFAULT_MAX_PLATFORM_ELEVATION, DEFAULT_MIN_PLATFORM_SIZE,
           DEFAULT_MAX_PLATFORM_SIZE, DEFAULT_MAX_PLATFORM_GENERATIONS,
           DEFAULT_EXTRA_RAMPS, DEFAULT_BLOCK_SIZE);
   }


   BlockTerrain(RANDOM randomSeed)
   {
      init(randomSeed, DEFAULT_WIDTH, DEFAULT_HEIGHT,
           DEFAULT_MAX_PLATFORM_ELEVATION, DEFAULT_MIN_PLATFORM_SIZE,
           DEFAULT_MAX_PLATFORM_SIZE, DEFAULT_MAX_PLATFORM_GENERATIONS,
           DEFAULT_EXTRA_RAMPS, DEFAULT_BLOCK_SIZE);
   }


   BlockTerrain(RANDOM randomSeed, int width, int height,
                int maxElevation, int minPlatformSize, int maxPlatformSize,
                int maxPlatformGenerations, int extraRamps, GLfloat blockSize)
   {
      init(randomSeed, width, height,
           maxElevation, minPlatformSize, maxPlatformSize,
           maxPlatformGenerations, extraRamps, blockSize);
   }


   // Destructor.
   ~BlockTerrain();

   // Print terrain.
   void print(FILE *out = stdout);

   // Get terrain height (Y) and facet normal vector at given XZ coordinates.
   void getGeometry(GLfloat x, GLfloat z, GLfloat& y, Vector& normal);

   // Is block an upper half-ramp?
   bool isUpperRamp(int rampX, int rampY);

   // Draw terrain.
   void draw();

protected:

   void init(RANDOM randomSeed, int width, int height,
             int maxElevation, int minPlatformSize, int maxPlatformSize,
             int maxPlatformGenerations, int extraRamps, GLfloat blockSize);

   // Platforms that are connectable by a ramp.
   struct ConnectablePlatforms
   {
      int platforms[2];
   };

   // Blocks that are connectable by a ramp.
   struct ConnectableBlocks
   {
      int x[2];
      int y[2];
   };

   // Generate terrain.
   void generate();

   // Generate T-maze terrain.
   void generateTmaze();

   // Create a platform.
   void createPlatform();

   // Mark platforms.
   void markPlatforms();

   // Mark a platform.
   void markPlatform(int mark, int x, int y);

   // Connect platforms with ramps into a fully connected terrain.
   // Return true if full connection achieved.
   bool connectPlatforms();

   // Mark groups.
   // Connected platforms belong to the same group.
   void markGroups();

   // Mark a group.
   void markGroup(int mark, int x, int y);

   // Get connectable platforms.
   // Default to platforms that are not already connected.
   void getConnectablePlatforms(
      vector<struct ConnectablePlatforms>& connectablePlatforms,
      bool                                 alreadyConnected = false);

   // Get connectable blocks for given platforms.
   // Return true if already connected.
   bool getConnectableBlocks(int platform1, int platform2,
                             vector<struct ConnectableBlocks>& connectableBlocks);

   // Connect blocks with a ramp.
   void connectBlocks(int block1x, int block1y, int block2x, int block2y);

   // Given a half-ramp location, get remaining ramp coordinates.
   void getRampInfo(int rampX, int rampY,
                    int& landingX, int& landingY,
                    int& connectedRampX, int& connectedRampY,
                    int& connectedLandingX, int& connectedLandingY);

   // Build the drawable blocks and a heightmap.
   void build();

   // Draw components.
   void drawBlock();
   void drawRampSurface();
   void drawLeftRamp();
   void drawRightRamp();

   // OpenGL displays for drawing.
   bool   displaysCreated;
   GLuint blockDisplay;
   GLuint rampSurfaceDisplay;
   GLuint leftRampDisplay;
   GLuint rightRampDisplay;

   // Terrain height map.
   // Height is defined as the Y dimension on the XZ plane.
   QuadTree *heightmap;

   // Textures.
   enum { NUM_BLOCK_TEXTURES=27 };
   GLuint blockTextures[NUM_BLOCK_TEXTURES];
   bool   texturesLoaded;
   void loadTextures();
};
#endif
