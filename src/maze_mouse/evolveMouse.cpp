// For conditions of distribution and use, see copyright notice in muzz.hpp

// Evolve maze-learning mouse by mutating and recombining brain parameters.

#include "evolveMouse.hpp"

// Version (SCCS "what" format).
const char *MouseEvolveVersion = MOUSE_EVOLVE_VERSION;

// Print version.
void
printEvolveVersion(FILE *out = stdout)
{
   fprintf(out, "%s\n", &MouseEvolveVersion[5]);
}


// Usage.
char *Usage[] =
{
   (char *)"To run:",
   (char *)"  evolve_mouse",
   (char *)"      [-generations <evolution generations>]",
   (char *)"      [-numMazeTests (number of maze tests)]",
   (char *)"      [-numDoorTrainingTrials (number of door association training trials)]",
   (char *)"      [-numMazeTrainingTrials (number of training trials per maze test)]",
   (char *)"      [-mutationRate <mutation rate>]",
   (char *)"      [-randomSeed <random seed> (for new run)]",
   (char *)"      [-print (print mouse brain parameters)]",
   (char *)"      [-input <evolution input file name>]",
   (char *)"      [-output <evolution output file name>]",
   (char *)"      [-logfile <log file name>]",
   (char *)"      [-ignoreInterrupts (ignore interrupts)]",
#ifdef WIN32
   (char *)"      [-attachConsole (attach to console)]",
#endif
   (char *)"To print version:",
   (char *)"  evolve_mouse",
   (char *)"      [-version (print version)]",
   NULL
};

// Print and log error.
void printError(char *buf)
{
   fprintf(stderr, "%s\n", buf);
   if (Log::LOGGING_FLAG == LOG_TO_FILE)
   {
      sprintf(Log::messageBuf, "%s", buf);
      Log::logError();
   }
}


// Print and log information.
void printInfo(char *buf)
{
   fprintf(stderr, "%s\n", buf);
   if (Log::LOGGING_FLAG == LOG_TO_FILE)
   {
      sprintf(Log::messageBuf, "%s", buf);
      Log::logInformation();
   }
}


// Print usage.
void printUsage()
{
   for (int i = 0; Usage[i] != NULL; i++)
   {
      printInfo(Usage[i]);
   }
}


// Evolution generation.
int Generation = 0;

// Generations to run.
int Generations = -1;

// Number of maze tests.
int NumMazeTests = DEFAULT_NUM_MAZE_TESTS;

// Number of door association training trials.
int NumDoorTrainingTrials = DEFAULT_NUM_DOOR_TRAINING_TRIALS;

// Number of maze training trials.
int NumMazeTrainingTrials = DEFAULT_NUM_MAZE_TRAINING_TRIALS;

// Are maze room doors all visible?
// Some of these doors are blocked.
bool SeeAllMazeDoors = true;

// Mutation rate.
PROBABILITY MutationRate = DEFAULT_MUTATION_RATE;

// Random numbers.
RANDOM RandomSeed  = INVALID_RANDOM;
Random *Randomizer = NULL;

// Print mouse brain parameters.
bool Print = false;

// Population file names.
char *InputFileName  = NULL;
char *OutputFileName = NULL;

// Room types.
#define START_ROOM     0
#define BEGIN_MAZE     1
#define MAZE_ROOM      2
#define END_MAZE       3
#define GOAL_ROOM      4
#define DEAD_ROOM      5

// Responses.
#define TAKE_DOOR_0    0
#define TAKE_DOOR_1    1
#define TAKE_DOOR_2    2
#define WAIT           3
#define HOP            4

// Cheese need and goal.
#define CHEESE_NEED    1.0
#define CHEESE_GOAL    0.5

// Maze-learning task.
int DoorAssociations[3];
vector<vector<int> > MazePaths;

// Maze room.
class Room
{
public:

   int            type;
   int            cx;
   int            cy;
   bool           hasCheese;
   vector<Room *> doors;

   Room(int type, int cx, int cy)
   {
      this->type = type;
      this->cx   = cx;
      this->cy   = cy;
   }


   // Set room doors to other rooms.
   void setDoors(vector<Room *>& doors)
   {
      this->doors = doors;
   }
};

// Maze.
vector<Room *> maze[7];
int            beginMazeIndex;
int            endMazeIndex;

// Mouse position.
int mouseX;
int mouseY;

// Build maze.
void buildMaze()
{
   vector<Room *> doors;
   maze[0].resize(3);
   maze[0][1]     = new Room(START_ROOM, 0, 1);
   maze[0][0]     = new Room(START_ROOM, 0, 0);
   maze[0][2]     = new Room(START_ROOM, 0, 2);
   beginMazeIndex = 1;
   maze[1].resize(1);
   maze[1][0] = new Room(BEGIN_MAZE, 1, 0);
   doors.resize(3);
   doors[0] = NULL;
   doors[1] = NULL;
   doors[2] = maze[1][0];
   maze[0][0]->setDoors(doors);
   doors[0] = NULL;
   doors[1] = maze[1][0];
   doors[2] = NULL;
   maze[0][1]->setDoors(doors);
   doors[0] = maze[1][0];
   doors[1] = NULL;
   doors[2] = NULL;
   maze[0][2]->setDoors(doors);
   maze[2].resize(3);
   maze[2][1] = new Room(MAZE_ROOM, 2, 1);
   maze[2][0] = new Room(MAZE_ROOM, 2, 0);
   maze[2][2] = new Room(MAZE_ROOM, 2, 2);
   doors[0]   = maze[2][0];
   doors[1]   = maze[2][1];
   doors[2]   = maze[2][2];
   maze[1][0]->setDoors(doors);
   maze[3].resize(5);
   maze[3][2] = new Room(MAZE_ROOM, 3, 2);
   maze[3][1] = new Room(MAZE_ROOM, 3, 1);
   maze[3][0] = new Room(MAZE_ROOM, 3, 0);
   maze[3][3] = new Room(MAZE_ROOM, 3, 3);
   maze[3][4] = new Room(MAZE_ROOM, 3, 4);
   doors[0]   = maze[3][0];
   doors[1]   = maze[3][1];
   doors[2]   = maze[3][2];
   maze[2][0]->setDoors(doors);
   doors[0] = maze[3][1];
   doors[1] = maze[3][2];
   doors[2] = maze[3][3];
   maze[2][1]->setDoors(doors);
   doors[0] = maze[3][2];
   doors[1] = maze[3][3];
   doors[2] = maze[3][4];
   maze[2][2]->setDoors(doors);
   maze[4].resize(3);
   maze[4][1] = new Room(MAZE_ROOM, 4, 1);
   maze[4][0] = new Room(MAZE_ROOM, 4, 0);
   maze[4][2] = new Room(MAZE_ROOM, 4, 2);
   doors[0]   = NULL;
   doors[1]   = NULL;
   doors[2]   = maze[4][0];
   maze[3][0]->setDoors(doors);
   doors[0] = NULL;
   doors[1] = maze[4][0];
   doors[2] = maze[4][1];
   maze[3][1]->setDoors(doors);
   doors[0] = maze[4][0];
   doors[1] = maze[4][1];
   doors[2] = maze[4][2];
   maze[3][2]->setDoors(doors);
   doors[0] = maze[4][1];
   doors[1] = maze[4][2];
   doors[2] = NULL;
   maze[3][3]->setDoors(doors);
   doors[0] = maze[4][2];
   doors[1] = NULL;
   doors[2] = NULL;
   maze[3][4]->setDoors(doors);
   endMazeIndex = 5;
   maze[5].resize(1);
   maze[5][0] = new Room(END_MAZE, 5, 0);
   doors[0]   = NULL;
   doors[1]   = NULL;
   doors[2]   = maze[5][0];
   maze[4][0]->setDoors(doors);
   doors[0] = NULL;
   doors[1] = maze[5][0];
   doors[2] = NULL;
   maze[4][1]->setDoors(doors);
   doors[0] = maze[5][0];
   doors[1] = NULL;
   doors[2] = NULL;
   maze[4][2]->setDoors(doors);
   maze[6].resize(3);
   maze[6][1] = new Room(GOAL_ROOM, 6, 1);
   maze[6][0] = new Room(GOAL_ROOM, 6, 0);
   maze[6][2] = new Room(GOAL_ROOM, 6, 2);
   doors[0]   = maze[6][0];
   doors[1]   = maze[6][1];
   doors[2]   = maze[6][2];
   maze[5][0]->setDoors(doors);
   doors[0] = NULL;
   doors[1] = NULL;
   doors[2] = NULL;
   maze[6][0]->setDoors(doors);
   maze[6][1]->setDoors(doors);
   maze[6][2]->setDoors(doors);
   mouseX = mouseY = 0;
}


// Initialize mouse brain.
void initBrain(Mona *mouse)
{
   vector<Mona::SENSOR> goalSensors;

   mouse->initNet(5, HOP + 1, 1, Randomizer->RAND());

   // Set a long second effect interval
   // for a higher level mediator.
   mouse->effectEventIntervals[1].resize(2);
   mouse->effectEventIntervalWeights[1].resize(2);
   mouse->effectEventIntervals[1][0]       = 2;
   mouse->effectEventIntervalWeights[1][0] = 0.5;
   mouse->effectEventIntervals[1][1]       = 10;
   mouse->effectEventIntervalWeights[1][1] = 0.5;

   // Set need and goal for cheese.
   mouse->setNeed(0, CHEESE_NEED);
   goalSensors.push_back(0.0);
   goalSensors.push_back(0.0);
   goalSensors.push_back(0.0);
   goalSensors.push_back((double)GOAL_ROOM);
   goalSensors.push_back(1.0);
   mouse->homeostats[0]->addGoal(goalSensors, 0,
                                 Mona::NULL_RESPONSE, CHEESE_GOAL);
}


// Parameter mutator.
class ParmMutator
{
public:

   // Probability of random mutation.
   static PROBABILITY RANDOM_MUTATION;

   typedef enum
   {
      INTEGER_PARM, FLOAT_PARM, DOUBLE_PARM
   }
   PARM_TYPE;
   PARM_TYPE type;
   enum { PARM_NAME_SIZE=50 };
   char   name[PARM_NAME_SIZE];
   void   *parm;
   double min;
   double max;
   double delta;

   // Constructors.
   ParmMutator()
   {
      type    = DOUBLE_PARM;
      name[0] = '\0';
      parm    = NULL;
      min     = max = delta = 0.0f;
   }


   ParmMutator(PARM_TYPE type, char *name, void *parm,
               double min, double max, double delta)
   {
      this->type = type;
      strncpy(this->name, name, PARM_NAME_SIZE - 1);
      this->parm  = parm;
      this->min   = min;
      this->max   = max;
      this->delta = delta;
   }


   // Mutate parameter.
   void mutate()
   {
      int    i;
      float  f;
      double d;

      if (!Randomizer->RAND_CHANCE(MutationRate)) { return; }
      switch (type)
      {
      case INTEGER_PARM:
         if (Randomizer->RAND_CHANCE(RANDOM_MUTATION))
         {
            *(int *)parm = (int)Randomizer->RAND_INTERVAL(min, max + 0.99);
         }
         else
         {
            i = *(int *)parm;
            if (Randomizer->RAND_BOOL())
            {
               i += (int)delta;
               if (i > (int)max) { i = (int)max; }
            }
            else
            {
               i -= (int)delta;
               if (i < (int)min) { i = (int)min; }
            }
            *(int *)parm = i;
         }
         break;

      case FLOAT_PARM:
         if (Randomizer->RAND_CHANCE(RANDOM_MUTATION))
         {
            *(float *)parm = (float)Randomizer->RAND_INTERVAL(min, max);
         }
         else
         {
            f = *(float *)parm;
            if (Randomizer->RAND_BOOL())
            {
               f += (float)delta;
               if (f > (float)max) { f = (float)max; }
            }
            else
            {
               f -= (float)delta;
               if (f < (float)min) { f = (float)min; }
            }
            *(float *)parm = f;
         }
         break;

      case DOUBLE_PARM:
         if (Randomizer->RAND_CHANCE(RANDOM_MUTATION))
         {
            *(double *)parm = (double)Randomizer->RAND_INTERVAL(min, max);
         }
         else
         {
            d = *(double *)parm;
            if (Randomizer->RAND_BOOL())
            {
               d += delta;
               if (d > max) { d = max; }
            }
            else
            {
               d -= delta;
               if (d < min) { d = min; }
            }
            *(double *)parm = d;
         }
         break;
      }
   }


   // Copy parameter from given one.
   void copy(ParmMutator *from)
   {
      switch (type)
      {
      case INTEGER_PARM:
         *(int *)parm = *(int *)from->parm;
         break;

      case FLOAT_PARM:
         *(float *)parm = *(float *)from->parm;
         break;

      case DOUBLE_PARM:
         *(double *)parm = *(double *)from->parm;
         break;
      }
   }
};
PROBABILITY ParmMutator::RANDOM_MUTATION = 0.1;

// Brain parameter mutator.
class BrainParmMutator
{
public:

   // Brain.
   Mona *brain;

   // Parameter mutators.
   vector<ParmMutator *> parmMutators;

   // Constructor.
   BrainParmMutator(Mona *brain)
   {
      this->brain = brain;

      // INITIAL_ENABLEMENT.
      ParmMutator *parmMutator =
         new ParmMutator(ParmMutator::DOUBLE_PARM, (char *)"INITIAL_ENABLEMENT",
                         (void *)&brain->INITIAL_ENABLEMENT, 0.1, 1.0, 0.1);
      assert(parmMutator != NULL);
      parmMutators.push_back(parmMutator);

      // DRIVE_ATTENUATION.
      parmMutator =
         new ParmMutator(ParmMutator::DOUBLE_PARM, (char *)"DRIVE_ATTENUATION",
                         (void *)&brain->DRIVE_ATTENUATION, 0.0, 1.0, 0.1);
      assert(parmMutator != NULL);
      parmMutators.push_back(parmMutator);

      // LEARNING_DECREASE_VELOCITY.
      parmMutator =
         new ParmMutator(ParmMutator::DOUBLE_PARM, (char *)"LEARNING_DECREASE_VELOCITY",
                         (void *)&brain->LEARNING_DECREASE_VELOCITY, 0.1, 0.9, 0.1);
      assert(parmMutator != NULL);
      parmMutators.push_back(parmMutator);

      // LEARNING_INCREASE_VELOCITY.
      parmMutator =
         new ParmMutator(ParmMutator::DOUBLE_PARM, (char *)"LEARNING_INCREASE_VELOCITY",
                         (void *)&brain->LEARNING_INCREASE_VELOCITY, 0.1, 0.9, 0.1);
      assert(parmMutator != NULL);
      parmMutators.push_back(parmMutator);

      // FIRING_STRENGTH_LEARNING_DAMPER.
      parmMutator =
         new ParmMutator(ParmMutator::DOUBLE_PARM, (char *)"FIRING_STRENGTH_LEARNING_DAMPER",
                         (void *)&brain->FIRING_STRENGTH_LEARNING_DAMPER, 0.05, 0.9, 0.05);
      assert(parmMutator != NULL);
      parmMutators.push_back(parmMutator);
   }


   // Destructor.
   ~BrainParmMutator()
   {
      for (int i = 0; i < (int)parmMutators.size(); i++)
      {
         delete parmMutators[i];
      }
      parmMutators.clear();
   }


   // Mutate.
   void mutate()
   {
      for (int i = 0; i < (int)parmMutators.size(); i++)
      {
         parmMutators[i]->mutate();
      }

      // Initialize brain with new parameters.
      initBrain(brain);
   }


   // Copy parameters from given brain.
   void copy(BrainParmMutator *from)
   {
      for (int i = 0; i < (int)parmMutators.size(); i++)
      {
         parmMutators[i]->copy(from->parmMutators[i]);
      }

      // Initialize brain with new parameters.
      initBrain(brain);
   }


   // Randomly merge parameters from given brains.
   void meld(BrainParmMutator *from1, BrainParmMutator *from2)
   {
      for (int i = 0; i < (int)parmMutators.size(); i++)
      {
         if (Randomizer->RAND_BOOL())
         {
            parmMutators[i]->copy(from1->parmMutators[i]);
         }
         else
         {
            parmMutators[i]->copy(from2->parmMutators[i]);
         }
      }

      // Initialize brain with new parameters.
      initBrain(brain);
   }
};

// Population member.
class Member
{
public:

   static int idDispenser;

   int    id;
   Mona   *mouse;
   double fitness;
   int    generation;

   // Brain parameter mutator.
   BrainParmMutator *brainParmMutator;

   // Constructors.
   Member(int generation = 0)
   {
      id = idDispenser;
      idDispenser++;

      // Create the mouse brain.
      mouse = new Mona();
      assert(mouse != NULL);
      initBrain(mouse);

      // Create brain parameter mutator.
      brainParmMutator = new BrainParmMutator(mouse);
      assert(brainParmMutator != NULL);
      brainParmMutator->mutate();

      fitness          = 0.0;
      this->generation = generation;
   }


   // Destructor.
   ~Member()
   {
      if (brainParmMutator != NULL) { delete brainParmMutator; }
      if (mouse != NULL) { delete mouse; }
   }


   // Evaluate.
   // Fitness is determined number of correct tests.
   void evaluate()
   {
      int i, j, k;

      vector<int> path;

      // Clear brain.
      initBrain(mouse);

      // Train door associations.
      for (i = 0; i < NumDoorTrainingTrials; i++)
      {
         for (j = 0; j < 3; j++)
         {
            path.clear();
            path.push_back(2 - j);
            path.push_back(HOP);
            path.push_back(DoorAssociations[j]);
            path.push_back(WAIT);
            runTrial(0, j, path, true);
         }
      }

      fitness = 0.0;
      for (i = 0; i < NumMazeTests; i++)
      {
         // Train maze path.
         for (j = 0; j < NumMazeTrainingTrials; j++)
         {
            path.clear();
            path = MazePaths[i];
            path.push_back(WAIT);
            runTrial(beginMazeIndex, 0, path, true);
         }

         // Test maze.
         for (j = 0; j < 3; j++)
         {
            path.clear();
            path.push_back(2 - j);
            for (k = 0; k < (int)MazePaths[i].size(); k++)
            {
               path.push_back(MazePaths[i][k]);
            }
            path.push_back(DoorAssociations[j]);
            path.push_back(WAIT);
            if (runTrial(0, j, path, false)) { fitness += 1.0; }
         }
      }
   }


   // Run a trial.
   bool runTrial(int x, int y, vector<int>& path, bool train)
   {
      int i, j, cx, cy, response;

      vector<Mona::SENSOR> sensors;
      bool                 hopped;

      // Set up sensors.
      sensors.resize(5);

      // Clear working memory.
      mouse->clearWorkingMemory();

      // Reset need.
      mouse->setNeed(0, CHEESE_NEED);

      // Place mouse.
      mouseX = x;
      mouseY = y;

      // Run maze path.
      hopped = false;
      for (i = 0; i < (int)path.size(); i++)
      {
         // Override response?
         if (train || (path[i] == WAIT))
         {
            mouse->responseOverride = path[i];
         }

         // Prepare sensors.
         for (j = 0; j < 3; j++)
         {
            if (maze[mouseX][mouseY]->doors[j] != NULL)
            {
               sensors[j] = 1.0;
            }
            else if (SeeAllMazeDoors &&
                     (mouseX > beginMazeIndex) && (mouseX < endMazeIndex))
            {
               // Maze room door is visible although possibly blocked.
               sensors[j] = 1.0;
            }
            else
            {
               sensors[j] = 0.0;
            }
         }
         sensors[3] = (Mona::SENSOR)maze[mouseX][mouseY]->type;
         if (mouseX == endMazeIndex + 1)
         {
            sensors[4] = 1.0;
         }
         else
         {
            sensors[4] = 0.0;
         }

         // Initiate sensory/response cycle.
         response = mouse->cycle(sensors);

         // Clear override.
         mouse->responseOverride = Mona::NULL_RESPONSE;

         // Successful trial?
         if (path[i] == WAIT) { return(true); }

         // Hop at start of maze is OK.
         if ((mouseX == beginMazeIndex) && (response == HOP) &&
             (path[i] != HOP) && !hopped)
         {
            hopped = true;
            i--;
            continue;
         }

         // Wrong response ends trial unsuccessfully.
         if (response != path[i]) { return(false); }

         // Move mouse.
         if (response == HOP)
         {
            mouseX = endMazeIndex;
            mouseY = 0;
         }
         else
         {
            cx     = (maze[mouseX][mouseY]->doors[response])->cx;
            cy     = (maze[mouseX][mouseY]->doors[response])->cy;
            mouseX = cx;
            mouseY = cy;
         }
      }
      return(false);
   }


   // Get fitness.
   double getFitness()
   {
      return(fitness);
   }


   // Create mutated member by varying brain parameters.
   void mutate(Member *member)
   {
      brainParmMutator->copy(member->brainParmMutator);
      brainParmMutator->mutate();
   }


   // Merge given member brain parameters into this brain.
   void mindMeld(Member *member1, Member *member2)
   {
      brainParmMutator->meld(member1->brainParmMutator,
                             member2->brainParmMutator);
   }


   // Load.
   void load(FILE *fp)
   {
      FREAD_INT(&id, fp);
      mouse->load(fp);
      FREAD_DOUBLE(&fitness, fp);
      FREAD_INT(&generation, fp);
   }


   // Save.
   void save(FILE *fp)
   {
      FWRITE_INT(&id, fp);
      mouse->save(fp);
      FWRITE_DOUBLE(&fitness, fp);
      FWRITE_INT(&generation, fp);
   }


   // Print.
   void print(FILE *out = stdout)
   {
      fprintf(out, "id=%d, fitness=%f, generation=%d\n",
              id, getFitness(), generation);
      fprintf(out, "mouse brain:\n");
      mouse->printParms(out);
   }
};
int Member::idDispenser = 0;

// Population.
Member *Population[POPULATION_SIZE];

// Start/end functions.
void logParameters();
void print();
void load();
void save();
void loadPopulation(FILE *fp);
void savePopulation(FILE *fp);

// Create maze-learning task.
void createTask();

// Evolve functions.
void evolve(), evaluate(), prune(), mutate(), mate();

#ifdef WIN32
#ifdef _DEBUG
// For Windows memory checking, set CHECK_MEMORY = 1.
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

   // Initialize logging..
   Log::LOGGING_FLAG = LOG_TO_PRINT;

   // Parse arguments.
   for (i = 1; i < argc; i++)
   {
      if (strcmp(argv[i], "-generations") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         Generations = atoi(argv[i]);
         if (Generations < 0)
         {
            printUsage();
            exit(1);
         }
         continue;
      }

      if (strcmp(argv[i], "-numMazeTests") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         NumMazeTests = atoi(argv[i]);
         if (NumMazeTests < 0)
         {
            printError((char *)"Invalid number of maze tests");
            printUsage();
            exit(1);
         }
         continue;
      }

      if (strcmp(argv[i], "-numDoorTrainingTrials") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         NumDoorTrainingTrials = atoi(argv[i]);
         if (NumDoorTrainingTrials < 0)
         {
            printError((char *)"Invalid number of door association training trials");
            printUsage();
            exit(1);
         }
         continue;
      }

      if (strcmp(argv[i], "-numMazeTrainingTrials") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         NumMazeTrainingTrials = atoi(argv[i]);
         if (NumMazeTrainingTrials < 0)
         {
            printError((char *)"Invalid number of maze association training trials");
            printUsage();
            exit(1);
         }
         continue;
      }

      if (strcmp(argv[i], "-mutationRate") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         MutationRate = atof(argv[i]);
         if (MutationRate < 0.0)
         {
            printUsage();
            exit(1);
         }
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
         continue;
      }

      if (strcmp(argv[i], "-print") == 0)
      {
         Print = true;
         continue;
      }

      if (strcmp(argv[i], "-input") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         InputFileName = argv[i];
         continue;
      }

      if (strcmp(argv[i], "-output") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         OutputFileName = argv[i];
         continue;
      }

      if (strcmp(argv[i], "-logfile") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         Log::LOGGING_FLAG = LOG_TO_FILE;
         Log::setLogFileName(argv[i]);
         continue;
      }

      if (strcmp(argv[i], "-ignoreInterrupts") == 0)
      {
         signal(SIGINT, SIG_IGN);
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
         printEvolveVersion();
         Mona::printVersion();
         exit(0);
      }

      printUsage();
      exit(1);
   }

   // Check operation combinations.
   if (Generations >= 0)
   {
      if (!Print && (OutputFileName == NULL))
      {
         printError((char *)"No print or file output");
         printUsage();
         exit(1);
      }
   }
   else
   {
      if (!Print)
      {
         printError((char *)"Must run or print");
         printUsage();
         exit(1);
      }
   }

   // Build the maze.
   buildMaze();

   // Initialize.
   if (InputFileName != NULL)
   {
      // Load population.
      if (RandomSeed != INVALID_RANDOM)
      {
         printError((char *)"Random seed is loaded from input file");
         printUsage();
         exit(1);
      }
      load();
   }
   else
   {
      if (RandomSeed == INVALID_RANDOM)
      {
         RandomSeed = (RANDOM)time(NULL);
      }
      Randomizer = new Random(RandomSeed);
      assert(Randomizer != NULL);
      Randomizer->SRAND(RandomSeed);

      // Create maze-learning task.
      createTask();

      // Create population.
      for (i = 0; i < POPULATION_SIZE; i++)
      {
         Population[i] = new Member(0);
         assert(Population[i] != NULL);
      }
   }

   // Log run parameters.
   if (Generations >= 0)
   {
      Log::logInformation((char *)"Initializing evolve:");
      if (Generations >= 0)
      {
         sprintf(Log::messageBuf, "generations=%d", Generations);
         Log::logInformation();
      }
      if (InputFileName != NULL)
      {
         sprintf(Log::messageBuf, "input=%s", InputFileName);
         Log::logInformation();
      }
      if (OutputFileName != NULL)
      {
         sprintf(Log::messageBuf, "output=%s", OutputFileName);
         Log::logInformation();
      }
      logParameters();

      // Evolution loop.
      Log::logInformation((char *)"Begin evolve:");
      for (i = 0; i < Generations; i++, Generation++)
      {
         sprintf(Log::messageBuf, "Generation=%d", Generation);
         Log::logInformation();
         evolve();

         // Save population?
         if (OutputFileName != NULL)
         {
            if ((i % SAVE_FREQUENCY) == 0)
            {
               save();
            }
         }
      }
   }

   // Save population.
   if (OutputFileName != NULL)
   {
      save();
   }

   // Print population.
   if (Print)
   {
      print();
   }

   // Release memory.
   for (i = 0; i < POPULATION_SIZE; i++)
   {
      if (Population[i] != NULL)
      {
         delete Population[i];
         Population[i] = NULL;
      }
   }

   // Close log.
   if (Generations >= 0)
   {
      Log::logInformation((char *)"End evolve");
   }
   Log::close();

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


// Log run parameters.
void logParameters()
{
   Log::logInformation((char *)"Evolve Parameters:");
   sprintf(Log::messageBuf, "FIT_POPULATION_SIZE = %d", FIT_POPULATION_SIZE);
   Log::logInformation();
   sprintf(Log::messageBuf, "NUM_MUTANTS = %d", NUM_MUTANTS);
   Log::logInformation();
   sprintf(Log::messageBuf, "NUM_OFFSPRING = %d", NUM_OFFSPRING);
   Log::logInformation();
   sprintf(Log::messageBuf, "NumMazeTests = %d", NumMazeTests);
   Log::logInformation();
   sprintf(Log::messageBuf, "NumDoorTrainingTrials = %d", NumDoorTrainingTrials);
   Log::logInformation();
   sprintf(Log::messageBuf, "NumMazeTrainingTrials = %d", NumMazeTrainingTrials);
   Log::logInformation();
   sprintf(Log::messageBuf, "MutationRate = %f", MutationRate);
   Log::logInformation();
   sprintf(Log::messageBuf, "RandomSeed = %lu", RandomSeed);
   Log::logInformation();
   sprintf(Log::messageBuf, "Door associations: 0->%d, 1->%d, 2->%d",
           DoorAssociations[0], DoorAssociations[1], DoorAssociations[2]);
   Log::logInformation();
   Log::logInformation((char *)"Maze paths (doors):");
   for (int i = 0; i < (int)MazePaths.size(); i++)
   {
      sprintf(Log::messageBuf, "%d %d %d %d",
              MazePaths[i][0], MazePaths[i][1], MazePaths[i][2], MazePaths[i][3]);
      Log::logInformation();
   }
}


// Print population.
void print()
{
   if ((Log::LOGGING_FLAG == LOG_TO_FILE) ||
       (Log::LOGGING_FLAG == LOG_TO_BOTH))
   {
      if (Log::logfp != NULL)
      {
         for (int i = 0; i < POPULATION_SIZE; i++)
         {
            Population[i]->print(Log::logfp);
         }
         fflush(Log::logfp);
      }
   }

   if ((Log::LOGGING_FLAG == LOG_TO_PRINT) ||
       (Log::LOGGING_FLAG == LOG_TO_BOTH))
   {
      for (int i = 0; i < POPULATION_SIZE; i++)
      {
         Population[i]->print();
      }
   }
}


// Load evolve.
void load()
{
   int  i, j, k, n;
   FILE *fp;
   char buf[200];

   assert(InputFileName != NULL);
   if ((fp = FOPEN_READ(InputFileName)) == NULL)
   {
      sprintf(buf, "Cannot load population file %s", InputFileName);
      printError(buf);
      exit(1);
   }
   FREAD_INT(&Member::idDispenser, fp);
   FREAD_LONG(&RandomSeed, fp);
   Randomizer = new Random(RandomSeed);
   assert(Randomizer != NULL);
   Randomizer->RAND_LOAD(fp);
   FREAD_INT(&Generation, fp);
   for (i = 0; i < 3; i++)
   {
      FREAD_INT(&DoorAssociations[i], fp);
   }
   MazePaths.clear();
   FREAD_INT(&i, fp);
   MazePaths.resize(i);
   for (j = 0; j < i; j++)
   {
      for (k = 0; k < 4; k++)
      {
         FREAD_INT(&n, fp);
         MazePaths[j].push_back(n);
      }
   }
   loadPopulation(fp);
   FCLOSE(fp);
}


// Save evolve.
void save()
{
   int  i, j, k, n;
   FILE *fp;
   char buf[200];

   assert(OutputFileName != NULL);
   if ((fp = FOPEN_WRITE(OutputFileName)) == NULL)
   {
      sprintf(buf, "Cannot save to population file %s", OutputFileName);
      printError(buf);
      exit(1);
   }
   FWRITE_INT(&Member::idDispenser, fp);
   FWRITE_LONG(&RandomSeed, fp);
   Randomizer->RAND_SAVE(fp);
   FWRITE_INT(&Generation, fp);
   for (i = 0; i < 3; i++)
   {
      FWRITE_INT(&DoorAssociations[i], fp);
   }
   i = (int)MazePaths.size();
   FWRITE_INT(&i, fp);
   for (j = 0; j < i; j++)
   {
      for (k = 0; k < 4; k++)
      {
         n = MazePaths[j][k];
         FWRITE_INT(&n, fp);
      }
   }
   savePopulation(fp);
   FCLOSE(fp);
}


// Load evolution population.
void loadPopulation(FILE *fp)
{
   for (int i = 0; i < POPULATION_SIZE; i++)
   {
      Population[i] = new Member();
      assert(Population[i] != NULL);
      Population[i]->load(fp);
   }
}


// Save evolution population.
void savePopulation(FILE *fp)
{
   for (int i = 0; i < POPULATION_SIZE; i++)
   {
      Population[i]->save(fp);
   }
}


// Create maze-learning task.
void createTask()
{
   int i, j, k;

   DoorAssociations[0] = Randomizer->RAND_CHOICE(3);
   DoorAssociations[1] = DoorAssociations[0];
   while (DoorAssociations[1] == DoorAssociations[0])
   {
      DoorAssociations[1] = Randomizer->RAND_CHOICE(3);
   }
   DoorAssociations[2] = DoorAssociations[0];
   while (DoorAssociations[2] == DoorAssociations[0] ||
          DoorAssociations[2] == DoorAssociations[1])
   {
      DoorAssociations[2] = Randomizer->RAND_CHOICE(3);
   }
   MazePaths.clear();
   MazePaths.resize(NumMazeTests);
   for (i = 0; i < NumMazeTests; i++)
   {
      while (true)
      {
         MazePaths[i].clear();
         for (j = k = 0; j < 4; j++)
         {
            MazePaths[i].push_back(Randomizer->RAND_CHOICE(3));
            k += (MazePaths[i][j] - 1);
         }
         if (k == 0)
         {
            break;
         }
      }
   }
}


// Evolution generation.
void evolve()
{
   // Evaluate member fitness.
   evaluate();

   // Prune unfit members.
   prune();

   // Create new members by mutation.
   mutate();

   // Create new members by mating.
   mate();
}


// Evaluate member fitnesses.
void evaluate()
{
   Log::logInformation((char *)"Evaluate:");

   for (int i = 0; i < POPULATION_SIZE; i++)
   {
      Population[i]->evaluate();
      sprintf(Log::messageBuf, "  Member=%d, Mouse=%d, Fitness=%f, Generation=%d",
              i, Population[i]->id, Population[i]->getFitness(),
              Population[i]->generation);
      Log::logInformation();
   }
}


// Prune unfit members.
void prune()
{
   double max;
   int    i, j, m;
   Member *member;
   Member *fitPopulation[FIT_POPULATION_SIZE];

   Log::logInformation((char *)"Select:");
   for (i = 0; i < FIT_POPULATION_SIZE; i++)
   {
      m = -1;
      for (j = 0; j < POPULATION_SIZE; j++)
      {
         member = Population[j];
         if (member == NULL)
         {
            continue;
         }
         if ((m == -1) || (member->getFitness() > max))
         {
            m   = j;
            max = member->getFitness();
         }
      }
      member           = Population[m];
      Population[m]    = NULL;
      fitPopulation[i] = member;
      sprintf(Log::messageBuf, "  Mouse=%d, Fitness=%f, Generation=%d",
              member->id, member->getFitness(), member->generation);
      Log::logInformation();
   }
   for (i = 0; i < POPULATION_SIZE; i++)
   {
      if (Population[i] != NULL)
      {
         delete Population[i];
         Population[i] = NULL;
      }
   }
   for (i = 0; i < FIT_POPULATION_SIZE; i++)
   {
      Population[i] = fitPopulation[i];
   }
}


// Mutate members.
void mutate()
{
   int    i, j;
   Member *member, *mutant;

   Log::logInformation((char *)"Mutate:");
   for (i = 0; i < NUM_MUTANTS; i++)
   {
      // Select a fit member to mutate.
      j      = Randomizer->RAND_CHOICE(FIT_POPULATION_SIZE);
      member = Population[j];

      // Create mutant member.
      mutant = new Member(member->generation + 1);
      assert(mutant != NULL);
      Population[FIT_POPULATION_SIZE + i] = mutant;
      sprintf(Log::messageBuf, "  Member=%d, Mouse=%d -> Member=%d, Mouse=%d",
              j, member->id, FIT_POPULATION_SIZE + i,
              mutant->id);
      Log::logInformation();

      // Mutate.
      mutant->mutate(member);
   }
}


// Produce offspring by "mind melding" parent brain parameters.
void mate()
{
   int    i, j, k;
   Member *member1, *member2, *offspring;

   Log::logInformation((char *)"Mate:");
   if (FIT_POPULATION_SIZE < 2)
   {
      return;
   }
   for (i = 0; i < NUM_OFFSPRING; i++)
   {
      // Select a pair of fit members to mate.
      j       = Randomizer->RAND_CHOICE(FIT_POPULATION_SIZE);
      member1 = Population[j];
      while ((k = Randomizer->RAND_CHOICE(FIT_POPULATION_SIZE)) == j)
      {
      }
      member2 = Population[k];

      // Create offspring.
      offspring = new Member((member1->generation > member2->generation ?
                              member1->generation : member2->generation) + 1);
      assert(offspring != NULL);
      Population[FIT_POPULATION_SIZE + NUM_MUTANTS + i] = offspring;
      sprintf(Log::messageBuf, "  Members=%d,%d, Mice=%d,%d -> Member=%d, Mouse=%d",
              j, k, member1->id, member2->id,
              FIT_POPULATION_SIZE + NUM_MUTANTS + i, offspring->id);
      Log::logInformation();

      // Combine parent brains into offspring.
      offspring->mindMeld(member1, member2);
   }
}
