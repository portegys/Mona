/*

Training and testing meta-mazes with the LSTM (Long Short-Term Memory neural network).
Meta-maze patterns given in a maze_maker maze discovery file.

Options: -epochs <training epochs> | -createFiles (create training and testing files only).
         [-contextMaze (generate context patterns) [-modular (modular training)]]
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
    "      -epochs <training epochs> | -createFiles (create training and testing files only)\n",
    "      [-contextMaze (generate context patterns) [-modular (modular training)]]\n",
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
    int i,j,k,s1,s2,nr,nd,cs,ed,ni,epochs;
    unsigned long ms,is[MAX_INSTANCES];
    FILE *fp,*fp2,*fp3,*cmdfp;
    char buf[LINESZ + 1],numbuf[NUMSZ + 1];
    char *mazeFile;
    bool createFiles,contextMaze,modular;

    createFiles = contextMaze = modular = false;
    mazeFile = NULL;
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

        if (strcmp(argv[i], "-createFiles") == 0)
        {
            createFiles = true;
            continue;
        }

        if (strcmp(argv[i], "-contextMaze") == 0)
        {
            contextMaze = true;
            continue;
        }

        if (strcmp(argv[i], "-modular") == 0)
        {
            modular = true;
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

    if (epochs < 0 && !createFiles)
    {
        printUsage();
        return 1;
    }

    if (epochs >= 0 && createFiles)
    {
        printUsage();
        return 1;
    }

    if (modular && !contextMaze)
    {
        printUsage();
        return 1;
    }

    if (mazeFile == NULL)
    {
        printUsage();
        return 1;
    }

    if ((fp = fopen(mazeFile, "r")) == NULL)
    {
        fprintf(stderr, "Cannot open file %s\n", mazeFile);
        return 1;
    }

    // Generate and test maze patterns.
    if (createFiles)
    {
        printf("Creating training and testing files...\n");
    }
    else
    {
        printf("Test results file: test.res\n");
        system("/bin/cat > test.res < /dev/null");
    }
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
            if (createFiles)
            {
                printf("Creating files for meta-maze seed %d\n", ms);
            }
            if (contextMaze)
            {
                if (modular)
                {
                    sprintf(buf, "./maze_tester -lstm -contextMaze -modular -markPath -numRooms %d -numDoors %d -numGoals 1 -contextSizes %d -effectDelayScale %d -metaSeed %d -instanceSeeds ", nr, nd, cs, ed, ms);
                }
                else
                {
                    sprintf(buf, "./maze_tester -lstm -contextMaze -markPath -numRooms %d -numDoors %d -numGoals 1 -contextSizes %d -effectDelayScale %d -metaSeed %d -instanceSeeds ", nr, nd, cs, ed, ms);
                }
            }
            else
            {
                sprintf(buf, "./maze_tester -lstm -markPath -numRooms %d -numDoors %d -numGoals 1 -contextSizes %d -effectDelayScale %d -metaSeed %d -instanceSeeds ", nr, nd, cs, ed, ms);
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
            if (createFiles)
            {
                sprintf(buf, "/bin/mv lstmtrain.txt %d_lstmtrain.txt", ms);
                system(buf);
                sprintf(buf, "/bin/mv lstmtest.txt %d_lstmtest.txt", ms);
                system(buf);
            }
            else
            {
                if ((fp2 = popen("grep \\^20 lstmtrain.txt | wc -l", "r")) == NULL)
                {
                    fprintf(stderr, "Cannot popen lstmtrain.txt\n");
                    exit(1);
                }
                fscanf(fp2, "%d", &s1);
                s1--;
                pclose(fp2);
                if ((fp2 = popen("grep \\^20 lstmtest.txt | wc -l", "r")) == NULL)
                {
                    fprintf(stderr, "Cannot popen lstmtest.txt\n");
                    exit(1);
                }
                fscanf(fp2, "%d", &s2);
                s2--;
                pclose(fp2);
                sprintf(buf, "cat lstmpars-template.txt | sed 's/%%EPOCHS%%/%d/' | sed 's/%%TRAINING_SET_SIZE%%/%d/' | sed 's/%%TEST_SET_SIZE%%/%d/' > lstmpars.txt", epochs, s1, s2);
                system(buf);
                sprintf(buf,"./lstm >> test.res");
                system(buf);
                sprintf(buf,"/bin/rm lstmtrain.txt lstmtest.txt");
                system(buf);
            }
        }
    }
    fclose(fp);

    return 0;
}
