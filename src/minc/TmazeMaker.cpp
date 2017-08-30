// For conditions of distribution and use, see copyright notice in minc.hpp

/*
 * Tmaze-making program.
 * Generate T-mazes using the given parameters and output in the
 * LENS neural network example file format.
 */

#include "Tmaze.hpp"
#include "TmazeGrammar.hpp"

// Mazes.
vector<Tmaze *> Mazes;

char *Usage[] =
{
   (char *)"Tmaze_maker\n",
   (char *)"      -mazeSeed <maze random seed>\n",
   (char *)"      -numMazeMarks <number of maze marks (must be >= 2)>\n",
   (char *)"      -numMazes <number of mazes to generate>\n",
   (char *)"      [-outputFile <output file name> (otherwise to standard output)]\n",
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
   int   i, j, m, mazeSeed, numMazeMarks, numMazes, intervals;
   char  *outputFile;
   Tmaze *maze;
   FILE  *fp;

   // Get options.
   mazeSeed   = numMazeMarks = numMazes = -1;
   outputFile = NULL;
   for (i = 1; i < argc; i++)
   {
      if (strcmp(argv[i], "-mazeSeed") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         mazeSeed = atoi(argv[i]);
         continue;
      }

      if (strcmp(argv[i], "-numMazeMarks") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         numMazeMarks = atoi(argv[i]);
         if (numMazeMarks < 2)
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
            exit(1);
         }
         numMazes = atoi(argv[i]);
         if (numMazes < 0)
         {
            printUsage();
            return(1);
         }
         continue;
      }

      if (strcmp(argv[i], "-outputFile") == 0)
      {
         i++;
         if (i >= argc)
         {
            printUsage();
            return(1);
         }
         outputFile = argv[i];
         continue;
      }

      printUsage();
      return(1);
   }

   if ((mazeSeed < 0) || (numMazeMarks < 0) || (numMazes < 0))
   {
      printUsage();
      return(1);
   }

   // Generate mazes.
   TmazeGrammar *tmg = new TmazeGrammar(mazeSeed, numMazeMarks);
   for (m = 0; m < numMazes; m++)
   {
      maze = tmg->generateMaze();
      assert(maze != NULL);
      Mazes.push_back(maze);
   }

   // Output mazes.
   if (outputFile != NULL)
   {
      if ((fp = fopen(outputFile, "w")) == NULL)
      {
         fprintf(stderr, "Cannot open maze output file %s for writing\n", outputFile);
         return(1);
      }
   }
   else
   {
      fp = stdout;
   }
   intervals = 0;
   for (m = 0; m < numMazes; m++)
   {
      maze = Mazes[m];
      if (intervals < (int)maze->path.size())
      {
         intervals = (int)maze->path.size();
      }
      fprintf(fp, "name: { ");
      for (i = 0; i < (int)maze->path.size(); i++)
      {
         fprintf(fp, "%d ", maze->path[i].mark);
      }
      fprintf(fp, "} %d\n", (int)maze->path.size());
      for (i = 0; i < (int)maze->path.size(); i++)
      {
         fprintf(fp, "I:");
         for (j = 0; j < numMazeMarks; j++)
         {
            if (j == maze->path[i].mark)
            {
               fprintf(fp, " 1");
            }
            else
            {
               fprintf(fp, " 0");
            }
         }
         fprintf(fp, " T: ");
         if (maze->path[i].direction)
         {
            fprintf(fp, "0 1");
         }
         else
         {
            fprintf(fp, "1 0");
         }
         if (i < (int)maze->path.size() - 1)
         {
            fprintf(fp, "\n");
         }
         else
         {
            fprintf(fp, ";\n");
         }
      }
   }
   if (outputFile != NULL)
   {
      fclose(fp);
   }
   return(0);
}
