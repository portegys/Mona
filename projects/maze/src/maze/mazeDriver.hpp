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
 * Mona maze-learning driver.
 */

#ifndef __MAZE_DRIVER__
#define __MAZE_DRIVER__

#include "maze.hpp"
#include "../mona/mona.hpp"

// Maze driver.
class MazeDriver
{
    public:

        // Training types.
        typedef enum { META_TRAIN, INSTANCE_TRAIN, NO_TRAIN }
        TRAIN_TYPE;

        // Maze configurations.
        typedef enum
        {
            COMPLETE,                             // Complete maze.
            ENDS,                                 // End rooms only for context maze.
            MIDDLE                                // Middle rooms only for context maze.
        } MAZE_CONFIG;

        // Parameters.
        static const Mona::NEED NEED_VALUE;
        static const Mona::NEED GOAL_VALUE;

        // Response training parameters.
        static const double INITIAL_RESPONSE_OVERRIDE_POTENTIAL;
        static const double RESPONSE_OVERRIDE_DECREMENT;

        // Room types.
        static const int START_ROOM_MARK;
        static const int BEGIN_MAZE_MARK;
        static const int END_MAZE_MARK;
        static const int GOAL_ROOM_MARK;
        static const int DEAD_ROOM_MARK;
        static const int MAZE_ROOM_MARK;

        // Constructors.
        MazeDriver(int numRooms, int numDoors, int numGoals,
            vector<int> &contextSizes, int effectDelayScale,
            RANDOM metaSeed,
            vector<RANDOM> &instanceSeeds,
            bool contextMaze, bool markPath);
        MazeDriver();
        MazeDriver(char *loadFile);

        // Destructor.
        ~MazeDriver();

        // Load configuration.
        void load(char *loadFile);

        // Save configuration.
        void save(char *saveFile);

        // Run: return number of successful trials.
        int run(int trials, int cycles, bool train=false,
            TRAIN_TYPE trainType=META_TRAIN);

        // Mark the maze.
        void markMaze();

        // Print special mediators.
        void printSpecialMediators();

    #ifdef ACTIVITY_TRACKING
        void dumpMonaNetwork(enum Mona::DUMP_TYPE, char *title,
        bool enablementDump, bool driveDump,
        bool delimiter, FILE *out=stdout);
    #else
        void dumpMonaNetwork(enum Mona::DUMP_TYPE, char *title,
        bool delimiter, FILE *out=stdout);
    #endif

        // Create SNNS neural network pattern files.
        // These are named:
        // <snns file prefix>_<meta-maze seed>_<doors per room>
        //   /ends/<context door>.pat
        //   /middle/<instance seed>_<rooms in maze>].pat
        //   /complete/<instance seed>_<rooms in maze>_<context door>.pat
        void createSNNSpatterns(char *snnsPrefix=NULL);

        // Create an SNNS pattern file.
        void createSNNSpatternFile(char *snnsPrefix,
            MazeMap *map, RANDOM instanceSeed, MAZE_CONFIG mazeConfig,
            int goal, int contextDoor);

        // Member data.
        int numRooms,numDoors,numGoals;
        vector<int> contextSizes;
        int effectDelayScale;
        RANDOM metaSeed;
        vector<RANDOM> instanceSeeds;
        bool contextMaze;
        bool markPath;
        Maze *maze;
        int numSensors,maxResponse,numNeeds;
        Mona::SENSOR *sensors;
        Mona *mona;
};
#endif
