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
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 */

/*
 * Maze.
 * A meta-maze is a set of probabilistically connected
 * rooms. A maze instance is produced by a specific
 * resolution of these probabilities based on a random
 * seed. Connections are contexts that can connect both
 * rooms and other contexts; the latter can produce distant
 * effects in a maze based on local room transitions
 * (see mazeComponent.hpp).
 */

#ifndef __MAZE__
#define __MAZE__

#define MAZE_VERSION "@(#) Maze version 1.0"

#include "mazeComponent.hpp"

// Maze environment.
class Maze
{
    public:

        // Version.
        static void version();

        // ID dispenser.
        int idDispenser;

        // Dimensions.
        int numRooms;
        int numDoors;
        int numGoals;

        // Maze rooms.
        vector<MazeRoom *> rooms;
        int currentRoom;

        // Maze context levels.
        vector< vector<MazeContext *> > contexts;

        // Instance.
        RANDOM instanceSeed;

        // Constructors.
        Maze(int numRooms, int numDoors, int numGoals);
        Maze(int numRooms, int numDoors, int numGoals,
            vector<int> &contextSizes, int effectDelayScale,
            RANDOM metaSeed, RANDOM instanceSeed,
            bool twoway=false);

        // Destructor.
        ~Maze();

        // Get current room.
        void getRoom(MazeRoom &room);

        // Choose door.
        bool chooseDoor(int door, bool force=false);
        bool chooseDoor(int door, ContextSignature **signature,
            bool force=false);

        // Expire contexts.
        void expireContexts();

        // Clone maze.
        Maze *clone();
        Maze *clone(RANDOM instanceSeed);

        // Print maze.
        void print(FILE *fp=stdout);

        // Find component by id.
        MazeComponent *findByID(int id);

        // Dump maze.
        void dump(FILE *fp=stdout);

    private:

        // Maximum unique context creation tries.
        static const int MAX_CREATE_TRIES;

        // Resolve cloned references.
        void resolveClone(MazeComponent *component, Maze *maze);
};

// Maze mapping tools:

// Room descriptor.
class RoomDescriptor
{
    public:

        int id;
        Maze *maze,*backMaze;
        int searchLength;

        class ConnectingRoom
        {
            public:
                int roomIndex;
                RoomDescriptor *roomDesc;
                int door;
                ContextSignature *signature;
                bool resolved;
                bool visited;
                int validInstanceCount;

                ConnectingRoom()
                {
                    roomIndex = door = -1;
                    roomDesc = NULL;
                    signature = NULL;
                    resolved = false;
                    visited = false;
                    validInstanceCount = -1;
                }

                ~ConnectingRoom()
                {
                    delete signature;
                }

                ConnectingRoom *clone();
        };
        vector<ConnectingRoom *> connectingRooms;
        vector<ConnectingRoom *> backConnectingRooms;

        // Constructor.
        RoomDescriptor(int id, Maze *maze);

        // Destructor.
        ~RoomDescriptor();

        // Given room in same context state?
        bool dejaVu(RoomDescriptor *roomDesc);

        // Backup/restore components.
        void backup();
        void restore();
};

// Maze map
class MazeMap
{
    public:

        // Maximum recursion level.
        static const int MAX_LEVEL;

        // Map type:
        // Room map shows rooms and connections.
        // Meta map shows probabilistic paths.
        // Instance map shows paths after probabilities resolved.
        typedef enum { ROOM_MAP, META_MAP, INSTANCE_MAP }
        MAP_TYPE;
        MAP_TYPE type;

        // Map set manager: manages map instances.
        bool mapSet;

        // ID dispenser.
        int idDispenser;

        // Maze map set.
        int instanceIndex;
        vector<RANDOM> instanceSeeds;
        vector<double> instanceFrequencies;
        vector<MazeMap *> maps;
        vector<bool> validMaps;

        // Maze map (non-set).
        vector<RoomDescriptor *> descriptors;
        RoomDescriptor *currentDescriptor;
        RoomDescriptor *backCurrentDescriptor;

        // Constructor.
        MazeMap(MAP_TYPE);

        // Destructor.
        ~MazeMap();

        // Clear map.
        void clear();

        // Generate and map a set of mazes.
        void mapMaze(Maze *maze, int instanceIndex,
            vector<RANDOM> &instanceSeeds,
            vector<double> &instanceFrequencies);

        // Generate and map a maze.
        void mapMaze(Maze *maze);

        // Take door leading toward given goal.
        int gotoGoal(int goal);

        // Number of search passes to make.
        static const int SEARCH_PASSES;

        // Search for goal starting in given room.
        // Probabilities are resolved by given seed.
        static bool searchGoal(int goal, RoomDescriptor *roomDesc,
            stack<int> &path, RANDOM randomSeed);

        // Update current location in map by taking door.
        void chooseDoor(int door, int validInstanceCount=(-1));

        // Backup/restore rooms.
        void backup();
        void restore();

        // Dump maze in text or graphical source (dot) format.
        typedef enum { TEXT, GRAPH }
        DUMP_TYPE;
        void dump(DUMP_TYPE, FILE *fp=stdout);
        void dump(DUMP_TYPE, RANDOM instanceSeed, int goal,
            vector<int> &ids, vector<int> &doors, FILE *fp=stdout);

    private:

        // Map maze: map type == ROOM_MAP.
        void mapMazeRooms(Maze *maze);

        // Map maze: map type == META_MAP or INSTANCE_MAP.
        void mapMazeContexts(RoomDescriptor *roomDesc, int level);

        // Take door leading toward given goal in a meta-maze
        // that has unknown specific instances. This is done
        // by simulating a set of instances.
        int gotoGoalSim(int goal);

        // Accumulate door payoff statistics leading toward
        // given goal and set of possible maze instances and their
        // relative frequencies of occurrence.
        int gotoGoalSub(int goal,
            RANDOM instanceSeed,
            double instanceFrequency,
            int validInstanceCount,
            vector<double> &doorAccum,
            vector<double> &pathLengthAccum,
            vector<int> &pathCountAccum);
};
#endif
