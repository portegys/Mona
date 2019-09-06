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
 * Mona maze-learning driver.
 */

#ifndef __MAZE_DRIVER__
#define __MAZE_DRIVER__

#include "maze.hpp"
#include "../mona/mona.hpp"
#include <map>

// Maze driver.
class MazeDriver
{
    public:

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
            bool modular=false);

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
        void dumpMazeMap(MazeMap::MAP_TYPE, FILE *fp=stdout);

        // SNNS/LSTM neural network input/output parameters.
        enum { NN_NUM_INPUT=10, NN_NUM_OUTPUT=5 };

        // Create SNNS neural network pattern files.
        // These are named:
        // <snns file prefix>_<meta-maze seed>_<doors per room>
        //   /ends/<context door>.pat
        //   /middle/<instance seed>_<rooms in maze>].pat
        //   /complete/<instance seed>_<rooms in maze>_<context door>.pat
        void createSNNSpatterns(char *snnsPrefix=NULL);

        // Create an SNNS pattern file.
        void createSNNSpatternFile(MazeMap *map, RANDOM instanceSeed,
            MAZE_CONFIG mazeConfig, int goal, int contextDoor);

        // Create LSTM neural network training and testing files.
        // These are named: lstmtrain.txt and lstmtest.txt
        // The modular flag indicates "modular" training.
        void createLSTMfiles(bool modular=false);

        // Create an LSTM pattern file.
        void createLSTMfile(FILE *fp, MazeMap *map, RANDOM instanceSeed,
            MAZE_CONFIG mazeConfig, int goal, int contextDoor);

        // Q-Learning train and test.
        int Qrun(int trials, int cycles);

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

        // Q-Learning mode.
        bool qlearn;

        // Q-Learning parameters.
        static const double DEFAULT_LEARNING_RATE;
        static const double DEFAULT_LEARNING_RATE_ATTENUATION;
        static const double DEFAULT_DISCOUNT_PARAMETER;
        static const double REWARD_VALUE;
        static const double MIN_Q_VALUE;
        float learningRate;
        float learningRateAttenuation;
        float discountParameter;

        // Q-Learning state.
        class Qstate
        {
            public:

                int value;
                std::vector<double> Q;

                // Constructors.
                Qstate()
                {
                    value = -1;
                }
                Qstate(int value, int numActions)
                {
                    this->value = value;
                    for (int i = 0; i < numActions; i++)
                    {
                        Q.push_back(MIN_Q_VALUE);
                    }
                }

                // Update action Q value.
                void update(int action, double reward, Qstate *next,
                    double rate, double discount)
                {
                    Q[action] = ((1.0 - rate) * Q[action]) + (rate * reward);
                    if (next != NULL)
                    {
                        Q[action] += rate * discount * next->maxQ();
                    }
                }

                // Get maximum Q value.
                double maxQ()
                {
                    int i,j;
                    double max;

                    max = 0.0;
                    for (i = 0, j = Q.size(); i < j; i++)
                    {
                        if (Q[i] > max) max = Q[i];
                    }
                    return max;
                }

                // Probabilistic action choice.
                int chooseAction()
                {
                    int i,j;
                    vector<double> w;
                    double v;

                    for (i = 0, j = Q.size(); i < j; i++)
                    {
                        w.push_back(Q[i]);
                        if (i > 0) w[i] += w[i - 1];
                    }
                    v = randomizer->RAND_PROB() * w[j - 1];
                    for (i = 0; v > w[i]; i++) {}
                    return i;
                }
        };

        // Q-Learning states.
        map<int, Qstate *> Qstates;
};
#endif
