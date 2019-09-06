/*

Train and test SNNS (Stuttgart Neural Network Simulator)
meta-maze patterns given in a maze_maker maze discovery file.

Options: -epochs <training epochs>
         [-contextMaze (generate context patterns)]
         [-filePrefix <file prefix>
            (file names: <prefix>-<metaSeed>-<instanceSeed>.pat)]
         -maze_maker_file <maze_maker maze discovery file>

Each maze entry in the maze_maker file looks like this:

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

#define EPOCHS 1000
#define LINESZ 500
#define NUMSZ 50
#define MAX_INSTANCES 50

char *Usage[] =
{
    "test_metamaze\n",
    "      -epochs <training epochs>\n",
    "      [-contextMaze (generate context patterns)]\n",
    "      [-filePrefix <file prefix>]\n",
    "      -maze_maker_file <maze_maker maze discovery file>\n",
    NULL
};

void printUsage()
{
    for (int i = 0; Usage[i] != NULL; i++)
    {
        fprintf(stderr, Usage[i]);
    }
}


int main(int argc, char *argv[])
{
    int i,j,k,nr,nd,cs,ed,ni,epochs;
    unsigned long ms,is[MAX_INSTANCES];
    FILE *fp,*cmdfp;
    char buf[LINESZ + 1],numbuf[NUMSZ + 1];
    char *mazeFile;
    bool contextMaze;
    char *filePrefix;

    contextMaze = false;
    mazeFile = filePrefix = NULL;
    epochs = -1;
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-epochs") == 0)
        {
            i++;
            if (i >= argc)
            {
                printUsage();
                exit(1);
            }
            epochs = atoi(argv[i]);
            continue;
        }

        if (strcmp(argv[i], "-contextMaze") == 0)
        {
            contextMaze = true;
            continue;
        }

        if (strcmp(argv[i], "-filePrefix") == 0)
        {
            i++;
            if (i >= argc)
            {
                printUsage();
                exit(1);
            }
            filePrefix = argv[i];
            continue;
        }

        if (strcmp(argv[i], "-maze_maker_file") == 0)
        {
            i++;
            if (i >= argc)
            {
                printUsage();
                exit(1);
            }
            mazeFile = argv[i];
            continue;
        }

        printUsage();
        return 1;
    }

    if (epochs < 0)
    {
        printUsage();
        return 1;
    }

    if (mazeFile == NULL)
    {
        printUsage();
        return 1;
    }

    if (filePrefix == NULL) filePrefix = "snns";

    if ((fp = fopen(mazeFile, "r")) == NULL)
    {
        fprintf(stderr, "Cannot open file %s\n", mazeFile);
        return 1;
    }
    
    // Make sure batchman is available.
    if (system("batchman -h >/dev/null 2>/dev/null") != 0)
    {
        fprintf(stderr, "batchman SNNS simulator not in PATH\n");
        return 1;
    }

    // Generate and test maze patterns.
    printf("Test results file: test.res\n");
    system("/bin/cat > test.res < /dev/null");
    while (fgets(buf, LINESZ, fp) != NULL)
    {
        // Get maze parameters.
        if (strncmp(buf, "numRooms=", 9) == 0)
        {
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

            // Generate patterns.
            if (contextMaze)
            {
                sprintf(buf, "./maze_tester -snns -snnsPrefix %s -contextMaze -markPath -numRooms %d -numDoors %d -numGoals 1 -contextSizes %d -effectDelayScale %d -metaSeed %d -instanceSeeds ", filePrefix, nr, nd, cs, ed, ms);
            }
            else
            {
                sprintf(buf, "./maze_tester -snns -snnsPrefix %s -markPath -numRooms %d -numDoors %d -numGoals 1 -contextSizes %d -effectDelayScale %d -metaSeed %d -instanceSeeds ", filePrefix, nr, nd, cs, ed, ms);
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
            system(buf);
            sprintf(buf, "/bin/rm -f batch.log results.res network.net patterns.txt");
            system(buf);
            sprintf(buf,"./runsnns.sh train.bat untrained.net %s_%d_%d %d initialize", filePrefix, ms, nd, epochs);
            system(buf);
            sprintf(buf, "/bin/rm -f batch.log results.res patterns.txt");
            system(buf);
            if (contextMaze)
            {
                sprintf(buf,"./runsnns.sh test.bat network.net %s_%d_%d/complete 1", filePrefix, ms, nd);
            }
            else
            {
                sprintf(buf,"./runsnns.sh test.bat network.net %s_%d_%d 1", filePrefix, ms, nd);
            }
            system(buf);
            system("./check_results results.res >> test.res");
            sprintf(buf,"/bin/rm -fr %s_%d_%d", filePrefix, ms, nd);
            system(buf);
        }
    }
    fclose(fp);

    return 0;
}
