// For conditions of distribution and use, see copyright notice in minc.hpp

// The minc T-maze world.

#include "TmazeGrammar.hpp"
#include "minc.hpp"
#ifdef WIN32
#include <process.h>
#endif

// Version (SCCS "what" format).
#define MINC_WORLD_VERSION    "@(#)Minc world version 1.0"
const char *MincWorldVersion = MINC_WORLD_VERSION;

// Parameters.
int    NumMazeMarks;
int    NumMazes;
RANDOM RandomSeed;
int    GeneralizationRuns;
int    DiscriminationRuns;
int    ResultRuns;

// Discrimination success criterion: number of consecutive successful runs.
int DISCRIMINATION_SUCCESS_RUNS = 10;

// T-maze grammar.
TmazeGrammar *MazeGrammar;

// T-mazes.
vector<Tmaze *> GeneralizationMazes;
Tmaze           *DiscriminationMaze;

// LENS examples.
#define GENERALIZATION_EXAMPLE    "TmazeGeneralization"
#define DISCRIMINATION_EXAMPLE    "TmazeDiscrimination"
char GeneralizationFileName[BUFSIZ];
char DiscriminationFileName[BUFSIZ];

char *Usage[] =
{
   (char *)"minc_world\n",
   (char *)"      -numMazeMarks <number of maze marks (must be >= 2)>\n",
   (char *)"      -numMazes <number of T-mazes>\n",
   (char *)"      -randomSeed <generation seed>\n",
   (char *)"      [-generalizationRuns <number of generalization runs>]\n",
   (char *)"      [-discriminationRuns <number of discrimination runs>]\n",
   (char *)"      -resultRuns <number of result runs>\n",
   NULL
};

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
   int    i, j, k, intervals;
   Random *randomizer;
   Tmaze  *maze;
   FILE   *fp;
   char   buf[100];

   NumMazeMarks       = -1;
   NumMazes           = -1;
   GeneralizationRuns = -1;
   DiscriminationRuns = -1;
   ResultRuns         = -1;
   RandomSeed         = INVALID_RANDOM;
   for (i = 1; i < argc; i++)
   {
      if (strcmp(argv[i], "-numMazeMarks") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         NumMazeMarks = atoi(argv[i]);
         if (NumMazeMarks < 2)
         {
            printUsage();
            return(1);
         }
         continue;
      }

      if (strcmp(argv[i], "-numMazes") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         NumMazes = atoi(argv[i]);
         if (NumMazes < 1)
         {
            printUsage();
            return(1);
         }
         continue;
      }

      if (strcmp(argv[i], "-randomSeed") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         RandomSeed = atoi(argv[i]);
         continue;
      }

      if (strcmp(argv[i], "-generalizationRuns") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         GeneralizationRuns = atoi(argv[i]);
         if (GeneralizationRuns < 0)
         {
            printUsage();
            return(1);
         }
         continue;
      }

      if (strcmp(argv[i], "-discriminationRuns") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         DiscriminationRuns = atoi(argv[i]);
         if (DiscriminationRuns < 0)
         {
            printUsage();
            return(1);
         }
         continue;
      }

      if (strcmp(argv[i], "-resultRuns") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         ResultRuns = atoi(argv[i]);
         if (ResultRuns < 0)
         {
            printUsage();
            return(1);
         }
         continue;
      }

      printUsage();
      return(1);
   }

   if ((NumMazeMarks == -1) || (NumMazes == -1) ||
       (RandomSeed == INVALID_RANDOM) || (ResultRuns == -1))
   {
      printUsage();
      return(1);
   }
   else
   {
      printf("Parameters:\n");
      printf("numMazeMarks=%d\n", NumMazeMarks);
      printf("numMazes=%d\n", NumMazes);
      printf("randomSeed=%lu\n", RandomSeed);
      printf("generalizationRuns=%d\n", GeneralizationRuns);
      printf("discriminationRuns=%d\n", DiscriminationRuns);
      printf("resultRuns=%d\n", ResultRuns);
   }

   // Get random numbers.
   randomizer = new Random(RandomSeed);
   assert(randomizer != NULL);

   // Generate mazes.
   MazeGrammar = new TmazeGrammar(RandomSeed, NumMazeMarks);
   assert(MazeGrammar != NULL);
   for (i = 0; i < (NumMazes * 100) && (int)GeneralizationMazes.size() < NumMazes; i++)
   {
      maze = MazeGrammar->generateMaze();
      assert(maze != NULL);
      for (j = 0; j < (int)GeneralizationMazes.size(); j++)
      {
         if (GeneralizationMazes[j]->isDuplicate(maze))
         {
            break;
         }
      }
      if (j == (int)GeneralizationMazes.size())
      {
         GeneralizationMazes.push_back(maze);
      }
      else
      {
         delete maze;
      }
   }
   if ((int)GeneralizationMazes.size() < NumMazes)
   {
      fprintf(stderr, "Cannot create %d unique files\n", NumMazes);
      return(1);
   }

   // Create generalization example file.
   printf("Generalization mazes:\n");
#ifdef WIN32
   sprintf(GeneralizationFileName, "%s_%d.ex", GENERALIZATION_EXAMPLE, _getpid());
#else
   sprintf(GeneralizationFileName, "/tmp/%s_%d.ex", GENERALIZATION_EXAMPLE, getpid());
#endif
   if ((fp = fopen(GeneralizationFileName, "w")) == NULL)
   {
      fprintf(stderr, "Cannot create generalization example file %s\n", GeneralizationFileName);
      return(1);
   }
   intervals = 0;
   for (i = 0; i < NumMazes; i++)
   {
      maze = GeneralizationMazes[i];
      if (intervals < (int)maze->path.size())
      {
         intervals = (int)maze->path.size();
      }
      printf("name: { ");
      fprintf(fp, "name: { ");
      for (j = 0; j < (int)maze->path.size(); j++)
      {
         printf("%d ", maze->path[j].mark);
         fprintf(fp, "%d ", maze->path[j].mark);
      }
      printf("} %d\n", (int)maze->path.size());
      fprintf(fp, "} %d\n", (int)maze->path.size());
      for (j = 0; j < (int)maze->path.size(); j++)
      {
         printf("I:");
         fprintf(fp, "I:");
         for (k = 0; k < NumMazeMarks; k++)
         {
            if (k == maze->path[j].mark)
            {
               printf(" 1");
               fprintf(fp, " 1");
            }
            else
            {
               printf(" 0");
               fprintf(fp, " 0");
            }
         }
         printf(" T: ");
         fprintf(fp, " T: ");
         if (maze->path[j].direction)
         {
            printf("0 1");
            fprintf(fp, "0 1");
         }
         else
         {
            printf("1 0");
            fprintf(fp, "1 0");
         }
         printf(" P: %f %f", maze->path[j].probability, 1.0 - maze->path[j].probability);
         if (j < (int)maze->path.size() - 1)
         {
            printf("\n");
            fprintf(fp, "\n");
         }
         else
         {
            printf(";\n");
            fprintf(fp, ";\n");
         }
      }
   }
   fclose(fp);

   // Create discrimination example file.
   printf("Discrimination maze:\n");
   DiscriminationMaze = maze = GeneralizationMazes[randomizer->RAND_CHOICE(NumMazes)];
#ifdef WIN32
   sprintf(DiscriminationFileName, "%s_%d.ex", DISCRIMINATION_EXAMPLE, _getpid());
#else
   sprintf(DiscriminationFileName, "/tmp/%s_%d.ex", DISCRIMINATION_EXAMPLE, getpid());
#endif
   if ((fp = fopen(DiscriminationFileName, "w")) == NULL)
   {
      fprintf(stderr, "Cannot create discrimination example file %s\n", DiscriminationFileName);
      return(1);
   }
   printf("name: { ");
   fprintf(fp, "name: { ");
   for (i = 0; i < (int)maze->path.size(); i++)
   {
      printf("%d ", maze->path[i].mark);
      fprintf(fp, "%d ", maze->path[i].mark);
   }
   printf("} %d\n", (int)maze->path.size());
   fprintf(fp, "} %d\n", (int)maze->path.size());
   for (i = 0; i < (int)maze->path.size(); i++)
   {
      printf("I:");
      fprintf(fp, "I:");
      for (j = 0; j < NumMazeMarks; j++)
      {
         if (j == maze->path[i].mark)
         {
            printf(" 1");
            fprintf(fp, " 1");
         }
         else
         {
            printf(" 0");
            fprintf(fp, " 0");
         }
      }
      printf(" T: ");
      fprintf(fp, " T: ");
      if (maze->path[i].direction)
      {
         printf("0 1");
         fprintf(fp, "0 1");
      }
      else
      {
         printf("1 0");
         fprintf(fp, "1 0");
      }
      printf(" P: %f %f", maze->path[i].probability, 1.0 - maze->path[i].probability);
      if (i < (int)maze->path.size() - 1)
      {
         printf("\n");
         fprintf(fp, "\n");
      }
      else
      {
         printf(";\n");
         fprintf(fp, ";\n");
      }
   }
   fclose(fp);

   // Start lens.
   if (startLens(argv[0]))
   {
      fprintf(stderr, "Lens failed to start\n");
      return(1);
   }

   // Load maze examples.
   sprintf(buf, (char *)"loadExamples %s -set %s -exmode PERMUTED",
           GeneralizationFileName, GENERALIZATION_EXAMPLE);
   lens(buf);
   sprintf(buf, (char *)"loadExamples %s -set %s", DiscriminationFileName,
           DISCRIMINATION_EXAMPLE);
   lens(buf);
#ifdef WIN32
   _unlink(GeneralizationFileName);
   _unlink(DiscriminationFileName);
#else
   unlink(GeneralizationFileName);
   unlink(DiscriminationFileName);
#endif

   // Create minc.
   Minc *minc = new Minc(NumMazeMarks, intervals, RandomSeed);
   assert(minc != NULL);

   // Generalization runs.
   if (GeneralizationRuns > 0)
   {
      minc->generalize((char *)GENERALIZATION_EXAMPLE, GeneralizationRuns);
   }

   // Discrimination runs.
   if (DiscriminationRuns > 0)
   {
      Minc::startTick = randomizer->RAND_CHOICE((int)DiscriminationMaze->path.size());
      Minc::tickCount = -1;
      for (i = 0; i < DiscriminationRuns; i++)
      {
         minc->discriminate((char *)DISCRIMINATION_EXAMPLE, DiscriminationMaze);
      }
   }

   // Result runs.
   printf("Result runs:\n");
   Minc::startTick = -1;
   for (i = j = 0; i < ResultRuns; i++)
   {
      if (minc->test((char *)DISCRIMINATION_EXAMPLE, DiscriminationMaze))
      {
         j++;
         printf("Result run=%d: success\n", i);
      }
      else
      {
         printf("Result run=%d: fail\n", i);
      }
   }
   printf("Result summary: success/total=%d/%d", j, ResultRuns);
   if (ResultRuns > 0)
   {
      printf(" (%f)", (float)j / (float)ResultRuns);
   }
   if (NumMazes > 0)
   {
      for (i = j = 0; i < NumMazes; i++)
      {
         maze = GeneralizationMazes[i];
         j   += (int)maze->path.size();
      }
      printf("; average maze path length=%f; discrimination maze path length=%d",
             (float)j / (float)NumMazes, (int)DiscriminationMaze->path.size());
   }
   printf("\n");
   return(0);
}
