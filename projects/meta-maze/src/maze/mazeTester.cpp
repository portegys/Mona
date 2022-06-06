/*
 * This software is provided under the terms of the GNU General
 * Public License as published by the Free Software Foundation.
 *
 * Copyright (c) 2003-2006 Tom Portegys, All Rights Reserved.
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
    "  maze_tester -trials <number of trials>\n",
    "      [-train (train maze)]\n",
    "      -cycles <run cycles>\n",
    "      [-randomSeed <random seed>]\n",
    "      [-metaSeed <meta-maze seed>]\n",
    "      [-instanceSeeds <maze instance seeds (comma-separated list)>]\n",
    "      [-contextMaze (learn initial/last door correspondences) [-modular (modular training)]]\n",
    "      [-markPath (uniquely mark interior path rooms)]\n",
    "      -numRooms <number of rooms>\n",
    "      -numDoors <number of doors>\n",
    "      -numGoals <number of goals>\n",
    "      -contextSizes <context level sizes> (comma-separated list)\n",
    "      -effectDelayScale <effect delay scale> (> 0)\n",
    "      [-save <save file name>]\n",
    "For resuming a run:\n",
    "  maze_tester -trials <number of trials>\n",
    "      [-train (train maze)]\n",
    "      -cycles <run cycles>\n",
    "      [-modular (modular training)]\n",
    "      [-randomSeed <random seed>]\n",
    "      -load <load file name>\n",
    "      [-save <save file name>]\n",
    "To dump maze (in Graphviz \"dot\" graph format):\n",
    "  maze_tester -dump meta | instance\n",
    "         (with options above except -train, -cycles)\n",
    "To create SNNS (Stuttgart Neural Network Simulator) pattern files:\n",
    "  maze_tester -snns\n",
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
    "To create LSTM (Long Short-Term Memory neural network) training and testing files:\n",
    "  maze_tester -lstm\n",
    "      [-randomSeed <random seed>]\n",
    "      [-metaSeed <meta-maze seed>]\n",
    "      [-instanceSeeds <maze instance seeds (comma-separated list)>]\n",
    "      [-contextMaze (learn initial/last door correspondences) [-modular (modular training)]]\n",
    "      [-markPath (uniquely mark interior path rooms)]\n",
    "      -numRooms <number of rooms>\n",
    "      -numDoors <number of doors>\n",
    "      -numGoals <number of goals>\n",
    "      -contextSizes <context level sizes> (comma-separated list)\n",
    "      -effectDelayScale <effect delay scale> (> 0)\n",
    "For Q-Learning train and test:\n",
    "  maze_tester -Qlearning\n",
    "      -trials <number of training trials>\n",
    "      -cycles <run cycles>\n",
    "      [-learningRate <learning rate>]\n",
    "      [-learningRateAttenuation <learning rate attenuation>]\n",
    "      [-discountParameter <discount parameter>]\n",
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
    "For version:\n",
    "  maze_tester -version\n",
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
// For Windows memory checking, set CHECK_MEMORY = 1
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
        int i,j,trials,cycles;
        int numRooms,numDoors,numGoals;
        int effectDelayScale;
        int trialsFindingGoals;
        RANDOM randomSeed,metaSeed;
        vector<RANDOM> instanceSeeds;
        bool train,markPath,contextMaze,modular;
        vector<int> contextSizes;
        char *loadFile,*saveFile,*snnsPrefix;
        MazeDriver *mazeDriver;
        bool snns,lstm,qlearn;
        float learningRate;
        float learningRateAttenuation;
        float discountParameter;
        bool dump;
        MazeMap::MAP_TYPE dumpType;

        trials = cycles = numRooms = numDoors = numGoals = -1;
        effectDelayScale = -1;
        randomSeed = metaSeed = INVALID_RANDOM;
        train = markPath = contextMaze = modular = dump = false;
        loadFile = saveFile = snnsPrefix = NULL;
        snns = lstm = qlearn = false;
        learningRate = MazeDriver::DEFAULT_LEARNING_RATE;
        learningRateAttenuation = MazeDriver::DEFAULT_LEARNING_RATE_ATTENUATION;
        discountParameter = MazeDriver::DEFAULT_DISCOUNT_PARAMETER;

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

            if (strcmp(argv[i], "-modular") == 0)
            {
                modular = true;
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
                if (numRooms < 0)
                {
                    printUsage();
                    exit(1);
                }
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
                if (numDoors < 0)
                {
                    printUsage();
                    exit(1);
                }
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
                if (numGoals < 0)
                {
                    printUsage();
                    exit(1);
                }
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
                    j = atoi(s);
                    if (j < 0)
                    {
                        printUsage();
                        exit(1);
                    }
                    contextSizes.push_back(j);
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
                if (effectDelayScale < 0)
                {
                    printUsage();
                    exit(1);
                }
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

            if (strcmp(argv[i], "-dump") == 0)
            {
                i++;
                if (i >= argc)
                {
                    printUsage();
                    exit(1);
                }
                dump = true;
                if (strcmp(argv[i], "meta") == 0)
                {
                    dumpType = MazeMap::META_MAP;
                } else if (strcmp(argv[i], "instance") == 0)
                {
                    dumpType = MazeMap::INSTANCE_MAP;
                }
                else
                {
                    printUsage();
                    exit(1);
                }
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
                if (strchr(snnsPrefix, '.') != NULL)
                {
                    fprintf(stderr, "SNNS file name prefix cannot include extension\n");
                    printUsage();
                    exit(1);
                }
                continue;
            }

            if (strcmp(argv[i], "-lstm") == 0)
            {
                lstm = true;
                continue;
            }

            if (strcmp(argv[i], "-Qlearning") == 0)
            {
                qlearn = true;
                continue;
            }

            if (strcmp(argv[i], "-learnRate") == 0)
            {
                i++;
                if (i >= argc)
                {
                    printUsage();
                    exit(1);
                }
                learningRate = atof(argv[i]);
                continue;
            }

            if (strcmp(argv[i], "-learnRateAttenuation") == 0)
            {
                i++;
                if (i >= argc)
                {
                    printUsage();
                    exit(1);
                }
                learningRateAttenuation = atof(argv[i]);
                continue;
            }

            if (strcmp(argv[i], "-discountParameter") == 0)
            {
                i++;
                if (i >= argc)
                {
                    printUsage();
                    exit(1);
                }
                discountParameter = atof(argv[i]);
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

        if (snns)
        {
            if (lstm || qlearn || modular || trials >= 0 || cycles >= 0 ||
                saveFile != NULL || dump)
            {
                printUsage();
                exit(1);
            }
        }
        else if (lstm)
        {
            if (qlearn || trials >= 0 || cycles >= 0 ||
                saveFile != NULL || dump)
            {
                printUsage();
                exit(1);
            }
            if (snnsPrefix != NULL)
            {
                printUsage();
                exit(1);
            }
        }
        else if (qlearn)
        {
            if (modular || trials < 0 || cycles < 0 || loadFile != NULL ||
                train || saveFile != NULL || dump)
            {
                printUsage();
                exit(1);
            }
            if (snnsPrefix != NULL)
            {
                printUsage();
                exit(1);
            }
        }
        else if (dump)
        {
            if (modular || trials >= 0 || cycles >= 0 ||
                saveFile != NULL)
            {
                printUsage();
                exit(1);
            }
            if (snnsPrefix != NULL)
            {
                printUsage();
                exit(1);
            }
        }
        else
        {
            if (trials < 0 || cycles < 0 || dump)
            {
                printUsage();
                exit(1);
            }
            if (snnsPrefix != NULL)
            {
                printUsage();
                exit(1);
            }
        }
        if (loadFile == NULL)
        {
            if (numRooms <= 0 || numDoors <= 0 || numGoals <= 0 ||
                contextSizes.size() == 0 || effectDelayScale <= 0)
            {
                printUsage();
                exit(1);
            }
        }
        else
        {
            if (numRooms >= 0 || numDoors >= 0 || numGoals >= 0 ||
                contextSizes.size() > 0 || effectDelayScale > 0 ||
                metaSeed != INVALID_RANDOM || instanceSeeds.size() > 0 ||
                contextMaze || markPath)
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
        if (!snns && !lstm && !qlearn && !dump)
        {
            // Modular training only allowed with context learning.
            if ((!train && modular) || (train && !mazeDriver->contextMaze && modular))
            {
                printUsage();
                exit(1);
            }

            trialsFindingGoals =
                mazeDriver->run(trials, cycles, train, modular);

            // Print results.
            printf("Results: found goal in %d/%d trials\n",
                trialsFindingGoals, trials);

            // Save run?
            if (saveFile != NULL)
            {
                mazeDriver->save(saveFile);
            }
        }
        else if (snns)
        {

            // Create SNNS pattern files.
            mazeDriver->createSNNSpatterns(snnsPrefix);
        }
        else if (lstm)
        {

            // Create LSTM training and testing files.
            mazeDriver->createLSTMfiles(modular);
        }
        else if (qlearn)
        {

            // Set Q-Learning parameters.
            mazeDriver->learningRate = learningRate;
            mazeDriver->learningRateAttenuation = learningRateAttenuation;
            mazeDriver->discountParameter = discountParameter;

            // Q-Learning train and test.
            trialsFindingGoals =
                mazeDriver->Qrun(trials, cycles);

            // Print results.
            printf("Results: found goal in %d/%d trials\n",
                trialsFindingGoals, trials);
        }
        else                                      // dump
        {
            mazeDriver->dumpMazeMap(dumpType);
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
