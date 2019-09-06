/*

Run a set of meta-maze tests given a maze_maker discovery file

Options: [-contextMaze [-modular (modular training)]] <maze_maker maze discovery file>

Each maze entry in the file looks like this:

numRooms=5, numDoors=3, numGoals=1
contextSizes: 5
effectDelayScale=10
metaSeed=1755873489
instance seeds/frequencies: 1755873490 (1.000000) 1755873495 (1.000000)

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define LINESZ 500
#define NUMSZ 50
#define MAX_INSTANCES 50

int main(int argc, char *argv[])
{
    int i,j,k,nr,nd,cs,ed,ni,avei,avef,numtest;
    unsigned long ms,is[MAX_INSTANCES];
    FILE *fp,*cmdfp;
    char buf[LINESZ + 1],numbuf[NUMSZ + 1];
    bool contextMaze,modular;

    contextMaze = modular = false;
    if (argc < 2 || argc > 4)
    {
        fprintf(stderr, "Usage: %s [-contextMaze [-modular (modular training)]] <maze_maker maze discovery file>\n", argv[0]);
        return 1;
    }
    if (argc == 2)
    {
        i = 1;
    }
    else if (argc == 3)
    {
        i = 2;
        if (strcmp(argv[1], "-contextMaze") == 0)
        {
            contextMaze = true;
        }
        else
        {
            fprintf(stderr, "Usage: %s [-contextMaze [-modular (modular training)]] <maze_maker maze discovery file>\n", argv[0]);
            return 1;
        }
    }
    else
    {
        i = 3;
        if ((strcmp(argv[1], "-contextMaze") == 0 && strcmp(argv[2], "-modular") == 0) ||
            (strcmp(argv[2], "-contextMaze") == 0 && strcmp(argv[1], "-modular") == 0))
        {
            contextMaze = true;
            modular = true;
        }
        else
        {
            fprintf(stderr, "Usage: %s [-contextMaze [-modular (modular training)]] <maze_maker maze discovery file>\n", argv[0]);
            return 1;
        }
    }

    if ((fp = fopen(argv[i], "r")) == NULL)
    {
        fprintf(stderr, "Cannot open file %s\n", argv[i]);
        return 1;
    }

    // Run mazes.
    avei = avef = numtest = 0;
    while (fgets(buf, LINESZ, fp) != NULL)
    {
        // Get maze parameters.
        if (strncmp(buf, "numRooms=", 9) == 0)
        {
            numtest++;
            for (i = j = 9; j < LINESZ && buf[j] != ','; j++) {}
            assert(buf[j] == ',');
            buf[j] = '\0';
            nr = atoi(&buf[i]);
            for (; i < LINESZ && buf[i] != '='; i++) {}
            assert(buf[i] == '=');
            i++;
            for (j = i + 1; j < LINESZ && buf[j] != ','; j++) {}
            assert(buf[j] == ',');
            buf[j] = '\0';
            nd = atoi(&buf[i]);
            assert(fgets(buf, LINESZ, fp) != NULL);
            for (i = 0; i < LINESZ && buf[i] != ' '; i++) {}
            assert(buf[i] == ' ');
            i++;
            for (j = i + 1; j < LINESZ && buf[j] >= '0' && buf[j] <= '9'; j++) {}
            assert(j < LINESZ);
            buf[j] = '\0';
            cs = atoi(&buf[i]);
            assert(fgets(buf, LINESZ, fp) != NULL);
            for (i = 0; i < LINESZ && buf[i] != '='; i++) {}
            assert(buf[i] == '=');
            i++;
            for (j = i + 1; j < LINESZ && buf[j] >= '0' && buf[j] <= '9'; j++) {}
            assert(j < LINESZ);
            buf[j] = '\0';
            ed = atoi(&buf[i]);
            assert(fgets(buf, LINESZ, fp) != NULL);
            for (i = 0; i < LINESZ && buf[i] != '='; i++) {}
            assert(buf[i] == '=');
            i++;
            for (j = i + 1; j < LINESZ && buf[j] >= '0' && buf[j] <= '9'; j++) {}
            assert(j < LINESZ);
            buf[j] = '\0';
            ms = (unsigned long)strtol(&buf[i], NULL, 10);
            assert(fgets(buf, LINESZ, fp) != NULL);
            for (i = 0; i < LINESZ && buf[i] != ':'; i++) {}
            assert(buf[i] == ':');
            i += 2;
            k = 0;
            while (true)
            {
                assert(k < MAX_INSTANCES);
                for (j = i + 1; j < LINESZ && isdigit(buf[j]); j++) {}
                assert(j < LINESZ);
                buf[j] = '\0';
                is[k] = (unsigned long)strtol(&buf[i], NULL, 10);
                k++;
                avei++;
                for (i = j + 1; i < LINESZ && buf[i] != ')'; i++) {}
                assert(buf[i] == ')');
                i++;
                for (; i < LINESZ; i++)
                {
                    if (isdigit(buf[i]) || buf[i] == '-') break;
                    if (buf[i] == '\r' || buf[i] == '\n') break;
                }
                assert(i < LINESZ);
                if (buf[i] == '\r' || buf[i] == '\n') break;
            }

            // Train maze.
            if (contextMaze)
            {
                if (modular)
                {
                    sprintf(buf, "./maze_tester -trials 200 -train -cycles 40 -contextMaze -modular -markPath -save maze_save.txt -numRooms %d -numDoors %d -numGoals 1 -contextSizes %d -effectDelayScale %d -metaSeed %d -instanceSeeds ", nr, nd, cs, ed, ms);
                }
                else
                {
                    sprintf(buf, "./maze_tester -trials 200 -train -cycles 40 -contextMaze -markPath -save maze_save.txt -numRooms %d -numDoors %d -numGoals 1 -contextSizes %d -effectDelayScale %d -metaSeed %d -instanceSeeds ", nr, nd, cs, ed, ms);
                }
            }
            else
            {
                sprintf(buf, "./maze_tester -trials 200 -train -cycles 40 -markPath -save maze_save.txt -numRooms %d -numDoors %d -numGoals 1 -contextSizes %d -effectDelayScale %d -metaSeed %d -instanceSeeds ", nr, nd, cs, ed, ms);
            }
            for (i = 0; i < k; i++)
            {
                if (i < k - 1)
                {
                    sprintf(numbuf, "%d,", is[i]);
                }
                else
                {
                    sprintf(numbuf, "%d", is[i]);
                }
                strcat(buf,numbuf);
            }
            strcat(buf, " > train.out 2>&1");
            system(buf);

            // Test maze.
            sprintf(buf, "./maze_tester -trials 100 -cycles 40 -load maze_save.txt | tail -1 | cut -d\"/\" -f1 | cut -d\" \" -f5");
            assert((cmdfp = popen(buf, "r")) != NULL);
            fscanf(cmdfp, "%s", numbuf);
            printf("%s/100\n", numbuf);
            avef += atoi(numbuf);
            fflush(stdout);
            pclose(cmdfp);
        }
    }
    if (numtest > 0)
    {
        printf("average instances=%f, average found=%f, tests=%d\n",
            (double)avei/(double)numtest, (double)avef/(double)numtest, numtest);
    }
    else
    {
        printf("No tests!\n");
    }
    fclose(fp);

    return 0;
}
