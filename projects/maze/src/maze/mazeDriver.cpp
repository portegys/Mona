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

#include "mazeDriver.hpp"
#ifdef WIN32
#include <windows.h>
#endif
#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#endif

// Parameters.
const Mona::NEED MazeDriver::NEED_VALUE = 10.0;
const Mona::NEED MazeDriver::GOAL_VALUE = 5.0;

// Response training parameters.
const double MazeDriver::INITIAL_RESPONSE_OVERRIDE_POTENTIAL = 10.0;
const double MazeDriver::RESPONSE_OVERRIDE_DECREMENT = 0.0;

// Room types.
const int MazeDriver::START_ROOM_MARK = 0;
const int MazeDriver::BEGIN_MAZE_MARK = 1;
const int MazeDriver::END_MAZE_MARK = 2;
const int MazeDriver::GOAL_ROOM_MARK = 3;
const int MazeDriver::DEAD_ROOM_MARK = 4;
const int MazeDriver::MAZE_ROOM_MARK = 5;

// Constructors.
MazeDriver::MazeDriver(int numRooms, int numDoors, int numGoals,
vector<int> &contextSizes, int effectDelayScale,
RANDOM metaSeed,
vector<RANDOM> &instanceSeeds,
bool contextMaze, bool markPath)
{
    int i,j;
    MazeRoom *room;
    VALUE_SET *goals;
    bool addGoal;
    char description[100];
    Mona::Receptor *receptor;
    Mona::Motor *motor;
    list<Mona::Neuron *>::iterator neuronItr;
    RANDOM instanceSeed;

    this->numRooms = numRooms;
    this->numDoors = numDoors;
    this->numGoals = numGoals;
    this->contextSizes.resize(contextSizes.size());
    for (i = 0, j = contextSizes.size(); i < j; i++)
    {
        this->contextSizes[i] = contextSizes[i];
    }
    this->effectDelayScale = effectDelayScale;
    this->metaSeed = metaSeed;
    this->instanceSeeds.resize(instanceSeeds.size());
    for (i = 0, j = instanceSeeds.size(); i < j; i++)
    {
        this->instanceSeeds[i] = instanceSeeds[i];
    }
    this->contextMaze = contextMaze;
    this->markPath = markPath;

    // Set parameters.
    numSensors = numDoors + numGoals + 1;
    maxResponse = numDoors;
    numNeeds = numGoals;

    // Create mona.
    mona = new Mona(numSensors, maxResponse, numNeeds);
    assert(mona != NULL);

    // Assign effect delay timers.
    for (i = 0, j = 1; i <= Mona::MAX_MEDIATOR_LEVEL; i++, j *= 2)
    {
        mona->eventTimers[i].resize(1);
        if (i == 0)
        {
            mona->eventTimers[i][0] = 1;
        }
        else
        {
            mona->eventTimers[i][0] = j * effectDelayScale;
        }
    }

    // Add needs.
    for (i = 0; i < numNeeds; i++)
    {
        sprintf(description, "Need %d", i);
        mona->initNeed(i, NEED_VALUE, description);
    }

    // Use room configurations to create receptors.
    if (instanceSeeds.size() > 0)
    {
        instanceSeed = instanceSeeds[0];
    }
    else
    {
        instanceSeed = metaSeed;
    }

    maze = new Maze(numRooms, numDoors, numGoals,
        contextSizes, effectDelayScale,
        metaSeed, instanceSeed, true);
    assert(maze != NULL);
    markMaze();
    sensors = new Mona::SENSOR[numSensors];
    assert(sensors != NULL);
    goals = new VALUE_SET;
    assert(goals != NULL);
    goals->alloc(numGoals);
    for (i = 0; i < numRooms; i++)
    {
        room = maze->rooms[i];
        for (j = 0; j < numDoors; j++)
        {
            if (room->doors[j])
            {
                sensors[j] = 1;
            }
            else
            {
                sensors[j] = 0;
            }
        }
        for (j = 0; j < numGoals; j++)
        {
            goals->set(j, 0.0);
        }
        addGoal = false;
        for (j = 0; j < numGoals; j++)
        {
            if (room->goals[j])
            {
                sensors[j + numDoors] = 1;
                goals->set(j, GOAL_VALUE);
                addGoal = true;
            }
            else
            {
                sensors[j + numDoors] = 0;
            }
        }
        sensors[numDoors + numGoals] = room->mark;
        for (neuronItr = mona->receptors.begin();
            neuronItr != mona->receptors.end(); neuronItr++)
        {
            if (((Mona::Receptor *)*neuronItr)->isDuplicate(sensors))
            {
                break;
            }
        }
        if (neuronItr == mona->receptors.end())
        {
            if (i == 0)
            {
                if (contextMaze)
                {
                    sprintf(description, "Begin maze room");
                }
                else
                {
                    sprintf(description, "Start room");
                }
            }
            else
            {
                sprintf(description, "Room %d", room->id);
            }
            if (contextMaze)
            {
                receptor = mona->newReceptor(sensors, description);
            }
            else
            {
                if (room->mark == GOAL_ROOM_MARK)
                {
                    sprintf(description, "Goal room %d", room->id);
                }
                receptor = mona->newReceptor(sensors, description);
                if (addGoal)
                {
                    receptor->goals.setGoals(*goals, 1.0);
                }
            }
        }
        else
        {
            receptor = (Mona::Receptor *)*neuronItr;
            sprintf(description, "%s,%d", receptor->id.description, room->id);
            receptor->id.setDescription(description);
        }
    }
    if (contextMaze)
    {
        for (i = 0; i < numGoals; i++)
        {
            sensors[i + numDoors] = 0;
        }
        sensors[numDoors + numGoals] = START_ROOM_MARK;
        for (i = 0; i < numDoors; i++)
        {
            for (j = 0; j < numDoors; j++)
            {
                sensors[j] = 0;
            }
            sensors[i] = 1;
            sprintf(description, "Start room door %d", i);
            receptor = mona->newReceptor(sensors, description);
        }
        for (i = 0; i < numDoors; i++)
        {
            sensors[i] = 1;
        }
        sensors[numDoors + numGoals] = END_MAZE_MARK;
        sprintf(description, "End maze room");
        receptor = mona->newReceptor(sensors, description);
        for (i = 0; i < numDoors; i++)
        {
            sensors[i] = 0;
        }
        for (i = 0; i < numGoals; i++)
        {
            sensors[i + numDoors] = 0;
        }
        sensors[numDoors] = 1;
        sensors[numDoors + numGoals] = GOAL_ROOM_MARK;
        for (neuronItr = mona->receptors.begin();
            neuronItr != mona->receptors.end(); neuronItr++)
        {
            if (((Mona::Receptor *)*neuronItr)->isDuplicate(sensors))
            {
                break;
            }
        }
        if (neuronItr == mona->receptors.end())
        {
            sprintf(description, "Goal room");
            receptor = mona->newReceptor(sensors, description);
            for (i = 0; i < numGoals; i++)
            {
                goals->set(i, 0.0);
            }
            goals->set(0, GOAL_VALUE);
            receptor->goals.setGoals(*goals, 1.0);
        }
    }
    delete goals;

    // Add motor neurons.
    for (i = 0; i <= maxResponse; i++)
    {
        if (i < numDoors)
        {
            sprintf(description, "Go door %d", i);
        }
        else
        {
            sprintf(description, "Wait");
        }
        motor = mona->newMotor(i, description);
    }
}


// Construct from file.
MazeDriver::MazeDriver(char *loadFile)
{
    load(loadFile);
}


// Default constructor.
MazeDriver::MazeDriver()
{
    numRooms = numDoors = numGoals = 0;
    effectDelayScale = 0;
    metaSeed = INVALID_RANDOM;
    maze = NULL;
    numSensors = maxResponse = numNeeds = 0;
    sensors = NULL;
    mona = NULL;
}


// Destructor.
MazeDriver::~MazeDriver()
{
    if (maze != NULL) delete maze;
    if (mona != NULL) delete mona;
    if (sensors != NULL) delete sensors;
}


// Load from file.
void MazeDriver::load(char *loadFile)
{
    int i,j,k;
    RANDOM l;
    FILE *fp;

    assert(loadFile != NULL);
    assert((fp = fopen(loadFile, "r")) != NULL);
    FREAD_INT(&numRooms, fp);
    FREAD_INT(&numDoors, fp);
    FREAD_INT(&numGoals, fp);
    FREAD_INT(&j, fp);
    for (i = 0; i < j; i++)
    {
        FREAD_INT(&k, fp);
        contextSizes.push_back(k);
    }
    FREAD_INT(&effectDelayScale, fp);
    FREAD_LONG(&metaSeed, fp);
    FREAD_INT(&j, fp);
    for (i = 0; i < j; i++)
    {
        FREAD_LONG(&l, fp);
        instanceSeeds.push_back(l);
    }
    FREAD_BOOL(&contextMaze, fp);
    FREAD_BOOL(&markPath, fp);

    // Load mona.
    assert((mona = Mona::load(fp)) != NULL);
    fclose(fp);
    numSensors = mona->numSensors;
    maxResponse = mona->maxResponse;
    numNeeds = mona->numNeeds;
    sensors = new Mona::SENSOR[numSensors];
    assert(sensors != NULL);

    // Force new maze.
    maze = NULL;
}


// Save to file.
void MazeDriver::save(char *saveFile)
{
    int i,j,k;
    RANDOM l;
    FILE *fp;

    assert(saveFile != NULL);
    assert((fp = fopen(saveFile, "w")) != NULL);
    FWRITE_INT(&numRooms, fp);
    FWRITE_INT(&numDoors, fp);
    FWRITE_INT(&numGoals, fp);
    j = contextSizes.size();
    FWRITE_INT(&j, fp);
    for (i = 0; i < j; i++)
    {
        k = contextSizes[i];
        FWRITE_INT(&k, fp);
    }
    FWRITE_INT(&effectDelayScale, fp);
    FWRITE_LONG(&metaSeed, fp);
    j = instanceSeeds.size();
    FWRITE_INT(&j, fp);
    for (i = 0; i < j; i++)
    {
        l = instanceSeeds[i];
        FWRITE_LONG(&l, fp);
    }
    FWRITE_BOOL(&contextMaze, fp);
    FWRITE_BOOL(&markPath, fp);
    mona->save(fp);
    fclose(fp);
}


// Run: return number of successful trials.
int MazeDriver::run(int trials, int cycles,
bool train, TRAIN_TYPE trainType)
{
    int trial,goalCount,trialsFindingGoals,cycle,i,j;
    RANDOM instanceSeed;
    vector<double> instanceFrequencies;
    int override;
    double potential;
    MazeMap *map;
    MazeRoom room;
    Mona::NEED need;
    int door;
    int response;
    int contextState;
    int contextDoor;
    MAZE_CONFIG mazeConfig;

    // Initialize training state.
    potential = INITIAL_RESPONSE_OVERRIDE_POTENTIAL;
    for (i = 0; i <= maxResponse; i++)
    {
        mona->responseInhibitors[i] = false;
        mona->responseOverridePotentials[i] = 0.0;
    }
    mona->responseRandomness = Mona::RESIDUAL_RESPONSE_RANDOMNESS;

    // Default instance frequencies.
    // If otherwise required, set them here.
    if (trainType == INSTANCE_TRAIN && instanceSeeds.size() == 0)
    {
        instanceSeeds.push_back(randomizer->RAND());
    }
    j = instanceSeeds.size();
    if (j > 0)
    {
        instanceFrequencies.resize(j);
        for (i = 0; i < j; i++)
        {
            instanceFrequencies[i] = 1.0;
        }
    }

    // Run.
    #if ( MONA_TRACE || MAZE_TRACE )
    printf("Begin run:\n");
    #endif
    trialsFindingGoals = 0;
    for (trial = 0; trial < trials; trial++)
    {
        #if ( MONA_TRACE || MAZE_TRACE )
        printf("==============================\nTrial=%d\n",trial);
        #endif

        // Create maze:
        // "Meta maze" probabilities are the same
        // for all trials; specific maze configurations
        // for each trial are randomly resolved using
        // these probabilities.
        if (maze == NULL)
        {
            if (instanceSeeds.size() > 0)
            {
                instanceSeed = instanceSeeds[randomizer->RAND_CHOICE(instanceSeeds.size())];
            }
            else
            {
                instanceSeed = randomizer->RAND();
            }
            maze = new Maze(numRooms, numDoors, numGoals,
                contextSizes, effectDelayScale,
                metaSeed, instanceSeed, true);
            assert(maze != NULL);
            markMaze();
        }
        else
        {
            if (instanceSeeds.size() > 0)
            {
                instanceSeed = instanceSeeds[0];
            }
            else
            {
                instanceSeed = metaSeed;
            }
        }
        #if ( MONA_TRACE || MAZE_TRACE )
        printf("Instance seed=%d\n",instanceSeed);
        #endif

        // If response training, map maze.
        if (train)
        {
            if (trainType == META_TRAIN)
            {
                map = new MazeMap(MazeMap::META_MAP);
                assert(map != NULL);
                if ((j = instanceSeeds.size()) > 0)
                {
                    for (i = 0; i < j; i++)
                    {
                        if (instanceSeed == instanceSeeds[i]) break;
                    }
                    map->mapMaze(maze, i, instanceSeeds,
                        instanceFrequencies);
                }
                else
                {
                    map->mapMaze(maze);
                }
            } else if (trainType == INSTANCE_TRAIN)
            {
                map = new MazeMap(MazeMap::INSTANCE_MAP);
                assert(map != NULL);
                map->mapMaze(maze);
            }
        }

        // Clear short term memory.
        mona->clearWorkingMemory();

        // Reset needs.
        for (i = 0; i < numNeeds; i++)
        {
            mona->needs.set(i, NEED_VALUE);
            mona->oldNeeds.set(i, NEED_VALUE);
        }

        #if ( MONA_TRACE || MAZE_TRACE )
        // Dump maze.
        maze->dump();
        #endif

        // If context maze, determine which door to use.
        if (contextMaze)
        {
            if (train)
            {
                if ((trial / 10) % 2)
                {
                    mazeConfig = ENDS;
                }
                else
                {
                    mazeConfig = MIDDLE;
                }
            }
            else
            {
                mazeConfig = COMPLETE;
            }
            if (mazeConfig == MIDDLE)
            {
                contextState = MAZE_ROOM_MARK;
            }
            else
            {
                contextState = START_ROOM_MARK;
                contextDoor = randomizer->RAND_CHOICE(numDoors);
            }
        }

        // Run cycles.
        for (cycle = goalCount = 0; cycle < cycles && goalCount == 0; cycle++)
        {
            #if ( MONA_TRACE || MAZE_TRACE )
            printf("------------------------------\nCycle=%d\n",cycle);
            #endif

            // Get current room.
            maze->getRoom(room);

            // If running context maze option, substitute rooms.
            if (contextMaze)
            {
                switch(contextState)
                {
                    case START_ROOM_MARK:
                        for (i = 0; i < numDoors; i++)
                        {
                            room.doors[i] = false;
                        }
                        room.doors[contextDoor] = true;
                        for (i = 0; i < numGoals; i++)
                        {
                            room.goals[i] = false;
                        }
                        room.mark = START_ROOM_MARK;
                        break;
                    case MAZE_ROOM_MARK:
                        if (room.mark == GOAL_ROOM_MARK)
                        {
                            contextState = END_MAZE_MARK;
                        }
                        else
                        {
                            break;
                        }
                    case END_MAZE_MARK:
                        for (i = 0; i < numDoors; i++)
                        {
                            room.doors[i] = true;
                        }
                        for (i = 0; i < numGoals; i++)
                        {
                            room.goals[i] = false;
                        }
                        room.mark = END_MAZE_MARK;
                        break;
                    case GOAL_ROOM_MARK:
                        for (i = 0; i < numDoors; i++)
                        {
                            room.doors[i] = false;
                        }
                        for (i = 0; i < numGoals; i++)
                        {
                            room.goals[i] = false;
                        }
                        room.goals[0] = true;
                        room.mark = GOAL_ROOM_MARK;
                        break;
                    case DEAD_ROOM_MARK:
                        for (i = 0; i < numDoors; i++)
                        {
                            room.doors[i] = false;
                        }
                        for (i = 0; i < numGoals; i++)
                        {
                            room.goals[i] = false;
                        }
                        room.mark = DEAD_ROOM_MARK;
                        break;
                }
            }

            #if ( MONA_TRACE || MAZE_TRACE )
            room.print();
            #endif

            // Show room to Mona.
            for (i = 0; i < numDoors; i++)
            {
                sensors[i] = (int)room.doors[i];
            }
            for (i = 0; i < numGoals; i++)
            {
                sensors[i+numDoors] = (int)room.goals[i];

                // Reduce need associated with goal.
                if (sensors[i+numDoors] == 1)
                {
                    goalCount++;
                    need = mona->getNeed(i);
                    need -= GOAL_VALUE;
                    if (need < 0.0) need = 0.0;
                    mona->setNeed(i, need);
                }
            }
            sensors[numDoors + numGoals] = room.mark;

            // Reached end of middle maze?
            if (contextMaze && mazeConfig == MIDDLE &&
                room.mark == END_MAZE_MARK)
            {
                goalCount++;
            }

            // If response training, tend to take door toward most needed goal.
            if (train)
            {
                for (i = 0; i <= maxResponse; i++)
                {
                    mona->responseInhibitors[i] = false;
                }
                for (i = 0; i < numDoors; i++)
                {
                    if (!room.doors[i])
                    {
                        mona->responseInhibitors[i] = true;
                    }
                }
                for (i = 0; i < numGoals; i++)
                {
                    if (room.goals[i]) break;
                }
                if (i == numGoals)
                {
                    mona->responseInhibitors[maxResponse] = true;
                }
                if (contextMaze &&
                    (contextState == START_ROOM_MARK ||
                    contextState == END_MAZE_MARK))
                {
                    if (mazeConfig == COMPLETE || mazeConfig == ENDS)
                    {
                        override = contextDoor;
                    }
                    else
                    {
                        override = maxResponse;
                    }
                } else if (contextMaze && mazeConfig == ENDS)
                {
                    override = maxResponse;
                }
                else
                {
                    for (i = j = 0, need = 0.0; i < numNeeds; i++)
                    {
                        if (mona->needs.get(i) > need)
                        {
                            j = i;
                            need = mona->needs.get(i);
                        }
                    }
                    override = map->gotoGoal(j);
                }
                for (i = 0; i <= maxResponse; i++)
                {
                    mona->responseOverridePotentials[i] = 0.0;
                }
                #ifdef NEVER
                if (override != -1)
                {
                    mona->responseOverridePotentials[override] = potential;
                }
                potential -= RESPONSE_OVERRIDE_DECREMENT;
                if (potential < 0.0) potential = 0.0;
                #else
                mona->responseOverride = override;
                #endif
            }

            // Get response.
            response = mona->cycle(sensors);

            // Respond to room.
            door = response;
            if (door >= 0 && door < numDoors)
            {
                if (!(contextMaze &&
                    (contextState == START_ROOM_MARK ||
                    contextState == END_MAZE_MARK)))
                {
                    maze->chooseDoor(door);

                    if (train)
                    {
                        // Update location in map.
                        map->chooseDoor(door);
                    }
                }
                #if ( MONA_TRACE || MAZE_TRACE )
                printf("Response: Door %d\n", door);
                #endif
            }
            else
            {
                #if ( MONA_TRACE || MAZE_TRACE )
                printf("Response: Wait\n");
                #endif
            }

            // Determine next context maze state.
            if (contextMaze &&
                (mazeConfig == COMPLETE ||
                mazeConfig == ENDS))
            {
                if (contextState == START_ROOM_MARK)
                {
                    if (door == contextDoor)
                    {
                        contextState = BEGIN_MAZE_MARK;
                    }
                } else if (contextState == BEGIN_MAZE_MARK)
                {
                    if (mazeConfig == ENDS)
                    {
                        contextState = END_MAZE_MARK;
                    }
                    else
                    {
                        contextState = MAZE_ROOM_MARK;
                    }
                } else if (contextState == END_MAZE_MARK)
                {
                    if (door == contextDoor)
                    {
                        contextState = GOAL_ROOM_MARK;
                    }
                    else
                    {
                        contextState = DEAD_ROOM_MARK;
                    }
                } else if (contextState == DEAD_ROOM_MARK)
                {
                    // Mona has seen dead room, so end trial.
                    break;
                }
            }
        }
        delete maze;
        maze = NULL;
        if (train) delete map;

        #if ( MONA_TRACE || MAZE_TRACE )
        printf("Trial=%d: %d goals found\n", trial, goalCount);
        #endif
        if (goalCount > 0) trialsFindingGoals++;
    }

    #if ( MONA_TRACE || MAZE_TRACE )
    printf("==============================\n");

    // Print special mediators.
    printSpecialMediators();
    #endif

    // Return resuls.
    return trialsFindingGoals;
}


// Create SNNS neural network pattern files.
// These are named:
// <snns file prefix>_<meta-maze seed>_<doors per room>
//   /ends/<context door>.pat
//   /middle/<instance seed>_<rooms in maze>].pat
//   /complete/<instance seed>_<rooms in maze>_<context door>.pat
void MazeDriver::createSNNSpatterns(char *snnsPrefix)
{
    int i,j,contextDoor;
    RANDOM instanceSeed;
    vector<double> instanceFrequencies;
    MazeMap *map;
    char patterndir[1024];
    #ifdef WIN32
    char basedir[1024];
    #endif

    if (snnsPrefix == NULL) snnsPrefix = "snns";

    if (instanceSeeds.size() == 0)
    {
        instanceSeeds.push_back(randomizer->RAND());
    }
    j = instanceSeeds.size();
    if (j > 0)
    {
        instanceFrequencies.resize(j);
        for (i = 0; i < j; i++)
        {
            instanceFrequencies[i] = 1.0;
        }
    }

    // Create pattern directory.
    sprintf(patterndir, "%s_%d_%d", snnsPrefix, metaSeed, numDoors);
    #ifdef WIN32
    if (CreateDirectory(patterndir, NULL) == 0 && GetLastError() != ERROR_ALREADY_EXISTS)
    #else
        if (mkdir(patterndir, 0755) == -1 && errno != EEXIST)
    #endif
    {
        fprintf(stderr, "Cannot create pattern directory %s\n", patterndir);
        exit(1);
    }
    #ifdef WIN32
    if (GetCurrentDirectory(1023, basedir) == 0)
    {
        fprintf(stderr, "Cannot get current directory\n");
        exit(1);
    }
    if (SetCurrentDirectory(patterndir) == 0)
    #else
        if (chdir(patterndir) == -1)
    #endif
    {
        fprintf(stderr, "Cannot change to pattern directory %s\n", patterndir);
        exit(1);
    }
    #ifdef WIN32
    if (GetCurrentDirectory(1023, patterndir) == 0)
    {
        fprintf(stderr, "Cannot get pattern directory\n");
        exit(1);
    }
    #endif

    // If context maze, generate maze "ends" and "complete" patterns.
    delete maze;
    if (contextMaze)
    {
        #ifdef WIN32
        if (CreateDirectory("ends", NULL) == 0 && GetLastError() != ERROR_ALREADY_EXISTS)
        #else
            if (mkdir("ends", 0755) == -1 && errno != EEXIST)
        #endif
        {
            fprintf(stderr, "Cannot create \"ends\" pattern subdirectory\n");
            exit(1);
        }
        #ifdef WIN32
        if (CreateDirectory("complete", NULL) == 0 && GetLastError() != ERROR_ALREADY_EXISTS)
        #else
            if (mkdir("complete", 0755) == -1 && errno != EEXIST)
        #endif
        {
            fprintf(stderr, "Cannot create \"complete\" pattern subdirectory\n");
            exit(1);
        }

        for (contextDoor = 0; contextDoor < numDoors; contextDoor++)
        {
            // Create maze and maze map.
            instanceSeed = instanceSeeds[0];
            maze = new Maze(numRooms, numDoors, numGoals,
                contextSizes, effectDelayScale,
                metaSeed, instanceSeed, true);
            assert(maze != NULL);
            markMaze();
            map = new MazeMap(MazeMap::META_MAP);
            assert(map != NULL);
            map->mapMaze(maze, 0, instanceSeeds,
                instanceFrequencies);

            // Generate pattern.
            #ifdef WIN32
            if (SetCurrentDirectory("ends") == 0)
            #else
                if (chdir("ends") == -1)
            #endif
            {
                fprintf(stderr, "Cannot change to pattern \"ends\" sub-directory\n");
                exit(1);
            }

            createSNNSpatternFile(snnsPrefix,
                map, instanceSeed, ENDS, 0, contextDoor);
            delete map;
            delete maze;
            #ifdef WIN32
            if (SetCurrentDirectory(patterndir) == 0)
            #else
                if (chdir("..") == -1)
            #endif
            {
                fprintf(stderr, "Cannot change to pattern directory\n");
                exit(1);
            }
            #ifdef WIN32
            if (SetCurrentDirectory("complete") == 0)
            #else
                if (chdir("complete") == -1)
            #endif
            {
                fprintf(stderr, "Cannot change to pattern \"complete\" sub-directory\n");
                exit(1);
            }

            for (i = 0, j = instanceSeeds.size(); i < j; i++)
            {
                // Create maze and maze map.
                instanceSeed = instanceSeeds[i];
                maze = new Maze(numRooms, numDoors, numGoals,
                    contextSizes, effectDelayScale,
                    metaSeed, instanceSeed, true);
                assert(maze != NULL);
                markMaze();
                map = new MazeMap(MazeMap::META_MAP);
                assert(map != NULL);
                map->mapMaze(maze, i, instanceSeeds,
                    instanceFrequencies);

                // Generate pattern.
                createSNNSpatternFile(snnsPrefix,
                    map, instanceSeed, COMPLETE, 0, contextDoor);
                delete map;
                delete maze;
            }
            #ifdef WIN32
            if (SetCurrentDirectory(patterndir) == 0)
            #else
                if (chdir("..") == -1)
            #endif
            {
                fprintf(stderr, "Cannot change to pattern directory\n");
                exit(1);
            }
        }
    }

    // Generate "middle" maze patterns.
    #ifdef WIN32
    if (CreateDirectory("middle", NULL) == 0 && GetLastError() != ERROR_ALREADY_EXISTS)
    #else
        if (mkdir("middle", 0755) == -1 && errno != EEXIST)
    #endif
    {
        fprintf(stderr, "Cannot create \"middle\" pattern subdirectory\n");
        exit(1);
    }
    #ifdef WIN32
    if (SetCurrentDirectory("middle") == 0)
    #else
        if (chdir("middle") == -1)
    #endif
    {
        fprintf(stderr, "Cannot change to pattern \"middle\" sub-directory\n");
        exit(1);
    }
    for (i = 0, j = instanceSeeds.size(); i < j; i++)
    {
        // Create maze and maze map.
        instanceSeed = instanceSeeds[i];
        maze = new Maze(numRooms, numDoors, numGoals,
            contextSizes, effectDelayScale,
            metaSeed, instanceSeed, true);
        assert(maze != NULL);
        markMaze();
        map = new MazeMap(MazeMap::META_MAP);
        assert(map != NULL);
        map->mapMaze(maze, i, instanceSeeds,
            instanceFrequencies);

        // Generate pattern.
        createSNNSpatternFile(snnsPrefix,
            map, instanceSeed, MIDDLE, 0, 0);
        delete map;
        delete maze;
    }

    // Restore.
    #ifdef WIN32
    if (SetCurrentDirectory(basedir) == 0)
    #else
        if (chdir("../..") == -1)
    #endif
    {
        fprintf(stderr, "Cannot change to base directory\n");
        exit(1);
    }
    maze = new Maze(numRooms, numDoors, numGoals,
        contextSizes, effectDelayScale,
        metaSeed, instanceSeeds[0], true);
    assert(maze != NULL);
    markMaze();
}


// Create an SNNS pattern file.
void MazeDriver::createSNNSpatternFile(char *snnsPrefix,
MazeMap *map, RANDOM instanceSeed, MAZE_CONFIG mazeConfig,
int goal, int contextDoor)
{
    int i,j,k,marks,count,door,contextState;
    time_t clock;
    MazeRoom room;
    bool done;
    char buf[100],*s;
    vector<char *> text;
    FILE *fp;

    // Determine starting point.
    if (mazeConfig == MIDDLE)
    {
        contextState = BEGIN_MAZE_MARK;
    }
    else
    {
        contextState = START_ROOM_MARK;
    }

    // Open pattern file.
    switch(mazeConfig)
    {
        case COMPLETE:
            sprintf(buf, "%d_%d_%d.pat", instanceSeed, numRooms, contextDoor);
            break;
        case ENDS:
            sprintf(buf, "%d.pat", contextDoor);
            break;
        case MIDDLE:
            sprintf(buf, "%d_%d.pat", instanceSeed, numRooms);
            break;
    }
    if ((fp = fopen(buf, "w")) == NULL)
    {
        fprintf(stderr, "Cannot open pattern file %s\n", buf);
        exit(1);
    }

    // Generate input/output pattern to solve maze.
    count = 0;
    done = false;
    for (marks = 0;; marks++)
    {
        if (pow(2.0, (double)marks) >= (numRooms + 6)) break;
    }
    while (!done)
    {
        // Get current room.
        maze->getRoom(room);

        // If generating context maze, substitute rooms.
        switch (contextState)
        {
            case START_ROOM_MARK:
                for (i = 0; i < numDoors; i++)
                {
                    room.doors[i] = false;
                }
                room.doors[contextDoor] = true;
                for (i = 0; i < numGoals; i++)
                {
                    room.goals[i] = false;
                }
                room.mark = START_ROOM_MARK;
                contextState = BEGIN_MAZE_MARK;
                break;
            case BEGIN_MAZE_MARK:
                if (mazeConfig == ENDS)
                {
                    for (i = 0; i < numDoors; i++)
                    {
                        room.doors[i] = true;
                    }
                    for (i = 0; i < numGoals; i++)
                    {
                        room.goals[i] = false;
                    }
                    room.mark = END_MAZE_MARK;
                    contextState = END_MAZE_MARK;
                }
                else
                {
                    contextState = MAZE_ROOM_MARK;
                }
                break;
            case MAZE_ROOM_MARK:
                if (room.mark == GOAL_ROOM_MARK)
                {
                    if (mazeConfig == COMPLETE)
                    {
                        for (i = 0; i < numDoors; i++)
                        {
                            room.doors[i] = true;
                        }
                        for (i = 0; i < numGoals; i++)
                        {
                            room.goals[i] = false;
                        }
                        room.mark = END_MAZE_MARK;
                        contextState = END_MAZE_MARK;
                    }
                    else
                    {
                        done = true;
                    }
                }
                break;
            case END_MAZE_MARK:
                done = true;
        }
        if (done) break;

        // Record room input pattern.
        count++;
        sprintf(buf, "# Input pattern %d:\n", count);
        s = new char[strlen(buf) + 1];
        assert(s != NULL);
        strcpy(s, buf);
        text.push_back(s);
        j = room.mark;
        buf[0] = '\0';
        for (i = marks - 1; i >= 0; i--)
        {
            k = (int)pow(2.0, (double)i);
            if (j >= k)
            {
                j -= k;
                strcat(buf, "1 ");
            }
            else
            {
                strcat(buf, "0 ");
            }
        }
        for (i = 0; i < numDoors; i++)
        {
            if (room.doors[i])
            {
                strcat(buf, "1 ");
            }
            else
            {
                strcat(buf, "0 ");
            }
        }
        strcat(buf, "\n");
        s = new char[strlen(buf) + 1];
        assert(s != NULL);
        strcpy(s, buf);
        text.push_back(s);

        // Take door toward goal.
        if (contextState == BEGIN_MAZE_MARK ||
            contextState == END_MAZE_MARK)
        {
            door = contextDoor;
        }
        else
        {
            door = map->gotoGoal(goal);
            if (door < 0)
            {
                fprintf(stderr, "Goal in maze unreachable");
                exit(1);
            }
            if (door < numDoors)
            {
                maze->chooseDoor(door);
                map->chooseDoor(door);
            }
        }

        // Record door output pattern.
        sprintf(buf, "# Output pattern %d:\n", count);
        s = new char[strlen(buf) + 1];
        assert(s != NULL);
        strcpy(s, buf);
        text.push_back(s);
        buf[0] = '\0';
        for (i = 0; i < numDoors; i++)
        {
            if (door == i)
            {
                strcat(buf, "1 ");
            }
            else
            {
                strcat(buf, "0 ");
            }
        }
        strcat(buf, "\n");
        s = new char[strlen(buf) + 1];
        assert(s != NULL);
        strcpy(s, buf);
        text.push_back(s);

        // Goal in initial room.
        if (door >= numDoors) done = true;
    }

    // Write pattern file.
    fprintf(fp, "SNNS pattern definition file V3.2\n");
    clock = time(0);
    fprintf(fp, "generated at %s\n\n", ctime(&clock));
    fprintf(fp, "No. of patterns : %d\n", count);
    fprintf(fp, "No. of input units : %d\n", numDoors + marks);
    fprintf(fp, "No. of output units : %d\n\n", numDoors);
    for (i = 0, j = text.size(); i < j; i++)
    {
        s = text[i];
        fprintf(fp, "%s", s);
        delete s;
    }
    fclose(fp);
}


void
#ifdef ACTIVITY_TRACKING
MazeDriver::dumpMonaNetwork(enum Mona::DUMP_TYPE dumpType, char *title,
bool enablementDump, bool driveDump, bool delimiter, FILE *out)
#else
MazeDriver::dumpMonaNetwork(enum Mona::DUMP_TYPE dumpType, char *title,
bool delimiter, FILE *out)
#endif
{
    #ifdef ACTIVITY_TRACKING
    mona->dumpNetwork(dumpType, title,
    enablementDump, driveDump, delimiter, out);
    #else
    mona->dumpNetwork(dumpType, title, delimiter, out);
    #endif
}


// Mark the maze.
void MazeDriver::markMaze()
{
    int i,j;
    MazeRoom *room;

    for (i = 0; i < numRooms; i++)
    {
        room = maze->rooms[i];
        if (i == 0)
        {
            if (contextMaze)
            {
                room->mark = BEGIN_MAZE_MARK;
            }
            else
            {
                room->mark = START_ROOM_MARK;
            }
        }
        else
        {
            room->mark = MAZE_ROOM_MARK;
            if (markPath) room->mark += i;
        }
        for (j = 0; j < numGoals; j++)
        {
            if (room->goals[j])
            {
                room->mark = GOAL_ROOM_MARK;
                break;
            }
        }
    }
}


// Print special mediators.
void MazeDriver::printSpecialMediators()
{
    Mona::Mediator *mediator,*mediator2;
    Mona::Receptor *receptor;
    Mona::Motor *motor;
    list<Mona::Neuron *>::iterator neuronItr;

    printf("Goal path mediators:\n");
    for (neuronItr = mona->mediators.begin();
        neuronItr != mona->mediators.end(); neuronItr++)
    {
        mediator = (Mona::Mediator *)*neuronItr;
        if (mediator->level != 0) continue;
        if (mediator->causes.size() != 1) continue;
        if (mediator->causes[0]->type != Mona::RECEPTOR) continue;
        receptor = (Mona::Receptor *)(mediator->causes[0]);
        if (receptor->sensorMask[numDoors + numGoals] !=
            START_ROOM_MARK) continue;
        if (mediator->effect->type != Mona::RECEPTOR) continue;
        receptor = (Mona::Receptor *)(mediator->effect);
        if (receptor->sensorMask[numDoors + numGoals] !=
            GOAL_ROOM_MARK) continue;
        mediator->print();
    }

    printf("Internal maze mediators:\n");
    for (neuronItr = mona->mediators.begin();
        neuronItr != mona->mediators.end(); neuronItr++)
    {
        mediator = (Mona::Mediator *)*neuronItr;
        if (mediator->level != 0) continue;
        if (mediator->causes.size() != 1) continue;
        if (mediator->causes[0]->type != Mona::RECEPTOR) continue;
        receptor = (Mona::Receptor *)(mediator->causes[0]);
        if (receptor->sensorMask[numDoors + numGoals] !=
            BEGIN_MAZE_MARK) continue;
        if (mediator->effect->type != Mona::RECEPTOR) continue;
        receptor = (Mona::Receptor *)(mediator->effect);
        if (receptor->sensorMask[numDoors + numGoals] !=
            END_MAZE_MARK) continue;
        mediator->print();
    }

    printf("Context mediators:\n");
    for (int i = 0; i < numDoors; i++)
    {
        for (neuronItr = mona->mediators.begin();
            neuronItr != mona->mediators.end(); neuronItr++)
        {
            mediator = (Mona::Mediator *)*neuronItr;
            if (mediator->level != 1) continue;
            if (mediator->causes.size() != 1) continue;
            if (mediator->intermediates.size() != 0) continue;
            if (mediator->causes[0]->type != Mona::MEDIATOR) continue;
            if (mediator->effect->type != Mona::MEDIATOR) continue;
            mediator2 = (Mona::Mediator *)mediator->causes[0];
            if (mediator2->level != 0) continue;
            if (mediator2->causes.size() != 1) continue;
            if (mediator2->intermediates.size() != 1) continue;
            if (mediator2->causes[0]->type != Mona::RECEPTOR) continue;
            receptor = (Mona::Receptor *)(mediator2->causes[0]);
            if (receptor->sensorMask[numDoors + numGoals] !=
                START_ROOM_MARK) continue;
            if (mediator2->effect->type != Mona::RECEPTOR) continue;
            receptor = (Mona::Receptor *)(mediator2->effect);
            if (receptor->sensorMask[numDoors + numGoals] !=
                BEGIN_MAZE_MARK) continue;
            if (mediator2->intermediates[0]->type != Mona::MOTOR) continue;
            motor = (Mona::Motor *)(mediator2->intermediates[0]);
            if (motor->response != i) continue;
            mediator2 = (Mona::Mediator *)mediator->effect;
            if (mediator2->level != 0) continue;
            if (mediator2->causes.size() != 1) continue;
            if (mediator2->intermediates.size() != 1) continue;
            if (mediator2->causes[0]->type != Mona::RECEPTOR) continue;
            receptor = (Mona::Receptor *)(mediator2->causes[0]);
            if (receptor->sensorMask[numDoors + numGoals] !=
                END_MAZE_MARK) continue;
            if (mediator2->effect->type != Mona::RECEPTOR) continue;
            receptor = (Mona::Receptor *)(mediator2->effect);
            if (receptor->sensorMask[numDoors + numGoals] !=
                GOAL_ROOM_MARK) continue;
            if (mediator2->intermediates[0]->type != Mona::MOTOR) continue;
            motor = (Mona::Motor *)(mediator2->intermediates[0]);
            if (motor->response != i) continue;
            printf("Context for door=%d:\n", i); mediator->print();
            break;
        }
    }
}
