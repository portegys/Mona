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
 * A maze component: room or context. A room
 * contains a mark, doors and goals. Doors connect rooms
 * and goals satisfy needs. A context is a probabilistic
 * linkage between rooms and other contexts. A context
 * linking rooms A and B determines the probability of
 * a directed connection from A to the B via a specific
 * door. A context linking contexts A and B modifies
 * the probability of B given the event of A.
 */

#ifndef __MAZE_COMPONENT__
#define __MAZE_COMPONENT__

#include "../common/common.h"

// Global random numbers.
extern Random *randomizer;

// Maze component.
class MazeComponent
{
    public:

        // Component data.
        int id;
        typedef enum { ROOM, CONTEXT }
        TYPE;
        TYPE type;

        // Component is cause of these contexts.
        vector<class MazeContext *> causeContexts;

        // Current context hierarchy.
        vector<class MazeContext *> contexts;
};

// Maze room.
class MazeRoom : public MazeComponent
{
    public:

        int mark;
        vector<bool> doors;
        vector<bool> goals;

        // Context signatures.
        vector<class ContextSignature *> contextSignatures;
        RANDOM instanceSeed;

        // Constructors.
        MazeRoom(int id, int mark=0);
        MazeRoom();

        // Destructor.
        ~MazeRoom();

        // Get current signature.
        class ContextSignature *getSignature();

        // Clone room.
        MazeRoom *clone();
        MazeRoom *clone(RANDOM instanceSeed);

        // Print room.
        void print(FILE *fp=stdout);

    private:

        // Get probability of access.
        double getAccessProbability();
};

// Maze context.
class MazeContext : public MazeComponent
{
    public:

        class MazeComponent *cause;
        class MazeComponent *effect;
        int door;
        double probability;
        int effectDelay;
        int effectTimer;

        // Constructor.
        MazeContext(int id);

        // Cause event.
        void fireCause();

        // Effect event.
        void fireEffect();

        // Clone context.
        MazeContext *clone();

        // Print context.
        void print(FILE *fp=stdout);

    private:

        // Boost timer.
        void boostTimer(int);
};

/*
 * Context "signature" for maze consistency.
 * A signature captures the determination of accessibility
 * of one room from another for a unique set of
 * contexts. The context probabilities are used to make
 * this one-time-per-maze determination. This simulates
 * a maze in which "paths" to "same" rooms remain constant.
 */
class ContextSignature
{
    public:

        vector<int> ids;
        double probability;
        bool success;
        bool visited;

        // Constructor.
        ContextSignature(vector<int> &ids);

        // Destructor.
        ~ContextSignature();

        // Get success.
        bool getSuccess(RANDOM randomSeed);

        // Context ids equal?
        bool match(ContextSignature *);
        bool match(vector<int> ids);

        // Clone.
        ContextSignature *clone();

        // Print signature.
        void print(FILE *fp=stdout);

    private:

        // Sort ids.
        void sort();
};
#endif
