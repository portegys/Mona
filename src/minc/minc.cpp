// For conditions of distribution and use, see copyright notice in minc.hpp

#include "minc.hpp"

// LENS parameters.
int Minc:: HIDDEN_UNITS  = 20;
real Minc::LEARNING_RATE = 0.2f;

// Lens output callback.
int Minc::  startTick;
int Minc::  tickCount;
Minc *Minc::activeMinc    = NULL;
bool Minc:: result        = false;
bool Minc:: printCallback = false;

// Minimum response potential for mona response override.
Mona::RESPONSE_POTENTIAL Minc::MIN_RESPONSE_POTENTIAL = 0.01;

// Identifier dispenser.
int Minc::idDispenser = 0;

// Constructors.
Minc::Minc(int numMarks, int intervals, RANDOM randomSeed)
{
   char buf[100];

   id              = idDispenser++;
   this->numMarks  = numMarks;
   this->intervals = intervals;

   // Create LENS NN.
   sprintf(buf, (char *)"net%d", id);
   lensNet = new char[strlen(buf) + 1];
   assert(lensNet != NULL);
   strcpy(lensNet, buf);
   sprintf(buf, (char *)"addNet %s -i %d %d %d ELMAN 2 SOFT_MAX",
           lensNet, intervals, numMarks, HIDDEN_UNITS);
   lens(buf);

   // Create Mona NN.
   mona = new Mona(1, 2, 1);
   assert(mona != NULL);

   // Random numbers.
   randomizer = new Random(randomSeed);
   assert(randomizer != NULL);
}


Minc::Minc()
{
   id         = -1;
   numMarks   = 0;
   intervals  = 0;
   lensNet    = NULL;
   mona       = NULL;
   randomSeed = 0;
   randomizer = NULL;
}


// Destructor.
Minc::~Minc()
{
   char buf[50];

   if (lensNet != NULL)
   {
      sprintf(buf, (char *)"deleteNets %s", lensNet);
      lens(buf);
      delete lensNet;
   }
   if (mona != NULL)
   {
      delete mona;
   }
   if (randomizer != NULL)
   {
      delete randomizer;
   }
}


// Generalize: train LENS.
void Minc::generalize(char *example, int numUpdates)
{
   char buf[100];

   // Use training example.
   sprintf(buf, (char *)"useTrainingSet %s", example);
   lens(buf);
   sprintf(buf, (char *)"setObj learningRate %f", LEARNING_RATE);
   lens(buf);
   sprintf(buf, (char *)"setObj numUpdates %d", numUpdates);
   lens(buf);

   // train.
   suppressLensOutput = 1;
   lens((char *)"train");
}


// Discriminate.
// Return true for success.
bool Minc::discriminate(char *example, Tmaze *maze)
{
   return(test(example, maze, false));
}


// Test.
// Return true for success.
bool Minc::test(char *example, Tmaze *maze, bool verbose)
{
   char buf[100];

   // Set up LENS output callback.
   netInputs = new real *[numMarks];
   assert(netInputs != NULL);
   netOutputs = new real *[2];
   assert(netOutputs != NULL);
   clientProc = lensCallback;
   activeMinc = this;
   result     = true;
   if (verbose)
   {
      suppressLensOutput = 0;
      printCallback      = true;
   }
   else
   {
      suppressLensOutput = 1;
      printCallback      = false;
   }
   this->maze = maze;

   // Use test example.
   sprintf(buf, (char *)"useTestingSet %s", example);
   lens(buf);

   // Run.
   lens((char *)"test");

   // Drop callback.
   delete netInputs;
   netInputs = NULL;
   delete netOutputs;
   netInputs  = NULL;
   clientProc = NULL;
   activeMinc = NULL;

   return(result);
}


// LENS output callback.
void Minc::lensCallback()
{
   int i, lensDirection, direction;

   vector<Mona::SENSOR> sensors;
   Mona                 *mona;
   Tmaze                *maze;
   Random               *randomizer;
   real                 outputActivation;

   if (!activeMinc || !result)
   {
      return;
   }
   mona       = activeMinc->mona;
   maze       = activeMinc->maze;
   randomizer = activeMinc->randomizer;
   if (exampleTick == 0)
   {
      mona->inflateNeed(0);
   }
   if (startTick >= 0)
   {
      if (tickCount == -1)
      {
         if (exampleTick == startTick)
         {
            tickCount = 0;
         }
         else
         {
            return;
         }
      }
      else
      {
         tickCount++;
      }
   }

   if (printCallback)
   {
      printf("tick=%d\n", exampleTick);
      if (netInputs != NULL)
      {
         printf("inputs:");
         for (i = 0; i < activeMinc->numMarks; i++)
         {
            printf(" %f", *netInputs[i]);
         }
         printf("\n");
      }
   }

   if (netOutputs != NULL)
   {
      // Make LENS output probabilistic.
      if (*netOutputs[Tmaze::LEFT] > *netOutputs[Tmaze::RIGHT])
      {
         outputActivation = *netOutputs[Tmaze::LEFT];
      }
      else
      {
         outputActivation = *netOutputs[Tmaze::RIGHT];
      }
      if (outputActivation > 0.0)
      {
         if (randomizer->RAND_INTERVAL(0.0, (*netOutputs[Tmaze::LEFT] + *netOutputs[Tmaze::RIGHT])) <
             *netOutputs[Tmaze::LEFT])
         {
            lensDirection = Tmaze::LEFT;
         }
         else
         {
            lensDirection = Tmaze::RIGHT;
         }
      }
      else
      {
         if (randomizer->RAND_BOOL())
         {
            lensDirection = Tmaze::LEFT;
         }
         else
         {
            lensDirection = Tmaze::RIGHT;
         }
      }

      // Cycle mona with LENS direction as conditional response override.
      mona->overrideResponseConditional(lensDirection, MIN_RESPONSE_POTENTIAL);
      sensors.push_back((Mona::SENSOR)(maze->path[exampleTick].mark));
      direction = (int)mona->cycle(sensors);
      if (printCallback)
      {
         printf("output=%d lens=%d: [%f %f]", direction,
                lensDirection, *netOutputs[0], *netOutputs[1]);
         printf(" mona=%d: [%f %f]", direction,
                mona->responsePotentials[0], mona->responsePotentials[1]);
      }

      // Reward mona if direction is correct.
      if (direction == maze->path[exampleTick].direction)
      {
         mona->setNeed(0, mona->getNeed(0) - (1.0 / (Mona::NEED)activeMinc->intervals));
      }
      else
      {
         // Otherwise expire wrong reponses and clear memory.
         mona->expireResponseEnablings((Mona::RESPONSE)direction);
         mona->clearWorkingMemory();
      }

      // Set output.
      if (direction == Tmaze::LEFT)
      {
         *netOutputs[Tmaze::LEFT]  = 1.0;
         *netOutputs[Tmaze::RIGHT] = 0.0;
      }
      else
      {
         *netOutputs[Tmaze::LEFT]  = 0.0;
         *netOutputs[Tmaze::RIGHT] = 1.0;
      }

      // Output error?
      if ((exampleTick < (int)maze->path.size()) &&
          (direction != maze->path[exampleTick].direction))
      {
         result = false;
         if (startTick >= 0)
         {
            startTick = randomizer->RAND_CHOICE((int)maze->path.size());
            tickCount = -1;
         }
         if (printCallback)
         {
            printf(" error");
         }
      }
      if (printCallback)
      {
         printf("\n");
      }
   }
}


// Load minc.
void Minc::load(char *filename)
{
   FILE *fp;

   if ((fp = FOPEN_READ(filename)) == NULL)
   {
      fprintf(stderr, "Cannot load minc from file %s\n", filename);
      exit(1);
   }
   load(fp);
   FCLOSE(fp);
}


void Minc::load(FILE *fp)
{
   int  i;
   bool b;
   char buf[100];

   FREAD_INT(&id, fp);
   if (id >= idDispenser)
   {
      idDispenser = id + 1;
   }
   FREAD_INT(&numMarks, fp);
   FREAD_INT(&intervals, fp);
   if (lensNet != NULL)
   {
      sprintf(buf, (char *)"deleteNets %s", lensNet);
      lens(buf);
      delete lensNet;
      lensNet = NULL;
   }
   FREAD_INT(&i, fp);
   if (i > 0)
   {
      lensNet = new char[i + 1];
      assert(lensNet != NULL);
      FREAD_STRING(lensNet, i, fp);
      sprintf(buf, (char *)"addNet %s -i %d %d %d ELMAN 2 SOFT_MAX",
              lensNet, intervals, numMarks, HIDDEN_UNITS);
      lens(buf);
   }
   if (mona != NULL)
   {
      delete mona;
      mona = NULL;
   }
   FREAD_BOOL(&b, fp);
   if (b)
   {
      mona = new Mona(1, 2, 1);
      assert(mona != NULL);
      mona->load(fp);
   }
   FREAD_LONG(&randomSeed, fp);
   if (randomizer != NULL)
   {
      delete randomizer;
      randomizer = NULL;
   }
   FREAD_BOOL(&b, fp);
   if (b)
   {
      randomizer->RAND_LOAD(fp);
   }
}


// Save minc.
void Minc::save(char *filename)
{
   FILE *fp;

   if ((fp = FOPEN_WRITE(filename)) == NULL)
   {
      fprintf(stderr, "Cannot save minc to file %s\n", filename);
      exit(1);
   }
   save(fp);
   FCLOSE(fp);
}


void Minc::save(FILE *fp)
{
   int  i;
   bool b;

   FWRITE_INT(&id, fp);
   FWRITE_INT(&numMarks, fp);
   FWRITE_INT(&intervals, fp);
   if (lensNet != NULL)
   {
      i = strlen(lensNet);
      FWRITE_INT(&i, fp);
      FWRITE_STRING(lensNet, i, fp);
   }
   else
   {
      i = 0;
      FWRITE_INT(&i, fp);
   }
   if (mona != NULL)
   {
      b = true;
      FWRITE_BOOL(&b, fp);
      mona->save(fp);
   }
   else
   {
      b = false;
      FWRITE_BOOL(&b, fp);
   }
   FWRITE_LONG(&randomSeed, fp);
   if (randomizer != NULL)
   {
      b = true;
      FWRITE_BOOL(&b, fp);
      randomizer->RAND_SAVE(fp);
   }
   else
   {
      b = false;
      FWRITE_BOOL(&b, fp);
   }
}
