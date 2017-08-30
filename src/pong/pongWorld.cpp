// For conditions of distribution and use, see copyright notice in pong.hpp

/*
 * The Pong game world.
 *
 * Mona senses the ball and paddle within a cell that is visible within a grid
 * spanning the game area. Mona can move about the game grid and move the paddle
 * one grid unit up or down. A game ends with either a score by the ball or a strike
 * by the paddle.
 */

#include "pong.hpp"
#include "../mona/mona.hpp"
#include <assert.h>
#ifndef WIN32
#include <unistd.h>
#endif

// Version (SCCS "what" format).
#define PONG_WORLD_VERSION    "@(#)Pong world version 1.0"
const char *PongWorldVersion = PONG_WORLD_VERSION;

// Usage.
char *Usage[] =
{
   (char *)"pong_world\n",
   (char *)"      -trainingRuns <number of training runs>\n",
   (char *)"      [-unforcedTrainingRunModulo <frequency of unforced training run = 1/modulo>]\n",
   (char *)"      [-recordTrainingPlayback <playback file name>]\n",
   (char *)"      [-trainingRandomSeed <generation seed>]\n",
   (char *)"      -testingRuns <number of test runs>\n",
   (char *)"      [-recordTestingPlayback <playback file name>]\n",
   (char *)"      [-testingRandomSeed <generation seed>]\n",
   (char *)"      [-gridDimensions <game grid dimensions (odd X odd)>]\n",
   (char *)"      [-startPosition <x> <y> (starting position on game grid)]\n",
   (char *)"      [-ballSpeed <grid cells/step>]\n",
   (char *)"      [-load <load file name>]\n",
   (char *)"      [-save <save file name>]\n",
   (char *)"      [-verbose]\n",
   (char *)"      [-createNuPICinput <NuPIC input file name>]*\n",
   (char *)"      [-createLensInput <Lens NN input file name>]**\n",
   (char *)"      [-createMoxWorxInput <MoxWorx input file name>]***\n",
   (char *)"      * https://github.com/numenta/NuPIC/wiki/NuPIC-Input-Data-File-Format\n",
   (char *)"      ** http://web.stanford.edu/group/mbc/LENSManual/Manual\n",
   (char *)"      *** https://github.com/portegys/MoxWorx\n",
   NULL
};

// Runs.
#define MAX_GAME_STEPS    200
int TrainingRuns = -1;
int UnforcedTrainingRunModulo = -1;
int TrainingRandomSeed        = 4517;
int TestingRuns       = -1;
int TestingRandomSeed = 3605;

// Game properties.
int   GridDimensions = 5;
float CellSize       = 1.0f / (float)GridDimensions;
int   StartX         = GridDimensions / 2;
int   StartY         = GridDimensions / 2;
float BallSpeed      = 0.0f;

// Files.
char           *TrainingPlaybackFile = NULL;
char           *TestingPlaybackFile  = NULL;
char           *LensInputFile        = NULL;
char           *NuPICinputFile       = NULL;
char           *MoxWorxInputFile     = NULL;
FILE           *LensFp;
FILE           *NuPICfp;
FILE           *MoxWorxFp;
vector<string> LensGame;
char           *LoadFile = NULL;
char           *SaveFile = NULL;

// Pong game.
Pong *pong;

// Paddle position.
int PaddleY;

// Game state.
int  CurrentGame;
int  BallPositionX, BallPositionY;
int  BallNextPositionX, BallNextPositionY;
int  PaddlePosition;
bool ShowBallNext = false;

// Mona.
Mona *mona;

// Mona sensors.
enum MONA_SENSORS
{
   BALL_SENSOR   = 0,
   PADDLE_SENSOR = 1,
   NUM_SENSORS   = 2
};

// Ball sensor values.
const float BALL_ABSENT       = 0.0f;
const float BALL_PRESENT      = 0.1f;
const float BALL_MOVING_LEFT  = 0.2f;
const float BALL_MOVING_RIGHT = 0.3f;
const float BALL_MOVING_UP    = 0.4f;
const float BALL_MOVING_DOWN  = 0.5f;
const float BALL_NULL         = 1.0f;

// Paddle sensor values.
const float PADDLE_ABSENT     = 0.0f;
const float PADDLE_PRESENT    = 0.5f;
const float PADDLE_NULL       = 1.0f;

// Cycle.
int cycle(bool overrideResponse = false);

// Mona response.
enum MONA_RESPONSE
{
   CHECK_BALL       = 0,
   TRACK_BALL_LEFT  = 1,
   TRACK_BALL_RIGHT = 2,
   PAN_LEFT         = 3,
   PAN_RIGHT        = 4,
   MOVE_PADDLE_UP   = 5,
   MOVE_PADDLE_DOWN = 6,
   NUM_RESPONSES    = 7,
   NULL_RESPONSE    = NUM_RESPONSES
};

// MoxWorx response.
enum MOXWORX_RESPONSE
{
   WAIT    = 0,
   FORWARD = 1,
   RIGHT   = 2,
   LEFT    = 3
};

// MoxWorx orientation.
enum MOXWORX_ORIENTATION
{
   NORTH = 0,
   EAST  = 1,
   SOUTH = 2,
   WEST  = 3
};
MOXWORX_ORIENTATION MoxWorxOrientation = NORTH;

enum OUTCOME
{
   CONTINUE = 0,
   WIN      = 1,
   LOSE     = 2
};

// Execute response.
OUTCOME execute(int response);

// Step pong.
OUTCOME stepPong();

// Mona needs.
enum MONA_NEEDS
{
   TRACK_BALL_NEED = 0,
   NUM_NEEDS       = 1
};

// Mona position.
int MonaX, MonaY;

// Reset game.
enum RESET_TYPE
{
   TRAINING = 0,
   TESTING  = 1
};
void reset(RESET_TYPE);

// Randomizers.
Random *TrainingRandomizer;
Random *TestingRandomizer;

// Verbose mode.
bool Verbose = false;

void printUsage()
{
   for (int i = 0; Usage[i] != NULL; i++)
   {
      fprintf(stderr, "%s", Usage[i]);
   }
}


int
main(int argc, char *argv[])
{
   int     i, j, k, win, lose, draw;
   OUTCOME outcome;
   FILE    *trainingPlaybackFp;
   FILE    *testingPlaybackFp;

#ifdef WIN32
   char *pongfile;
#else
   char pongfile[128];
#endif

   vector<float> sensors;
   int           pongTest         = 0;
   bool          gotTestingRandom = false;

   for (i = 1; i < argc; i++)
   {
      if (strcmp(argv[i], "-trainingRuns") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         TrainingRuns = atoi(argv[i]);
         if (TrainingRuns < 0)
         {
            printUsage();
            return(1);
         }
         continue;
      }

      if (strcmp(argv[i], "-unforcedTrainingRunModulo") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         UnforcedTrainingRunModulo = atoi(argv[i]);
         if (UnforcedTrainingRunModulo <= 0)
         {
            printUsage();
            return(1);
         }
         continue;
      }

      if (strcmp(argv[i], "-recordTrainingPlayback") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         TrainingPlaybackFile = argv[i];
         continue;
      }

      if (strcmp(argv[i], "-trainingRandomSeed") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         TrainingRandomSeed = atoi(argv[i]);
         continue;
      }

      if (strcmp(argv[i], "-testingRuns") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         TestingRuns = atoi(argv[i]);
         if (TestingRuns < 0)
         {
            printUsage();
            return(1);
         }
         continue;
      }

      if (strcmp(argv[i], "-recordTestingPlayback") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         TestingPlaybackFile = argv[i];
         continue;
      }

      if (strcmp(argv[i], "-testingRandomSeed") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         TestingRandomSeed = atoi(argv[i]);
         gotTestingRandom  = true;
         continue;
      }

      if (strcmp(argv[i], "-gridDimensions") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         GridDimensions = atoi(argv[i]);
         if (GridDimensions < 1)
         {
            printUsage();
            return(1);
         }
         if ((GridDimensions % 2) == 0)
         {
            printUsage();
            return(1);
         }
         CellSize = 1.0f / (float)GridDimensions;
         continue;
      }

      if (strcmp(argv[i], "-startPosition") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         StartX = atoi(argv[i]);
         if (StartX < 0)
         {
            printUsage();
            return(1);
         }
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         StartY = atoi(argv[i]);
         if (StartY < 0)
         {
            printUsage();
            return(1);
         }
         continue;
      }

      if (strcmp(argv[i], "-ballSpeed") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         BallSpeed = (float)atof(argv[i]);
         if (BallSpeed <= 0.0f)
         {
            printUsage();
            return(1);
         }
         continue;
      }

      if (strcmp(argv[i], "-load") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
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
            return(1);
         }
         SaveFile = argv[i];
         continue;
      }

      if (strcmp(argv[i], "-verbose") == 0)
      {
         Verbose = true;
         continue;
      }

      if (strcmp(argv[i], "-pongTest") == 0)
      {
         i++;
         if (i >= argc)
         {
            fprintf(stderr, "Invalid pongTest option\n");
            return(1);
         }
         pongTest = atoi(argv[i]);
         if ((pongTest < 0) || (pongTest > 2))
         {
            fprintf(stderr, "Invalid pongTest value\n");
            return(1);
         }
         continue;
      }

      if (strcmp(argv[i], "-createLensInput") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         LensInputFile = argv[i];
         continue;
      }

      if (strcmp(argv[i], "-createNuPICinput") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         NuPICinputFile = argv[i];
         continue;
      }

      if (strcmp(argv[i], "-createMoxWorxInput") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         MoxWorxInputFile = argv[i];
         continue;
      }

      printUsage();
      return(1);
   }

   if (TrainingRuns == -1)
   {
      printUsage();
      return(1);
   }

   if ((LensInputFile != NULL) || (NuPICinputFile != NULL) || (MoxWorxInputFile != NULL))
   {
      if ((TestingRuns != -1) || (pongTest != 0))
      {
         printUsage();
         return(1);
      }
      if ((TestingPlaybackFile != NULL) || gotTestingRandom)
      {
         printUsage();
         return(1);
      }
   }
   else
   {
      if ((TestingRuns == -1) && (pongTest == 0))
      {
         printUsage();
         return(1);
      }
   }

   if ((StartX >= GridDimensions) || (StartY >= GridDimensions))
   {
      printUsage();
      return(1);
   }

   if (BallSpeed == 0.0f)
   {
      BallSpeed = CellSize;
   }

   // Get random numbers.
   TrainingRandomizer = new Random(TrainingRandomSeed);
   assert(TrainingRandomizer != NULL);
   TestingRandomizer = new Random(TestingRandomSeed);
   assert(TestingRandomizer != NULL);

   // Create Pong game.
   pong = new Pong();
   assert(pong != NULL);
   pong->paddle.setLength(CellSize);

   // Create Mona.
   if (LoadFile == NULL)
   {
      mona = new Mona(NUM_SENSORS, NUM_RESPONSES, NUM_NEEDS);
      assert(mona != NULL);
      mona->setNeed(TRACK_BALL_NEED, 1.0);
      sensors.resize(NUM_SENSORS, 0.0f);
      sensors[BALL_SENSOR]   = BALL_PRESENT;
      sensors[PADDLE_SENSOR] = PADDLE_ABSENT;
      mona->addGoal(TRACK_BALL_NEED, sensors, 0, 0.5);
      sensors[BALL_SENSOR]   = BALL_PRESENT;
      sensors[PADDLE_SENSOR] = PADDLE_PRESENT;
      mona->addGoal(TRACK_BALL_NEED, sensors, 0, 0.5);
   }
   else
   {
      mona = new Mona();
      assert(mona != NULL);
      if (!mona->load(LoadFile))
      {
         fprintf(stderr, "Cannot load from file %s\n", LoadFile);
         return(1);
      }
   }

   // Training runs.
   if (TrainingPlaybackFile != NULL)
   {
      trainingPlaybackFp = fopen(TrainingPlaybackFile, "w");
      if (trainingPlaybackFp == NULL)
      {
         fprintf(stderr, "Cannot open training playback file %s\n", TrainingPlaybackFile);
         return(1);
      }
      else
      {
         fprintf(trainingPlaybackFp, "Dimension %d\n", GridDimensions);
      }
   }
   else
   {
      trainingPlaybackFp = NULL;
   }
   if (NuPICinputFile != NULL)
   {
      NuPICfp = fopen(NuPICinputFile, "w");
      if (NuPICfp == NULL)
      {
         fprintf(stderr, "Cannot open NuPIC file %s\n", NuPICinputFile);
         return(1);
      }
      else
      {
         fprintf(NuPICfp, "game, ball_sensor, paddle_sensor, response\n");
         fprintf(NuPICfp, "int, float, float, float\n");
         fprintf(NuPICfp, "S,,,\n");
      }
   }
   else
   {
      NuPICfp = NULL;
   }
   if (LensInputFile != NULL)
   {
      LensFp = fopen(LensInputFile, "w");
      if (LensFp == NULL)
      {
         fprintf(stderr, "Cannot open Lens file %s\n", LensInputFile);
         return(1);
      }
   }
   else
   {
      LensFp = NULL;
   }
   if (MoxWorxInputFile != NULL)
   {
      MoxWorxFp = fopen(MoxWorxInputFile, "w");
      if (MoxWorxFp == NULL)
      {
         fprintf(stderr, "Cannot open MoxWorx file %s\n", MoxWorxInputFile);
         return(1);
      }
      fprintf(MoxWorxFp, "%d\n", GridDimensions);
   }
   else
   {
      MoxWorxFp = NULL;
   }
   win = lose = draw = 0;
   for (i = 0; i < TrainingRuns; i++)
   {
      // Reset game.
      CurrentGame = i;
      reset(TRAINING);

      // Run game.
      if (Verbose || (pongTest != 0))
      {
         printf("Training game=%d\n", i);
         printf("Step=0\n");
         printf("Mona location=%d/%d\n", MonaX, MonaY);
         printf("Ball position=%0.2f/%0.2f, velocity=%0.2f/%0.2f, paddle = %0.2f\n",
                pong->ball.position.x, pong->ball.position.y,
                pong->ball.velocity.x, pong->ball.velocity.y, pong->paddle.position);
      }
      if ((trainingPlaybackFp != NULL) && (pongTest == 0))
      {
         fprintf(trainingPlaybackFp, "Game %d\n", i);
         fprintf(trainingPlaybackFp, "Step 0 %0.2f %0.2f %0.2f %0.2f %0.2f\n",
                 pong->ball.position.x, pong->ball.position.y, pong->paddle.position,
                 ((float)MonaX * CellSize) + (CellSize * 0.5f), ((float)MonaY * CellSize) + (CellSize * 0.5f));
      }
      if (LensFp != NULL)
      {
         LensGame.clear();
      }
      for (j = 0; j < MAX_GAME_STEPS; j++)
      {
         if (Verbose || (pongTest != 0))
         {
            printf("Step=%d\n", j + 1);
         }

         // Cycle Mona and execute response.
         if (pongTest == 0)
         {
            if ((UnforcedTrainingRunModulo > 0) && (i > 0) &&
                ((i % UnforcedTrainingRunModulo) == 0))
            {
               outcome = execute(cycle());
            }
            else
            {
               outcome = execute(cycle(true));
            }
            if (trainingPlaybackFp != NULL)
            {
               fprintf(trainingPlaybackFp, "Step %d %0.2f %0.2f %0.2f %0.2f %0.2f\n", j + 1,
                       pong->ball.position.x, pong->ball.position.y, pong->paddle.position,
                       ((float)MonaX * CellSize) + (CellSize * 0.5f), ((float)MonaY * CellSize) + (CellSize * 0.5f));
            }
         }
         else
         {
            outcome = stepPong();
            if (pongTest == 1)
            {
               PaddleY = (int)(pong->ball.position.y / CellSize);
               pong->paddle.position = ((float)PaddleY * CellSize) + (CellSize * 0.5f);
            }
         }
         if (Verbose || (pongTest != 0))
         {
            printf("Outcome=");
            switch (outcome)
            {
            case CONTINUE:
               printf("CONTINUE");
               break;

            case WIN:
               printf("WIN");
               break;

            case LOSE:
               printf("LOSE");
               break;
            }
            printf(", ball position=%0.2f/%0.2f, velocity=%0.2f/%0.2f, paddle=%0.2f\n",
                   pong->ball.position.x, pong->ball.position.y,
                   pong->ball.velocity.x, pong->ball.velocity.y, pong->paddle.position);
         }
         if (outcome == WIN)
         {
            cycle(true);
            win++;
            if (trainingPlaybackFp != NULL)
            {
               fprintf(trainingPlaybackFp, "Win %d\n", win);
            }
            if (Verbose || (pongTest != 0))
            {
               printf("Win\n");
            }
            if (LensFp != NULL)
            {
               fprintf(LensFp, "name: { game_%d } %d\n", i, (int)LensGame.size());
               for (k = 0; k < (int)LensGame.size(); k++)
               {
                  fprintf(LensFp, "%s", LensGame[k].c_str());
                  if (k == (int)LensGame.size() - 1)
                  {
                     fprintf(LensFp, ";");
                  }
                  fprintf(LensFp, "\n");
               }
            }
            break;
         }
         else if (outcome == LOSE)
         {
            lose++;
            if (trainingPlaybackFp != NULL)
            {
               fprintf(trainingPlaybackFp, "Lose %d\n", lose);
            }
            if (Verbose || (pongTest != 0))
            {
               printf("Lose\n");
            }
            break;
         }
      }
      if (j == MAX_GAME_STEPS)
      {
         draw++;
         if (trainingPlaybackFp != NULL)
         {
            fprintf(trainingPlaybackFp, "Draw %d\n", draw);
         }
         if (Verbose || (pongTest != 0))
         {
            printf("Draw\n");
         }
      }
      if (Verbose || (pongTest != 0))
      {
         printf("Score: win=%d, lose=%d, draw=%d\n", win, lose, draw);
      }
   }
   if (pongTest != 0)
   {
      return(0);
   }

   if (trainingPlaybackFp != NULL)
   {
      fclose(trainingPlaybackFp);
      trainingPlaybackFp = NULL;
   }
   if (NuPICfp != NULL)
   {
      fclose(NuPICfp);
      NuPICfp = NULL;
   }
   if (LensFp != NULL)
   {
      fclose(LensFp);
      LensFp = NULL;
   }
   if (MoxWorxFp != NULL)
   {
      fclose(MoxWorxFp);
      MoxWorxFp = NULL;
   }

   if (SaveFile != NULL)
   {
      if (!mona->save(SaveFile))
      {
         fprintf(stderr, "Cannot save to file %s\n", SaveFile);
         return(1);
      }
   }

   // Testing runs?
   if (TestingRuns == -1)
   {
      return(0);
   }
   if (TestingPlaybackFile != NULL)
   {
      testingPlaybackFp = fopen(TestingPlaybackFile, "w");
      if (testingPlaybackFp == NULL)
      {
         fprintf(stderr, "Cannot open testing playback file %s\n", TestingPlaybackFile);
         return(1);
      }
      else
      {
         fprintf(testingPlaybackFp, "Dimension %d\n", GridDimensions);
      }
   }
   else
   {
      testingPlaybackFp = NULL;
   }
   win = lose = draw = 0;
#ifdef WIN32
   pongfile = tmpnam(NULL);
#else
   sprintf(pongfile, "/tmp/pong_%d.mona", getpid());
#endif
   mona->save(pongfile);
   for (i = 0; i < TestingRuns; i++)
   {
      // Reset game.
      CurrentGame = i;
      mona->load(pongfile);
      reset(TESTING);

      // Run game.
      if (Verbose)
      {
         printf("Testing game=%d\n", i);
         printf("Step=0\n");
         printf("Mona location=%d/%d\n", MonaX, MonaY);
         printf("Ball position=%0.2f/%0.2f, velocity=%0.2f/%0.2f, paddle = %0.2f\n",
                pong->ball.position.x, pong->ball.position.y,
                pong->ball.velocity.x, pong->ball.velocity.y, pong->paddle.position);
      }
      if (testingPlaybackFp != NULL)
      {
         fprintf(testingPlaybackFp, "Game %d\n", i);
         fprintf(testingPlaybackFp, "Step 0 %0.2f %0.2f %0.2f %0.2f %0.2f\n",
                 pong->ball.position.x, pong->ball.position.y, pong->paddle.position,
                 ((float)MonaX * CellSize) + (CellSize * 0.5f), ((float)MonaY * CellSize) + (CellSize * 0.5f));
      }
      for (j = 0; j < MAX_GAME_STEPS; j++)
      {
         if (Verbose || (pongTest != 0))
         {
            printf("Step=%d\n", j + 1);
         }

         // Cycle Mona and execute response.
         outcome = execute(cycle());
         if (testingPlaybackFp != NULL)
         {
            fprintf(testingPlaybackFp, "Step %d %0.2f %0.2f %0.2f %0.2f %0.2f\n", j + 1,
                    pong->ball.position.x, pong->ball.position.y, pong->paddle.position,
                    ((float)MonaX * CellSize) + (CellSize * 0.5f), ((float)MonaY * CellSize) + (CellSize * 0.5f));
         }
         if (Verbose)
         {
            printf("Outcome=");
            switch (outcome)
            {
            case CONTINUE:
               printf("CONTINUE");
               break;

            case WIN:
               printf("WIN");
               break;

            case LOSE:
               printf("LOSE");
               break;
            }
            printf(", ball position=%0.2f/%0.2f, velocity=%0.2f/%0.2f, paddle=%0.2f\n",
                   pong->ball.position.x, pong->ball.position.y,
                   pong->ball.velocity.x, pong->ball.velocity.y, pong->paddle.position);
         }
         if (outcome == WIN)
         {
            execute(cycle());
            win++;
            if (testingPlaybackFp != NULL)
            {
               fprintf(testingPlaybackFp, "Step %d %0.2f %0.2f %0.2f %0.2f %0.2f\n", j + 2,
                       pong->ball.position.x, pong->ball.position.y, pong->paddle.position,
                       ((float)MonaX * CellSize) + (CellSize * 0.5f), ((float)MonaY * CellSize) + (CellSize * 0.5f));
               fprintf(testingPlaybackFp, "Win %d\n", win);
            }
            if (Verbose)
            {
               printf("Win\n");
            }
            break;
         }
         else if (outcome == LOSE)
         {
            execute(cycle());
            lose++;
            if (testingPlaybackFp != NULL)
            {
               fprintf(testingPlaybackFp, "Step %d %0.2f %0.2f %0.2f %0.2f %0.2f\n", j + 2,
                       pong->ball.position.x, pong->ball.position.y, pong->paddle.position,
                       ((float)MonaX * CellSize) + (CellSize * 0.5f), ((float)MonaY * CellSize) + (CellSize * 0.5f));
               fprintf(testingPlaybackFp, "Lose %d\n", lose);
            }
            if (Verbose)
            {
               printf("Lose\n");
            }
            break;
         }
      }
      if (j == MAX_GAME_STEPS)
      {
         draw++;
         if (testingPlaybackFp != NULL)
         {
            fprintf(testingPlaybackFp, "Draw %d\n", draw);
         }
         if (Verbose)
         {
            printf("Draw\n");
         }
      }
      if (Verbose)
      {
         printf("Score: win=%d, lose=%d, draw=%d\n", win, lose, draw);
      }
   }
#ifdef WIN32
   _unlink(pongfile);
#else
   unlink(pongfile);
#endif

   if (testingPlaybackFp != NULL)
   {
      fclose(testingPlaybackFp);
      testingPlaybackFp = NULL;
   }

   return(0);
}


// Reset game.
void reset(RESET_TYPE type)
{
   float  x, y, v;
   Random *randomizer;

   x = ((float)StartX * CellSize) + (CellSize * 0.5f);
   y = ((float)StartY * CellSize) + (CellSize * 0.5f);
   pong->setBallPosition(x, y);
   if (type == TRAINING)
   {
      randomizer = TrainingRandomizer;
   }
   else
   {
      randomizer = TestingRandomizer;
   }
   pong->setBallVelocity((float)randomizer->RAND_INTERVAL(0.1, 1.0),
                         (float)randomizer->RAND_INTERVAL(-1.0, 1.0));
   if (randomizer->RAND_BOOL())
   {
      pong->ball.velocity.x = -pong->ball.velocity.x;
   }
   if (fabs(pong->ball.velocity.y) > fabs(pong->ball.velocity.x))
   {
      v = pong->ball.velocity.x;
      pong->ball.velocity.x = pong->ball.velocity.y;
      pong->ball.velocity.y = v;
   }
   pong->setBallSpeed(BallSpeed);
   PaddleY = GridDimensions / 2;
   pong->paddle.setPosition(0.5f);
   ShowBallNext = false;
   MonaX        = StartX;
   MonaY        = StartY;
   mona->clearWorkingMemory();
   mona->inflateNeed(TRACK_BALL_NEED);
   MoxWorxOrientation = NORTH;
}


// Cycle Mona and return response.
int cycle(bool overrideResponse)
{
   int        response;
   Pong::Ball ball;
   char       buf[BUFSIZ];

   vector<float> sensors;
   sensors.resize(NUM_SENSORS);

   BallPositionX = (int)(pong->ball.position.x / CellSize);
   if (BallPositionX < 0)
   {
      BallPositionX = 0;
   }
   if (BallPositionX >= GridDimensions)
   {
      BallPositionX = GridDimensions - 1;
   }
   BallPositionY = (int)(pong->ball.position.y / CellSize);
   if (BallPositionY < 0)
   {
      BallPositionY = 0;
   }
   if (BallPositionY >= GridDimensions)
   {
      BallPositionY = GridDimensions - 1;
   }

   Vector p = pong->ball.position;
   Vector d = pong->ball.velocity;
   ball = pong->ball;
   pong->step();
   BallNextPositionX = (int)(pong->ball.position.x / CellSize);
   if (BallNextPositionX < 0)
   {
      BallNextPositionX = 0;
   }
   if (BallNextPositionX >= GridDimensions)
   {
      BallNextPositionX = GridDimensions - 1;
   }
   BallNextPositionY = (int)(pong->ball.position.y / CellSize);
   if (BallNextPositionY < 0)
   {
      BallNextPositionY = 0;
   }
   if (BallNextPositionY >= GridDimensions)
   {
      BallNextPositionY = GridDimensions - 1;
   }
   pong->ball = ball;

   if ((MonaX != BallPositionX) || (MonaY != BallPositionY))
   {
      sensors[BALL_SENSOR] = BALL_ABSENT;
   }
   else
   {
      if (ShowBallNext)
      {
         if (MonaY == BallNextPositionY)
         {
            if (MonaX == BallNextPositionX)
            {
               sensors[BALL_SENSOR] = BALL_PRESENT;
            }
            else if (MonaX > BallNextPositionX)
            {
               sensors[BALL_SENSOR] = BALL_MOVING_LEFT;
            }
            else
            {
               sensors[BALL_SENSOR] = BALL_MOVING_RIGHT;
            }
         }
         else if (MonaY > BallNextPositionY)
         {
            sensors[BALL_SENSOR] = BALL_MOVING_DOWN;
         }
         else
         {
            sensors[BALL_SENSOR] = BALL_MOVING_UP;
         }
      }
      else
      {
         sensors[BALL_SENSOR] = BALL_PRESENT;
      }
   }

   sensors[PADDLE_SENSOR] = PADDLE_ABSENT;
   PaddlePosition         = (int)(pong->paddle.position / CellSize);
   if (MonaX == GridDimensions - 1)
   {
      if (MonaY == PaddlePosition)
      {
         sensors[PADDLE_SENSOR] = PADDLE_PRESENT;
      }
   }

   if (Verbose)
   {
      printf("Ball current location=%d/%d, next location=%d/%d",
             BallPositionX, BallPositionY, BallNextPositionX, BallNextPositionY);
      printf(", Sensor[BALL_SENSOR]=");
      if (sensors[BALL_SENSOR] == BALL_ABSENT)
      {
         printf("BALL_ABSENT");
      }
      if (sensors[BALL_SENSOR] == BALL_PRESENT)
      {
         printf("BALL_PRESENT");
      }
      if (sensors[BALL_SENSOR] == BALL_MOVING_LEFT)
      {
         printf("BALL_MOVING_LEFT");
      }
      if (sensors[BALL_SENSOR] == BALL_MOVING_RIGHT)
      {
         printf("BALL_MOVING_RIGHT");
      }
      if (sensors[BALL_SENSOR] == BALL_MOVING_UP)
      {
         printf("BALL_MOVING_UP");
      }
      if (sensors[BALL_SENSOR] == BALL_MOVING_DOWN)
      {
         printf("BALL_MOVING_DOWN");
      }
      printf("\n");
      printf("Paddle location=%d", PaddlePosition);
      printf(", Sensor[PADDLE_SENSOR]=");
      if (sensors[PADDLE_SENSOR] == PADDLE_ABSENT)
      {
         printf("PADDLE_ABSENT");
      }
      if (sensors[PADDLE_SENSOR] == PADDLE_PRESENT)
      {
         printf("PADDLE_PRESENT");
      }
      printf("\n");
      printf("Mona location=%d/%d\n", MonaX, MonaY);
   }

   if (overrideResponse)
   {
      if (sensors[BALL_SENSOR] == BALL_ABSENT)
      {
         if (sensors[PADDLE_SENSOR] == PADDLE_PRESENT)
         {
            if (MonaY > BallNextPositionY)
            {
               mona->overrideResponse(MOVE_PADDLE_DOWN);
            }
            else if (MonaY < BallNextPositionY)
            {
               mona->overrideResponse(MOVE_PADDLE_UP);
            }
            else
            {
               mona->overrideResponse(PAN_LEFT);
            }
         }
         else
         {
            if (MonaX < BallPositionX)
            {
               mona->overrideResponse(PAN_RIGHT);
            }
            else
            {
               mona->overrideResponse(PAN_LEFT);
            }
         }
      }
      else if (sensors[BALL_SENSOR] == BALL_PRESENT)
      {
         mona->overrideResponse(CHECK_BALL);
      }
      else if (sensors[BALL_SENSOR] == BALL_MOVING_LEFT)
      {
         mona->overrideResponse(TRACK_BALL_LEFT);
      }
      else if (sensors[BALL_SENSOR] == BALL_MOVING_RIGHT)
      {
         mona->overrideResponse(TRACK_BALL_RIGHT);
      }
      else if (sensors[BALL_SENSOR] == BALL_MOVING_UP)
      {
         if (sensors[PADDLE_SENSOR] == PADDLE_PRESENT)
         {
            mona->overrideResponse(MOVE_PADDLE_UP);
         }
         else
         {
            mona->overrideResponse(PAN_RIGHT);
         }
      }
      else if (sensors[BALL_SENSOR] == BALL_MOVING_DOWN)
      {
         if (sensors[PADDLE_SENSOR] == PADDLE_PRESENT)
         {
            mona->overrideResponse(MOVE_PADDLE_DOWN);
         }
         else
         {
            mona->overrideResponse(PAN_RIGHT);
         }
      }
   }

   response = mona->cycle(sensors);
   if (NuPICfp != NULL)
   {
      fprintf(NuPICfp, "%d,%0.2f,%0.2f,%0.2f\n", CurrentGame,
              sensors[BALL_SENSOR], sensors[PADDLE_SENSOR], (float)response * 0.1f);
   }
   if (LensFp != NULL)
   {
      // Paddle sensor values.
      if (sensors[BALL_SENSOR] == BALL_ABSENT)
      {
         sprintf(buf, "I: 1 0 0 0 0 0 ");
      }
      else if (sensors[BALL_SENSOR] == BALL_PRESENT)
      {
         sprintf(buf, "I: 0 1 0 0 0 0 ");
      }
      else if (sensors[BALL_SENSOR] == BALL_MOVING_LEFT)
      {
         sprintf(buf, "I: 0 0 1 0 0 0 ");
      }
      else if (sensors[BALL_SENSOR] == BALL_MOVING_RIGHT)
      {
         sprintf(buf, "I: 0 0 0 1 0 0 ");
      }
      else if (sensors[BALL_SENSOR] == BALL_MOVING_UP)
      {
         sprintf(buf, "I: 0 0 0 0 1 0 ");
      }
      else
      {
         sprintf(buf, "I: 0 0 0 0 0 1 ");
      }
      if (sensors[PADDLE_SENSOR] == PADDLE_ABSENT)
      {
         strcat(buf, "1 0 ");
      }
      else
      {
         strcat(buf, "0 1 ");
      }
      switch (response)
      {
      case CHECK_BALL:
         strcat(buf, "T: 1 0 0 0 0 0 0");
         break;

      case TRACK_BALL_LEFT:
         strcat(buf, "T: 0 1 0 0 0 0 0");
         break;

      case TRACK_BALL_RIGHT:
         strcat(buf, "T: 0 0 1 0 0 0 0");
         break;

      case PAN_LEFT:
         strcat(buf, "T: 0 0 0 1 0 0 0");
         break;

      case PAN_RIGHT:
         strcat(buf, "T: 0 0 0 0 1 0 0");
         break;

      case MOVE_PADDLE_UP:
         strcat(buf, "T: 0 0 0 0 0 1 0");
         break;

      case MOVE_PADDLE_DOWN:
         strcat(buf, "T: 0 0 0 0 0 0 1");
         break;
      }
      LensGame.push_back(buf);
   }
   if (MoxWorxFp != NULL)
   {
      static int   lastMonaX = -1;
      static int   lastMonaY = -1;
      static float lastSensors[2];
      if ((MonaX != lastMonaX) || (MonaY != lastMonaY) ||
          (lastSensors[0] != sensors[0]) || (lastSensors[1] != sensors[1]))
      {
         lastMonaX      = MonaX;
         lastMonaY      = MonaY;
         lastSensors[0] = sensors[0];
         lastSensors[1] = sensors[1];
         sprintf(buf, "%d,%d,%d", CurrentGame,
                 (int)(sensors[BALL_SENSOR] * 10.0f), (int)(sensors[PADDLE_SENSOR] * 2.0f));
         if (sensors[BALL_SENSOR] == BALL_ABSENT)
         {
            switch (response)
            {
            case MOVE_PADDLE_UP:
               switch (MoxWorxOrientation)
               {
               case NORTH:
                  fprintf(MoxWorxFp, "%s,%d\n", buf, FORWARD);
                  break;

               case EAST:
                  fprintf(MoxWorxFp, "%s,%d\n", buf, LEFT);
                  MoxWorxOrientation = NORTH;
                  fprintf(MoxWorxFp, "%s,%d\n", buf, FORWARD);
                  break;
               }
               break;

            case MOVE_PADDLE_DOWN:
               switch (MoxWorxOrientation)
               {
               case EAST:
                  fprintf(MoxWorxFp, "%s,%d\n", buf, RIGHT);
                  MoxWorxOrientation = SOUTH;
                  fprintf(MoxWorxFp, "%s,%d\n", buf, FORWARD);
                  break;

               case SOUTH:
                  fprintf(MoxWorxFp, "%s,%d\n", buf, FORWARD);
                  break;
               }
               break;

            case PAN_LEFT:
               switch (MoxWorxOrientation)
               {
               case NORTH:
                  fprintf(MoxWorxFp, "%s,%d\n", buf, LEFT);
                  break;

               case SOUTH:
                  fprintf(MoxWorxFp, "%s,%d\n", buf, RIGHT);
                  break;
               }
               MoxWorxOrientation = WEST;
               for (int i = GridDimensions - 1; i > BallPositionX; i--)
               {
                  if (i == GridDimensions - 1)
                  {
                     fprintf(MoxWorxFp, "%s,%d\n", buf, FORWARD);
                  }
                  else
                  {
                     fprintf(MoxWorxFp, "%d,%d,%d,%d\n", CurrentGame,
                             (int)(BALL_ABSENT * 10.0f), (int)(PADDLE_ABSENT * 2.0f), FORWARD);
                  }
               }
               break;

            default:
               fprintf(MoxWorxFp, "%s,%d\n", buf, FORWARD);
               break;
            }
         }
         else if (sensors[PADDLE_SENSOR] == PADDLE_PRESENT)
         {
            fprintf(MoxWorxFp, "%s,%d\n", buf, WAIT);
         }
         else if (sensors[BALL_SENSOR] == BALL_PRESENT)
         {
            switch (MoxWorxOrientation)
            {
            case NORTH:
               fprintf(MoxWorxFp, "%s,%d\n", buf, WAIT);
               break;

            case EAST:
               fprintf(MoxWorxFp, "%s,%d\n", buf, LEFT);
               MoxWorxOrientation = NORTH;
               break;

            case WEST:
               fprintf(MoxWorxFp, "%s,%d\n", buf, RIGHT);
               MoxWorxOrientation = NORTH;
               break;
            }
         }
         else if (sensors[BALL_SENSOR] == BALL_MOVING_LEFT)
         {
            switch (MoxWorxOrientation)
            {
            case NORTH:
               fprintf(MoxWorxFp, "%s,%d\n", buf, LEFT);
               MoxWorxOrientation = WEST;
               break;

            case EAST:
               fprintf(MoxWorxFp, "%s,%d\n", buf, LEFT);
               fprintf(MoxWorxFp, "%s,%d\n", buf, LEFT);
               MoxWorxOrientation = WEST;
               break;
            }
            fprintf(MoxWorxFp, "%s,%d\n", buf, FORWARD);
         }
         else if (sensors[BALL_SENSOR] == BALL_MOVING_RIGHT)
         {
            switch (MoxWorxOrientation)
            {
            case NORTH:
               fprintf(MoxWorxFp, "%s,%d\n", buf, RIGHT);
               MoxWorxOrientation = EAST;
               break;

            case WEST:
               fprintf(MoxWorxFp, "%s,%d\n", buf, RIGHT);
               fprintf(MoxWorxFp, "%s,%d\n", buf, RIGHT);
               MoxWorxOrientation = EAST;
               break;
            }
            fprintf(MoxWorxFp, "%s,%d\n", buf, FORWARD);
         }
         else if ((sensors[BALL_SENSOR] == BALL_MOVING_UP) ||
                  (sensors[BALL_SENSOR] == BALL_MOVING_DOWN))
         {
            switch (MoxWorxOrientation)
            {
            case NORTH:
               fprintf(MoxWorxFp, "%s,%d\n", buf, RIGHT);
               MoxWorxOrientation = EAST;
               break;

            case WEST:
               fprintf(MoxWorxFp, "%s,%d\n", buf, RIGHT);
               fprintf(MoxWorxFp, "%s,%d\n", buf, RIGHT);
               MoxWorxOrientation = EAST;
               break;
            }
            for (int i = BallPositionX, j = GridDimensions - 1; i < j; i++)
            {
               if (i == BallPositionX)
               {
                  fprintf(MoxWorxFp, "%s,%d\n", buf, FORWARD);
               }
               else
               {
                  fprintf(MoxWorxFp, "%d,%d,%d,%d\n", CurrentGame,
                          (int)(BALL_ABSENT * 10.0f), (int)(PADDLE_ABSENT * 2.0f), FORWARD);
               }
            }
         }
      }
   }
   if (Verbose)
   {
      printf("Response=");
      switch (response)
      {
      case CHECK_BALL:
         printf("CHECK_BALL");
         break;

      case TRACK_BALL_LEFT:
         printf("TRACK_BALL_LEFT");
         break;

      case TRACK_BALL_RIGHT:
         printf("TRACK_BALL_RIGHT");
         break;

      case PAN_LEFT:
         printf("PAN_LEFT");
         break;

      case PAN_RIGHT:
         printf("PAN_RIGHT");
         break;

      case MOVE_PADDLE_UP:
         printf("MOVE_PADDLE_UP");
         break;

      case MOVE_PADDLE_DOWN:
         printf("MOVE_PADDLE_DOWN");
         break;
      }
      if (overrideResponse)
      {
         printf("(overridden)");
      }
      printf("\n");
      printf("Need=%f\n", mona->getNeed(TRACK_BALL_NEED));
   }
   mona->inflateNeed(TRACK_BALL_NEED);
   return(response);
}


// Execute response.
OUTCOME execute(int response)
{
   switch (response)
   {
   case CHECK_BALL:
      if (ShowBallNext)
      {
         if ((BallPositionX == BallNextPositionX) &&
             (BallPositionY == BallNextPositionY))
         {
            stepPong();
         }
      }
      ShowBallNext = true;
      break;

   case TRACK_BALL_LEFT:
      ShowBallNext = false;
      if (MonaX > 0)
      {
         MonaX--;
      }
      return(stepPong());

   case TRACK_BALL_RIGHT:
      ShowBallNext = false;
      if (MonaX < GridDimensions - 1)
      {
         MonaX++;
      }
      return(stepPong());

   case PAN_LEFT:
      if (MonaX > 0)
      {
         MonaX--;
         while (MonaX > 0)
         {
            if ((MonaX == BallPositionX) && (MonaY == BallPositionY))
            {
               break;
            }
            MonaX--;
         }
      }
      break;

   case PAN_RIGHT:
      if (MonaX < GridDimensions - 1)
      {
         MonaX++;
         while (MonaX < GridDimensions - 1)
         {
            if ((MonaX == BallPositionX) && (MonaY == BallPositionY))
            {
               break;
            }
            MonaX++;
         }
      }
      break;

   case MOVE_PADDLE_UP:
      ShowBallNext = false;
      if ((MonaX == GridDimensions - 1) && (MonaY == PaddleY))
      {
         if (PaddleY < GridDimensions - 1)
         {
            PaddleY++;
            pong->paddle.position = ((float)PaddleY * CellSize) + (CellSize * 0.5f);
            MonaY = PaddleY;
         }
      }
      return(stepPong());

   case MOVE_PADDLE_DOWN:
      ShowBallNext = false;
      if ((MonaX == GridDimensions - 1) && (MonaY == PaddleY))
      {
         if (PaddleY > 0)
         {
            PaddleY--;
            pong->paddle.position = ((float)PaddleY * CellSize) + (CellSize * 0.5f);
            MonaY = PaddleY;
         }
      }
      return(stepPong());
   }
   return(CONTINUE);
}


// Step pong.
OUTCOME stepPong()
{
   switch (pong->step())
   {
   case Pong::CLEAR:
      if ((MonaX == GridDimensions - 1) &&
          (MonaX == (int)(pong->ball.position.x / CellSize)) &&
          (MonaY == (int)(pong->ball.position.y / CellSize)))
      {
         return(WIN);
      }
      else
      {
         return(CONTINUE);
      }

   case Pong::SCORE:
      if ((MonaX == GridDimensions - 1) &&
          (MonaX == (int)(pong->ball.position.x / CellSize)) &&
          (MonaY == (int)(pong->ball.position.y / CellSize)))
      {
         return(WIN);
      }
      else
      {
         return(LOSE);
      }

   case Pong::STRIKE:
      return(WIN);
   }
   return(CONTINUE);
}
