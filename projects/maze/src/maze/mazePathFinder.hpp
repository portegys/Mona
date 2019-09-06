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
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 */

#ifndef __MAZE_PATH_FINDER__
#define __MAZE_PATH_FINDER__

#include "maze.hpp"
#include "../common/common.h"

// Find paths to goals in a meta-maze.
class MazePathFinder
{
    public:

        int numRooms, numDoors, numGoals;
        vector<int> contextSizes;
        int effectDelayScale;
        RANDOM metaSeed;
        vector<RANDOM> instanceSeeds;
        vector<double> instanceFrequencies;
        bool twoway;
        Maze *maze;
        MazeMap *map;
        double score;

        class PathElement
        {
            public:
                MazeRoom room;
                int door;

                bool equals(PathElement &e)
                {
                    int i,j;

                    if (room.mark != e.room.mark) return false;
                    if (room.doors.size() != e.room.doors.size()) return false;
                    for (i = 0, j = room.doors.size(); i < j; i++)
                    {
                        if (room.doors[i] != e.room.doors[i]) return false;
                    }
                    if (room.goals.size() != e.room.goals.size()) return false;
                    for (i = 0, j = room.goals.size(); i < j; i++)
                    {
                        if (room.goals[i] != e.room.goals[i]) return false;
                    }
                    if (door != e.door) return false;
                    return true;
                }
        };

        // Goal paths.
        vector<RANDOM> goalInstances;
        vector<vector<PathElement> > goalPaths;

        // Constructors.
        MazePathFinder(int numRooms, int numDoors, int numGoals,
            vector<int> &contextSizes, int effectDelayScale,
            RANDOM metaSeed,
            vector<RANDOM> &instanceSeeds,
            vector<double> &instanceFrequencies,
            bool twoway)
        {
            this->numRooms = numRooms;
            this->numDoors = numDoors;
            this->numGoals = numGoals;
            this->contextSizes = contextSizes;
            this->effectDelayScale = effectDelayScale;
            this->metaSeed = metaSeed;
            this->instanceSeeds = instanceSeeds;
            assert(instanceSeeds.size() == instanceFrequencies.size());
            this->instanceFrequencies = instanceFrequencies;
            this->twoway = twoway;
            maze = NULL;
            map = NULL;
            score = 0.0;
        }

        MazePathFinder()
        {
            numRooms = numDoors = numGoals = -1;
            effectDelayScale = -1;
            metaSeed = INVALID_RANDOM;
            twoway = false;
            maze = NULL;
            map = NULL;
            score = 0.0;
        }

        ~MazePathFinder()
        {
            if (maze != NULL) delete maze;
            if (map != NULL) delete map;
        }

        // Clone this state.
        MazePathFinder *clone()
        {
            MazePathFinder *mp;

            mp = new MazePathFinder();
            assert(mp != NULL);
            mp->numRooms = numRooms;
            mp->numDoors = numDoors;
            mp->numGoals = numGoals;
            mp->contextSizes = contextSizes;
            mp->effectDelayScale = effectDelayScale;
            mp->metaSeed = metaSeed;
            mp->instanceSeeds = instanceSeeds;
            mp->instanceFrequencies = instanceFrequencies;
            mp->twoway = twoway;
            return mp;
        }

        // States are equal?
        bool equals(MazePathFinder *state)
        {
            int i,j;

            if (state->numRooms != numRooms) return false;
            if (state->numDoors != numDoors) return false;
            if (state->numGoals != numGoals) return false;
            if (state->contextSizes.size() != contextSizes.size()) return false;
            for (i = 0, j = contextSizes.size(); i < j; i++)
            {
                if (state->contextSizes[i] != contextSizes[i]) return false;
            }
            if (state->effectDelayScale != effectDelayScale) return false;
            if (state->metaSeed != metaSeed) return false;
            if (state->instanceSeeds.size() != instanceSeeds.size()) return false;
            for (i = 0, j = contextSizes.size(); i < j; i++)
            {
                if (state->contextSizes[i] != contextSizes[i]) return false;
            }
            if (state->instanceFrequencies.size() != instanceFrequencies.size()) return false;
            for (i = 0, j = instanceFrequencies.size(); i < j; i++)
            {
                if (state->instanceFrequencies[i] != instanceFrequencies[i]) return false;
            }
            if (state->twoway != twoway) return false;
            return true;
        }

        // Create maze and map.
        void generate(RANDOM instanceSeed)
        {
            int i,j;

            if (maze != NULL) delete maze;
            if (map != NULL) delete map;
            maze = new Maze(numRooms, numDoors, numGoals,
                contextSizes, effectDelayScale,
                metaSeed, instanceSeed, twoway);
            assert(maze != NULL);
            map = new MazeMap(MazeMap::META_MAP);
            assert(map != NULL);
            for (i = 0, j = instanceSeeds.size(); i < j; i++)
            {
                if (instanceSeed == instanceSeeds[i]) break;
            }
            map->mapMaze(maze, i, instanceSeeds,
                instanceFrequencies);
        }

        // Follow training path in meta-maze to given goal.
        // Return success, and record input-output for path.
        // The path is the statistically best path given the
        // set of possible maze instances. However, the actual
        // outcome of choosing a door is determined by the
        // instance of the current maze. Thus a path may fail,
        // or succeed only after trying a more generally optimal
        // path.
        bool findPath(int goal, vector<PathElement> &path)
        {
            PathElement pathElem;

            assert(maze != NULL);
            assert(map != NULL);
            while (true)
            {
                maze->getRoom(pathElem.room);
                pathElem.door = map->gotoGoal(goal);
                path.push_back(pathElem);
                if (pathElem.door == -1) return false;
                if (pathElem.door == maze->numDoors) return true;
                maze->chooseDoor(pathElem.door);
                map->chooseDoor(pathElem.door);
            }
        }

        // Evaluate the meta-maze by generating instances
        // of it and determining the number and lengths of
        // the goal paths in them. Record this as a score.
        void evaluate()
        {
            int i,j,k,p,q,r,s;
            vector<PathElement> path;

            goalInstances.clear();
            goalPaths.clear();
            score = 0.0;
            for (i = 0; i < numGoals; i++)
            {
                for (j = 0, k = instanceSeeds.size(); j < k; j++)
                {
                    // Create maze and map.
                    generate(instanceSeeds[j]);

                    // Follow training path in meta-maze.
                    path.clear();
                    if (findPath(i, path))
                    {
                        // Unique path?
                        for (p = 0, q = goalPaths.size(); p < q; p++)
                        {
                            if (path.size() != goalPaths[p].size()) continue;
                            for (r = 0, s = goalPaths[p].size(); r < s; r++)
                            {
                                if (!goalPaths[p][r].equals(path[r])) break;
                            }
                            if (r < s) continue;
                            break;
                        }
                        if (p == q)
                        {
                            goalInstances.push_back(instanceSeeds[j]);
                            goalPaths.push_back(path);
                        }
                    }
                }
            }

            // Score the state.
            double length = 0.0;
            for (i = 0, j = goalPaths.size(); i < j; i++)
            {
                length += (double)(goalPaths[i].size());
            }
            if (j == 0) return;
            length /= (double)j;

            // The score is a combination of the number of unique successful
            // paths as the most significant component and the average
            // path length as the lesser component.
            score = (double)(goalPaths.size()) * 1000.0;
            assert(length < 1000.0);
            score += length;
        }

        // Print the meta-maze configuration and goal paths.
        void print()
        {
            int i,j,p,q;

            printf("numRooms=%d, numDoors=%d, numGoals=%d\n",
                numRooms, numDoors, numGoals);
            printf("contextSizes: ");
            for (i = 0, j = contextSizes.size(); i < j; i++)
            {
                printf("%d", contextSizes[i]);
                if (i < j-1) printf(",");
            }
            printf("\n");
            printf("effectDelayScale=%d\n", effectDelayScale);
            printf("metaSeed=%d\n", metaSeed);
            printf("instance seeds/frequencies:");
            for (i = 0, j = instanceSeeds.size(); i < j; i++)
            {
                printf(" %d (%f)", instanceSeeds[i], instanceFrequencies[i]);
            }
            printf("\n");
            printf("score=%f\n", score);
            printf("goal paths:\n");
            for (i = 0, j = goalPaths.size(); i < j; i++)
            {
                printf("instanceSeed=%d\n", goalInstances[i]);
                for (p = 0, q = goalPaths[i].size(); p < q; p++)
                {
                    goalPaths[i][p].room.print();
                    printf("Door=%d\n", goalPaths[i][p].door);
                }
            }
        }

        // Dump in Mona input format.
        void dumpMonaFormat(FILE *fp=stdout)
        {
            int i,j;

            fprintf(fp, "%d %d %d ", numRooms, numDoors, numGoals);
            for (i = 0, j = contextSizes.size(); i < j; i++)
            {
                fprintf(fp, "%d", contextSizes[i]);
                if (i < j-1) fprintf(fp, ",");
            }
            fprintf(fp, " ");
            fprintf(fp, "%d ", effectDelayScale);
            fprintf(fp, "%d ", metaSeed);
        }

        // Dump in SNNS format.
        // Assumes path has been pre-generated.
        void dumpSNNSformat(int instance, FILE *fp=stdout)
        {
            int i,j,p,q;
            PathElement pathElem;
            time_t clock;

            fprintf(fp, "SNNS pattern definition file V3.2\n");
            clock = time(0);
            fprintf(fp, "generated at %s\n\n", ctime(&clock));
            fprintf(fp, "No. of patterns : %d\n", goalPaths[instance].size());
            fprintf(fp, "No. of input units : %d\n", numDoors);
            fprintf(fp, "No. of output units : %d\n\n", numDoors + 1);
            for (i = 0, j = goalPaths[instance].size(); i < j; i++)
            {
                pathElem = goalPaths[instance][i];
                fprintf(fp, "# Input pattern %d:\n", i + 1);
                fprintf(fp, "%f ", (double)pathElem.room.mark);
                for (p = 0, q = pathElem.room.doors.size(); p < q; p++)
                {
                    if (pathElem.room.doors[p])
                    {
                        fprintf(fp, "1.0 ");
                    }
                    else
                    {
                        fprintf(fp, "0.0 ");
                    }
                }
                for (p = 0, q = pathElem.room.goals.size(); p < q; p++)
                {
                    if (pathElem.room.goals[p])
                    {
                        fprintf(fp, "1.0");
                    }
                    else
                    {
                        fprintf(fp, "0.0");
                    }
                    if (p < q-1) fprintf(fp, " ");
                }
                fprintf(fp, "\n");
                fprintf(fp, "# Output pattern %d:\n", i + 1);
                fprintf(fp, "%f\n", (double)pathElem.door);
            }
        }
};

// Path element comparisons.
bool operator==(MazePathFinder::PathElement &e1,
MazePathFinder::PathElement &e2)
{
    return e1.equals(e2);
}


bool operator!=(MazePathFinder::PathElement &e1,
MazePathFinder::PathElement &e2)
{
    if (e1 == e2) return false; else return true;
}
#endif
