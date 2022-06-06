/*
 * This software is provided under the terms of the GNU General
 * Public License as published by the Free Software Foundation.
 *
 * Copyright (c) 2003-2005 Tom Portegys, All Rights Reserved.
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for NON-COMMERCIAL purposes and without
 * fee is hereby granted provided that this copyright notice
 * appears in all copies.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 */

/*
 * The Mona maze-learning program.
 * See usage below for options.
 *
 * The learner can be trained to respond to a general
 * meta-maze or specific maze instances. For meta-maze
 * training, initially the learner is directed to respond
 * optimally to the probabilistic maze, but as learner
 * responses reveal information about the specific
 * instance of the maze, the learner is directed
 * accordingly. For maze instance training, the learner
 * is directed as though the maze probabilities are
 * resolved from the start. Response training can be
 * tailored to taper off in strength gradually.
 *
 * Optionally, the learner must retain contextual
 * knowledge while traversing the maze, and then apply
 * the retained knowledge to reach a goal. Specifically,
 * the learner is presented with an initial set of doors,
 * only one of which leads to the beginning of a maze
 * That door choice must be repeated at the maze exit
 * in order to obtain the goal.
 */

#include "mazeDriver.hpp"

char *Usage[] =
{
    "For a new run:\n",
    "  mazeTest -trials <number of trials>\n",
    "      [-train meta | instance\n",
    "         (train for meta-maze or maze instance)]\n",
    "      -cycles <run cycles>\n",
    "      [-randomSeed <random seed>]\n",
    "      [-metaSeed <meta-maze seed>]\n",
    "      [-instanceSeeds <maze instance seeds (comma-separated list)>]\n",
    "      [-contextMaze (learn initial/last door correspondences)]\n",
    "      [-markPath (uniquely mark interior path rooms)]\n",
    "      -numRooms <number of rooms>\n",
    "      -numDoors <number of doors>\n",
    "      -numGoals <number of goals>\n",
    "      -contextSizes <context level sizes> (comma-separated list)\n",
    "      -effectDelayScale <effect delay scale> (> 0)\n",
    "      [-save <save file name>]\n",
    "For resuming a run:\n",
    "  mazeTest -trials <number of trials>\n",
    "      [-train meta | instance\n",
    "         (train for meta-maze or maze instance)]\n",
    "      -cycles <run cycles>\n",
    "      [-randomSeed <random seed>]\n",
    "      -load <load file name>\n",
    "      [-save <save file name>]\n",
    "To create SNNS (Stuttgart Neural Network Simulator) pattern files:\n",
    "  mazeTest -snns\n",
    "      [-randomSeed <random seed>]\n",
    "      [-metaSeed <meta-maze seed>]\n",
    "      [-instanceSeeds <maze instance seeds (comma-separated list)>]\n",
    "      [-contextMaze (learn initial/last door correspondences)]\n",
    "      [-markPath (uniquely mark interior path rooms)]\n",
    "      -numRooms <number of rooms>\n",
    "      -numDoors <number of doors>\n",
    "      -numGoals <number of goals>\n",
    "      -contextSizes <context level sizes> (comma-separated list)\n",
    "      -effectDelayScale <effect delay scale> (> 0)\n",
    "      [-snnsPrefix <pattern file name prefix> (defaults to \"snns\")]\n",
    "For version:\n",
    "  mazeTest -version\n",
    NULL
};

void printUsage()
{
    for (int i = 0; Usage[i] != NULL; i++)
    {
        fprintf(stderr, Usage[i]);
    }
}


#ifdef WIN32
#include <windows.h>
#ifdef _DEBUG
// For Windows memory checking, set CHECK_MEMORY = 0
#define CHECK_MEMORY 0
#if ( CHECK_MEMORY == 1 )
#define MEMORY_CHECK_FILE "memory.txt"
#include <crtdbg.h>
#endif
#endif
#endif

// Global random numbers.
Random *randomizer;

int
main(int argc, char *argv[])
{
    #if ( CHECK_MEMORY == 1 )
    {
        #endif
        int i,trials,cycles;
        int numRooms,numDoors,numGoals;
        int effectDelayScale;
        int trialsFindingGoals;
        RANDOM randomSeed,metaSeed;
        vector<RANDOM> instanceSeeds;
        bool train,markPath,contextMaze;
        MazeDriver::TRAIN_TYPE trainType;
        vector<int> contextSizes;
        char *loadFile,*saveFile,*snnsPrefix;
        MazeDriver *mazeDriver;
        bool snns;

        trials = cycles = numRooms = numDoors = numGoals = -1;
        effectDelayScale = -1;
        randomSeed = metaSeed = INVALID_RANDOM;
        train = markPath = contextMaze = false;
        trainType = MazeDriver::NO_TRAIN;
        loadFile = saveFile = snnsPrefix = NULL;
        snns = false;
        for (i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "-trials") == 0)
            {
                i++;
                if (i >= argc)
                {
                    printUsage();
                    exit(1);
                }
                trials = atoi(argv[i]);
                if (trials < 0)
                {
                    printUsage();
                    exit(1);
                }
                continue;
            }

            if (strcmp(argv[i], "-train") == 0)
            {
                train = true;
                i++;
                if (i >= argc)
                {
                    printUsage();
                    exit(1);
                }
                if (strcmp(argv[i], "meta") == 0)
                {
                    trainType = MazeDriver::META_TRAIN;
                } else if (strcmp(argv[i], "instance") == 0)
                {
                    trainType = MazeDriver::INSTANCE_TRAIN;
                }
                else
                {
                    printUsage();
                    exit(1);
                }
                continue;
            }

            if (strcmp(argv[i], "-cycles") == 0)
            {
                i++;
                if (i >= argc)
                {
                    printUsage();
                    exit(1);
                }
                cycles = atoi(argv[i]);
                if (cycles < 0)
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
                randomSeed = atoi(argv[i]);
                continue;
            }

            if (strcmp(argv[i], "-metaSeed") == 0)
            {
                i++;
                if (i >= argc)
                {
                    printUsage();
                    exit(1);
                }
                metaSeed = atoi(argv[i]);
                continue;
            }

            if (strcmp(argv[i], "-instanceSeeds") == 0)
            {
                i++;
                if (i >= argc)
                {
                    printUsage();
                    exit(1);
                }
                char *s,*s2;
                s = argv[i]; s2 = strchr(s, ',');
                while (s != NULL)
                {
                    if (s2 != NULL) s2[0] = '\0';
                    instanceSeeds.push_back(atoi(s));
                    if (s2 != NULL)
                    {
                        s2[0] = ','; s2++;
                        s = s2; s2 = strchr(s, ',');
                    }
                    else
                    {
                        s = s2;
                    }
                }
                continue;
            }

            if (strcmp(argv[i], "-contextMaze") == 0)
            {
                contextMaze = true;
                continue;
            }

            if (strcmp(argv[i], "-markPath") == 0)
            {
                markPath = true;
                continue;
            }

            if (strcmp(argv[i], "-numRooms") == 0)
            {
                i++;
                if (i >= argc)
                {
                    printUsage();
                    exit(1);
                }
                numRooms = atoi(argv[i]);
                continue;
            }

            if (strcmp(argv[i], "-numDoors") == 0)
            {
                i++;
                if (i >= argc)
                {
                    printUsage();
                    exit(1);
                }
                numDoors = atoi(argv[i]);
                continue;
            }

            if (strcmp(argv[i], "-numGoals") == 0)
            {
                i++;
                if (i >= argc)
                {
                    printUsage();
                    exit(1);
                }
                numGoals = atoi(argv[i]);
                continue;
            }

            if (strcmp(argv[i], "-contextSizes") == 0)
            {
                i++;
                if (i >= argc)
                {
                    printUsage();
                    exit(1);
                }
                char *s,*s2;
                s = argv[i]; s2 = strchr(s, ',');
                while (s != NULL)
                {
                    if (s2 != NULL) s2[0] = '\0';
                    contextSizes.push_back(atoi(s));
                    if (s2 != NULL)
                    {
                        s2[0] = ','; s2++;
                        s = s2; s2 = strchr(s, ',');
                    }
                    else
                    {
                        s = s2;
                    }
                }
                continue;
            }

            if (strcmp(argv[i], "-effectDelayScale") == 0)
            {
                i++;
                if (i >= argc)
                {
                    printUsage();
                    exit(1);
                }
                effectDelayScale = atoi(argv[i]);
                continue;
            }

            if (strcmp(argv[i], "-load") == 0)
            {
                i++;
                if (i >= argc)
                {
                    printUsage();
                    exit(1);
                }
                loadFile = argv[i];
                continue;
            }

            if (strcmp(argv[i], "-save") == 0)
            {
                i++;
                if (i >= argc)
                {
                    printUsage();
                    exit(1);
                }
                saveFile = argv[i];
                continue;
            }

            if (strcmp(argv[i], "-snns") == 0)
            {
                snns = true;
                continue;
            }

            if (strcmp(argv[i], "-snnsPrefix") == 0)
            {
                i++;
                if (i >= argc)
                {
                    printUsage();
                    exit(1);
                }
                snnsPrefix = argv[i];
                continue;
            }

            if (strcmp(argv[i], "-version") == 0)
            {
                Mona::version();
                Maze::version();
                exit(0);
            }

            printUsage();
            exit(1);
        }

        if ((trials < 0 || cycles < 0) && !snns)
        {
            printUsage();
            exit(1);
        }
        if (loadFile == NULL)
        {
            if (numRooms <= 0 || numDoors <= 0 || numGoals <= 0 ||
                effectDelayScale <= 0)
            {
                printUsage();
                exit(1);
            }
        }
        else
        {
            if (numRooms >= 0 || numDoors >= 0 || numGoals >= 0 ||
                effectDelayScale > 0 || metaSeed != INVALID_RANDOM || snns)
            {
                printUsage();
                exit(1);
            }
        }
        if (randomSeed == INVALID_RANDOM)
        {
            randomSeed = (RANDOM)time(NULL);
        }
        randomizer = new Random(randomSeed);
        assert(randomizer != NULL);
        if (metaSeed == INVALID_RANDOM)
        {
            metaSeed = randomSeed;
        }

        // Load run?
        if (loadFile != NULL)
        {
            // Load driver.
            mazeDriver = new MazeDriver(loadFile);
            assert(mazeDriver != NULL);

        }
        else
        {
            // Create new driver.
            mazeDriver = new MazeDriver(numRooms, numDoors, numGoals,
                contextSizes, effectDelayScale,
                metaSeed, instanceSeeds, contextMaze, markPath);
            assert(mazeDriver != NULL);
        }

        // Run trials.
        if (!snns)
        {
            trialsFindingGoals =
                mazeDriver->run(trials, cycles, train, trainType);

            // Print results.
            printf("Results: found goal in %d/%d trials\n",
                trialsFindingGoals, trials);

            // Save run?
            if (saveFile != NULL)
            {
                mazeDriver->save(saveFile);
            }
        }
        else
        {
            // Create SNNS pattern files.
            mazeDriver->createSNNSpatterns(snnsPrefix);
        }

        delete mazeDriver;
        delete randomizer;

        #if ( CHECK_MEMORY == 1 )
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
    _CrtSetReportFile(_CRT_ERROR, hFile );
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

    return 0;
}
