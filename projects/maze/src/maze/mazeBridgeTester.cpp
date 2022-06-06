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
 * The Mona maze-learning "bridge" experiment.
 * See usage below for options.
 *
 * In this experiment, the learner must retain, or
 * "bridge" knowledge over a period of time while
 * performing an intermediate task and then apply
 * the retained knowledge to achieve a goal.
 * Specifically, the learner is initially placed in
 * a room with one of a number of possible doors
 * present. The door leads to the beginning of a
 * maze. At the end of the maze, all possible doors
 * are present. In order to obtain the goal, the
 * same door taken at the beginning must be chosen.
 * The intermediate task is navigating a path through
 * the maze.
 */

#include "mazeBridgeDriver.hpp"

char *Usage[] =
{
    "For a new run:\n",
    "  mazeBridgeTester -trials <number of trials> | -train\n",
    "      [-mazeSeed <maze generation seed>]\n",
    "      [-randomSeed <random seed>]\n",
    "      -numDoors <number of doors in rooms>\n",
    "      -bridgeLength <number of maze rooms in bridge>\n",
    "      [-save <save file name>]\n",
    "For resuming a run:\n",
    "  mazeBridgeTester -trials <number of trials> | -train\n",
    "      [-randomSeed <random seed>]\n",
    "      -load <load file name>\n",
    "      [-save <save file name>]\n",
    "To create an SNNS (neural network simulator) pattern file:\n",
    "  mazeBridgeTester -snnsPrefix <file name prefix> (with above options)\n",
    "For version:\n",
    "  mazeBridgeTester -version\n",
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
#include <string.h>

// Global random numbers.
Random *randomizer;

int
main(int argc, char *argv[])
{
    #if ( CHECK_MEMORY == 1 )
    {
        #endif
        int i,trials,cycles;
        RANDOM mazeSeed,randomSeed;
        bool train;
        char *loadFile,*saveFile,*snnsPrefix;
        int numDoors,bridgeLength;
        int trialsFindingGoals;
        MazeBridgeDriver *mazeBridgeDriver;

        trials = cycles = -1;
        randomSeed = mazeSeed = INVALID_RANDOM;
        train = false;
        numDoors = bridgeLength = -1;
        loadFile = saveFile = snnsPrefix = NULL;
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

            if (strcmp(argv[i], "-mazeSeed") == 0)
            {
                i++;
                if (i >= argc)
                {
                    printUsage();
                    exit(1);
                }
                mazeSeed = atoi(argv[i]);
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

            if (strcmp(argv[i], "-bridgeLength") == 0)
            {
                i++;
                if (i >= argc)
                {
                    printUsage();
                    exit(1);
                }
                bridgeLength = atoi(argv[i]);
                if (bridgeLength < 0)
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

            if (strcmp(argv[i], "-version") == 0)
            {
                Mona::version();
                Maze::version();
                exit(0);
            }

            printUsage();
            exit(1);
        }

        if (randomSeed == INVALID_RANDOM)
        {
            randomSeed = time(NULL);
        }
        randomizer = new Random(randomSeed);
        assert(randomizer != NULL);

        if (mazeSeed == INVALID_RANDOM)
        {
            mazeSeed = randomizer->RAND();
        }

        if (snnsPrefix != NULL)
        {
            if (trials >= 0 || train)
            {
                printUsage();
                exit(1);
            }

        }
        else
        {
            if ((trials < 0 && !train) || (trials >= 0 && train))
            {
                printUsage();
                exit(1);
            }
        }

        // Load run?
        if (loadFile != NULL)
        {
            if (bridgeLength != -1 || numDoors != -1)
            {
                printUsage();
                exit(1);
            }

            // Load driver.
            mazeBridgeDriver = new MazeBridgeDriver(loadFile);
            assert(mazeBridgeDriver != NULL);

        }
        else
        {

            if (bridgeLength == -1 || numDoors == -1)
            {
                printUsage();
                exit(1);
            }

            // Create new driver.
            mazeBridgeDriver =
                new MazeBridgeDriver(mazeSeed, numDoors, bridgeLength, false);
            assert(mazeBridgeDriver != NULL);
        }

        // Run trials.
        if (train)
        {
            trialsFindingGoals = mazeBridgeDriver->train();
        } else if (trials >= 0)
        {
            trialsFindingGoals = mazeBridgeDriver->run(trials);

            // Print results.
            printf("Results: found goal in %d/%d trials\n",
                trialsFindingGoals, trials);
        }
        else
        {

            // Create SNNS pattern file.
            mazeBridgeDriver->createSNNSpatternFile(snnsPrefix, 5);
        }

        // Save run?
        if (saveFile != NULL)
        {
            mazeBridgeDriver->save(saveFile);
        }

        delete mazeBridgeDriver;
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
