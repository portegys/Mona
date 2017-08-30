// Learn T-maze.

#include "../../src/minc/TmazeGrammar.hpp"
#ifdef __cplusplus
extern "C" {
#include "../Src/lens.h"
}
#endif

// Number of marks.
int NumMarks;

// Mazes.
vector<Tmaze * > Mazes;

// Testing mode.
bool Testing = false;
int TestNum = -1;
vector<bool> TestResults;

bool printClient = true;

// Client callback from NN test.
void myClientProc()
{
  int i;
  Tmaze *maze;

  if (!Testing) return;
  if (exampleTick == 0)
  {
    TestNum++;
    TestResults.push_back(true);
    if (printClient) printf("test=%d\n", TestNum);
  }
  if (TestNum < 0 || TestNum >= (int)Mazes.size()) return;
  maze = Mazes[TestNum];

  if (printClient)
  {
    printf("tick=%d\n",exampleTick);
    if (netInputs != NULL)
    {
      printf("inputs:");
      for (i = 0; i < NumMarks; i++) {
        printf(" %f", *netInputs[i]);
      }
      printf("\n");
    }
  }
  if (netOutputs != NULL)
  {
    if (printClient)
    {
      printf("outputs:");
      for (i = 0; i < 2; i++) {
        printf(" %f", *netOutputs[i]);
      }
    }
    if (*netOutputs[0] > *netOutputs[1])
    {
      if (maze->path[exampleTick].direction)
      {
        TestResults[TestNum] = false;
        if (printClient)
        {
          printf(" error");
        }
      }
    } else if (*netOutputs[0] < *netOutputs[1])
    {
      if (!maze->path[exampleTick].direction)
      {
        TestResults[TestNum] = false;
        if (printClient)
        {
          printf(" error");
        }
      }
    }
    if (printClient)
    {
      printf("\n");
    }
  }
}

int main(int argc, char *argv[])
{
   int  i, j, m, randomSeed, numMazes, hidden, intervals;
   Tmaze *maze;
   FILE *fp;
   char line[1024];

   // Generate maze.
   printf("Enter random seed: ");
   scanf("%s", line);
   randomSeed = atoi(line);
   printf("Enter number of maze marks: ");
   scanf("%s", line);
   NumMarks = atoi(line);
   printf("Enter number of mazes: ");
   scanf("%s", line);
   numMazes = atoi(line);
   TmazeGrammar            *tmg = new TmazeGrammar(randomSeed, NumMarks);
   for (m = 0; m < numMazes; m++)
   {
     maze = tmg->generateMaze();
     Mazes.push_back(maze);
   }

   // Create example file.
   fp = fopen("Tmaze.ex", "w");
   intervals = 0;
   for (m = 0; m < numMazes; m++)
   {
     maze = Mazes[m];
     if (intervals < (int)maze->path.size()) intervals = (int)maze->path.size();
     fprintf(fp, "name: { ");
     for (i = 0; i < (int)maze->path.size(); i++)
     {
       fprintf(fp, "%d ", maze->path[i].mark);
     }
     fprintf(fp, "} %d\n", (int)maze->path.size());
     for (i = 0; i < (int)maze->path.size(); i++)
     {
       fprintf(fp, "I:");
       for (j = 0; j < NumMarks; j++)
       {
         if (j == maze->path[i].mark)
         {
           fprintf(fp, " 1");
         } else {
           fprintf(fp, " 0");
         }
       }
       fprintf(fp, " T: ");
       if (maze->path[i].direction)
       {
         fprintf(fp, "0 1");
       } else {
         fprintf(fp, "1 0");
       }
       if (i < (int)maze->path.size() - 1)
       {
         fprintf(fp, "\n");
       } else {
         fprintf(fp, ";\n");
       }
     }
   }
   fclose(fp);
   
    // Access NN.
    netInputs = new real *[NumMarks];
    netOutputs = new real *[2];
    clientProc = myClientProc;

   // Start lens.
   if (startLens(argv[0]))
   {
      fprintf(stderr, "Lens Failed\n");
      return(1);
   }

   // Create NN.
   suppressLensOutput = 1;
   suppressLensOutput = 0; // flibber
   printf("Enter number of hidden units: ");
   scanf("%s", line);
   hidden = atoi(line);
   sprintf(line, "addNet Tmaze -i %d %d %d ELMAN 2 SOFT_MAX", intervals, NumMarks, hidden);
   lens(line);

   // Load training example.
   lens((char *)"loadExamples Tmaze.ex");
   lens((char *)"setObj learningRate 0.2");
   lens((char *)"setObj numUpdates 500");

   // Train.
   //lens("train");
   
   // Test.
   //lens("test");

   while (fgets(line, 1024, stdin))
   {
      if (strncmp(line, "test", 4) == 0)
      {
        Testing = true;
        TestNum = -1;
        TestResults.clear();
      } else {
        Testing = false;
      }
      lens(line);
      if (strncmp(line, "test", 4) == 0)
      {
        Testing = false;
        printf("Test\tResult\n");
        for (i = 0; i < (int)TestResults.size(); i++)
        {
          printf("%d\t%d\n", i, (int)TestResults[i]);
        }
      }
   }

   return(0);
}
