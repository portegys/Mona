// For conditions of distribution and use, see copyright notice in muzz.hpp

/*
 *
 * The Muzz World.
 *
 * Muzz creatures interact with a block world.
 * Each muzz has a mona neural network for a brain.
 *
 */

#ifdef WIN32
#include <windows.h>
#include <io.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "../gui/TimeUtils.h"
#include "../gui/EasyGL.h"
#include "muzzWorld.hpp"

// Version.
const char *MuzzWorldVersion = MUZZ_WORLD_VERSION;

// Print version.
void
printVersion(FILE *out)
{
   fprintf(out, "%s\n", &MuzzWorldVersion[4]);
}


#ifndef MUZZ_WORLD_DRIVER
char *Usage[] =
{
   (char *)"Usage: muzz_world\n",
   (char *)"      [-cycles <number of cycles>]\n",
   (char *)"      [-initHunger (initial hunger)]\n",
   (char *)"      [-initThirst (initial thirst)]\n",
   (char *)"      [-numMuzzes <number of muzzes>]\n",
   (char *)"      [-numMushrooms <number of mushrooms>]\n",
   (char *)"      [-numPools <number of pools>]\n",
   (char *)"      [-load <load file name>]\n",
   (char *)"      [-save <save file name>]\n",
   (char *)"      [-randomSeed <random seed>]\n",
   (char *)"      [-objectSeed <object placement seed>]\n",
   (char *)"      [-TmazeTerrain (create T-maze terrain)]\n",
   (char *)"      [-terrainSeed <terrain generation seed>]\n",
   (char *)"      [-terrainDimension <terrain size = dimension x dimension> (minimum=2)]\n",
   (char *)"      [-noGraphics (turn off graphics)]\n",
   (char *)"      [-pause (graphics only)]\n",
   (char *)"      [-numTrainingTrials <number of training trials>]\n",
   (char *)"          (Pauses when training trials are completed)\n",
   (char *)"      [-forcedResponseTrain (train correct responses by forcibly overriding them)]\n",
#ifdef WIN32
   (char *)"      [-attachConsole (attach to console)]\n",
#endif
   (char *)"      [-version (print version)]\n",
   NULL
};

void printUsage()
{
   for (int i = 0; Usage[i] != NULL; i++)
   {
      fprintf(stderr, (char *)"%s", Usage[i]);
   }
}


#endif

// Run cycles.
int Cycles       = -1;
int CycleCounter = 0;

// Show graphics.
bool Graphics = true;

// Smooth movements.
bool SmoothMove = true;

// Random numbers.
RANDOM RandomSeed  = INVALID_RANDOM;
Random *Randomizer = NULL;
RANDOM ObjectSeed  = INVALID_RANDOM;
RANDOM TerrainSeed = INVALID_RANDOM;

// Files.
char *SaveFile = NULL;
char *LoadFile = NULL;

// Graphics window dimensions.
#define WINDOW_WIDTH     850
#define WINDOW_HEIGHT    690
int WindowWidth  = WINDOW_WIDTH;
int WindowHeight = WINDOW_HEIGHT;
int GUIheight    = WINDOW_HEIGHT / 8;
int MainWindow;

// Viewports.
#define TERRAIN_VIEWPORT         0
#define MUZZ_VIEWPORT            1
#define CONTROLS_VIEWPORT        2
#define MUZZ_STATUS_VIEWPORT     3
#define TERRAIN_HELP_VIEWPORT    4
#define MUZZ_HELP_VIEWPORT       5
#define NUM_VIEWPORTS            6
typedef enum
{
   MUZZ_VIEW_ONLY = 0, TERRAIN_VIEW_ONLY = 1, VIEW_BOTH = 2
}
VIEW_SELECTION;
VIEW_SELECTION ViewSelection        = VIEW_BOTH;
VIEW_SELECTION PendingViewSelection = (VIEW_SELECTION)(-1);
struct ViewportDimension
{
   GLint   x, y;
   GLint   width, height;
   GLfloat aspect;
}
Viewports[NUM_VIEWPORTS];

// Picking.
#define BUFSIZE    1024
GLuint selectBuf[BUFSIZE];
GLint  pickHits;
GLenum renderMode = GL_RENDER;
int    cursorX, cursorY;
void startPicking();

void processHits(GLint hits, GLuint buffer[], int sw);
void stopPicking();

// Muzzes.
int            NUM_MUZZES = DEFAULT_NUM_MUZZES;
vector<Muzz *> Muzzes;

// Muzz states.
#define MUZZ_FRUSTUM_ANGLE     60.0f
#define MUZZ_FRUSTUM_NEAR      0.01f
#define MUZZ_FRUSTUM_FAR       10.0f
#define MUZZ_TURN_DELTA        5.0f
#define MUZZ_MOVEMENT_DELTA    0.002f
int CurrentMuzz;
struct MuzzState
{
   int                            sensors[Muzz::NUM_SENSORS];
   int                            response;
   int                            x, z;
   int                            dir;
   BlockTerrain::Block::DIRECTION forward;
   BlockTerrain::Block::DIRECTION backward;
   int                            lookAtX, lookAtZ;
   int                            moveType;
   GLfloat                        moveAmount;
};
vector<struct MuzzState> MuzzStates;

// Move a muzz.
bool moveMuzz(int muzzIndex);

// Sense muzz world.
void senseMuzz(int muzzIndex);

void senseMuzzDir(int muzzIndex, int direction, GLfloat view[3],
                  int &senseX, int &senseZ, int &terrain, int &object);

// Run a muzz.
#define INVALID_RESPONSE    (-100)
void runMuzz(int muzzIndex, int forcedResponse = INVALID_RESPONSE);

// Place muzz in world without changing initial placement.
void placeMuzz(int muzzIndex, int placeX, int placeY,
               BlockTerrain::Block::DIRECTION placeDir);

// Terrain.
// See blockTerrain.hpp for terrain generation parameters.
typedef enum
{
   STANDARD_TERRAIN = 0, TMAZE_TERRAIN = 1
}
TERRAIN_TYPE;
TERRAIN_TYPE TerrainType            = STANDARD_TERRAIN;
int          TerrainDimension       = DEFAULT_TERRAIN_DIMENSION;
BlockTerrain *Terrain               = NULL;
TmazeTerrain *MazeTerrain           = NULL;

// Terrain view/movement.
#define TERRAIN_FRUSTUM_ANGLE          60.0f
#define TERRAIN_FRUSTUM_NEAR           0.01f
#define TERRAIN_FRUSTUM_FAR            10.0f
#define TERRAIN_MOVEMENT_DELTA         0.01f
#define TERRAIN_INITIAL_VIEW_HEIGHT    0.5f
GLfloat TerrainViewPosition[3];

// Mushrooms.
int                NUM_MUSHROOMS    = DEFAULT_NUM_MUSHROOMS;
vector<Mushroom *> Mushrooms;
GLfloat            MushroomColor[3] = { 1.0f, 1.0f, 0.0f };

// Water pools.
int            NUM_POOLS = DEFAULT_NUM_POOLS;
vector<Pool *> Pools;
GLfloat        PoolColor[3] = { 0.0f, 0.0f, 1.0f };

// Training.
int NumTrainingTrials   = -1;
int CurrentTrial        = 0;
int CurrentTrialStep    = 0;
int TrialResetDelay     = -1;
int TrainingTrialResume = -1;

// Return runMuzzWorld when all muzzes have gotten food and water.
bool RunMuzzWorldUntilGoalsGotten = false;

// Forced response training.
bool ForcedResponseTrain = false;
vector<struct SensoryResponse> ForcedResponseSequence;
enum
{
   INIT_INDEX=(-1), ERROR_INDEX=(-2)
};
int ResponseIdx = INIT_INDEX;

// Get response sequence to obtain food and water.
struct SensoryResponse
{
   vector<int> sensors;
   int         response;
   int         x, y, dir;
};
typedef enum
{
   BREADTH_SEARCH = 0, DEPTH_SEARCH = 1
}
SEARCH_TYPE;
bool getResponseSequenceToGoals(int muzzIndex,
                                vector<struct SensoryResponse> &responseSequence,
                                SEARCH_TYPE, int maxSearchDepth = (-1));

// Camera.
Camera camera;

// Frame rate management.
#define TARGET_FRAME_RATE    100.0f
FrameRate frameRate(TARGET_FRAME_RATE);

/*
 *  Available fonts:
 *  GLUT_BITMAP_8_BY_13
 *  GLUT_BITMAP_9_BY_15
 *  GLUT_BITMAP_TIMES_ROMAN_10
 *  GLUT_BITMAP_TIMES_ROMAN_24
 *  GLUT_BITMAP_HELVETICA_10
 *  GLUT_BITMAP_HELVETICA_12
 *  GLUT_BITMAP_HELVETICA_18
 */
#define FONT          GLUT_BITMAP_9_BY_15
#define LINE_SPACE    15

// Modes.
typedef enum
{
   MANUAL_MODE = 0, TERRAIN_MODE = 1, HELP_MODE = 2
}
MODE;
MODE Mode               = TERRAIN_MODE;
bool TerrainMode        = true;
bool WireView           = false;
bool Pause              = false;
bool Step               = false;
int  ManualResponse     = INVALID_RESPONSE;

// 2D functions.
void helpInfo(int viewport), drawPartitions();
void enter2Dmode(), exit2Dmode();
void draw2Dstring(GLfloat x, GLfloat y, void *font, char *string);
void enter2DMode(int width, int height), exit2DMode();

// Save muzz world to file.
bool saveMuzzWorld(char *saveFile);

// Muzz control help.
char *MuzzControlHelp[] =
{
   (char *)"Checks:",
   (char *)"  Manual : \"Drive\" selected muzz",
   (char *)"   Pause : Pause world",
   (char *)"",
   (char *)"Buttons:",
   (char *)"   Reset : Reset world to initial state",
   (char *)"    Step : Single-step world",
   (char *)"    Help : Show help",
   (char *)"    Quit : Terminate",
   (char *)"",
   (char *)"Keys:",
   (char *)"    Up arrow : Move forward",
   (char *)"  Left arrow : Turn left",
   (char *)" Right arrow : Turn right",
   (char *)"           e : Eat",
   (char *)"           d : Drink",
   (char *)"           p : Print selected muzz brain",
   (char *)"           s : Save world to file",
   (char *)"                 \"muzz.world\"",
   (char *)"           f : Full screen",
   NULL
};

// Terrain view control help.
char *TerrainViewControlHelp[] =
{
   (char *)"    Up arrow : Scroll camera up",
   (char *)"  Down arrow : Scroll camera down",
   (char *)"  Left arrow : Scroll camera left",
   (char *)" Right arrow : Scroll camera right",
   (char *)"  PgUp arrow : Zoom camera out",
   (char *)"PgDown arrow : Zoom camera in",
   (char *)"  Left mouse : Select/deselect muzz",
   (char *)"               and select view pane",
   NULL
};

// GUI components.
class EventsHandler : public GUIEventListener
{
public:
   virtual void actionPerformed(GUIEvent& evt);
};
EventsHandler handler;
FPSCounter    counter;
GUICheckBox   *manualCheck;
GUICheckBox   *pauseCheck;
GUIFrame      *guiFrame = NULL;

// Annotate graph?
bool AnnotateGraph = true;

// Run muzzes.
void runMuzzes()
{
   int  i;
   bool moveDone, gotGoals, foundGoals;

   // Complete movements for previous responses before proceeding.
   moveDone = true;
   for (i = 0; i < NUM_MUZZES; i++)
   {
      if (!moveMuzz(i))
      {
         moveDone = false;
      }
   }
   if (!moveDone)
   {
      return;
   }

   // Determine sensory input for muzzes.
   for (i = 0; i < NUM_MUZZES; i++)
   {
      senseMuzz(i);
   }

   // Manual mode?
   if (Mode == MANUAL_MODE)
   {
      // Only current muzz can run.
      if (CurrentMuzz != -1)
      {
         // Manual response provided?
         if (ManualResponse != INVALID_RESPONSE)
         {
            // Override muzz with manual response.
            Muzzes[CurrentMuzz]->brain->responseOverride = ManualResponse;
            runMuzz(CurrentMuzz);
            Muzzes[CurrentMuzz]->brain->responseOverride = Mona::NULL_RESPONSE;

            // Wait for next manual response.
            ManualResponse = INVALID_RESPONSE;
         }

         // Reset training when leaving manual mode.
         if (CurrentTrial < NumTrainingTrials)
         {
            TrainingTrialResume = CurrentTrial;
         }
      }
      return;
   }

   // Resume training?
   if (TrainingTrialResume != -1)
   {
      i = TrainingTrialResume;
      resetTraining();
      resetMuzzWorld();
      CurrentTrial        = i;
      TrainingTrialResume = -1;
      if (CurrentTrial > NumTrainingTrials)
      {
         CurrentTrial = NumTrainingTrials;
      }
   }

   // One-time search for forced response training? Much faster.
   if (ForcedResponseTrain && (ResponseIdx == INIT_INDEX) &&
       (CurrentTrial < NumTrainingTrials))
   {
      if (getResponseSequenceToGoals(0, ForcedResponseSequence, DEPTH_SEARCH))
      {
         ResponseIdx = 0;
      }
      else
      {
         ResponseIdx = ERROR_INDEX;
      }
   }

   // Muzzes already have goals?
   gotGoals = true;
   for (i = 0; i < NUM_MUZZES; i++)
   {
      if (!Muzzes[i]->gotFood || !Muzzes[i]->gotWater)
      {
         gotGoals = false;
      }
   }

   // Run the muzzes.
   if (gotGoals)
   {
      foundGoals = false;
   }
   else
   {
      foundGoals = true;
   }
   gotGoals = true;
   for (i = 0; i < NUM_MUZZES; i++)
   {
      // For forced response training, get response to obtain food and water.
      if (ForcedResponseTrain && (CurrentTrial < NumTrainingTrials))
      {
         if ((ResponseIdx >= 0) && (ResponseIdx <
                                    (int)ForcedResponseSequence.size()))
         {
            Muzzes[i]->brain->responseOverride =
               ForcedResponseSequence[ResponseIdx].response;
            ResponseIdx++;
         }
         else
         {
            Muzzes[i]->brain->responseOverride = Mona::NULL_RESPONSE;
         }
      }

      // Run the muzz.
      runMuzz(i);

      // Goals not gotten?
      if (!Muzzes[i]->gotFood || !Muzzes[i]->gotWater)
      {
         gotGoals = foundGoals = false;
      }
   }

   // Increment training trial.
   if (CurrentTrial < NumTrainingTrials)
   {
      // Increment trial step.
      CurrentTrialStep++;

      // Trial complete?
      if (gotGoals)
      {
         if (TrialResetDelay == -1)
         {
            TrialResetDelay = 1;
         }
         else
         {
            TrialResetDelay--;
         }
      }
      if (TrialResetDelay == 0)
      {
         // Reset for next trial.
         TrialResetDelay = -1;
         CurrentTrial++;
         CurrentTrialStep = 0;
         if (ForcedResponseTrain && (ResponseIdx >= 0))
         {
            ResponseIdx = 0;
         }
         resetMuzzWorld();
      }
   }
}


// Idle muzzes.
void idleMuzzes()
{
   // Complete movements for previous responses.
   for (int i = 0; i < NUM_MUZZES; i++)
   {
      moveMuzz(i);
   }
}


// Reset muzz world.
void resetMuzzWorld()
{
   int i;

   for (i = 0; i < NUM_MUZZES; i++)
   {
      Muzzes[i]->brain->responseOverride = Mona::NULL_RESPONSE;
      Muzzes[i]->reset();
      if (NUM_MUSHROOMS == 0)
      {
         Muzzes[i]->clearNeed(Muzz::FOOD);
      }
      if (NUM_POOLS == 0)
      {
         Muzzes[i]->clearNeed(Muzz::WATER);
      }
      MuzzStates[i].response   = Muzz::WAIT;
      MuzzStates[i].moveType   = Muzz::FORWARD;
      MuzzStates[i].moveAmount = 0.0f;
   }
   for (i = 0; i < NUM_MUSHROOMS; i++)
   {
      Mushrooms[i]->setAlive(true);
   }
}


// Move muzz.
// Returns true when movement is complete.
bool moveMuzz(int i)
{
   int     j, x, z, x2, z2, x3, z3;
   GLfloat a, p[3], p2[3], f[3], u[3];
   bool    move;

   // Continue queued movement.
   if (MuzzStates[i].moveAmount > 0.0)
   {
      move = true;
   }
   else
   {
      move = false;
   }
   Muzzes[i]->getPosition(p);
   x = (int)(p[0] / Terrain->BLOCK_SIZE);
   z = (int)(p[2] / Terrain->BLOCK_SIZE);
   if (move)
   {
      switch (MuzzStates[i].moveType)
      {
      case Muzz::FORWARD:
         if (SmoothMove)
         {
            a = MUZZ_MOVEMENT_DELTA * frameRate.speedFactor;
            if (a > MuzzStates[i].moveAmount)
            {
               a = MuzzStates[i].moveAmount;
            }
         }
         else
         {
            a = MuzzStates[i].moveAmount;
         }
         MuzzStates[i].moveAmount -= a;
         Muzzes[i]->forward(a);

         // Check for collision.
         Muzzes[i]->getPosition(p);
         x2 = (int)(p[0] / Terrain->BLOCK_SIZE);
         z2 = (int)(p[2] / Terrain->BLOCK_SIZE);
         if ((x != x2) || (z != z2))
         {
            for (j = 0; j < NUM_MUZZES; j++)
            {
               if (j == i)
               {
                  continue;
               }
               Muzzes[j]->getPosition(p2);
               x3 = (int)(p2[0] / Terrain->BLOCK_SIZE);
               z3 = (int)(p2[2] / Terrain->BLOCK_SIZE);
               if ((x2 == x3) && (z2 == z3))
               {
                  Muzzes[i]->backward(a);
                  MuzzStates[i].moveAmount = 0.0f;
                  break;
               }
            }
         }
         break;

      case Muzz::RIGHT:
         if (SmoothMove)
         {
            a = MUZZ_TURN_DELTA * frameRate.speedFactor;
            if (a > MuzzStates[i].moveAmount)
            {
               a = MuzzStates[i].moveAmount;
            }
         }
         else
         {
            a = MuzzStates[i].moveAmount;
         }
         MuzzStates[i].moveAmount -= a;
         Muzzes[i]->right(a);
         break;

      case Muzz::LEFT:
         if (SmoothMove)
         {
            a = MUZZ_TURN_DELTA * frameRate.speedFactor;
            if (a > MuzzStates[i].moveAmount)
            {
               a = MuzzStates[i].moveAmount;
            }
         }
         else
         {
            a = MuzzStates[i].moveAmount;
         }
         MuzzStates[i].moveAmount -= a;
         Muzzes[i]->left(a);
         break;
      }
   }

   // Continue movement?
   if (MuzzStates[i].moveAmount > 0.0f)
   {
      return(false);
   }
   MuzzStates[i].moveAmount = 0.0f;

   // Center muzz on block, orient orthogonally and
   // store position and direction.
   Muzzes[i]->getPosition(p);
   x = (int)(p[0] / Terrain->BLOCK_SIZE);
   z = (int)(p[2] / Terrain->BLOCK_SIZE);
   if (move)
   {
      p2[0] = ((GLfloat)x + 0.5f) * Terrain->BLOCK_SIZE;
      p2[1] = p[1];
      p2[2] = ((GLfloat)z + 0.5f) * Terrain->BLOCK_SIZE;
      for (j = 0; j < 3; j++)
      {
         p2[j] = p[j] + ((p2[j] - p[j]) * 0.1f);
      }
      Muzzes[i]->setPosition(p2);
   }
   Muzzes[i]->getForward(f);
   f[1] = 0.0f;
   cSpacial::normalize(f);
   u[0] = 0.0f;
   u[1] = 1.0f;
   u[2] = 0.0f;
   if (fabs(f[0]) > fabs(f[2]))
   {
      if (f[0] < 0.0f)
      {
         MuzzStates[i].dir      = 3;
         MuzzStates[i].forward  = BlockTerrain::Block::WEST;
         MuzzStates[i].backward = BlockTerrain::Block::EAST;
         if (move)
         {
            Muzzes[i]->loadRotation(90.0f, u);
            Muzzes[i]->forward(0.0f);
         }
      }
      else
      {
         MuzzStates[i].dir      = 1;
         MuzzStates[i].forward  = BlockTerrain::Block::EAST;
         MuzzStates[i].backward = BlockTerrain::Block::WEST;
         if (move)
         {
            Muzzes[i]->loadRotation(270.0f, u);
            Muzzes[i]->forward(0.0f);
         }
      }
   }
   else
   {
      if (f[2] < 0.0f)
      {
         MuzzStates[i].dir      = 0;
         MuzzStates[i].forward  = BlockTerrain::Block::NORTH;
         MuzzStates[i].backward = BlockTerrain::Block::SOUTH;
         if (move)
         {
            Muzzes[i]->loadRotation(180.0f, u);
            Muzzes[i]->forward(0.0f);
         }
      }
      else
      {
         MuzzStates[i].dir      = 2;
         MuzzStates[i].forward  = BlockTerrain::Block::SOUTH;
         MuzzStates[i].backward = BlockTerrain::Block::NORTH;
         if (move)
         {
            Muzzes[i]->loadRotation(0.0f, u);
            Muzzes[i]->forward(0.0f);
         }
      }
   }
   Muzzes[i]->getPosition(p);
   MuzzStates[i].x = (int)(p[0] / Terrain->BLOCK_SIZE);
   MuzzStates[i].z = (int)(p[2] / Terrain->BLOCK_SIZE);

   return(true);
}


// Sense muzz world.
void senseMuzz(int i)
{
   int     terrain, object, lookx, lookz;
   GLfloat view[3];

   // Determine sensory input.
   Muzzes[i]->getRight(view);
   for (int j = 0; j < 3; j++)
   {
      view[j] = -view[j];
   }
   senseMuzzDir(i, (MuzzStates[i].dir + 1) % 4, view, lookx, lookz, terrain, object);
   if ((terrain == Muzz::WALL) || (terrain == Muzz::DROP) ||
       ((object == Muzz::MUSHROOM) && Muzzes[i]->gotFood) ||
       (object == Muzz::MUZZ))
   {
      MuzzStates[i].sensors[Muzz::RIGHT_SENSOR] = Muzz::CLOSED;
   }
   else
   {
      MuzzStates[i].sensors[Muzz::RIGHT_SENSOR] = Muzz::OPEN;
   }
   Muzzes[i]->getRight(view);
   senseMuzzDir(i, (MuzzStates[i].dir + 3) % 4, view, lookx, lookz, terrain, object);
   if ((terrain == Muzz::WALL) || (terrain == Muzz::DROP) ||
       ((object == Muzz::MUSHROOM) && Muzzes[i]->gotFood) ||
       (object == Muzz::MUZZ))
   {
      MuzzStates[i].sensors[Muzz::LEFT_SENSOR] = Muzz::CLOSED;
   }
   else
   {
      MuzzStates[i].sensors[Muzz::LEFT_SENSOR] = Muzz::OPEN;
   }
   Muzzes[i]->getForward(view);
   senseMuzzDir(i, MuzzStates[i].dir, view, lookx, lookz, terrain, object);
   MuzzStates[i].sensors[Muzz::TERRAIN_SENSOR] = terrain;
   MuzzStates[i].sensors[Muzz::OBJECT_SENSOR]  = object;
   if ((terrain == Muzz::WALL) || (terrain == Muzz::DROP) ||
       ((object == Muzz::MUSHROOM) && Muzzes[i]->gotFood) ||
       (object == Muzz::MUZZ))
   {
      MuzzStates[i].sensors[Muzz::FORWARD_SENSOR] = Muzz::CLOSED;
   }
   else
   {
      MuzzStates[i].sensors[Muzz::FORWARD_SENSOR] = Muzz::OPEN;
   }
   MuzzStates[i].lookAtX = lookx;
   MuzzStates[i].lookAtZ = lookz;
}


// Sense muzz world in a given direction and view vector.
void senseMuzzDir(int muzzIndex, int direction, GLfloat view[3],
                  int& senseX, int& senseZ, int& terrain, int& object)
{
   int     i, h, x, z, x2, z2, x3, z3;
   GLfloat p[3], p2[3], v[3];
   BlockTerrain::Block::DIRECTION forward, backward;

   Muzzes[muzzIndex]->getPosition(p);
   x = (int)(p[0] / Terrain->BLOCK_SIZE);
   z = (int)(p[2] / Terrain->BLOCK_SIZE);
   switch (direction)
   {
   case 0:                                        // North.
      if (z == 0)
      {
         terrain = Muzz::DROP;
         object  = Muzz::EMPTY;
         return;
      }
      else
      {
         senseX = x2 = x;
         senseZ = z2 = z - 1;
      }
      forward  = BlockTerrain::Block::NORTH;
      backward = BlockTerrain::Block::SOUTH;
      break;

   case 1:                                        // East.
      if (x == Terrain->WIDTH - 1)
      {
         terrain = Muzz::DROP;
         object  = Muzz::EMPTY;
         return;
      }
      else
      {
         senseX = x2 = x + 1;
         senseZ = z2 = z;
      }
      forward  = BlockTerrain::Block::EAST;
      backward = BlockTerrain::Block::WEST;
      break;

   case 2:                                        // South.
      if (z == Terrain->HEIGHT - 1)
      {
         terrain = Muzz::DROP;
         object  = Muzz::EMPTY;
         return;
      }
      else
      {
         senseX = x2 = x;
         senseZ = z2 = z + 1;
      }
      forward  = BlockTerrain::Block::SOUTH;
      backward = BlockTerrain::Block::NORTH;
      break;

   case 3:                                        // West.
      if (x == 0)
      {
         terrain = Muzz::DROP;
         object  = Muzz::EMPTY;
         return;
      }
      else
      {
         senseX = x2 = x - 1;
         senseZ = z2 = z;
      }
      forward  = BlockTerrain::Block::WEST;
      backward = BlockTerrain::Block::EAST;
      break;
   }

   // Object in view?
   // Cannot see objects when crosswise on ramp.
   if ((Terrain->blocks[x][z].type != BlockTerrain::Block::RAMP) ||
       ((Terrain->blocks[x][z].type == BlockTerrain::Block::RAMP) &&
        ((Terrain->blocks[x][z].rampDir == forward) ||
         (Terrain->blocks[x][z].rampDir == backward))))
   {
      // Determine point in view.
      x3    = (int)(p[0] / Terrain->BLOCK_SIZE);
      p2[0] = ((GLfloat)x3 + 0.5f) * Terrain->BLOCK_SIZE;
      p2[1] = p[1];
      z3    = (int)(p[2] / Terrain->BLOCK_SIZE);
      p2[2] = ((GLfloat)z3 + 0.5f) * Terrain->BLOCK_SIZE;
      for (i = 0; i < 3; i++)
      {
         v[i] = view[i];
      }
      cSpacial::normalize(v);
      for (i = 0; i < 3; i++)
      {
         p[i] = p2[i] + (v[i] * Terrain->BLOCK_SIZE);
      }
      for (i = 0; i < NUM_MUZZES; i++)
      {
         if (i == muzzIndex)
         {
            continue;
         }
         Muzzes[i]->getPosition(p2);
         if (fabs(cSpacial::pointDistance(p, p2)) < (Terrain->BLOCK_SIZE * 0.4f))
         {
            x3 = (int)(p2[0] / Terrain->BLOCK_SIZE);
            z3 = (int)(p2[2] / Terrain->BLOCK_SIZE);
            if (Terrain->blocks[x3][z3].type != BlockTerrain::Block::RAMP)
            {
               terrain = Muzz::PLATFORM;
            }
            else
            {
               if (Terrain->blocks[x3][z3].rampDir == forward)
               {
                  terrain = Muzz::RAMP_UP;
               }
               else
               {
                  terrain = Muzz::RAMP_DOWN;
               }
            }
            object = Muzz::MUZZ;
            return;
         }
      }

      for (i = 0; i < NUM_MUSHROOMS; i++)
      {
         if (!Mushrooms[i]->isAlive())
         {
            continue;
         }
         Mushrooms[i]->getPosition(p2);
         if (fabs(cSpacial::pointDistance(p, p2)) < (Terrain->BLOCK_SIZE * 0.4f))
         {
            terrain = Muzz::PLATFORM;
            object  = Muzz::MUSHROOM;
            return;
         }
      }

      for (i = 0; i < NUM_POOLS; i++)
      {
         Pools[i]->getPosition(p2);
         if (fabs(cSpacial::pointDistance(p, p2)) < (Terrain->BLOCK_SIZE * 0.4f))
         {
            terrain = Muzz::PLATFORM;
            object  = Muzz::POOL;
            return;
         }
      }
   }

   // View terrain.
   h = Terrain->blocks[x2][z2].elevation - Terrain->blocks[x][z].elevation;
   switch (Terrain->blocks[x][z].type)
   {
   case BlockTerrain::Block::PLATFORM:
      switch (Terrain->blocks[x2][z2].type)
      {
      case BlockTerrain::Block::PLATFORM:
      case BlockTerrain::Block::LANDING:
         if (h < 0)
         {
            terrain = Muzz::DROP;
            object  = Muzz::EMPTY;
         }
         else if (h > 0)
         {
            terrain = Muzz::WALL;
#if (SENSE_BLOCK_ID == 1)
            object = (Terrain->blocks[x2][z2].id << 8) |
                     (unsigned char )((int)'A' + Terrain->blocks[x2][z2].textureIndexes[Terrain->blocks[x][z].elevation + 1]);
#else
            object = (int)'A' + Terrain->blocks[x2][z2].textureIndexes[Terrain->blocks[x][z].elevation + 1];
#endif
         }
         else
         {
            terrain = Muzz::PLATFORM;
#if (SENSE_BLOCK_ID == 1)
            object = (Terrain->blocks[x2][z2].id << 8) |
                     (unsigned char )((int)'A' + Terrain->blocks[x2][z2].textureIndexes[Terrain->blocks[x][z].elevation]);
#else
            object = (int)'A' + Terrain->blocks[x2][z2].textureIndexes[Terrain->blocks[x][z].elevation];
#endif
         }
         break;

      case BlockTerrain::Block::RAMP:
         if (h < 0)
         {
            terrain = Muzz::DROP;
            object  = Muzz::EMPTY;
         }
         else if (h > 0)
         {
            terrain = Muzz::WALL;
            object  = Muzz::EMPTY;
         }
         else
         {
            if (Terrain->isUpperRamp(x2, z2))
            {
               terrain = Muzz::DROP;
               object  = Muzz::EMPTY;
            }
            else
            {
               terrain = Muzz::WALL;
               object  = Muzz::EMPTY;
            }
         }
         break;
      }
      break;

   case BlockTerrain::Block::LANDING:
      switch (Terrain->blocks[x2][z2].type)
      {
      case BlockTerrain::Block::PLATFORM:
      case BlockTerrain::Block::LANDING:
         if (h < 0)
         {
            terrain = Muzz::DROP;
            object  = Muzz::EMPTY;
         }
         else if (h > 0)
         {
            terrain = Muzz::WALL;
#if (SENSE_BLOCK_ID == 1)
            object = (Terrain->blocks[x2][z2].id << 8) |
                     (unsigned char )((int)'A' + Terrain->blocks[x2][z2].textureIndexes[Terrain->blocks[x][z].elevation + 1]);
#else
            object = (int)'A' + Terrain->blocks[x2][z2].textureIndexes[Terrain->blocks[x][z].elevation + 1];
#endif
         }
         else
         {
            terrain = Muzz::PLATFORM;
#if (SENSE_BLOCK_ID == 1)
            object = (Terrain->blocks[x2][z2].id << 8) |
                     (unsigned char )((int)'A' + Terrain->blocks[x2][z2].textureIndexes[Terrain->blocks[x][z].elevation]);
#else
            object = (int)'A' + Terrain->blocks[x2][z2].textureIndexes[Terrain->blocks[x][z].elevation];
#endif
         }
         break;

      case BlockTerrain::Block::RAMP:
         if (Terrain->blocks[x2][z2].rampDir == forward)
         {
            terrain = Muzz::RAMP_UP;
            object  = Muzz::EMPTY;
         }
         else if (Terrain->blocks[x2][z2].rampDir == backward)
         {
            terrain = Muzz::RAMP_DOWN;
            object  = Muzz::EMPTY;
         }
         else if (h < 0)
         {
            terrain = Muzz::DROP;
            object  = Muzz::EMPTY;
         }
         else if (h > 0)
         {
            terrain = Muzz::WALL;
            object  = Muzz::EMPTY;
         }
         else
         {
            if (Terrain->isUpperRamp(x2, z2))
            {
               terrain = Muzz::DROP;
               object  = Muzz::EMPTY;
            }
            else
            {
               terrain = Muzz::WALL;
               object  = Muzz::EMPTY;
            }
         }
         break;
      }
      break;

   case BlockTerrain::Block::RAMP:
      switch (Terrain->blocks[x2][z2].type)
      {
      case BlockTerrain::Block::PLATFORM:
         if (h < 0)
         {
            terrain = Muzz::DROP;
            object  = Muzz::EMPTY;
         }
         else if (h > 0)
         {
            terrain = Muzz::WALL;
            object  = Muzz::EMPTY;
         }
         else
         {
            if (Terrain->isUpperRamp(x, z))
            {
               terrain = Muzz::WALL;
               object  = Muzz::EMPTY;
            }
            else
            {
               terrain = Muzz::DROP;
               object  = Muzz::EMPTY;
            }
         }
         break;

      case BlockTerrain::Block::LANDING:
         if ((Terrain->blocks[x][z].rampDir == forward) ||
             (Terrain->blocks[x][z].rampDir == backward))
         {
            terrain = Muzz::PLATFORM;
#if (SENSE_BLOCK_ID == 1)
            object = (Terrain->blocks[x2][z2].id << 8) |
                     (unsigned char )((int)'A' + Terrain->blocks[x2][z2].textureIndexes[Terrain->blocks[x][z].elevation]);
#else
            object = (int)'A' + Terrain->blocks[x2][z2].textureIndexes[Terrain->blocks[x][z].elevation];
#endif
         }
         else if (h < 0)
         {
            terrain = Muzz::DROP;
            object  = Muzz::EMPTY;
         }
         else if (h > 0)
         {
            terrain = Muzz::WALL;
            object  = Muzz::EMPTY;
         }
         else
         {
            if (Terrain->isUpperRamp(x, z))
            {
               terrain = Muzz::WALL;
               object  = Muzz::EMPTY;
            }
            else
            {
               terrain = Muzz::DROP;
               object  = Muzz::EMPTY;
            }
         }
         break;

      case BlockTerrain::Block::RAMP:
         if ((Terrain->blocks[x][z].rampDir == forward) &&
             (Terrain->blocks[x2][z2].rampDir == forward))
         {
            terrain = Muzz::RAMP_UP;
            object  = Muzz::EMPTY;
         }
         else if ((Terrain->blocks[x][z].rampDir == backward) &&
                  (Terrain->blocks[x2][z2].rampDir == backward))
         {
            terrain = Muzz::RAMP_DOWN;
            object  = Muzz::EMPTY;
         }
         else if (h < 0)
         {
            terrain = Muzz::DROP;
            object  = Muzz::EMPTY;
         }
         else if (h > 0)
         {
            terrain = Muzz::WALL;
            object  = Muzz::EMPTY;
         }
         else
         {
            if (Terrain->isUpperRamp(x, z))
            {
               terrain = Muzz::DROP;
               object  = Muzz::EMPTY;
            }
            else if (Terrain->isUpperRamp(x2, z2))
            {
               terrain = Muzz::WALL;
               object  = Muzz::EMPTY;
            }
            else
            {
               terrain = Muzz::DROP;
               object  = Muzz::EMPTY;
            }
         }
         break;
      }
      break;
   }
}


// Run muzz.
void runMuzz(int i, int forcedResponse)
{
   int     j, dir, x, z, x2, z2, x3, z3;
   bool    hadFood;
   GLfloat p[3], p2[3], f[3];
   BlockTerrain::Block::DIRECTION forward;
   BlockTerrain::Block::DIRECTION backward;

   // Get muzz response.
   hadFood = Muzzes[i]->gotFood;
   if (forcedResponse == INVALID_RESPONSE)
   {
      // Let brain decide.
      MuzzStates[i].response = Muzzes[i]->cycle(MuzzStates[i].sensors);
   }
   else
   {
      // Force response.
      MuzzStates[i].response = forcedResponse;
      if ((MuzzStates[i].sensors[Muzz::OBJECT_SENSOR] == Muzz::MUSHROOM) &&
          (MuzzStates[i].response == Muzz::EAT))
      {
         Muzzes[i]->gotFood = true;
      }
      if ((MuzzStates[i].sensors[Muzz::OBJECT_SENSOR] == Muzz::POOL) &&
          (MuzzStates[i].response == Muzz::DRINK))
      {
         Muzzes[i]->gotWater = true;
      }
   }

   // Process response.
   x        = MuzzStates[i].x;
   z        = MuzzStates[i].z;
   dir      = MuzzStates[i].dir;
   forward  = MuzzStates[i].forward;
   backward = MuzzStates[i].backward;
   switch (MuzzStates[i].response)
   {
   case Muzz::WAIT:
      break;

   case Muzz::FORWARD:
      MuzzStates[i].moveType   = Muzz::FORWARD;
      MuzzStates[i].moveAmount = Terrain->BLOCK_SIZE;
      if ((MuzzStates[i].sensors[Muzz::OBJECT_SENSOR] == Muzz::MUSHROOM) ||
          (MuzzStates[i].sensors[Muzz::OBJECT_SENSOR] == Muzz::MUZZ))
      {
         MuzzStates[i].moveAmount = 0.0f;
         break;
      }
      switch (dir)
      {
      case 0:                                     // North.
         if (z == 0)
         {
            MuzzStates[i].moveAmount = 0.0f;
            return;
         }
         x2 = x;
         z2 = z - 1;
         break;

      case 1:                                     // East.
         if (x == Terrain->WIDTH - 1)
         {
            MuzzStates[i].moveAmount = 0.0f;
            return;
         }
         x2 = x + 1;
         z2 = z;
         break;

      case 2:                                     // South.
         if (z == Terrain->HEIGHT - 1)
         {
            MuzzStates[i].moveAmount = 0.0f;
            return;
         }
         x2 = x;
         z2 = z + 1;
         break;

      case 3:                                     // West.
         if (x == 0)
         {
            MuzzStates[i].moveAmount = 0.0f;
            return;
         }
         x2 = x - 1;
         z2 = z;
         break;
      }
      if (Terrain->blocks[x][z].type == BlockTerrain::Block::RAMP)
      {
         if ((Terrain->blocks[x][z].rampDir != forward) &&
             (Terrain->blocks[x][z].rampDir != backward))
         {
            MuzzStates[i].moveAmount = 0.0f;
            return;
         }
      }
      if (((Terrain->blocks[x][z].type != BlockTerrain::Block::RAMP) ||
           (Terrain->blocks[x2][z2].type != BlockTerrain::Block::RAMP)) &&
          (Terrain->blocks[x][z].elevation != Terrain->blocks[x2][z2].elevation))
      {
         MuzzStates[i].moveAmount = 0.0f;
      }
      else if ((Terrain->blocks[x][z].type != BlockTerrain::Block::RAMP) &&
               (Terrain->blocks[x2][z2].type == BlockTerrain::Block::RAMP) &&
               (Terrain->blocks[x2][z2].rampDir != forward) &&
               (Terrain->blocks[x2][z2].rampDir != backward))
      {
         MuzzStates[i].moveAmount = 0.0f;
      }
      else
      {
         switch (Terrain->blocks[x][z].type)
         {
         case BlockTerrain::Block::LANDING:
            if (Terrain->blocks[x2][z2].type == BlockTerrain::Block::RAMP)
            {
               MuzzStates[i].moveAmount = (Terrain->BLOCK_SIZE * 0.5f) +
                                          (Terrain->BLOCK_SIZE * (2.236f / 4.0f));
            }
            break;

         case BlockTerrain::Block::RAMP:
            if (Terrain->blocks[x2][z2].type == BlockTerrain::Block::RAMP)
            {
               MuzzStates[i].moveAmount = Terrain->BLOCK_SIZE * (2.236f / 2.0f);
            }
            else if (Terrain->blocks[x2][z2].type == BlockTerrain::Block::LANDING)
            {
               MuzzStates[i].moveAmount = (Terrain->BLOCK_SIZE * 0.5f) +
                                          (Terrain->BLOCK_SIZE * (2.236f / 4.0f));
            }
            break;
         }
      }
      break;

   case Muzz::RIGHT:
      MuzzStates[i].moveType   = Muzz::RIGHT;
      MuzzStates[i].moveAmount = 90.0f;
      break;

   case Muzz::LEFT:
      MuzzStates[i].moveType   = Muzz::LEFT;
      MuzzStates[i].moveAmount = 90.0f;
      break;

   case Muzz::EAT:
      if (!hadFood && Muzzes[i]->gotFood &&
          (MuzzStates[i].sensors[Muzz::OBJECT_SENSOR] == Muzz::MUSHROOM))
      {
         Muzzes[i]->getPosition(p);
         x3    = (int)(p[0] / Terrain->BLOCK_SIZE);
         p2[0] = ((GLfloat)x3 + 0.5f) * Terrain->BLOCK_SIZE;
         p2[1] = p[1];
         z3    = (int)(p[2] / Terrain->BLOCK_SIZE);
         p2[2] = ((GLfloat)z3 + 0.5f) * Terrain->BLOCK_SIZE;
         Muzzes[i]->getForward(f);
         cSpacial::normalize(f);
         for (j = 0; j < 3; j++)
         {
            p[j] = p2[j] + (f[j] * Terrain->BLOCK_SIZE);
         }
         for (j = 0; j < NUM_MUSHROOMS; j++)
         {
            if (!Mushrooms[j]->isAlive())
            {
               continue;
            }
            Mushrooms[j]->getPosition(p2);
            if (fabs(cSpacial::pointDistance(p, p2)) < (Terrain->BLOCK_SIZE * 0.4f))
            {
               Mushrooms[j]->setAlive(false);
            }
         }
      }
      break;

   case Muzz::DRINK:
      break;
   }
}


// Place muzz in world without changing its initial stored placement.
void placeMuzz(int muzzIndex, int placeX, int placeZ, BlockTerrain::Block::DIRECTION placeDir)
{
   GLfloat p[3];
   Vector  n;

   // Clear pending movement.
   MuzzStates[muzzIndex].moveType   = Muzz::FORWARD;
   MuzzStates[muzzIndex].moveAmount = 0.0f;

   Muzzes[muzzIndex]->clearSpacial();
   p[0] = ((GLfloat)placeX * Terrain->BLOCK_SIZE) + (Terrain->BLOCK_SIZE * 0.5f);
   p[2] = ((GLfloat)placeZ * Terrain->BLOCK_SIZE) + (Terrain->BLOCK_SIZE * 0.5f);
   Terrain->getGeometry(p[0], p[2], p[1], n);
   Muzzes[muzzIndex]->setPosition(p);
   switch (placeDir)
   {
   case BlockTerrain::Block::NORTH:
      Muzzes[muzzIndex]->setYaw(0.0f);
      break;

   case BlockTerrain::Block::EAST:
      Muzzes[muzzIndex]->setYaw(90.0f);
      break;

   case BlockTerrain::Block::SOUTH:
      Muzzes[muzzIndex]->setYaw(180.0f);
      break;

   case BlockTerrain::Block::WEST:
      Muzzes[muzzIndex]->setYaw(270.0f);
      break;
   }
   Muzzes[muzzIndex]->forward(0.0f);
}


// Reset training.
void resetTraining()
{
   CurrentTrial        = 0;
   CurrentTrialStep    = 0;
   TrialResetDelay     = -1;
   TrainingTrialResume = -1;
   if (ResponseIdx > 0)
   {
      ResponseIdx = 0;
   }
}


// Draw world.
void drawWorld(int skipMuzz)
{
   int i;

   glColor3f(1.0f, 1.0f, 1.0f);
   if (WireView)
   {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   }
   else
   {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   }
   glPushMatrix();

   // Draw the terrain.
   Terrain->draw();

   // Draw muzzes.
   for (i = 0; i < NUM_MUZZES; i++)
   {
      if (i != skipMuzz)
      {
         // Name the muzz for selection purposes.
         glPushMatrix();
         glPushName(i + 1);
         Muzzes[i]->draw();
         glPopName();
         glPopMatrix();
      }
   }

   // Draw mushrooms.
   for (i = 0; i < NUM_MUSHROOMS; i++)
   {
      if (Mushrooms[i]->isAlive())
      {
         Mushrooms[i]->draw();
      }
   }

   // Draw water pools.
   for (i = 0; i < NUM_POOLS; i++)
   {
      Pools[i]->draw();
   }

   glPopMatrix();
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


// Display muzz view.
void displayMuzzView(void)
{
   if (ViewSelection == TERRAIN_VIEW_ONLY)
   {
      return;
   }

   // Clear transform matrix.
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   // Place camera at current muzz view point.
   if (CurrentMuzz != -1)
   {
      // Camera follows muzz.
      Muzzes[CurrentMuzz]->aimCamera(camera);

      // Set viewport and frustum.
      glViewport(Viewports[MUZZ_VIEWPORT].x, Viewports[MUZZ_VIEWPORT].y,
                 Viewports[MUZZ_VIEWPORT].width, Viewports[MUZZ_VIEWPORT].height);
      camera.setFrustum(MUZZ_FRUSTUM_ANGLE, Viewports[MUZZ_VIEWPORT].aspect,
                        MUZZ_FRUSTUM_NEAR, MUZZ_FRUSTUM_FAR);

      // Draw the world.
      drawWorld(CurrentMuzz);
   }
   else
   {
      // Set viewport.
      glViewport(Viewports[MUZZ_VIEWPORT].x, Viewports[MUZZ_VIEWPORT].y,
                 Viewports[MUZZ_VIEWPORT].width, Viewports[MUZZ_VIEWPORT].height);
   }

   // Label view.
   glLineWidth(2.0);
   enter2Dmode();
   draw2Dstring(5, 12, FONT, (char *)"Muzz");
   exit2Dmode();
}


// Display the terrain view.
void displayTerrainView(void)
{
   if (ViewSelection == MUZZ_VIEW_ONLY)
   {
      return;
   }

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   // Set viewport and frustum.
   glViewport(Viewports[TERRAIN_VIEWPORT].x, Viewports[TERRAIN_VIEWPORT].y,
              Viewports[TERRAIN_VIEWPORT].width, Viewports[TERRAIN_VIEWPORT].height);
   camera.setFrustum(TERRAIN_FRUSTUM_ANGLE, Viewports[TERRAIN_VIEWPORT].aspect,
                     MUZZ_FRUSTUM_NEAR, MUZZ_FRUSTUM_FAR);

   // Position camera.
   camera.clearSpacial();
   camera.setPosition(TerrainViewPosition);
   camera.setPitch(-90.0f);
   camera.setRoll(180.0f);
   camera.place();

   // Rendering to select a muzz?
   if (renderMode == GL_SELECT)
   {
      startPicking();
   }

   // Draw the world with a small perspective angle.
   glPushMatrix();
   glRotatef(20.0f, 1.0f, 0.0f, 1.0f);
   drawWorld(-1);

   if (renderMode == GL_SELECT)
   {
      stopPicking();
   }
   else
   {
      // Highlight current muzz.
      if (CurrentMuzz != -1)
      {
         Muzzes[CurrentMuzz]->highlight();
      }
   }
   glPopMatrix();

   // Label view.
   glLineWidth(2.0);
   enter2Dmode();
   draw2Dstring(5, 12, FONT, (char *)"Terrain");
   exit2Dmode();
}


// Display the controls view.
void displayControls(void)
{
   int   vw, vh;
   GLint viewport[4];
   char  buf[100];

   // Set viewport.
   glViewport(Viewports[CONTROLS_VIEWPORT].x, Viewports[CONTROLS_VIEWPORT].y,
              Viewports[CONTROLS_VIEWPORT].width, Viewports[CONTROLS_VIEWPORT].height);
   glGetIntegerv(GL_VIEWPORT, viewport);
   vw = viewport[2];
   vh = viewport[3];

   // Render the GUI.
   counter.markFrameStart();
   enter2DMode(guiFrame->getWidth(), guiFrame->getHeight());
   guiFrame->render(counter.getFrameInterval());
   counter.markFrameEnd();

   // Show frame rate.
   sprintf(buf, "FPS: %.2f", frameRate.FPS);
   draw2Dstring(5, 15, FONT, buf);

   // Show cycle?
   if (Cycles >= 0)
   {
      // Show training trial also?
      if ((NumTrainingTrials >= 0) && (CurrentTrial >= 0) &&
          (CurrentTrial <= NumTrainingTrials))
      {
         sprintf(buf, "Cycle=%d/%d  Training trial=%d/%d",
                 CycleCounter, Cycles, CurrentTrial, NumTrainingTrials);
         draw2Dstring(5, vh - 10, FONT, buf);
      }
      else
      {
         sprintf(buf, "Cycle=%d/%d", CycleCounter, Cycles);
         draw2Dstring(5, vh - 10, FONT, buf);
      }
   }
   else
   {
      // Show training trial?
      if ((NumTrainingTrials >= 0) && (CurrentTrial >= 0) &&
          (CurrentTrial <= NumTrainingTrials))
      {
         sprintf(buf, "Training trial=%d/%d",
                 CurrentTrial, NumTrainingTrials);
         draw2Dstring(5, vh - 10, FONT, buf);
      }
   }
   exit2DMode();
}


// Display the muzz status panel.
void displayMuzzStatus(void)
{
   int   vw, vh, xoff, yoff;
   GLint viewport[4];
   char  buf[50];

   // Set viewport.
   glViewport(Viewports[MUZZ_STATUS_VIEWPORT].x, Viewports[MUZZ_STATUS_VIEWPORT].y,
              Viewports[MUZZ_STATUS_VIEWPORT].width, Viewports[MUZZ_STATUS_VIEWPORT].height);

   enter2Dmode();
   if (CurrentMuzz != -1)
   {
      sprintf(buf, "Muzz %d status", Muzzes[CurrentMuzz]->id);
      draw2Dstring(5, 12, FONT, buf);
   }
   else
   {
      draw2Dstring(5, 12, FONT, (char *)"Muzz status");
   }
   glGetIntegerv(GL_VIEWPORT, viewport);
   vw = viewport[2];
   vh = viewport[3];

   // Show sensors.
   xoff = 5;
   yoff = vh / 3;
   draw2Dstring(xoff, yoff, FONT, (char *)"Sensors:");
   xoff += 5;
   yoff += 15;
   if (CurrentMuzz != -1)
   {
      if (MuzzStates[CurrentMuzz].sensors[Muzz::RIGHT_SENSOR] != Muzz::CLOSED)
      {
         if (MuzzStates[CurrentMuzz].sensors[Muzz::LEFT_SENSOR] != Muzz::CLOSED)
         {
            if (MuzzStates[CurrentMuzz].sensors[Muzz::FORWARD_SENSOR] != Muzz::CLOSED)
            {
               draw2Dstring(xoff, yoff, FONT, (char *)" open<-open->open");
            }
            else
            {
               draw2Dstring(xoff, yoff, FONT, (char *)" open<-closed->open");
            }
         }
         else
         {
            if (MuzzStates[CurrentMuzz].sensors[Muzz::FORWARD_SENSOR] != Muzz::CLOSED)
            {
               draw2Dstring(xoff, yoff, FONT, (char *)" closed<-open->open");
            }
            else
            {
               draw2Dstring(xoff, yoff, FONT, (char *)" closed<-closed->open");
            }
         }
      }
      else
      {
         if (MuzzStates[CurrentMuzz].sensors[Muzz::LEFT_SENSOR] != Muzz::CLOSED)
         {
            if (MuzzStates[CurrentMuzz].sensors[Muzz::FORWARD_SENSOR] != Muzz::CLOSED)
            {
               draw2Dstring(xoff, yoff, FONT, (char *)" open<-open->closed");
            }
            else
            {
               draw2Dstring(xoff, yoff, FONT, (char *)" open<-closed->closed");
            }
         }
         else
         {
            if (MuzzStates[CurrentMuzz].sensors[Muzz::FORWARD_SENSOR] != Muzz::CLOSED)
            {
               draw2Dstring(xoff, yoff, FONT, (char *)" closed<-open->closed");
            }
            else
            {
               draw2Dstring(xoff, yoff, FONT, (char *)" closed<-closed->closed");
            }
         }
      }
      yoff += 15;
      switch (MuzzStates[CurrentMuzz].sensors[Muzz::TERRAIN_SENSOR])
      {
      case Muzz::PLATFORM:
         draw2Dstring(xoff, yoff, FONT, (char *)" Terrain=platform");
         break;

      case Muzz::WALL:
         draw2Dstring(xoff, yoff, FONT, (char *)" Terrain=wall");
         break;

      case Muzz::DROP:
         draw2Dstring(xoff, yoff, FONT, (char *)" Terrain=drop off");
         break;

      case Muzz::RAMP_UP:
         draw2Dstring(xoff, yoff, FONT, (char *)" Terrain=ramp up");
         break;

      case Muzz::RAMP_DOWN:
         draw2Dstring(xoff, yoff, FONT, (char *)" Terrain=ramp down");
         break;
      }
      yoff += 15;
      switch (MuzzStates[CurrentMuzz].sensors[Muzz::OBJECT_SENSOR])
      {
      case Muzz::MUSHROOM:
         draw2Dstring(xoff, yoff, FONT, (char *)"  Object=mushroom");
         break;

      case Muzz::POOL:
         draw2Dstring(xoff, yoff, FONT, (char *)"  Object=pool");
         break;

      case Muzz::MUZZ:
         draw2Dstring(xoff, yoff, FONT, (char *)"  Object=muzz");
         break;

      case Muzz::EMPTY:
         draw2Dstring(xoff, yoff, FONT, (char *)"  Object=empty");
         break;

      default:
#if (SENSE_BLOCK_ID == 1)
         if ((char)(MuzzStates[CurrentMuzz].sensors[Muzz::OBJECT_SENSOR] & 0xff) == '[')
         {
            sprintf(buf, "  Object=blank(%d)", MuzzStates[CurrentMuzz].sensors[Muzz::OBJECT_SENSOR]);
         }
         else
         {
            sprintf(buf, "  Object=%c(%d)", (char)(MuzzStates[CurrentMuzz].sensors[Muzz::OBJECT_SENSOR] & 0xff),
                    MuzzStates[CurrentMuzz].sensors[Muzz::OBJECT_SENSOR]);
         }
#else
         if ((char)MuzzStates[CurrentMuzz].sensors[Muzz::OBJECT_SENSOR] == '[')
         {
            sprintf(buf, "  Object=blank");
         }
         else
         {
            sprintf(buf, "  Object=%c", MuzzStates[CurrentMuzz].sensors[Muzz::OBJECT_SENSOR]);
         }
#endif
         draw2Dstring(xoff, yoff, FONT, buf);
         break;
      }
   }
   else
   {
      draw2Dstring(xoff, yoff, FONT, (char *)" NA<-NA->NA");
      yoff += 15;
      draw2Dstring(xoff, yoff, FONT, (char *)" Terrain=NA");
      yoff += 15;
      draw2Dstring(xoff, yoff, FONT, (char *)"  Object=NA");
   }

   // Show response.
   xoff = (vw / 2) + 15;
   yoff = vh / 4;
   if (CurrentMuzz != -1)
   {
      switch (MuzzStates[CurrentMuzz].response)
      {
      case Muzz::WAIT:
         draw2Dstring(xoff, yoff, FONT, (char *)"Response=wait");
         break;

      case Muzz::FORWARD:
         draw2Dstring(xoff, yoff, FONT, (char *)"Response=forward");
         break;

      case Muzz::RIGHT:
         draw2Dstring(xoff, yoff, FONT, (char *)"Response=right");
         break;

      case Muzz::LEFT:
         draw2Dstring(xoff, yoff, FONT, (char *)"Response=left");
         break;

      case Muzz::EAT:
         draw2Dstring(xoff, yoff, FONT, (char *)"Response=eat");
         break;

      case Muzz::DRINK:
         draw2Dstring(xoff, yoff, FONT, (char *)"Response=drink");
         break;
      }
   }
   else
   {
      draw2Dstring(xoff, yoff, FONT, (char *)"Response=NA");
   }

   // Show needs.
   yoff += 15;
   draw2Dstring(xoff, yoff, FONT, (char *)"Needs:");
   xoff += 5;
   yoff += 15;
   if (CurrentMuzz != -1)
   {
      sprintf(buf, " Water=%.2f", Muzzes[CurrentMuzz]->getNeed(Muzz::WATER));
      draw2Dstring(xoff, yoff, FONT, buf);
      yoff += 15;
      sprintf(buf, "  Food=%.2f", Muzzes[CurrentMuzz]->getNeed(Muzz::FOOD));
      draw2Dstring(xoff, yoff, FONT, buf);
   }
   else
   {
      draw2Dstring(xoff, yoff, FONT, (char *)" Water=NA");
      yoff += 15;
      draw2Dstring(xoff, yoff, FONT, (char *)"  Food=NA");
   }
   exit2Dmode();
}


// Display muzz help view.
void displayMuzzHelpView(void)
{
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glViewport(Viewports[MUZZ_HELP_VIEWPORT].x, Viewports[MUZZ_HELP_VIEWPORT].y,
              Viewports[MUZZ_HELP_VIEWPORT].width, Viewports[MUZZ_HELP_VIEWPORT].height);
   glLineWidth(2.0);
   enter2Dmode();
   helpInfo(MUZZ_HELP_VIEWPORT);
   draw2Dstring(5, 12, FONT, (char *)"Muzz");
   exit2Dmode();
}


// Display the terrain help view.
void displayTerrainHelpView(void)
{
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glViewport(Viewports[TERRAIN_HELP_VIEWPORT].x, Viewports[TERRAIN_HELP_VIEWPORT].y,
              Viewports[TERRAIN_HELP_VIEWPORT].width, Viewports[TERRAIN_HELP_VIEWPORT].height);
   glLineWidth(2.0);
   enter2Dmode();
   helpInfo(TERRAIN_HELP_VIEWPORT);
   draw2Dstring(5, 12, FONT, (char *)"Terrain");
   exit2Dmode();
}


// Display function.
void display(void)
{
   // Clear display.
   glutSetWindow(MainWindow);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Normal rendering?
   if (renderMode == GL_RENDER)
   {
      // Display viewports.
      if (Mode == HELP_MODE)
      {
         displayMuzzHelpView();
         displayTerrainHelpView();
      }
      else
      {
         displayMuzzView();
         displayTerrainView();
      }
      displayControls();
      displayMuzzStatus();

      // Partition window.
      drawPartitions();

      glutSwapBuffers();
      glFlush();

      // Update frame rate.
      frameRate.update();
   }
   else
   {
      // Rendering for muzz selection.
      displayTerrainView();
   }
}


// Configure viewport dimensions.
void configureViewports()
{
   int i;

   switch (ViewSelection)
   {
   case VIEW_BOTH:
      i = TERRAIN_VIEWPORT;
      Viewports[i].x      = 0;
      Viewports[i].y      = GUIheight;
      Viewports[i].width  = WindowWidth / 2;
      Viewports[i].height = WindowHeight - GUIheight;
      if (Viewports[i].height > 0)
      {
         Viewports[i].aspect = (GLfloat)Viewports[i].width / (GLfloat)Viewports[i].height;
      }
      else
      {
         Viewports[i].aspect = 1.0;
      }
      i = MUZZ_VIEWPORT;
      Viewports[i].x      = WindowWidth / 2;
      Viewports[i].y      = GUIheight;
      Viewports[i].width  = WindowWidth / 2;
      Viewports[i].height = WindowHeight - GUIheight;
      if (Viewports[i].height > 0)
      {
         Viewports[i].aspect = (GLfloat)Viewports[i].width / (GLfloat)Viewports[i].height;
      }
      else
      {
         Viewports[i].aspect = 1.0;
      }
      break;

   case TERRAIN_VIEW_ONLY:
      i = TERRAIN_VIEWPORT;
      Viewports[i].x      = 0;
      Viewports[i].y      = GUIheight;
      Viewports[i].width  = WindowWidth;
      Viewports[i].height = WindowHeight - GUIheight;
      if (Viewports[i].height > 0)
      {
         Viewports[i].aspect = (GLfloat)Viewports[i].width / (GLfloat)Viewports[i].height;
      }
      else
      {
         Viewports[i].aspect = 1.0;
      }
      break;

   case MUZZ_VIEW_ONLY:
      i = MUZZ_VIEWPORT;
      Viewports[i].x      = 0;
      Viewports[i].y      = GUIheight;
      Viewports[i].width  = WindowWidth;
      Viewports[i].height = WindowHeight - GUIheight;
      if (Viewports[i].height > 0)
      {
         Viewports[i].aspect = (GLfloat)Viewports[i].width / (GLfloat)Viewports[i].height;
      }
      else
      {
         Viewports[i].aspect = 1.0;
      }
      break;
   }
   i = CONTROLS_VIEWPORT;
   Viewports[i].x      = 0;
   Viewports[i].y      = 0;
   Viewports[i].width  = WindowWidth / 2;
   Viewports[i].height = GUIheight;
   if (Viewports[i].height > 0)
   {
      Viewports[i].aspect = (GLfloat)Viewports[i].width / (GLfloat)Viewports[i].height;
   }
   else
   {
      Viewports[i].aspect = 1.0;
   }
   i = MUZZ_STATUS_VIEWPORT;
   Viewports[i].x      = WindowWidth / 2;
   Viewports[i].y      = 0;
   Viewports[i].width  = WindowWidth / 2;
   Viewports[i].height = GUIheight;
   if (Viewports[i].height > 0)
   {
      Viewports[i].aspect = (GLfloat)Viewports[i].width / (GLfloat)Viewports[i].height;
   }
   else
   {
      Viewports[i].aspect = 1.0;
   }
   i = TERRAIN_HELP_VIEWPORT;
   Viewports[i].x      = 0;
   Viewports[i].y      = GUIheight;
   Viewports[i].width  = WindowWidth / 2;
   Viewports[i].height = WindowHeight - GUIheight;
   if (Viewports[i].height > 0)
   {
      Viewports[i].aspect = (GLfloat)Viewports[i].width / (GLfloat)Viewports[i].height;
   }
   else
   {
      Viewports[i].aspect = 1.0;
   }
   i = MUZZ_HELP_VIEWPORT;
   Viewports[i].x      = WindowWidth / 2;
   Viewports[i].y      = GUIheight;
   Viewports[i].width  = WindowWidth / 2;
   Viewports[i].height = WindowHeight - GUIheight;
   if (Viewports[i].height > 0)
   {
      Viewports[i].aspect = (GLfloat)Viewports[i].width / (GLfloat)Viewports[i].height;
   }
   else
   {
      Viewports[i].aspect = 1.0;
   }

   // Set GUI frame dimensions.
   guiFrame->setDimensions(Viewports[CONTROLS_VIEWPORT].width,
                           Viewports[CONTROLS_VIEWPORT].height);
   guiFrame->forceUpdate(true);
}


// Reshape.
void reshape(int width, int height)
{
   // Hack to make sure window is what it is reported to be...
   static bool init = true;

   if (init)
   {
      init = false;
      glutReshapeWindow(width, height);
   }

   WindowWidth = width;
   if (height == 0)
   {
      height = 1;
   }
   WindowHeight = height;
   GUIheight    = WindowHeight / 8;
   configureViewports();
   glutPostRedisplay();
}


// Keyboard input.
#define DELETE_KEY       127
#define RETURN_KEY       13
#define BACKSPACE_KEY    8
void
keyboard(unsigned char key, int x, int y)
{
   // Void manual response.
   ManualResponse = INVALID_RESPONSE;

   // Toggle help mode.
   if ((Mode == HELP_MODE) && (key == ' '))
   {
      if (TerrainMode)
      {
         Mode = TERRAIN_MODE;
      }
      else
      {
         Mode = MANUAL_MODE;
      }
      frameRate.reset();
   }
   else
   {
      // Check GUI key event.
      guiFrame->checkKeyboardEvents(KeyEvent(key), KE_PRESSED);

      switch (key)
      {
      // Eat and drink responses.
      case 'e':
         ManualResponse = Muzz::EAT;
         break;

      case 'd':
         ManualResponse = Muzz::DRINK;
         break;

      // Dumps.
      case 'p':
         if (CurrentMuzz != -1)
         {
            Muzzes[CurrentMuzz]->printBrain();
         }
         else
         {
            printf("Please select a muzz\n");
         }
         break;

      case 'r':
         if (CurrentMuzz != -1)
         {
            Muzzes[CurrentMuzz]->printResponsePotentials();
         }
         else
         {
            printf("Please select a muzz\n");
         }
         break;

      case 's':
         saveMuzzWorld((char *)"muzz.world");
         break;

      // Toggle wireframe mode.
      case 'w':
         WireView = !WireView;
         break;

      // Full screen.
      case 'f':
         glutFullScreen();
         break;

      // User help.
      case 'h':
         Mode = HELP_MODE;
         break;

      // Quit.
      case 'q':
         termMuzzWorld();
         glutDestroyWindow(MainWindow);
         exit(0);
      }
   }

   // Re-display.
   glutPostRedisplay();
}


// Special keyboard input.
void
specialKeyboard(int key, int x, int y)
{
   switch (Mode)
   {
   case MANUAL_MODE:
      if ((CurrentMuzz == -1) ||
          (ManualResponse != INVALID_RESPONSE))
      {
         break;
      }
      switch (key)
      {
      case GLUT_KEY_UP:
         ManualResponse = Muzz::FORWARD;
         break;

      case GLUT_KEY_RIGHT:
         ManualResponse = Muzz::RIGHT;
         break;

      case GLUT_KEY_LEFT:
         ManualResponse = Muzz::LEFT;
         break;
      }
      break;

   case TERRAIN_MODE:
      switch (key)
      {
      case GLUT_KEY_UP:
         TerrainViewPosition[2] -= TERRAIN_MOVEMENT_DELTA;
         break;

      case GLUT_KEY_DOWN:
         TerrainViewPosition[2] += TERRAIN_MOVEMENT_DELTA;
         break;

      case GLUT_KEY_RIGHT:
         TerrainViewPosition[0] += TERRAIN_MOVEMENT_DELTA;
         break;

      case GLUT_KEY_LEFT:
         TerrainViewPosition[0] -= TERRAIN_MOVEMENT_DELTA;
         break;

      case GLUT_KEY_PAGE_DOWN:
         TerrainViewPosition[1] -= TERRAIN_MOVEMENT_DELTA;
         break;

      case GLUT_KEY_PAGE_UP:
         TerrainViewPosition[1] += TERRAIN_MOVEMENT_DELTA;
         break;
      }
      break;
   }

   // Re-display.
   glutPostRedisplay();
}


// Idle function.
void idle()
{
   // Check cycles.
   if ((Cycles >= 0) && (!Pause || Step) && (Mode == TERRAIN_MODE))
   {
      CycleCounter++;
      if (CycleCounter > Cycles)
      {
         termMuzzWorld();
         glutDestroyWindow(MainWindow);
         exit(0);
      }
   }

   // Check for completion of training trials.
   // Pause to allow manual test trial.
   static bool testPause = false;
   if (!testPause && (CurrentTrial == NumTrainingTrials))
   {
      Pause = true;
      pauseCheck->setChecked(true);
      testPause = true;
   }

   // Run muzzes.
   if (Mode != HELP_MODE)
   {
      bool smoothMoveSave = SmoothMove;
      if (Step)
      {
         SmoothMove = false;
         Pause      = false;
      }
      if (!Pause)
      {
         runMuzzes();
      }
      else
      {
         idleMuzzes();
      }
      SmoothMove = smoothMoveSave;
      if (Step)
      {
         Pause = true;
         pauseCheck->setChecked(true);
         Step = false;
      }
   }

   // Re-display.
   glutPostRedisplay();
}


// Mouse callbacks.
void mouseClicked(int button, int state, int x, int y)
{
   if (button != GLUT_LEFT_BUTTON)
   {
      return;
   }
   cursorX = x;
   cursorY = y;

   // Switch views and initiate muzz selection.
   if ((Mode != HELP_MODE) && (state == GLUT_DOWN))
   {
      int vy = WindowHeight - cursorY;
      PendingViewSelection = (VIEW_SELECTION)(-1);
      switch (ViewSelection)
      {
      case MUZZ_VIEW_ONLY:
         if (((cursorX >= Viewports[MUZZ_VIEWPORT].x) &&
              (cursorX < (Viewports[MUZZ_VIEWPORT].x + Viewports[MUZZ_VIEWPORT].width))) &&
             ((vy >= Viewports[MUZZ_VIEWPORT].y) &&
              (vy < (Viewports[MUZZ_VIEWPORT].y + Viewports[MUZZ_VIEWPORT].height))))
         {
            ViewSelection = VIEW_BOTH;
            configureViewports();
            glutPostRedisplay();
         }
         break;

      case TERRAIN_VIEW_ONLY:
         if (((cursorX >= Viewports[TERRAIN_VIEWPORT].x) &&
              (cursorX < (Viewports[TERRAIN_VIEWPORT].x + Viewports[TERRAIN_VIEWPORT].width))) &&
             ((vy >= Viewports[TERRAIN_VIEWPORT].y) &&
              (vy < (Viewports[TERRAIN_VIEWPORT].y + Viewports[TERRAIN_VIEWPORT].height))))
         {
            // Reconfigure viewports after picking render.
            PendingViewSelection = VIEW_BOTH;

            // Setup render to pick a muzz.
            renderMode = GL_SELECT;
         }
         break;

      case VIEW_BOTH:
         if (((cursorX >= Viewports[MUZZ_VIEWPORT].x) &&
              (cursorX < (Viewports[MUZZ_VIEWPORT].x + Viewports[MUZZ_VIEWPORT].width))) &&
             ((vy >= Viewports[MUZZ_VIEWPORT].y) &&
              (vy < (Viewports[MUZZ_VIEWPORT].y + Viewports[MUZZ_VIEWPORT].height))))
         {
            ViewSelection = MUZZ_VIEW_ONLY;
            configureViewports();
            glutPostRedisplay();
         }
         if (((cursorX >= Viewports[TERRAIN_VIEWPORT].x) &&
              (cursorX < (Viewports[TERRAIN_VIEWPORT].x + Viewports[TERRAIN_VIEWPORT].width))) &&
             ((vy >= Viewports[TERRAIN_VIEWPORT].y) &&
              (vy < (Viewports[TERRAIN_VIEWPORT].y + Viewports[TERRAIN_VIEWPORT].height))))
         {
            // Reconfigure viewports after picking render.
            PendingViewSelection = TERRAIN_VIEW_ONLY;

            // Setup render to pick a muzz.
            renderMode = GL_SELECT;
         }
         break;
      }
   }

   // Adjust for GUI viewport.
   x -= Viewports[CONTROLS_VIEWPORT].x;
   y -= (WindowHeight - Viewports[CONTROLS_VIEWPORT].height);
   MouseEvent event = MouseEvent(MB_BUTTON1, x, y, guiFrame->getHeight() - y);
   guiFrame->checkMouseEvents(event, (state == GLUT_DOWN) ? ME_CLICKED : ME_RELEASED);
}


void mouseDragged(int x, int y)
{
   x -= Viewports[CONTROLS_VIEWPORT].x;
   y -= (WindowHeight - Viewports[CONTROLS_VIEWPORT].height);
   MouseEvent event = MouseEvent(MB_UNKNOWN_BUTTON, x, y, guiFrame->getHeight() - y);
   guiFrame->checkMouseEvents(event, ME_DRAGGED);
}


void mouseMoved(int x, int y)
{
   x -= Viewports[CONTROLS_VIEWPORT].x;
   y -= (WindowHeight - Viewports[CONTROLS_VIEWPORT].height);
   MouseEvent event = MouseEvent(MB_UNKNOWN_BUTTON, x, y, guiFrame->getHeight() - y);
   guiFrame->checkMouseEvents(event, ME_MOVED);
}


// GUI event handler.
void EventsHandler::actionPerformed(GUIEvent& evt)
{
   const std::string& callbackString   = evt.getCallbackString();
   GUIRectangle       *sourceRectangle = evt.getEventSource(),
   *parent        = sourceRectangle ? sourceRectangle->getParent() : NULL;
   int widgetType = sourceRectangle->getWidgetType();

   if (widgetType == WT_CHECK_BOX)
   {
      GUICheckBox *checkbox = (GUICheckBox *)sourceRectangle;

      // Void manual response.
      ManualResponse = INVALID_RESPONSE;

      // Muzz/terrain mode?
      if (callbackString == "manual")
      {
         TerrainMode = !checkbox->isChecked();
         if (!TerrainMode && (CurrentMuzz == -1))
         {
            TerrainMode = true;
            checkbox->setChecked(false);
         }
         if (Mode != HELP_MODE)
         {
            if (TerrainMode)
            {
               Mode = TERRAIN_MODE;
            }
            else
            {
               Mode = MANUAL_MODE;
            }
         }
      }

      if (callbackString == "pause")
      {
         Pause = checkbox->isChecked();
      }
   }

   if (widgetType == WT_BUTTON)
   {
      GUIButton *button = (GUIButton *)sourceRectangle;

      // Void manual response.
      ManualResponse = INVALID_RESPONSE;

      // Reset world?
      if (callbackString == "reset")
      {
         if (button->isClicked())
         {
            if (NumTrainingTrials >= 0)
            {
               resetTraining();
            }
            resetMuzzWorld();
            CycleCounter = 0;
         }
      }

      if (callbackString == "step")
      {
         if (button->isClicked())
         {
            if (Mode == TERRAIN_MODE)
            {
               Step = true;
            }
         }
      }

      if (callbackString == "help")
      {
         if (button->isClicked())
         {
            if (Mode != HELP_MODE)
            {
               Mode = HELP_MODE;
            }
            else
            {
               if (TerrainMode)
               {
                  Mode = TERRAIN_MODE;
               }
               else
               {
                  Mode = MANUAL_MODE;
               }
               frameRate.reset();
            }
         }
      }

      if (callbackString == "exit")
      {
         if (button->isClicked())
         {
            termMuzzWorld();
            glutDestroyWindow(MainWindow);
            exit(0);
         }
      }
   }
}


// Picking.
void startPicking()
{
   GLint viewport[4];

   glGetIntegerv(GL_VIEWPORT, viewport);
   glSelectBuffer(BUFSIZE, selectBuf);
   glRenderMode(GL_SELECT);
   glInitNames();
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   gluPickMatrix((GLdouble)cursorX, (GLdouble)(WindowHeight - cursorY), 5.0, 5.0, viewport);
   gluPerspective(TERRAIN_FRUSTUM_ANGLE, Viewports[TERRAIN_VIEWPORT].aspect,
                  MUZZ_FRUSTUM_NEAR, MUZZ_FRUSTUM_FAR);
   glMatrixMode(GL_MODELVIEW);
}


void processHits(GLint hits, GLuint buffer[], int sw)
{
   GLint  i, numberOfNames;
   GLuint names, *ptr, minZ, *ptrNames;

   numberOfNames = 0;
   ptr           = (GLuint *)buffer;
   minZ          = 0xffffffff;
   for (i = 0; i < hits; i++)
   {
      names = *ptr;
      ptr++;
      if (*ptr < minZ)
      {
         numberOfNames = names;
         minZ          = *ptr;
         ptrNames      = ptr + 2;
      }

      ptr += names + 2;
   }
   if (numberOfNames > 0)
   {
      ptr = ptrNames;
      i   = *ptr - 1;
      if (CurrentMuzz != i)
      {
         CurrentMuzz = i;
         MuzzStates[i].moveAmount = 0.0f;
      }
      else
      {
         MuzzStates[i].moveAmount = 0.0f;
         CurrentMuzz = -1;
         manualCheck->setChecked(false);
         TerrainMode = true;
         if (Mode == MANUAL_MODE)
         {
            Mode = TERRAIN_MODE;
         }
      }
      ManualResponse = INVALID_RESPONSE;

      // Mouse click was used for picking so cancel pending view selection.
      PendingViewSelection = (VIEW_SELECTION)(-1);
   }
}


void stopPicking()
{
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glFlush();
   pickHits = glRenderMode(GL_RENDER);
   if (pickHits != 0)
   {
      processHits(pickHits, selectBuf, 0);
   }
   renderMode = GL_RENDER;

   // Change to pending view if picking did not occur.
   if (PendingViewSelection != (VIEW_SELECTION)(-1))
   {
      ViewSelection        = PendingViewSelection;
      PendingViewSelection = (VIEW_SELECTION)(-1);
      configureViewports();
      glutPostRedisplay();
   }
}


// Help for controls.
void helpInfo(int viewport)
{
   int i, v;

   glColor3f(1.0f, 1.0f, 1.0f);
   v = 30;
   draw2Dstring(5, v, FONT, (char *)"Controls:");
   v += (2 * LINE_SPACE);
   if (viewport == MUZZ_HELP_VIEWPORT)
   {
      for (i = 0; MuzzControlHelp[i] != NULL; i++)
      {
         draw2Dstring(5, v, FONT, MuzzControlHelp[i]);
         v += LINE_SPACE;
      }
   }
   else if (viewport == TERRAIN_HELP_VIEWPORT)
   {
      for (i = 0; TerrainViewControlHelp[i] != NULL; i++)
      {
         draw2Dstring(5, v, FONT, TerrainViewControlHelp[i]);
         v += LINE_SPACE;
      }
   }
   v += LINE_SPACE;
   draw2Dstring(5, v, FONT, (char *)"Press space bar to continue...");
}


// Draw window partitions.
void drawPartitions()
{
   glViewport(0, 0, WindowWidth, WindowHeight);
   glLineWidth(2.0);
   enter2Dmode();

   glBegin(GL_LINES);
   if ((Mode == HELP_MODE) || (ViewSelection == VIEW_BOTH))
   {
      glVertex2f(Viewports[MUZZ_HELP_VIEWPORT].width, 0);
      glVertex2f(Viewports[MUZZ_HELP_VIEWPORT].width, Viewports[MUZZ_HELP_VIEWPORT].height);
   }
   glVertex2f(0, Viewports[MUZZ_HELP_VIEWPORT].height);
   glVertex2f(WindowWidth, Viewports[MUZZ_HELP_VIEWPORT].height);
   glVertex2f(Viewports[MUZZ_STATUS_VIEWPORT].x, Viewports[MUZZ_HELP_VIEWPORT].height);
   glVertex2f(Viewports[MUZZ_STATUS_VIEWPORT].x, WindowHeight);
   glEnd();

   exit2Dmode();
   glLineWidth(1.0);
}


void enter2Dmode()
{
   GLint viewport[4];

   glColor3f(1.0, 1.0, 1.0);
   glDisable(GL_BLEND);
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_LIGHTING);

   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   glGetIntegerv(GL_VIEWPORT, viewport);
   gluOrtho2D(0, viewport[2], 0, viewport[3]);

   // Invert the y axis, down is positive.
   glScalef(1, -1, 1);

   // Move the origin from the bottom left corner to the upper left corner.
   glTranslatef(0, -viewport[3], 0);

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
}


void exit2Dmode()
{
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
}


// Print string on screen at specified location.
void draw2Dstring(GLfloat x, GLfloat y, void *font, char *string)
{
   char *c;

   glRasterPos2f(x, y);
   for (c = string; *c != '\0'; c++)
   {
      glutBitmapCharacter(font, *c);
   }
}


// GUI 2D mode.
void enter2DMode(GLint winWidth, GLint winHeight)
{
   Tuple4i viewport;

   if ((winWidth <= 0) || (winHeight <= 0))
   {
      glGetIntegerv(GL_VIEWPORT, viewport);
      winWidth  = viewport.z;
      winHeight = viewport.w;
   }

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   gluOrtho2D(0, winWidth, winHeight, 0);
   glDisable(GL_DEPTH_TEST);
}


void exit2DMode()
{
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
   glEnable(GL_DEPTH_TEST);
}


// Initialize.
void initMuzzWorld()
{
   int       i, j, k, x, z, x2, z2;
   float     color[3];
   FILE      *fp;
   Random    *objectRandomizer;
   GLfloat   p[3], p2[3];
   const int maxtries = 1000;

   // Initialize graphics (even if turned off).
   glutInitWindowSize(WindowWidth, WindowHeight);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
   MainWindow = glutCreateWindow("Muzz World");
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutSpecialFunc(specialKeyboard);
   glutIdleFunc(idle);
   glutMouseFunc(mouseClicked);
   glutMotionFunc(mouseDragged);
   glutPassiveMotionFunc(mouseMoved);
   glClearColor(0.0, 0.0, 0.0, 1.0);
   glEnable(GL_DEPTH_TEST);
   glShadeModel(GL_SMOOTH);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);
   glPointSize(3.0);
   glLineWidth(1.0);

   // Load?
   if (LoadFile != NULL)
   {
      if ((fp = FOPEN_READ(LoadFile)) == NULL)
      {
         fprintf(stderr, "Cannot load from file %s\n", LoadFile);
         glutDestroyWindow(MainWindow);
         exit(1);
      }
      FREAD_INT(&NUM_MUZZES, fp);
      FREAD_INT(&NUM_MUSHROOMS, fp);
      FREAD_INT(&NUM_POOLS, fp);
      FREAD_LONG(&RandomSeed, fp);
      Randomizer = new Random(RandomSeed);
      assert(Randomizer != NULL);
      Randomizer->RAND_LOAD(fp);
      FREAD_LONG(&ObjectSeed, fp);
      FREAD_INT(&i, fp);
      TerrainType = (TERRAIN_TYPE)i;
      FREAD_LONG(&TerrainSeed, fp);
      FREAD_INT(&TerrainDimension, fp);
   }
   else
   {
      if (RandomSeed == INVALID_RANDOM)
      {
         RandomSeed = (RANDOM)time(NULL);
         Randomizer = new Random(RandomSeed);
         assert(Randomizer != NULL);
      }
      if (ObjectSeed == INVALID_RANDOM)
      {
         ObjectSeed = RandomSeed;
      }
      if (TerrainSeed == INVALID_RANDOM)
      {
         TerrainSeed = RandomSeed;
      }
      fp = NULL;
   }

   // Create terrain.
   if (TerrainType == STANDARD_TERRAIN)
   {
      Terrain = new BlockTerrain(TerrainSeed, TerrainDimension, TerrainDimension,
                                 BlockTerrain::DEFAULT_MAX_PLATFORM_ELEVATION, BlockTerrain::DEFAULT_MIN_PLATFORM_SIZE,
                                 max((int)BlockTerrain::DEFAULT_MIN_PLATFORM_SIZE, TerrainDimension / 2),
                                 BlockTerrain::DEFAULT_MAX_PLATFORM_GENERATIONS, BlockTerrain::DEFAULT_EXTRA_RAMPS,
                                 BlockTerrain::DEFAULT_BLOCK_SIZE);
      assert(Terrain != NULL);
   }
   else
   {
      MazeTerrain = new TmazeTerrain(TerrainSeed, TerrainDimension, TerrainDimension,
                                     BlockTerrain::DEFAULT_BLOCK_SIZE);
      assert(MazeTerrain != NULL);
      Terrain = (BlockTerrain *)MazeTerrain;
   }
   TerrainViewPosition[0] = ((float)Terrain->WIDTH * Terrain->BLOCK_SIZE) / 2.0f;
   TerrainViewPosition[1] = TERRAIN_INITIAL_VIEW_HEIGHT;
   TerrainViewPosition[2] = ((float)Terrain->WIDTH * Terrain->BLOCK_SIZE) / 2.0f;

   // Create objects based on object seed.
   objectRandomizer = new Random(ObjectSeed);
   assert(objectRandomizer != NULL);

   // Create muzzes.
   Muzzes.resize(NUM_MUZZES);
   MuzzStates.resize(NUM_MUZZES);
   for (i = 0; i < NUM_MUZZES; i++)
   {
      for (k = 0; k < maxtries; k++)
      {
         for (j = 0; j < 3; j++)
         {
            color[j] = objectRandomizer->RAND_INTERVAL(0.0, 1.0);
         }
         Muzzes[i] = new Muzz(color, Terrain, objectRandomizer->RAND(), Randomizer);
         assert(Muzzes[i] != NULL);
         if (TerrainType == TMAZE_TERRAIN)
         {
            Muzzes[i]->place(MazeTerrain->TmazePath[0].first,
                             MazeTerrain->TmazePath[0].second, BlockTerrain::Block::NORTH);
         }
         if (fp != NULL)
         {
            Muzzes[i]->load(fp);
            break;
         }
         else
         {
            // Check for overlay.
            Muzzes[i]->getPosition(p);
            x = (int)(p[0] / Terrain->BLOCK_SIZE);
            z = (int)(p[2] / Terrain->BLOCK_SIZE);
            for (j = 0; j < i; j++)
            {
               Muzzes[j]->getPosition(p2);
               x2 = (int)(p2[0] / Terrain->BLOCK_SIZE);
               z2 = (int)(p2[2] / Terrain->BLOCK_SIZE);
               if ((x == x2) && (z == z2))
               {
                  break;
               }
            }
            if (j < i)
            {
               delete Muzzes[i];
               Muzzes[i] = NULL;
               continue;
            }
            else
            {
               break;
            }
         }
      }
      if (k == maxtries)
      {
         fprintf(stderr, "Cannot place muzz on terrain\n");
         glutDestroyWindow(MainWindow);
         exit(1);
      }
   }
   if (fp != NULL)
   {
      FREAD_INT(&CurrentMuzz, fp);
      for (i = 0; i < NUM_MUZZES; i++)
      {
         for (j = 0; j < Muzz::NUM_SENSORS; j++)
         {
            FREAD_INT(&MuzzStates[i].sensors[j], fp);
         }
         FREAD_INT(&MuzzStates[i].response, fp);
         FREAD_INT(&MuzzStates[i].x, fp);
         FREAD_INT(&MuzzStates[i].z, fp);
         FREAD_INT(&MuzzStates[i].dir, fp);
         FREAD_INT(&j, fp);
         MuzzStates[i].forward = (BlockTerrain::Block::DIRECTION)j;
         FREAD_INT(&j, fp);
         MuzzStates[i].backward = (BlockTerrain::Block::DIRECTION)j;
         FREAD_INT(&MuzzStates[i].lookAtX, fp);
         FREAD_INT(&MuzzStates[i].lookAtZ, fp);
         FREAD_INT(&MuzzStates[i].moveType, fp);
         FREAD_FLOAT(&MuzzStates[i].moveAmount, fp);
      }
   }
   else
   {
      if (NUM_MUZZES == 0)
      {
         CurrentMuzz = -1;
      }
      else
      {
         CurrentMuzz = 0;
      }
      for (i = 0; i < NUM_MUZZES; i++)
      {
         MuzzStates[i].response   = Muzz::WAIT;
         MuzzStates[i].moveType   = Muzz::FORWARD;
         MuzzStates[i].moveAmount = 0.0f;
      }
   }

   // Create mushrooms.
   Mushrooms.resize(NUM_MUSHROOMS);
   for (i = 0; i < NUM_MUSHROOMS; i++)
   {
      for (k = 0; k < maxtries; k++)
      {
         Mushrooms[i] = new Mushroom(MushroomColor, Terrain, objectRandomizer);
         assert(Mushrooms[i] != NULL);
         if (TerrainType == TMAZE_TERRAIN)
         {
            j = (int)MazeTerrain->TmazePath.size() - 1;
            Mushrooms[i]->place(MazeTerrain->TmazePath[j].first,
                                MazeTerrain->TmazePath[j].second);
         }
         if (fp != NULL)
         {
            Mushrooms[i]->load(fp);
            break;
         }
         else
         {
            // Check for overlay.
            Mushrooms[i]->getPosition(p);
            x = (int)(p[0] / Terrain->BLOCK_SIZE);
            z = (int)(p[2] / Terrain->BLOCK_SIZE);
            for (j = 0; j < NUM_MUZZES; j++)
            {
               Muzzes[j]->getPosition(p2);
               x2 = (int)(p2[0] / Terrain->BLOCK_SIZE);
               z2 = (int)(p2[2] / Terrain->BLOCK_SIZE);
               if ((x == x2) && (z == z2))
               {
                  break;
               }
            }
            if (j < NUM_MUZZES)
            {
               delete Mushrooms[i];
               Mushrooms[i] = NULL;
               continue;
            }
            for (j = 0; j < i; j++)
            {
               Mushrooms[j]->getPosition(p2);
               x2 = (int)(p2[0] / Terrain->BLOCK_SIZE);
               z2 = (int)(p2[2] / Terrain->BLOCK_SIZE);
               if ((x == x2) && (z == z2))
               {
                  break;
               }
            }
            if (j < i)
            {
               delete Mushrooms[i];
               Mushrooms[i] = NULL;
               continue;
            }
            else
            {
               break;
            }
         }
      }
      if (k == maxtries)
      {
         fprintf(stderr, "Cannot place mushroom on terrain\n");
         glutDestroyWindow(MainWindow);
         exit(1);
      }
   }

   // Create water pools.
   Pools.resize(NUM_POOLS);
   for (i = 0; i < NUM_POOLS; i++)
   {
      for (k = 0; k < maxtries; k++)
      {
         Pools[i] = new Pool(PoolColor, Terrain, objectRandomizer);
         assert(Pools[i] != NULL);
         if (fp != NULL)
         {
            Pools[i]->load(fp);
            break;
         }
         else
         {
            // Check for overlay.
            // OK to overlay with muzz.
            Pools[i]->getPosition(p);
            x = (int)(p[0] / Terrain->BLOCK_SIZE);
            z = (int)(p[2] / Terrain->BLOCK_SIZE);
            for (j = 0; j < NUM_MUSHROOMS; j++)
            {
               Mushrooms[j]->getPosition(p2);
               x2 = (int)(p2[0] / Terrain->BLOCK_SIZE);
               z2 = (int)(p2[2] / Terrain->BLOCK_SIZE);
               if ((x == x2) && (z == z2))
               {
                  break;
               }
            }
            if (j < NUM_MUSHROOMS)
            {
               delete Pools[i];
               Pools[i] = NULL;
               continue;
            }
            for (j = 0; j < i; j++)
            {
               Pools[j]->getPosition(p2);
               x2 = (int)(p2[0] / Terrain->BLOCK_SIZE);
               z2 = (int)(p2[2] / Terrain->BLOCK_SIZE);
               if ((x == x2) && (z == z2))
               {
                  break;
               }
            }
            if (j < i)
            {
               delete Pools[i];
               Pools[i] = NULL;
               continue;
            }
            else
            {
               break;
            }
         }
      }
      if (k == maxtries)
      {
         fprintf(stderr, "Cannot place pool on terrain\n");
         glutDestroyWindow(MainWindow);
         exit(1);
      }
   }
   delete objectRandomizer;

   // Load misc. items.
   if (fp != NULL)
   {
      FREAD_INT(&WindowWidth, fp);
      FREAD_INT(&WindowHeight, fp);
      GUIheight = WindowHeight / 8;
      FREAD_INT(&i, fp);
      ViewSelection = (VIEW_SELECTION)i;
      for (i = 0; i < 3; i++)
      {
         FREAD_FLOAT(&TerrainViewPosition[i], fp);
      }
      FREAD_INT(&i, fp);
      Mode = (MODE)i;
      FREAD_BOOL(&TerrainMode, fp);
      FREAD_BOOL(&WireView, fp);
      FCLOSE(fp);
      fp = NULL;
   }

   if (Graphics)
   {
      // Initialize camera.
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      camera.setPosition(TerrainViewPosition);
      camera.setPitch(-90.0f);

      // Initialize GUI.
      GLeeInit();
      char *path = getResourcePath((char *)"images/");
      assert(path != NULL);
      if (path == NULL)
      {
         fprintf(stderr, "Cannot find images\n");
         exit(1);
      }
      MediaPathManager::registerPath(path);
      free(path);
      path = getResourcePath((char *)"GUI/");
      assert(path != NULL);
      if (path == NULL)
      {
         fprintf(stderr, "Cannot find GUI files\n");
         exit(1);
      }
      MediaPathManager::registerPath(path);
      free(path);
      guiFrame = new GUIFrame();
      assert(guiFrame != NULL);
      if (!guiFrame->GUIPanel::loadXMLSettings("MuzzGUILayout.xml"))
      {
         fprintf(stderr, "Cannot load MuzzGUILayout.xml\n");
         exit(1);
      }
      guiFrame->setGUIEventListener(&handler);
      manualCheck = (GUICheckBox *)guiFrame->getWidgetByCallbackString("manual");
      manualCheck->setAlphaFadeScale(1000.0);
      if (!TerrainMode)
      {
         manualCheck->setChecked(true);
      }
      pauseCheck = (GUICheckBox *)guiFrame->getWidgetByCallbackString("pause");
      pauseCheck->setAlphaFadeScale(1000.0);
      if (Pause)
      {
         pauseCheck->setChecked(true);
      }

      // Configure viewports.
      glutReshapeWindow(WindowWidth, WindowHeight);
      configureViewports();
   }
}


// Run.
void runMuzzWorld()
{
   // Reset muzzes for training?
   if (NumTrainingTrials >= 0)
   {
      resetTraining();
      resetMuzzWorld();
   }

   CycleCounter = 0;
   if (Graphics)
   {
      // Main loop - does not return.
      frameRate.reset();
      glutMainLoop();
   }
   else
   {
      // Run muzzes.
      while (CycleCounter < Cycles)
      {
         // Return when all muzzes have goals?
         if (RunMuzzWorldUntilGoalsGotten)
         {
            int i;
            for (i = 0; i < NUM_MUZZES; i++)
            {
               if (Muzzes[i] != NULL)
               {
                  if (Muzzes[i]->gotFood && (Muzzes[i]->gotFoodCycle == -1))
                  {
                     Muzzes[i]->gotFoodCycle = CycleCounter;
                  }
                  if (Muzzes[i]->gotWater && (Muzzes[i]->gotWaterCycle == -1))
                  {
                     Muzzes[i]->gotWaterCycle = CycleCounter;
                  }
               }
            }
            for (i = 0; i < NUM_MUZZES; i++)
            {
               if (Muzzes[i] != NULL)
               {
                  if (!Muzzes[i]->gotWater || !Muzzes[i]->gotFood)
                  {
                     break;
                  }
               }
            }
            if (i == NUM_MUZZES)
            {
               return;
            }
         }

         // Return when training trials are completed?
         if (CurrentTrial == NumTrainingTrials)
         {
            return;
         }

         // Run a cycle.
         runMuzzes();

         CycleCounter++;
      }
   }
}


// Save muzz world.
bool saveMuzzWorld(char *saveFile)
{
   int  i, j;
   FILE *fp;

   if ((fp = FOPEN_WRITE(saveFile)) == NULL)
   {
      return(false);
   }
   FWRITE_INT(&NUM_MUZZES, fp);
   FWRITE_INT(&NUM_MUSHROOMS, fp);
   FWRITE_INT(&NUM_POOLS, fp);
   FWRITE_LONG(&RandomSeed, fp);
   Randomizer->RAND_SAVE(fp);
   FWRITE_LONG(&ObjectSeed, fp);
   i = (int)TerrainType;
   FWRITE_INT(&i, fp);
   FWRITE_LONG(&TerrainSeed, fp);
   FWRITE_INT(&TerrainDimension, fp);
   for (i = 0; i < NUM_MUZZES; i++)
   {
      Muzzes[i]->save(fp);
   }
   FWRITE_INT(&CurrentMuzz, fp);
   for (i = 0; i < NUM_MUZZES; i++)
   {
      for (j = 0; j < Muzz::NUM_SENSORS; j++)
      {
         FWRITE_INT(&MuzzStates[i].sensors[j], fp);
      }
      FWRITE_INT(&MuzzStates[i].response, fp);
      FWRITE_INT(&MuzzStates[i].x, fp);
      FWRITE_INT(&MuzzStates[i].z, fp);
      FWRITE_INT(&MuzzStates[i].dir, fp);
      j = (int)MuzzStates[i].forward;
      FWRITE_INT(&j, fp);
      j = (int)MuzzStates[i].backward;
      FWRITE_INT(&j, fp);
      FWRITE_INT(&MuzzStates[i].lookAtX, fp);
      FWRITE_INT(&MuzzStates[i].lookAtZ, fp);
      FWRITE_INT(&MuzzStates[i].moveType, fp);
      FWRITE_FLOAT(&MuzzStates[i].moveAmount, fp);
   }
   for (i = 0; i < NUM_MUSHROOMS; i++)
   {
      Mushrooms[i]->save(fp);
   }
   for (i = 0; i < NUM_POOLS; i++)
   {
      Pools[i]->save(fp);
   }
   FWRITE_INT(&WindowWidth, fp);
   FWRITE_INT(&WindowHeight, fp);
   i = (int)ViewSelection;
   FWRITE_INT(&i, fp);
   for (i = 0; i < 3; i++)
   {
      FWRITE_FLOAT(&TerrainViewPosition[i], fp);
   }
   i = (int)Mode;
   FWRITE_INT(&i, fp);
   FWRITE_BOOL(&TerrainMode, fp);
   FWRITE_BOOL(&WireView, fp);
   FCLOSE(fp);
   return(true);
}


// Terminate.
void termMuzzWorld()
{
   int i;

   // Save?
   if (SaveFile != NULL)
   {
      if (!saveMuzzWorld(SaveFile))
      {
         fprintf(stderr, "Cannot save to file %s\n", SaveFile);
         glutDestroyWindow(MainWindow);
         exit(1);
      }
   }

   // Release storage.
   for (i = 0; i < NUM_MUZZES; i++)
   {
      delete Muzzes[i];
      Muzzes[i] = NULL;
   }
   for (i = 0; i < NUM_MUSHROOMS; i++)
   {
      if (Mushrooms[i] != NULL)
      {
         delete Mushrooms[i];
         Mushrooms[i] = NULL;
      }
   }
   for (i = 0; i < NUM_POOLS; i++)
   {
      if (Pools[i] != NULL)
      {
         delete Pools[i];
         Pools[i] = NULL;
      }
   }
   if (MazeTerrain != NULL)
   {
      delete MazeTerrain;
      MazeTerrain = NULL;
      Terrain     = NULL;
   }
   if (Terrain != NULL)
   {
      delete Terrain;
      Terrain = NULL;
   }
   if (Randomizer != NULL)
   {
      delete Randomizer;
      Randomizer = NULL;
   }
   if (guiFrame != NULL)
   {
      delete guiFrame;
      guiFrame = NULL;
   }
   ForcedResponseSequence.clear();
}


// Response search state.
class ResponseSearch
{
public:
   struct SensoryResponse experience;
   ResponseSearch         *parent;
   int x, z, dir;
   BlockTerrain::Block::DIRECTION forward, backward;
   int          moveType;
   GLfloat      moveAmount;
   Muzz         *muzz;
   vector<bool> mushrooms;
   int          depth;
   int          goalDist;

   // Constructor.
   // "Lobotomize" muzz to save space.
   ResponseSearch(int response, ResponseSearch *parent,
                  Muzz *parentMuzz, int depth)
   {
      experience.response = response;
      this->parent        = parent;
      x          = z = dir = -1;
      forward    = backward = BlockTerrain::Block::NORTH;
      moveType   = Muzz::WAIT;
      moveAmount = 0.0f;
      muzz       = new Muzz(Terrain);
      assert(muzz != NULL);
      muzz->gotFood       = parentMuzz->gotFood;
      muzz->gotWater      = parentMuzz->gotWater;
      muzz->gotFoodCycle  = parentMuzz->gotFoodCycle;
      muzz->gotWaterCycle = parentMuzz->gotWaterCycle;
      delete muzz->m_spacial;
      muzz->m_spacial = parentMuzz->m_spacial->clone();
      assert(muzz->m_spacial != NULL);
      muzz->m_placePosition[0] = parentMuzz->m_placePosition[0];
      muzz->m_placePosition[1] = parentMuzz->m_placePosition[1];
      muzz->m_placePosition[2] = parentMuzz->m_placePosition[2];
      muzz->m_placeDirection   = parentMuzz->m_placeDirection;
      for (int i = 0; i < NUM_MUSHROOMS; i++)
      {
         mushrooms.push_back(false);
      }
      this->depth = depth;
      goalDist    = -1;
   }


   // Destructor.
   ~ResponseSearch()
   {
      delete muzz;
   }


   // Set city-block distance to closest needed goal.
   // Distance is zero when goals are achieved.
   // Return true if needed goals are available.
   bool setGoalDist()
   {
      int       i, x, z, x2, z2, d;
      GLfloat   p[3], p2[3];
      bool      blocked;
      const int maxDist = 10000;

      goalDist = -1;
      if (muzz->gotFood && muzz->gotWater)
      {
         goalDist = 0;
         return(true);
      }
      muzz->getPosition(p);
      x = (int)(p[0] / Terrain->BLOCK_SIZE);
      z = (int)(p[2] / Terrain->BLOCK_SIZE);
      if (!muzz->gotFood)
      {
         blocked = true;
         for (i = 0; i < NUM_MUSHROOMS; i++)
         {
            if (mushrooms[i])
            {
               blocked = false;
               Mushrooms[i]->getPosition(p2);
               x2 = (int)(p2[0] / Terrain->BLOCK_SIZE);
               z2 = (int)(p2[2] / Terrain->BLOCK_SIZE);
               d  = 0;
               if (x > x2) { d += x - x2; } else{ d += x2 - x; }
               if (z > z2) { d += z - z2; } else{ d += z2 - z; }
               if ((goalDist < 0) || (d < goalDist)) { goalDist = d; }
            }
         }
         if (blocked) { return(false); }
      }
      if (!muzz->gotWater)
      {
         if (NUM_POOLS == 0) { return(false); }
         for (i = 0; i < NUM_POOLS; i++)
         {
            Pools[i]->getPosition(p2);
            x2 = (int)(p2[0] / Terrain->BLOCK_SIZE);
            z2 = (int)(p2[2] / Terrain->BLOCK_SIZE);
            d  = 0;
            if (x > x2) { d += x - x2; } else{ d += x2 - x; }
            if (z > z2) { d += z - z2; } else{ d += z2 - z; }
            if ((goalDist < 0) || (d < goalDist)) { goalDist = d; }
         }
      }

      // Add distance representing unattained goals.
      if (!muzz->gotFood) { goalDist += maxDist; }
      if (!muzz->gotWater) { goalDist += maxDist; }

      return(true);
   }


   // State equal?
   bool equals(ResponseSearch *r)
   {
      if ((x == r->x) && (z == r->z) && (dir == r->dir) &&
          (moveType == r->moveType) &&
          (moveAmount == r->moveAmount) &&
          (muzz->gotFood == r->muzz->gotFood) &&
          (muzz->gotWater == r->muzz->gotWater))
      {
         return(true);
      }
      else
      {
         return(false);
      }
   }
};

// Less-than comparison for sort.
bool ltcmpSearch(ResponseSearch *a, ResponseSearch *b)
{
   if (a->goalDist < b->goalDist)
   {
      return(true);
   }
   else
   {
      return(false);
   }
}


// Get response sequence to obtain food and water.
// Breadth search is optimal, depth search is fast.
bool getResponseSequenceToGoals(int muzzIndex,
                                vector<struct SensoryResponse>& responseSequence, SEARCH_TYPE searchType,
                                int maxSearchDepth)
{
   int  i, j;
   bool ret, expand, unblocked;

   list<ResponseSearch *>           closed;
   list<ResponseSearch *>           open;
   list<ResponseSearch *>::iterator itr;
   ResponseSearch                         *current, *child;
   list<struct SensoryResponse>           responseList;
   list<struct SensoryResponse>::iterator responseItr;
   Muzz                   *saveMuzz     = Muzzes[muzzIndex];
   struct MuzzState       saveMuzzState = MuzzStates[muzzIndex];
   vector<bool>           saveMushrooms;
   struct SensoryResponse experience;
   GLfloat                p[3], f[3];

   // To restore muzz ID dispenser after search.
   int idDispenser = Muzz::idDispenser;

   // Create search space root.
   current = new ResponseSearch(INVALID_RESPONSE, NULL,
                                Muzzes[muzzIndex], 0);
   assert(current != NULL);
   Muzzes[muzzIndex]->getPosition(p);
   current->x = (int)(p[0] / Terrain->BLOCK_SIZE);
   current->z = (int)(p[2] / Terrain->BLOCK_SIZE);
   Muzzes[muzzIndex]->getForward(f);
   f[1] = 0.0f;
   cSpacial::normalize(f);
   if (fabs(f[0]) > fabs(f[2]))
   {
      if (f[0] < 0.0f)
      {
         current->dir      = 3;
         current->forward  = BlockTerrain::Block::WEST;
         current->backward = BlockTerrain::Block::EAST;
      }
      else
      {
         current->dir      = 1;
         current->forward  = BlockTerrain::Block::EAST;
         current->backward = BlockTerrain::Block::WEST;
      }
   }
   else
   {
      if (f[2] < 0.0f)
      {
         current->dir      = 0;
         current->forward  = BlockTerrain::Block::NORTH;
         current->backward = BlockTerrain::Block::SOUTH;
      }
      else
      {
         current->dir      = 2;
         current->forward  = BlockTerrain::Block::SOUTH;
         current->backward = BlockTerrain::Block::NORTH;
      }
   }
   current->moveType   = MuzzStates[muzzIndex].moveType;
   current->moveAmount = MuzzStates[muzzIndex].moveAmount;
   saveMushrooms.resize(NUM_MUSHROOMS);
   for (i = 0; i < NUM_MUSHROOMS; i++)
   {
      current->mushrooms[i] = saveMushrooms[i] = Mushrooms[i]->isAlive();
   }
   unblocked = current->setGoalDist();
   open.push_back(current);

   // Search for food and water.
   responseSequence.clear();
   ret = false;
   while (open.size() > 0 && unblocked)
   {
      itr     = open.begin();
      current = *itr;
      open.erase(itr);

      // Found food and water?
      if (current->muzz->gotFood && current->muzz->gotWater)
      {
         // Load response sequence.
         closed.push_back(current);
         ret = true;
         while (current->experience.response != INVALID_RESPONSE)
         {
            current->experience.x   = current->x;
            current->experience.y   = current->z;
            current->experience.dir = current->dir;
            responseList.push_front(current->experience);
            current = current->parent;
         }
         for (responseItr = responseList.begin();
              responseItr != responseList.end(); responseItr++)
         {
            responseSequence.push_back(*responseItr);
         }
         break;
      }

      // Expand children?
      if ((maxSearchDepth == -1) || (current->depth < maxSearchDepth))
      {
         expand = true;

         // Check for duplicate.
         for (itr = closed.begin(); itr != closed.end(); itr++)
         {
            if (current->equals(*itr))
            {
               expand = false;
               break;
            }
         }
      }
      else
      {
         expand = false;
      }
      closed.push_back(current);
      if (expand)
      {
         // Expand possible responses.
         for (i = 0; i < Muzz::NUM_RESPONSES; i++)
         {
            if (i == Muzz::WAIT)
            {
               continue;
            }
            child = new ResponseSearch(i, current, current->muzz,
                                       current->depth + 1);
            assert(child != NULL);
            open.push_back(child);
            MuzzStates[muzzIndex].moveType   = current->moveType;
            MuzzStates[muzzIndex].moveAmount = current->moveAmount;
            for (j = 0; j < NUM_MUSHROOMS; j++)
            {
               Mushrooms[j]->setAlive(current->mushrooms[j]);
            }
            Muzzes[muzzIndex] = child->muzz;
            moveMuzz(muzzIndex);
            senseMuzz(muzzIndex);
            runMuzz(muzzIndex, i);
            for (j = 0; j < saveMuzz->brain->numSensors; j++)
            {
               child->experience.sensors.push_back(MuzzStates[muzzIndex].sensors[j]);
            }
            child->x          = MuzzStates[muzzIndex].x;
            child->z          = MuzzStates[muzzIndex].z;
            child->dir        = MuzzStates[muzzIndex].dir;
            child->forward    = MuzzStates[muzzIndex].forward;
            child->backward   = MuzzStates[muzzIndex].backward;
            child->moveType   = MuzzStates[muzzIndex].moveType;
            child->moveAmount = MuzzStates[muzzIndex].moveAmount;
            for (j = 0; j < NUM_MUSHROOMS; j++)
            {
               child->mushrooms[j] = Mushrooms[j]->isAlive();
            }
            unblocked = child->setGoalDist();
         }

         if (searchType == DEPTH_SEARCH)
         {
            // Sort open list in ascending order of goal distance.
            open.sort(ltcmpSearch);
         }
      }
   }

   // Clean up.
   for (itr = closed.begin(); itr != closed.end(); itr++)
   {
      delete *itr;
   }
   closed.clear();
   for (itr = open.begin(); itr != open.end(); itr++)
   {
      delete *itr;
   }
   open.clear();

   Muzzes[muzzIndex]     = saveMuzz;
   MuzzStates[muzzIndex] = saveMuzzState;
   for (i = 0; i < NUM_MUSHROOMS; i++)
   {
      Mushrooms[i]->setAlive(saveMushrooms[i]);
   }

   // Pad response sequence with a "no-op".
   if (ret)
   {
      experience.response = Muzz::WAIT;
      responseSequence.push_back(experience);
   }

   // Restore muzz ID dispenser after search.
   Muzz::idDispenser = idDispenser;

   return(ret);
}


#ifndef MUZZ_WORLD_DRIVER
#ifdef WIN32
#ifdef _DEBUG
// For Windows memory checking, set CHECK_MEMORY = 1 and turn off graphics.
#define CHECK_MEMORY         0
#if (CHECK_MEMORY == 1)
#define MEMORY_CHECK_FILE    "memory.txt"
#include <crtdbg.h>
#endif
#endif
#endif

// Main.
int main(int argc, char *argv[])
{
   int i;

#if (CHECK_MEMORY == 1)
   {
#endif

#ifdef WIN32
   // Attach to parent console?
   if (_isatty(1))
   {
      for (i = 1; i < argc; i++)
      {
         if (strcmp(argv[i], "-attachConsole") == 0)
         {
            break;
         }
      }
      if (i < argc)
      {
         FreeConsole();
         if (AttachConsole(ATTACH_PARENT_PROCESS))
         {
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);
            freopen("CONIN$", "r", stdin);
         }
      }
   }
#endif

   // Process glut args.
   glutInit(&argc, argv);

   bool gotNumMuzzes        = false;
   bool gotNumMushrooms     = false;
   bool gotNumPools         = false;
   bool gotTerrainDimension = false;

   for (i = 1; i < argc; i++)
   {
      if (strcmp(argv[i], "-cycles") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         Cycles = atoi(argv[i]);
         if (Cycles < 0)
         {
            printUsage();
            exit(1);
         }
         continue;
      }

      if (strcmp(argv[i], "-initHunger") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         Muzz::INIT_HUNGER = atof(argv[i]);
         if ((Muzz::INIT_HUNGER < 0.0) || (Muzz::INIT_HUNGER > 1.0))
         {
            printUsage();
            exit(1);
         }
         continue;
      }

      if (strcmp(argv[i], "-initThirst") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         Muzz::INIT_THIRST = atof(argv[i]);
         if ((Muzz::INIT_THIRST < 0.0) || (Muzz::INIT_THIRST > 1.0))
         {
            printUsage();
            exit(1);
         }
         continue;
      }

      if (strcmp(argv[i], "-numMuzzes") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         NUM_MUZZES = atoi(argv[i]);
         if (NUM_MUZZES < 0)
         {
            printUsage();
            exit(1);
         }
         gotNumMuzzes = true;
         continue;
      }

      if (strcmp(argv[i], "-numMushrooms") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         NUM_MUSHROOMS = atoi(argv[i]);
         if (NUM_MUSHROOMS < 0)
         {
            printUsage();
            exit(1);
         }
         gotNumMushrooms = true;
         continue;
      }

      if (strcmp(argv[i], "-numPools") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         NUM_POOLS = atoi(argv[i]);
         if (NUM_POOLS < 0)
         {
            printUsage();
            exit(1);
         }
         gotNumPools = true;
         continue;
      }

      if (strcmp(argv[i], "-load") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         LoadFile = argv[i];
         continue;
      }

      if (strcmp(argv[i], "-save") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         SaveFile = argv[i];
         continue;
      }

      if (strcmp(argv[i], "-randomSeed") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         RandomSeed = (RANDOM)atoi(argv[i]);
         if ((Randomizer != NULL) || (RandomSeed == INVALID_RANDOM))
         {
            printUsage();
            exit(1);
         }
         Randomizer = new Random(RandomSeed);
         assert(Randomizer != NULL);
         continue;
      }

      if (strcmp(argv[i], "-objectSeed") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         ObjectSeed = (RANDOM)atoi(argv[i]);
         continue;
      }

      if (strcmp(argv[i], "-TmazeTerrain") == 0)
      {
         TerrainType = TMAZE_TERRAIN;
         continue;
      }

      if (strcmp(argv[i], "-terrainSeed") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         TerrainSeed = (RANDOM)atoi(argv[i]);
         continue;
      }

      if (strcmp(argv[i], "-terrainDimension") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         if ((TerrainDimension = atoi(argv[i])) < 2)
         {
            fprintf(stderr, "Invalid terrain dimension\n");
            printUsage();
            exit(1);
         }
         gotTerrainDimension = true;
         continue;
      }

      if (strcmp(argv[i], "-noGraphics") == 0)
      {
         Graphics = false;
         continue;
      }

      if (strcmp(argv[i], "-pause") == 0)
      {
         Pause = true;
         continue;
      }

      if (strcmp(argv[i], "-numTrainingTrials") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         if ((NumTrainingTrials = atoi(argv[i])) < 0)
         {
            fprintf(stderr, "Invalid number of training trials\n");
            printUsage();
            exit(1);
         }
         continue;
      }

      if (strcmp(argv[i], "-forcedResponseTrain") == 0)
      {
         ForcedResponseTrain = true;
         continue;
      }

#ifdef WIN32
      if (strcmp(argv[i], "-attachConsole") == 0)
      {
         continue;
      }
#endif

      if (strcmp(argv[i], "-version") == 0)
      {
         printVersion();
         Mona::printVersion();
         exit(0);
      }

      printUsage();
      exit(1);
   }

   if (!Graphics)
   {
      if (Pause)
      {
         fprintf(stderr, "Pause option invalid without graphics\n");
         printUsage();
         exit(1);
      }
      if (Cycles == -1)
      {
         fprintf(stderr, "Cycles option required when graphics is turned off\n");
         printUsage();
         exit(1);
      }
   }
#ifdef WIN32
#ifdef _DEBUG
#if (CHECK_MEMORY == 1)
   else
   {
      fprintf(stderr, "Graphics must be turned off for memory leak checking\n");
      exit(1);
   }
#endif
#endif
#endif

   if ((LoadFile != NULL) &&
       (gotNumMuzzes || gotNumMushrooms || gotNumPools))
   {
      fprintf(stderr, "Object quantities are loaded from file\n");
      printUsage();
      exit(1);
   }

   if ((LoadFile != NULL) &&
       ((RandomSeed != INVALID_RANDOM) ||
        (ObjectSeed != INVALID_RANDOM) ||
        (TerrainSeed != INVALID_RANDOM)))
   {
      fprintf(stderr, "Random seeds are loaded from file\n");
      printUsage();
      exit(1);
   }

   if ((LoadFile != NULL) &&
       (gotTerrainDimension || (TerrainType == TMAZE_TERRAIN)))
   {
      fprintf(stderr, "Terrain parameters are loaded from file\n");
      printUsage();
      exit(1);
   }

   if (TerrainType == TMAZE_TERRAIN)
   {
      if (NUM_MUZZES != 1)
      {
         fprintf(stderr, "numMuzzes must equal 1 for T-maze terrain\n");
         printUsage();
         exit(1);
      }
      if (NUM_MUSHROOMS != 1)
      {
         fprintf(stderr, "numMushrooms must equal 1 for T-maze terrain\n");
         printUsage();
         exit(1);
      }
      if (NUM_POOLS != 0)
      {
         fprintf(stderr, "numPools must equal 0 for T-maze terrain\n");
         printUsage();
         exit(1);
      }
   }

   if ((NumTrainingTrials >= 0) && (NUM_MUZZES != 1))
   {
      fprintf(stderr, "Number of muzzes must equal 1 for training\n");
      printUsage();
      exit(1);
   }

   if (ForcedResponseTrain && (NumTrainingTrials < 0))
   {
      fprintf(stderr, "Forced response training option invalid without number of training trials option\n");
      printUsage();
      exit(1);
   }

   // If graphics and not counting cycles and not training, smooth movements.
   if (Graphics && (Cycles < 0) && (NumTrainingTrials < 0))
   {
      SmoothMove = true;
   }
   else
   {
      SmoothMove = false;
   }

   // Initialize.
   initMuzzWorld();

   // Run.
   runMuzzWorld();

   // Terminate.
   termMuzzWorld();

#if (CHECK_MEMORY == 1)
}


// Check for memory leaks.
printf("Checking for memory leaks, report in file %s\n",
       MEMORY_CHECK_FILE);
HANDLE hFile = CreateFile(
   MEMORY_CHECK_FILE,
   GENERIC_WRITE,
   FILE_SHARE_WRITE,
   NULL,
   OPEN_ALWAYS,
   0,
   NULL
   );
if (hFile == INVALID_HANDLE_VALUE)
{
   fprintf(stderr, "Cannot open memory check file %s",
           MEMORY_CHECK_FILE);
   exit(1);
}
_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
_CrtSetReportFile(_CRT_WARN, hFile);
_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
_CrtSetReportFile(_CRT_ERROR, hFile);
if (!_CrtDumpMemoryLeaks())
{
   printf("No memory leaks\n");
}
else
{
   printf("Memory leaks found\n");
}
CloseHandle(hFile);
#endif

   return(0);
}
#endif
