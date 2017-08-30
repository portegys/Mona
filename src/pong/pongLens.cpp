// For conditions of distribution and use, see copyright notice in pong.hpp

// Pong train and test with Lens NN.
// Reference: http://web.stanford.edu/group/mbc/LENSManual/Manual

#ifdef __cplusplus
extern "C" {
#include "../../lens/Src/lens.h"
}
#endif
#ifdef WIN32
#include <process.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
using namespace std;

// Pong game examples files.
char *PongTrainingExamplesFile = NULL;
char *PongTestingExamplesFile  = NULL;

char *Usage[] =
{
   (char *)"pong_lens\n",
   (char *)"      -trainingExamples <Pong game training examples file>\n",
   (char *)"      [-trainingEpochs <number of training epochs>]\n",
   (char *)"      -testingExamples <Pong game testing examples file>\n",
   NULL
};


void printUsage()
{
   for (int i = 0; Usage[i] != NULL; i++)
   {
      fprintf(stderr, "%s", Usage[i]);
   }
}


// Number of training epochs.
int TrainingEpochs = 1;

// Lens parameters.
int  HIDDEN_UNITS  = 20;
real LEARNING_RATE = 0.2f;

// Lens callback.
void lensCallback();

bool PrintCallback = false;

// Testing target outputs.
vector<vector<vector<float> > > TestingTargetOutputs;
int   TestNumber;
int   TickCounter;
float ErrorAccumulator;
int   GameCorrectLengthAccum;
int   GameLengthAccum;
bool  GameError;
int   GameCorrectLength;
int   GameLength;

int
main(int argc, char *argv[])
{
   int  i, n, s, intervals;
   FILE *fp;
   char buf[BUFSIZ];
   int  t[7];

   for (i = 1; i < argc; i++)
   {
      if (strcmp(argv[i], "-trainingExamples") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         PongTrainingExamplesFile = argv[i];
         continue;
      }

      if (strcmp(argv[i], "-trainingEpochs") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         TrainingEpochs = atoi(argv[i]);
         if (TrainingEpochs < 0)
         {
            printUsage();
            return(1);
         }
         continue;
      }

      if (strcmp(argv[i], "-testingExamples") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         PongTestingExamplesFile = argv[i];
         continue;
      }

      printUsage();
      return(1);
   }

   if ((PongTrainingExamplesFile == NULL) || (PongTestingExamplesFile == NULL))
   {
      printUsage();
      return(1);
   }

   // Start lens.
   if (startLens(argv[0]))
   {
      fprintf(stderr, "Lens failed to start\n");
      return(1);
   }

   // Determine max intervals.
   if ((fp = fopen(PongTrainingExamplesFile, "r")) == NULL)
   {
      fprintf(stderr, "Cannot open %s\n", PongTrainingExamplesFile);
      return(1);
   }
   intervals = 0;
   while (fgets(buf, BUFSIZ, fp) != NULL)
   {
      if (strncmp(buf, "name:", 5) == 0)
      {
         for (i = 0; buf[i] != '}' && buf[i] != '\0'; i++)
         {
         }
         if (buf[i] == '}')
         {
            i++;
            i = atoi(&buf[i]);
            if (intervals < i)
            {
               intervals = i;
            }
         }
      }
   }
   fclose(fp);
   if ((fp = fopen(PongTestingExamplesFile, "r")) == NULL)
   {
      fprintf(stderr, "Cannot open %s\n", PongTestingExamplesFile);
      return(1);
   }
   n = -1;
   while (fgets(buf, BUFSIZ, fp) != NULL)
   {
      if (strncmp(buf, "name:", 5) == 0)
      {
         for (i = 0; buf[i] != '}' && buf[i] != '\0'; i++)
         {
         }
         if (buf[i] == '}')
         {
            i++;
            i = atoi(&buf[i]);
            if (intervals < i)
            {
               intervals = i;
            }
            n++;
            TestingTargetOutputs.resize(n + 1);
            s = -1;
         }
      }
      else
      {
         s++;
         TestingTargetOutputs[n].resize(s + 1);
         for (i = 0; buf[i] != 'T' && buf[i] != '\0'; i++)
         {
         }
         if (buf[i] == 'T')
         {
            i += 3;
            sscanf(&buf[i], "%d %d %d %d %d %d %d", &t[0], &t[1], &t[2], &t[3], &t[4], &t[5], &t[6]);
            for (i = 0; i < 7; i++)
            {
               TestingTargetOutputs[n][s].push_back((float)t[i]);
            }
         }
      }
   }
   fclose(fp);

   // Load pong examples.
   suppressLensOutput = 0;
   sprintf(buf, (char *)"loadExamples %s -set pong_training_examples -exmode PERMUTED", PongTrainingExamplesFile);
   lens(buf);
   sprintf(buf, (char *)"loadExamples %s -set pong_testing_examples", PongTestingExamplesFile);
   lens(buf);

   // Create LENS NN.
   sprintf(buf, (char *)"addNet pong_net -i %d 8 %d ELMAN 7 SOFT_MAX", intervals, HIDDEN_UNITS);
   lens(buf);

   // Train.
   sprintf(buf, (char *)"useTrainingSet pong_training_examples");
   lens(buf);
   sprintf(buf, (char *)"setObj learningRate %f", LEARNING_RATE);
   lens(buf);
   sprintf(buf, (char *)"setObj numUpdates %d", TrainingEpochs);
   lens(buf);
   lens((char *)"train");

   // Set up LENS output callback.
   netInputs = new real *[8];
   assert(netInputs != NULL);
   netOutputs = new real *[7];
   assert(netOutputs != NULL);
   clientProc             = lensCallback;
   suppressLensOutput     = 0;
   PrintCallback          = true;
   TestNumber             = -1;
   TickCounter            = 0;
   ErrorAccumulator       = 0.0f;
   GameCorrectLengthAccum = 0;
   GameLengthAccum        = 0;
   GameError         = false;
   GameCorrectLength = 0;
   GameLength        = 0;

   // Test.
   sprintf(buf, (char *)"useTestingSet pong_testing_examples");
   lens(buf);
   lens((char *)"test");
   GameCorrectLengthAccum += GameCorrectLength;
   GameLengthAccum        += GameLength;

   // Print mean error.
   printf("Mean error = ");
   if (TickCounter > 0)
   {
      printf("%f", ErrorAccumulator / (float)TickCounter);
   }
   else
   {
      printf("unavailable");
   }
   printf("\n");
   printf("Mean correct in game play sequence = ");
   if (GameLengthAccum > 0)
   {
      printf("%f", (float)GameCorrectLengthAccum / (float)GameLengthAccum);
   }
   else
   {
      printf("unavailable");
   }
   printf("\n");

   // Drop callback.
   delete netInputs;
   netInputs = NULL;
   delete netOutputs;
   netInputs  = NULL;
   clientProc = NULL;

   return(0);
}


// Lens callback.
void lensCallback()
{
   int   i;
   float error;

   if (exampleTick == 0)
   {
      TestNumber++;
      GameCorrectLengthAccum += GameCorrectLength;
      GameLengthAccum        += GameLength;
      GameError         = false;
      GameCorrectLength = 0;
      GameLength        = 0;
   }
   TickCounter++;
   error = 0.0f;
   for (i = 0; i < 7; i++)
   {
      error += fabs(*netOutputs[i] - TestingTargetOutputs[TestNumber][exampleTick][i]);
   }
   ErrorAccumulator += error;
   GameLength++;
   if (error > 0.25f)
   {
      GameError = true;
   }
   if (!GameError)
   {
      GameCorrectLength++;
   }
   if (PrintCallback)
   {
      printf("tick=%d\n", exampleTick);
      printf("input: ");
      for (i = 0; i < 8; i++)
      {
         printf("%f ", *netInputs[i]);
      }
      printf("output: ");
      for (i = 0; i < 7; i++)
      {
         printf("%f ", *netOutputs[i]);
      }
      printf("target: ");
      for (i = 0; i < 7; i++)
      {
         printf("%f ", TestingTargetOutputs[TestNumber][exampleTick][i]);
      }
      printf("\n");
   }
}
