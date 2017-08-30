// For conditions of distribution and use, see copyright notice in muzz.hpp

// Evolve muzzes by mutating and recombining brain parameters.

#include "evolveMuzzes.hpp"

// Version (SCCS "what" format).
const char *MuzzEvolveVersion = MUZZ_EVOLVE_VERSION;

// Print version.
void
printEvolveVersion(FILE *out = stdout)
{
   fprintf(out, "%s\n", &MuzzEvolveVersion[4]);
}


// Usage.
char *Usage[] =
{
   (char *)"To run:",
   (char *)"  evolve_muzzes",
   (char *)"      -generations <evolution generations>",
   (char *)"      [-muzzCycles (muzz cycles per run)]",
   (char *)"      [-initHunger (initial hunger)]",
   (char *)"      [-initThirst (initial thirst)]",
   (char *)"      [-input <evolution input file name> (for run continuation)]",
   (char *)"      -output <evolution output file name>",
   (char *)"      [-mutationRate <mutation rate>]",
   (char *)"      [-randomSeed <random seed> (for new run)]",
   (char *)"      [-objectSeed <object generation seed>]",
   (char *)"      [-terrainSeed <terrain generation seed>]",
   (char *)"      [-terrainDimension <terrain size = dimension x dimension> (minimum=2)]",
   (char *)"      [-minTrainingTrials (minimum number of training trials per test trial)]",
   (char *)"      [-maxTrainingTrials (maximum number of training trials per test trial)]",
   (char *)"      [-logfile <log file name>]",
   (char *)"      [-ignoreInterrupts (ignore interrupts)]",
#ifdef WIN32
   (char *)"      [-attachConsole (attach to console)]",
#endif
   (char *)"To run an evolved muzz with graphics:",
   (char *)"  evolve_muzzes",
   (char *)"      -muzz <identifier> (? to list available muzzes)",
   (char *)"      [-muzzCycles (number of cycles)]",
   (char *)"      [-initHunger (initial hunger)]",
   (char *)"      [-initThirst (initial thirst)]",
   (char *)"      -input <evolution input file name>",
   (char *)"      [-pause (start paused)]",
   (char *)"To print version:",
   (char *)"  evolve_muzzes",
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
int Generations;

// Mutation rate.
PROBABILITY MutationRate = DEFAULT_MUTATION_RATE;

// Number of training trials.
int MinTrainingTrials = DEFAULT_MIN_TRAINING_TRIALS;
int MaxTrainingTrials = DEFAULT_MAX_TRAINING_TRIALS;

// Population file names.
char *InputFileName;
char *OutputFileName;

// Save default muzz.
Muzz *SaveMuzz;

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

      // RESPONSE_RANDOMNESS.
      parmMutator =
         new ParmMutator(ParmMutator::DOUBLE_PARM, (char *)"RESPONSE_RANDOMNESS",
                         (void *)&brain->RESPONSE_RANDOMNESS, 0.01, 0.2, 0.01);
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

      // Initialize effect event intervals and weights.
      brain->initEffectEventIntervals();
      brain->initEffectEventIntervalWeights();
      brain->initMaxLearningEffectEventIntervals();

      // Initialize brain with new parameters.
      Muzz::initBrain(brain);
   }


   // Copy parameters from given brain.
   void copy(BrainParmMutator *from)
   {
      for (int i = 0; i < (int)parmMutators.size(); i++)
      {
         parmMutators[i]->copy(from->parmMutators[i]);
      }

      // Initialize effect intervals.
      brain->initEffectEventIntervals();
      brain->initEffectEventIntervalWeights();
      brain->initMaxLearningEffectEventIntervals();

      // Initialize brain with new parameters.
      Muzz::initBrain(brain);
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

      // Initialize effect intervals.
      brain->initEffectEventIntervals();
      brain->initEffectEventIntervalWeights();
      brain->initMaxLearningEffectEventIntervals();

      // Initialize brain with new parameters.
      Muzz::initBrain(brain);
   }
};

// Population member.
class Member
{
public:

   Muzz   *muzz;
   double fitness;
   int    generation;

   // Brain parameter mutator.
   BrainParmMutator *brainParmMutator;

   // Muzz placement.
   static GLfloat MuzzX, MuzzY, MuzzDir;

   // Constructors.
   Member(int generation = 0)
   {
      float color[3];

      for (int i = 0; i < 3; i++)
      {
         color[i] = Randomizer->RAND_INTERVAL(0.0, 1.0);
      }
      muzz = new Muzz(color, Terrain, Randomizer->RAND(), Randomizer);
      assert(muzz != NULL);
      muzz->place(MuzzX, MuzzY, MuzzDir);

      // Create brain parameter mutator.
      brainParmMutator = new BrainParmMutator(muzz->brain);
      assert(brainParmMutator != NULL);
      brainParmMutator->mutate();

      fitness          = 0.0;
      this->generation = generation;
   }


   // Destructor.
   ~Member()
   {
      if (brainParmMutator != NULL) { delete brainParmMutator; }
      if (muzz != NULL) { delete muzz; }
   }


   // Evaluate.
   // Fitness is determined by cycle count to goals,
   // so lesser value is fitter.
   void evaluate(int trainTrials)
   {
      int foodCycle, waterCycle;

      // Use member muzz.
      Muzzes[0] = muzz;

      // Run muzz world.
      if (Graphics)
      {
         // Should not return.
         resetMuzzWorld();
         runMuzzWorld();
         Muzzes[0] = NULL;
         return;
      }

      // Training?
      if (trainTrials > 0)
      {
         ForcedResponseTrain = true;
         NumTrainingTrials   = trainTrials;
         runMuzzWorld();
         ForcedResponseTrain = false;
         NumTrainingTrials   = -1;
      }

      // Test run.
      resetMuzzWorld();
      NumTrainingTrials            = -1;
      RunMuzzWorldUntilGoalsGotten = true;
      runMuzzWorld();
      RunMuzzWorldUntilGoalsGotten = false;

      foodCycle  = Muzzes[0]->gotFoodCycle;
      waterCycle = Muzzes[0]->gotWaterCycle;

      Muzzes[0] = NULL;

      if (foodCycle != -1)
      {
         if (waterCycle != -1)
         {
            if (foodCycle > waterCycle)
            {
               sprintf(Log::messageBuf, "Found food and water at cycle %d", foodCycle);
            }
            else
            {
               sprintf(Log::messageBuf, "Found food and water at cycle %d", waterCycle);
            }
         }
         else
         {
            sprintf(Log::messageBuf, "Found only food at cycle %d", foodCycle);
         }
      }
      else
      {
         if (waterCycle != -1)
         {
            sprintf(Log::messageBuf, "Found only water at cycle %d", waterCycle);
         }
         else
         {
            sprintf(Log::messageBuf, "Food and water not found");
         }
      }
      Log::logInformation();

      // Determine fitness.
      if (foodCycle == -1) { foodCycle = Cycles; }
      if (waterCycle == -1) { waterCycle = Cycles; }
      fitness = (double)max(foodCycle, waterCycle);
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
      muzz->load(fp);
      FREAD_DOUBLE(&fitness, fp);
      FREAD_INT(&generation, fp);
   }


   // Save.
   void save(FILE *fp)
   {
      muzz->save(fp);
      FWRITE_DOUBLE(&fitness, fp);
      FWRITE_INT(&generation, fp);
   }


   // Print.
   void print()
   {
      printf("fitness=%f, generation=%d\n", getFitness(), generation);
      printf("muzz brain:\n");
      muzz->printBrain();
   }
};

// Muzz placement in world.
GLfloat Member::MuzzX   = 0.0f;
GLfloat Member::MuzzY   = 0.0f;
GLfloat Member::MuzzDir = 0.0f;

// Population.
Member *Population[POPULATION_SIZE];

// Start/end functions.
void logParameters();
void loadPopulation(FILE *fp);
void savePopulation(FILE *fp);

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
   int  i;
   FILE *fp;
   int  muzzID;
   char buf[200];

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

   // Process glut args.
   glutInit(&argc, argv);

   // Parse arguments.
   Generations   = -1;
   InputFileName = OutputFileName = NULL;
   RandomSeed    = INVALID_RANDOM;
   muzzID        = -1;
   bool gotTerrainDimension  = false;
   bool gotMinTrainingTrials = false;
   bool gotMaxTrainingTrials = false;
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

      if (strcmp(argv[i], "-muzzCycles") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         if ((Cycles = atoi(argv[i])) < 0)
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
            printError((char *)"Invalid terrain dimension");
            printUsage();
            exit(1);
         }
         gotTerrainDimension = true;
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

      if (strcmp(argv[i], "-muzz") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         if (argv[i][0] == '?')
         {
            muzzID = -2;
         }
         else
         {
            muzzID = atoi(argv[i]);
            if (muzzID < 0)
            {
               printError((char *)"Invalid muzz identifier");
               printUsage();
               exit(1);
            }
         }
         continue;
      }

      if (strcmp(argv[i], "-minTrainingTrials") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         MinTrainingTrials = atoi(argv[i]);
         if (MinTrainingTrials < 0)
         {
            printError((char *)"Invalid minimum number of training trials");
            printUsage();
            exit(1);
         }
         gotMinTrainingTrials = true;
         continue;
      }

      if (strcmp(argv[i], "-maxTrainingTrials") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            exit(1);
         }
         MaxTrainingTrials = atoi(argv[i]);
         if (MaxTrainingTrials < 0)
         {
            printError((char *)"Invalid maximum number of training trials");
            printUsage();
            exit(1);
         }
         gotMaxTrainingTrials = true;
         continue;
      }

      if (strcmp(argv[i], "-pause") == 0)
      {
         Pause = true;
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
         printVersion();
         Mona::printVersion();
         exit(0);
      }

      printUsage();
      exit(1);
   }

   if (muzzID == -1)
   {
      if (Generations == -1)
      {
         printError((char *)"Generations option required");
         printUsage();
         exit(1);
      }
      if (OutputFileName == NULL)
      {
         printError((char *)"Output file required");
         printUsage();
         exit(1);
      }
      if ((InputFileName != NULL) &&
          ((RandomSeed != INVALID_RANDOM) ||
           (ObjectSeed != INVALID_RANDOM) ||
           (TerrainSeed != INVALID_RANDOM)))
      {
         printError((char *)"Random seeds are loaded from file");
         printUsage();
         exit(1);
      }
      if ((InputFileName != NULL) && gotTerrainDimension)
      {
         printError((char *)"Terrain dimension is loaded from file");
         printUsage();
         exit(1);
      }
      if (Pause)
      {
         printError((char *)"Pause option invalid without muzz identifier");
         printUsage();
         exit(1);
      }
      if (gotMinTrainingTrials && gotMaxTrainingTrials && (MinTrainingTrials > MaxTrainingTrials))
      {
         printError((char *)"Minimum number of training trials cannot exceed maximum");
         printUsage();
         exit(1);
      }
      if (gotMinTrainingTrials && (MinTrainingTrials > MaxTrainingTrials))
      {
         MaxTrainingTrials = MinTrainingTrials;
      }
      if (gotMaxTrainingTrials && (MinTrainingTrials > MaxTrainingTrials))
      {
         MinTrainingTrials = MaxTrainingTrials;
      }
   }
   else
   {
      if (InputFileName == NULL)
      {
         printError((char *)"Input file required with muzz identifier");
         printUsage();
         exit(1);
      }
      if (OutputFileName != NULL)
      {
         printError((char *)"Output file invalid with muzz identifier");
         printUsage();
         exit(1);
      }
      if (Generations != -1)
      {
         printError((char *)"Generations option invalid with muzz identifier");
         printUsage();
         exit(1);
      }
      if ((RandomSeed != INVALID_RANDOM) ||
          (ObjectSeed != INVALID_RANDOM) ||
          (TerrainSeed != INVALID_RANDOM))
      {
         printError((char *)"Random seeds are loaded from file");
         printUsage();
         exit(1);
      }
      if (gotTerrainDimension)
      {
         printError((char *)"Terrain dimension is loaded from file");
         printUsage();
         exit(1);
      }
      if (gotMinTrainingTrials || gotMaxTrainingTrials)
      {
         printError((char *)"Training trials options invalid with muzz identifier");
         printUsage();
         exit(1);
      }
   }

   // Seed random numbers.
   if (InputFileName != NULL)
   {
      if ((RandomSeed != INVALID_RANDOM) ||
          (ObjectSeed != INVALID_RANDOM) ||
          (TerrainSeed != INVALID_RANDOM))
      {
         printError((char *)"Random seeds are loaded from input file");
         printUsage();
         exit(1);
      }
      if (gotTerrainDimension)
      {
         printError((char *)"Terrain dimension is loaded from input file");
         printUsage();
         exit(1);
      }
      if ((fp = FOPEN_READ(InputFileName)) == NULL)
      {
         sprintf(buf, "Cannot load population file %s", InputFileName);
         printError(buf);
         exit(1);
      }
      FREAD_LONG(&RandomSeed, fp);
      Randomizer = new Random(RandomSeed);
      assert(Randomizer != NULL);
      Randomizer->RAND_LOAD(fp);
      FREAD_LONG(&ObjectSeed, fp);
      FREAD_LONG(&TerrainSeed, fp);
      FREAD_INT(&TerrainDimension, fp);
      FREAD_INT(&Generation, fp);
   }

   else
   {
      fp = NULL;
      if (RandomSeed == INVALID_RANDOM)
      {
         RandomSeed = (RANDOM)time(NULL);
      }
      Randomizer = new Random(RandomSeed);
      assert(Randomizer != NULL);
      Randomizer->SRAND(RandomSeed);
      if (ObjectSeed == INVALID_RANDOM)
      {
         ObjectSeed = RandomSeed;
      }
      if (TerrainSeed == INVALID_RANDOM)
      {
         TerrainSeed = RandomSeed;
      }
   }

   if (muzzID == -1)
   {
      Log::logInformation((char *)"Initializing evolve:");
      sprintf(Log::messageBuf, "generations=%d", Generations);
      Log::logInformation();
      if (InputFileName != NULL)
      {
         sprintf(Log::messageBuf, "input=%s", InputFileName);
         Log::logInformation();
      }
      sprintf(Log::messageBuf, "output=%s", OutputFileName);
      Log::logInformation();
   }

   // Initialize muzz world.
   if (muzzID == -1)
   {
      Graphics = false;
      if (Cycles == -1)
      {
         Cycles = DEFAULT_MUZZ_CYCLES;
      }
   }

   SaveFile   = LoadFile = NULL;
   SmoothMove = false;
   initMuzzWorld();
   Member::MuzzX   = Muzzes[0]->getPlaceX();
   Member::MuzzY   = Muzzes[0]->getPlaceY();
   Member::MuzzDir = Muzzes[0]->getPlaceDirection();

   // Remove default muzz since population member will be used.
   SaveMuzz  = Muzzes[0];
   Muzzes[0] = NULL;

   // Log run parameters.
   if (muzzID == -1)
   {
      logParameters();
   }

   // Create population.
   if (fp == NULL)
   {
      for (i = 0; i < POPULATION_SIZE; i++)
      {
         Population[i] = new Member(0);
         assert(Population[i] != NULL);
      }
   }

   else
   {
      // Continue run.
      loadPopulation(fp);
      FCLOSE(fp);
   }

   // Running a muzz?
   if (muzzID != -1)
   {
      if (muzzID == -2)
      {
         sprintf(buf, "Muzz identifiers:");
         printf("%s\n", buf);
         if (Log::LOGGING_FLAG == LOG_TO_FILE)
         {
            sprintf(Log::messageBuf, "%s", buf);
            Log::logInformation();
         }
         for (i = 0; i < POPULATION_SIZE; i++)
         {
            sprintf(buf, "%d", Population[i]->muzz->id);
            printf("%s\n", buf);
            if (Log::LOGGING_FLAG == LOG_TO_FILE)
            {
               sprintf(Log::messageBuf, "%s", buf);
               Log::logInformation();
            }
         }
      }
      else
      {
         for (i = 0; i < POPULATION_SIZE; i++)
         {
            if ((Population[i] != NULL) && (Population[i]->muzz->id == muzzID))
            {
               break;
            }
         }
         if (i == POPULATION_SIZE)
         {
            printError((char *)"Invalid muzz identifier");
            exit(1);
         }
         if (Cycles == -1)
         {
            SmoothMove = true;
         }
         else
         {
            SmoothMove = false;
         }
         Population[i]->evaluate(0);
      }
   }
   else
   {
      // Evolution loop.
      Log::logInformation((char *)"Begin evolve:");

      for (i = 0; i < Generations; i++, Generation++)
      {
         sprintf(Log::messageBuf, "Generation=%d", Generation);
         Log::logInformation();
         evolve();

         // Save population?
         if ((i % SAVE_FREQUENCY) == 0)
         {
            if ((fp = FOPEN_WRITE(OutputFileName)) == NULL)
            {
               sprintf(buf, "Cannot save to population file %s", OutputFileName);
               printError(buf);
               exit(1);
            }
            FWRITE_LONG(&RandomSeed, fp);
            Randomizer->RAND_SAVE(fp);
            FWRITE_LONG(&ObjectSeed, fp);
            FWRITE_LONG(&TerrainSeed, fp);
            FWRITE_INT(&TerrainDimension, fp);
            FWRITE_INT(&Generation, fp);
            savePopulation(fp);
            FCLOSE(fp);
         }
      }

      // Save population.
      if ((fp = FOPEN_WRITE(OutputFileName)) == NULL)
      {
         sprintf(buf, "Cannot save to population file %s", OutputFileName);
         printError(buf);
         exit(1);
      }
      FWRITE_LONG(&RandomSeed, fp);
      Randomizer->RAND_SAVE(fp);
      FWRITE_LONG(&ObjectSeed, fp);
      FWRITE_LONG(&TerrainSeed, fp);
      FWRITE_INT(&TerrainDimension, fp);
      FWRITE_INT(&Generation, fp);
      savePopulation(fp);
      FCLOSE(fp);
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

   Muzzes[0] = SaveMuzz;
   termMuzzWorld();

   // Close log.
   if (muzzID == -1)
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
   sprintf(Log::messageBuf, "RandomSeed = %lu", RandomSeed);
   Log::logInformation();
   sprintf(Log::messageBuf, "ObjectSeed = %lu", ObjectSeed);
   Log::logInformation();
   sprintf(Log::messageBuf, "TerrainSeed = %lu", TerrainSeed);
   Log::logInformation();
   sprintf(Log::messageBuf, "FIT_POPULATION_SIZE = %d", FIT_POPULATION_SIZE);
   Log::logInformation();
   sprintf(Log::messageBuf, "NUM_MUTANTS = %d", NUM_MUTANTS);
   Log::logInformation();
   sprintf(Log::messageBuf, "NUM_OFFSPRING = %d", NUM_OFFSPRING);
   Log::logInformation();
   sprintf(Log::messageBuf, "MutationRate = %f", MutationRate);
   Log::logInformation();
   sprintf(Log::messageBuf, "MUZZ_CYCLES = %d", Cycles);
   Log::logInformation();
   Log::logInformation((char *)"Muzz Parameters:");
   sprintf(Log::messageBuf, "INIT_HUNGER = %f", Muzz::INIT_HUNGER);
   Log::logInformation();
   sprintf(Log::messageBuf, "INIT_THIRST = %f", Muzz::INIT_THIRST);
   Log::logInformation();
   sprintf(Log::messageBuf, "EAT_GOAL_VALUE = %f", Muzz::EAT_GOAL_VALUE);
   Log::logInformation();
   sprintf(Log::messageBuf, "DRINK_GOAL_VALUE = %f", Muzz::DRINK_GOAL_VALUE);
   Log::logInformation();
   sprintf(Log::messageBuf, "MinTrainingTrials = %d", MinTrainingTrials);
   Log::logInformation();
   sprintf(Log::messageBuf, "MaxTrainingTrials = %d", MaxTrainingTrials);
   Log::logInformation();
   Log::logInformation((char *)"Terrain Parameters:");
   sprintf(Log::messageBuf, "WIDTH = %d", Terrain->WIDTH);
   Log::logInformation();
   sprintf(Log::messageBuf, "HEIGHT = %d", Terrain->HEIGHT);
   Log::logInformation();
   sprintf(Log::messageBuf, "MAX_PLATFORM_ELEVATION = %d", Terrain->MAX_PLATFORM_ELEVATION);
   Log::logInformation();
   sprintf(Log::messageBuf, "MIN_PLATFORM_SIZE = %d", Terrain->MIN_PLATFORM_SIZE);
   Log::logInformation();
   sprintf(Log::messageBuf, "MAX_PLATFORM_SIZE = %d", Terrain->MAX_PLATFORM_SIZE);
   Log::logInformation();
   sprintf(Log::messageBuf, "MAX_PLATFORM_GENERATIONS = %d", Terrain->MAX_PLATFORM_GENERATIONS);
   Log::logInformation();
   sprintf(Log::messageBuf, "EXTRA_RAMPS = %d", Terrain->EXTRA_RAMPS);
   Log::logInformation();
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
   int i, j;

   Log::logInformation((char *)"Evaluate:");

   for (i = 0; i < POPULATION_SIZE; i++)
   {
      Muzz::initBrain(Population[i]->muzz->brain);

      // Evaluate.
      j = Randomizer->RAND_CHOICE(MaxTrainingTrials - MinTrainingTrials + 1) + MinTrainingTrials;
      Population[i]->evaluate(j);
      sprintf(Log::messageBuf, "  Member=%d, Muzz=%d, Fitness=%f, Generation=%d",
              i, Population[i]->muzz->id, Population[i]->getFitness(), Population[i]->generation);
      Log::logInformation();
   }
}


// Prune unfit members.
void prune()
{
   double min;
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
         if ((m == -1) || (member->getFitness() < min))
         {
            m   = j;
            min = member->getFitness();
         }
      }
      member           = Population[m];
      Population[m]    = NULL;
      fitPopulation[i] = member;
      sprintf(Log::messageBuf, "  Muzz=%d, Fitness=%f, Generation=%d",
              member->muzz->id, member->getFitness(), member->generation);
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
      sprintf(Log::messageBuf, "  Member=%d, Muzz=%d -> Member=%d, Muzz=%d",
              j, member->muzz->id, FIT_POPULATION_SIZE + i, mutant->muzz->id);
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
      sprintf(Log::messageBuf, "  Members=%d,%d, Muzzes=%d,%d -> Member=%d, Muzz=%d",
              j, k, member1->muzz->id, member2->muzz->id, FIT_POPULATION_SIZE + NUM_MUTANTS + i, offspring->muzz->id);
      Log::logInformation();

      // Combine parent brains into offspring.
      offspring->mindMeld(member1, member2);
   }
}
