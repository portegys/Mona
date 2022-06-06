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
 * The Mona instinct-learning program.
 * See usage below for options.
 *
 * This program demonstrates the learning of "instincts"
 * which are hard-wired mediators that are evolved and
 * inherited by organisms. The purpose of instincts is to
 * assist an organism in succeeding in its environment,
 * serving as a boost to experiential learning.
 * The task is the "Monkey and Bananas" problem.
 */

#include "monkey_and_bananas.hpp"

char *Usage[] =
{
    "Usage:\n",
    "instinct -steps <steps>\n",
    "\t[-input <instincts file name>]\n",
    "\t[-scramble (scramble item locations)]\n",
    "\t[-randomSeed <random seed>]\n",
    "\t[-printInstincts]\n",
    "\t[-display <text | graphics>]\n",
    NULL
};

void printUsage()
{
    for (int i = 0; Usage[i] != NULL; i++)
    {
        fprintf(stderr, Usage[i]);
    }
}


int
main(int argc, char *argv[])
{
    int i,x,y,wx,wy,bx,mx,location,b0,b1,b2,steps;
    bool scramble,printInstincts;
    MonkeyAndBananas::DISPLAY_TYPE displayType;
    MonkeyInstincts *instincts;
    MonkeyAndBananas *monkey;
    MonkeyAndBananas::DIRECTION direction;
    char **world;
    char *inputFileName;
    RANDOM randomSeed;
    Random *randomizer;

    steps = -1;
    scramble = printInstincts = false;
    inputFileName = NULL;
    randomSeed = INVALID_RANDOM;
    displayType = MonkeyAndBananas::NO_DISPLAY;
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-steps") == 0)
        {
            i++;
            if (i >= argc)
            {
                printUsage();
                exit(1);
            }
            steps = atoi(argv[i]);
            if (steps < 0)
            {
                printUsage();
                exit(1);
            }
            continue;
        }

        if (strcmp(argv[i], "-input") == 0)
        {
            i++;
            if (i >= argc)
            {
                printUsage();
                exit(1);
            }
            inputFileName = argv[i];
            continue;
        }

        if (strcmp(argv[i], "-scramble") == 0)
        {
            scramble = true;
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

        if (strcmp(argv[i], "-printInstincts") == 0)
        {
            printInstincts = true;
            continue;
        }

        if (strcmp(argv[i], "-display") == 0)
        {
            i++;
            if (i >= argc)
            {
                printUsage();
                exit(1);
            }
            if (strcmp(argv[i], "text") == 0)
            {
                displayType = MonkeyAndBananas::TEXT_DISPLAY;
            } else if (strcmp(argv[i], "graphics") == 0)
            {
                displayType = MonkeyAndBananas::GRAPHICS_DISPLAY;
            }
            else
            {
                printUsage();
                exit(1);
            }
            continue;
        }

        printUsage();
        exit(1);
    }

    if (steps == -1)
    {
        printUsage();
        exit(1);
    }

    // Get random numbers.
    if (randomSeed == INVALID_RANDOM)
    {
        randomSeed = time(NULL);
    }
    randomizer = new Random(randomSeed);
    assert(randomizer != NULL);

    // Create instincts.
    instincts = new MonkeyInstincts(randomizer->RAND());
    assert(instincts != NULL);
    if (inputFileName != NULL) instincts->load(inputFileName);

    // Determine monkey and box locations.
    for (i = 0; MonkeyAndBananas::DEFAULT_WORLD[i] != NULL; i++) {}
    wy = i;
    wx = strlen(MonkeyAndBananas::DEFAULT_WORLD[0]);
    world = new char*[i + 1];
    assert(world != NULL);
    for (i = 0; MonkeyAndBananas::DEFAULT_WORLD[i] != NULL; i++)
    {
        world[i] = new char[strlen(MonkeyAndBananas::DEFAULT_WORLD[i]) + 1];
        assert(world[i] != NULL);
        strcpy(world[i], MonkeyAndBananas::DEFAULT_WORLD[i]);
        assert(i == 0 || strlen(world[i - 1]) == strlen(world[i]));
    }
    world[i] = NULL;
    if (scramble)
    {
        for (y = 0; y < wy; y++)
        {
            for (x = 0; x < wx; x++)
            {
                if (world[y][x] == MonkeyAndBananas::MONKEY ||
                    world[y][x] == MonkeyAndBananas::BOX)
                {
                    world[y][x] = MonkeyAndBananas::AIR;
                }
            }
        }
        for (bx = 0; bx < wx && world[wy - 1][bx] != MonkeyAndBananas::FLOOR; bx++) {}
        mx = bx + 1;
        bx += 4;
        b0 = randomizer->RAND_CHOICE(wx - bx) + bx;
        world[wy - 2][b0] = MonkeyAndBananas::BOX;
        #ifdef ONE_BOX_WORLD
        for (i = 0; i < 1000; i++)
        {
            location = randomizer->RAND_CHOICE(wx - mx) + mx;
            if (location == b0) continue;
            break;
        }
        assert(i < 1000);
        #else
        for (i = 0; i < 1000; i++)
        {
            b1 = randomizer->RAND_CHOICE(wx - bx) + bx;
            if (b1 == b0) continue;
            break;
        }
        assert(i < 1000);
        world[wy - 2][b1] = MonkeyAndBananas::BOX;
        for (i = 0; i < 1000; i++)
        {
            b2 = randomizer->RAND_CHOICE(wx - bx) + bx;
            if (b2 == b0) continue;
            if (b2 == b1) continue;
            break;
        }
        assert(i < 1000);
        world[wy - 2][b2] = MonkeyAndBananas::BOX;
        for (i = 0; i < 1000; i++)
        {
            location = randomizer->RAND_CHOICE(wx - bx) + bx;
            if (location == b0) continue;
            if (location == b1) continue;
            if (location ==  b2) continue;
            break;
        }
        assert(i < 1000);
        #endif
        if (randomizer->RAND_BOOL())
        {
            direction = MonkeyAndBananas::LOOK_LEFT;
        }
        else
        {
            direction = MonkeyAndBananas::LOOK_RIGHT;
        }
    }
    else
    {
        location = MonkeyAndBananas::DEFAULT_MONKEY_LOCATION;
        direction = MonkeyAndBananas::DEFAULT_MONKEY_DIRECTION;
    }

    // Create Monkey and Bananas task.
    monkey = new MonkeyAndBananas(world,
        location, direction, displayType, instincts);
    assert(monkey != NULL);
    monkey->instincts->brain->MAX_MEDIATORS =
        monkey->instincts->instincts.size();
    if (printInstincts) instincts->print();

    // Run.
    monkey->run(steps);

    // Clean up.
    delete monkey;
    for (i = 0; world[i] != NULL; i++)
    {
        delete world[i];
    }
    delete world;
    delete randomizer;

    return 0;
}
