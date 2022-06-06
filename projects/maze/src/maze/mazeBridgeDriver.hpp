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
 * Mona maze-learning "bridge" experiment driver.
 */

#ifndef __MAZE_BRIDGE_DRIVER__
#define __MAZE_BRIDGE_DRIVER__

#include "mazeDriver.hpp"

// Maze driver.
class MazeBridgeDriver : public MazeDriver
{
    public:

        // Constructors.
        MazeBridgeDriver(RANDOM mazeSeed,
            int numDoors, int bridgeLength, bool markPath);
        MazeBridgeDriver(char *loadFile);

        // Destructor.
        ~MazeBridgeDriver();

        // Run: return number of successful trials.
        int run(int trials);

        // Train.
        int train();

        // Maze state.
        class MazeState
        {
            public:

                MazeRoom *room;
                int validDoor;

                MazeState(int id, int numDoors)
                {
                    room = new MazeRoom(id);
                    assert(room != NULL);
                    validDoor = -1;
                }

                ~MazeState()
                {
                    delete room;
                }
        };

        // The maze.
        vector<MazeState *> bridgeMaze;
        int currentRoom;

        // Maze types.
        typedef enum
        {
            COMPLETE=0,
            ENDS=1,
            MIDDLE=2,
            MIDDLE_EXTINCTION=3,
            END_EXTINCTION=4,
            NUM_MAZE_TYPES=5
        } MAZE_TYPE;
        MAZE_TYPE mazeType;

        // Build maze.
        int startRoom,startMaze,endMaze,goalRoom,deadRoom;
        int mazeMoveCount,mazeErrorMove;
        void buildMaze(MAZE_TYPE mazeType=COMPLETE);

        // Training type frequencies.
        static const int TRAIN_TYPE_FREQS[NUM_MAZE_TYPES];

        // Maximum dead room exposures.
        static const int MAX_DEAD_ROOM_EXPOSURES;

        // Dump the maze.
        void dumpMaze();

        // Get the current room in the maze.
        void getRoom(MazeRoom &, int index=(-1));

        // Door chosen.
        void chooseDoor(int door);

        // Get door to goal.
        int getDoor();

        // Clear maze.
        void clearMaze();

        // Print bridge mediators.
        void printBridges();

        // Create SNNS neural network pattern files.
        bool createSNNSpatternFile(char *filePrefix, int maxDoors);
};
#endif
