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

#include "mazeBridgeDriver.hpp"
#ifdef WIN32
#include <windows.h>
#endif
#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#endif

// Training type frequencies.
const int MazeBridgeDriver::TRAIN_TYPE_FREQS[NUM_MAZE_TYPES] =
{ 0, 1, 1, 0, 0 };

// Maximum dead room exposures.
const int MazeBridgeDriver::MAX_DEAD_ROOM_EXPOSURES = 10;

// Constructors.
MazeBridgeDriver::MazeBridgeDriver(RANDOM mazeSeed,
int numDoors, int bridgeLength, bool markPath)
{
    int i,j;
    MazeRoom *room;
    VALUE_SET *goals;
    bool addGoal;
    char description[100];
    Mona::Receptor *receptor;
    Mona::Motor *motor;
    list<Mona::Neuron *>::iterator neuronItr;

    numRooms = bridgeLength + 5;
    this->numDoors = numDoors;
    numGoals = 1;
    effectDelayScale = bridgeLength + 2;
    metaSeed = mazeSeed;
    this->markPath = markPath;

    // Set parameters.
    numSensors = numDoors + numGoals + 1;
    maxResponse = numDoors;
    numNeeds = numGoals;

    // Create mona.
    mona = new Mona(numSensors, maxResponse, numNeeds);
    assert(mona != NULL);

    // Assign effect delay timers.
    for (i = 0; i <= Mona::MAX_MEDIATOR_LEVEL; i++)
    {
        mona->eventTimers[i].resize(1);
        if (i == 0)
        {
            mona->eventTimers[i][0] = 1;
        }
        else
        {
            mona->eventTimers[i][0] = effectDelayScale;
        }
    }

    // Add needs.
    for (i = 0; i < numNeeds; i++)
    {
        sprintf(description, "Need %d", i);
        mona->initNeed(i, NEED_VALUE, description);
    }

    // Use room configurations to create receptors.
    instanceSeeds.push_back(randomizer->RAND());
    buildMaze(COMPLETE);
    sensors = new Mona::SENSOR[numSensors];
    assert(sensors != NULL);
    goals = new VALUE_SET;
    assert(goals != NULL);
    goals->alloc(numGoals);
    for (i = 0; i < numRooms; i++)
    {
        room = bridgeMaze[i]->room;
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
            sprintf(description, "Room %d", i);
            receptor = mona->newReceptor(sensors, description);
            if (addGoal)
            {
                receptor->goals.setGoals(*goals, 1.0);
            }
        }
        else
        {
            receptor = (Mona::Receptor *)*neuronItr;
            sprintf(description, "%s,%d", receptor->id.description, i);
            receptor->id.setDescription(description);
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


// Construct maze from file.
MazeBridgeDriver::MazeBridgeDriver(char *loadFile)
{
    load(loadFile);
}


// Destructor.
MazeBridgeDriver::~MazeBridgeDriver()
{
    clearMaze();
}


// Run: return number of successful trials.
int MazeBridgeDriver::run(int trials)
{
    int trial,maxCycles,goalCount,trialsFindingGoals;
    int cycle,deadRoomCount,i;
    Mona::NEED need;
    MazeRoom room;
    int door;
    int response;

    // Initialize training state.
    for (i = 0; i <= maxResponse; i++)
    {
        mona->responseInhibitors[i] = false;
        mona->responseOverridePotentials[i] = 0.0;
    }

    // Limit mediators for efficiency.
    mona->MAX_MEDIATORS = 200;

    // Set maximum cycles.
    maxCycles = numRooms * 10;

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

        // Build maze.
        instanceSeeds[0] = randomizer->RAND();
        buildMaze(COMPLETE);

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
        dumpMaze();
        #endif

        for (cycle = goalCount = deadRoomCount = 0;
            cycle < maxCycles || currentRoom == deadRoom; cycle++)
        {
            #if ( MONA_TRACE || MAZE_TRACE )
            printf("------------------------------\nCycle=%d\n",cycle);
            #endif
            // Get current room.
            getRoom(room);

            #if ( MONA_TRACE || MAZE_TRACE )
            room.print();
            #endif

            // Show room to mona and get response.
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
            response = mona->cycle(sensors);

            // Respond to room.
            door = response;
            #if ( MONA_TRACE || MAZE_TRACE )
            if (door >= 0 && door < numDoors)
            {
                printf("Response: Door %d\n", door);
            }
            else
            {
                printf("Response: Wait\n");
            }
            #endif
            chooseDoor(door);

            // Ensure dead room exposures to extinguish artifacts.
            if (currentRoom == deadRoom)
            {
                deadRoomCount++;
                if (deadRoomCount > MAX_DEAD_ROOM_EXPOSURES) break;
            }

            // Stop trial when goal reached.
            if (room.goals[0]) break;
        }
        clearMaze();

        #if ( MONA_TRACE || MAZE_TRACE )
        printf("Trial=%d: %d goals found\n", trial, goalCount);
        #endif
        if (goalCount > 0) trialsFindingGoals++;
    }

    #if ( MONA_TRACE || MAZE_TRACE )
    printf("==============================\n");
    #endif

    #if ( MONA_TRACE || MAZE_TRACE )
    // Print bridge mediators.
    printBridges();
    #endif

    // Return results.
    return trialsFindingGoals;
}


// Train.
int MazeBridgeDriver::train()
{
    int trials,trial,maxCycles,goalCount,trialsFindingGoals;
    int cycle,deadRoomCount,i;
    int trainTypeCounts[NUM_MAZE_TYPES],trainTypeIndex;
    int override;
    double potential;
    Mona::NEED need;
    MazeRoom room;
    int door;
    int response;

    // Initialize training state.
    potential = INITIAL_RESPONSE_OVERRIDE_POTENTIAL;
    for (i = 0; i <= maxResponse; i++)
    {
        mona->responseInhibitors[i] = false;
        mona->responseOverridePotentials[i] = 0.0;
    }

    // Limit mediators for efficiency.
    mona->MAX_MEDIATORS = 200;

    // Set maximum cycles.
    maxCycles = numRooms * 2;

    // Train for end and complete path.
    trainTypeIndex = -1;
    for (i = 0; i < NUM_MAZE_TYPES; i++)
    {
        trainTypeCounts[i] = TRAIN_TYPE_FREQS[i];
        if (trainTypeIndex == -1 && trainTypeCounts[i] > 0)
        {
            trainTypeIndex = i;
        }
    }
    assert(trainTypeIndex != -1);

    #if ( MONA_TRACE || MAZE_TRACE )
    printf("Begin training:\n");
    #endif
    trialsFindingGoals = 0;

    trials = 200;
    for (trial = 0; trial < trials; trial++)
    {
        // Cycle through the training regimen.
        while (trainTypeCounts[trainTypeIndex] == 0 &&
            trainTypeIndex < NUM_MAZE_TYPES)
        {
            trainTypeIndex++;
        }
        if (trainTypeIndex == NUM_MAZE_TYPES)
        {
            trainTypeIndex = -1;
            for (i = 0; i < NUM_MAZE_TYPES; i++)
            {
                trainTypeCounts[i] = TRAIN_TYPE_FREQS[i];
                if (trainTypeIndex == -1 && trainTypeCounts[i] > 0)
                {
                    trainTypeIndex = i;
                }
            }
        }
        trainTypeCounts[trainTypeIndex]--;
        instanceSeeds[0] = randomizer->RAND();
        if ((trial / 10) % 2)
        {
            buildMaze(ENDS);
        }
        else
        {
            buildMaze(MIDDLE);
        }

        #if ( MONA_TRACE || MAZE_TRACE )
        printf("==============================\nTrial=%d, training=",trial);
        switch(mazeType)
        {
            case COMPLETE: printf("complete maze"); break;
            case ENDS: printf("maze ends"); break;
            case MIDDLE: printf("middle maze"); break;
            case MIDDLE_EXTINCTION: printf("middle extinction"); break;
            case END_EXTINCTION: printf("end extinction"); break;
        }
        printf("\n");
        #endif

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
        dumpMaze();
        #endif

        for (cycle = goalCount = deadRoomCount = 0;
            cycle < maxCycles || currentRoom == deadRoom; cycle++)
        {
            #if ( MONA_TRACE || MAZE_TRACE )
            printf("------------------------------\nCycle=%d\n",cycle);
            #endif
            // Get current room.
            getRoom(room);

            #if ( MONA_TRACE || MAZE_TRACE )
            room.print();
            #endif

            // Show room to mona.
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

            // Reached end of maze?
            if (mazeType == MIDDLE && room.mark == END_MAZE_MARK)
            {
                goalCount++;
            }

            // If response training, tend to take door toward most needed goal.
            override = getDoor();
            for (i = 0; i <= maxResponse; i++)
            {
                mona->responseOverridePotentials[i] = 0.0;
            }
            if (override != -1 && mazeType != MIDDLE_EXTINCTION &&
                mazeType != END_EXTINCTION && mazeType != COMPLETE)
            {
                mona->responseOverridePotentials[override] = potential;
            }
            potential -= RESPONSE_OVERRIDE_DECREMENT;
            if (potential < 0.0) potential = 0.0;

            // Get response.
            response = mona->cycle(sensors);

            // Respond to room.
            door = response;
            #if ( MONA_TRACE || MAZE_TRACE )
            if (door >= 0 && door < numDoors)
            {
                printf("Response: Door %d\n", door);
            }
            else
            {
                printf("Response: Wait\n");
            }
            #endif
            chooseDoor(door);

            // Ensure dead room exposures to extinquish artifacts.
            if (currentRoom == deadRoom)
            {
                deadRoomCount++;
                if (deadRoomCount > MAX_DEAD_ROOM_EXPOSURES) break;
            }

            if (mazeType == MIDDLE)
            {
                // Stop trial when end of maze reached.
                if (goalCount > 0) break;
            }
            else
            {
                // Stop trial when goal reached.
                if (room.goals[0]) break;
            }
        }
        clearMaze();

        #if ( MONA_TRACE || MAZE_TRACE )
        printf("Trial=%d: %d goals found\n", trial, goalCount);
        #endif
        if (goalCount > 0) trialsFindingGoals++;
    }

    #if ( MONA_TRACE || MAZE_TRACE )
    printf("==============================\n");
    #endif

    #if ( MONA_TRACE || MAZE_TRACE )
    // Print bridge mediators.
    printBridges();
    #endif

    // Run to stablize training.
    return run(100);
}


// Build the bridge maze.
void MazeBridgeDriver::buildMaze(MAZE_TYPE mazeType, int contextDoor)
{
    int i,j,openDoor;
    MazeRoom *room;

    // Record maze type.
    this->mazeType = mazeType;

    // Get a maze.
    clearMaze();
    bridgeMaze.resize(numRooms);
    for (i = 0; i < numRooms; i++)
    {
        bridgeMaze[i] = new MazeState(i, numDoors);
        assert(bridgeMaze[i] != NULL);
        room = bridgeMaze[i]->room;
        room->doors.resize(numDoors);
        room->goals.resize(1);
        if (i == 0)
        {
            for (j = 0; j < numDoors; j++)
            {
                room->doors[j] = false;
            }
            room->goals[0] = false;
        } else if (i < numRooms - 2)
        {
            for (j = 0; j < numDoors; j++)
            {
                room->doors[j] = true;
            }
            room->goals[0] = false;
        }
        else
        {
            for (j = 0; j < numDoors; j++)
            {
                room->doors[j] = false;
            }
            if (i == numRooms - 2)
            {
                room->goals[0] = true;
            }
            else
            {
                room->goals[0] = false;
            }
        }
    }

    // Determine the initial open door.
    if (contextDoor == -1)
    {
        randomizer->RAND_PUSH();
        randomizer->SRAND(instanceSeeds[0]);
        openDoor = randomizer->RAND_CHOICE(numDoors);
        randomizer->RAND_POP();
    }
    else
    {
        openDoor = contextDoor;
    }

    // Establish meta-maze random state.
    randomizer->RAND_PUSH();
    randomizer->SRAND(metaSeed);

    // Configure the maze.
    room = bridgeMaze[0]->room;
    room->mark = START_ROOM_MARK;
    room->doors[openDoor] = true;
    startRoom = 0;
    bridgeMaze[0]->validDoor = openDoor;
    room = bridgeMaze[1]->room;
    room->mark = BEGIN_MAZE_MARK;
    startMaze = 1;
    bridgeMaze[1]->validDoor = randomizer->RAND_CHOICE(numDoors);
    for (i = 2, j = numRooms - 3; i < j; i++)
    {
        room = bridgeMaze[i]->room;
        room->mark = MAZE_ROOM_MARK;
        if (markPath) room->mark += (i - 2);
        bridgeMaze[i]->validDoor = randomizer->RAND_CHOICE(numDoors);
    }
    i = numRooms - 3;
    room = bridgeMaze[i]->room;
    room->mark = END_MAZE_MARK;
    endMaze = i;
    bridgeMaze[i]->validDoor = openDoor;
    i++;
    room = bridgeMaze[i]->room;
    room->mark = GOAL_ROOM_MARK;
    goalRoom = i;
    bridgeMaze[i]->validDoor = -1;
    i++;
    room = bridgeMaze[i]->room;
    room->mark = DEAD_ROOM_MARK;
    deadRoom = i;
    bridgeMaze[i]->validDoor = -1;

    switch(mazeType)
    {
        case COMPLETE:
        case ENDS:
            currentRoom = startRoom;
            break;
        case MIDDLE:
            currentRoom = startMaze;
            break;
        case MIDDLE_EXTINCTION:
            assert(numRooms > 5);
            currentRoom = startMaze + 1;
            mazeMoveCount = 0;
            mazeErrorMove = randomizer->RAND_CHOICE(numRooms - 5);
            break;
        case END_EXTINCTION:
            currentRoom = endMaze;
            break;
    }

    // Restore random state.
    randomizer->RAND_POP();
}


// Dump maze.
void MazeBridgeDriver::dumpMaze()
{
    int i;
    MazeRoom room;

    printf("Maze Dump:\n");
    for (i = 0; i < numRooms; i++)
    {
        // Get current room.
        getRoom(room, i);
        printf("Valid door = %d, ", bridgeMaze[i]->validDoor);
        room.print();
    }
}


// Get current room.
void MazeBridgeDriver::getRoom(MazeRoom &roomOut, int index)
{
    int i,j;
    MazeRoom *room;

    roomOut.doors.clear();
    roomOut.goals.clear();
    if (index == -1) index = currentRoom;
    if (index < 0 || index >= numRooms) return;
    room = bridgeMaze[index]->room;
    roomOut.id = room->id;
    roomOut.mark = room->mark;

    // Copy doors.
    roomOut.doors.resize(room->doors.size());
    for (i = 0, j = room->doors.size(); i < j; i++)
    {
        roomOut.doors[i] = room->doors[i];
    }

    // Copy goals.
    roomOut.goals.resize(room->goals.size());
    for (i = 0, j = room->goals.size(); i < j; i++)
    {
        roomOut.goals[i] = room->goals[i];
    }
}


// Door chosen.
void MazeBridgeDriver::chooseDoor(int door)
{
    if (currentRoom == -1) return;

    // Special case: jump across maze if training maze ends.
    if (mazeType == ENDS && currentRoom == startMaze)
    {
        currentRoom = endMaze;
        return;
    }

    if (bridgeMaze[currentRoom]->validDoor != -1 &&
        bridgeMaze[currentRoom]->validDoor == door)
    {
        switch(mazeType)
        {
            case COMPLETE:
            case ENDS:
            case END_EXTINCTION:
                currentRoom++;
                break;
            case MIDDLE:
                if (currentRoom == endMaze)
                {
                    currentRoom = deadRoom;
                }
                else
                {
                    currentRoom++;
                }
                break;
            case MIDDLE_EXTINCTION:
                mazeMoveCount = 0;
                mazeErrorMove = randomizer->RAND_CHOICE(numRooms - 5);
                if (mazeMoveCount == mazeErrorMove)
                {
                    currentRoom = deadRoom;
                }
                else
                {
                    mazeMoveCount++;
                    currentRoom++;
                }
                break;
        }
        return;
    }

    // If wrong door chosen in room other than room 0,
    // banish to dead-end room.
    if (currentRoom != startRoom)
    {
        currentRoom = deadRoom;
    }
}


// Get door to goal.
int MazeBridgeDriver::getDoor()
{
    if (currentRoom == -1 ||
        bridgeMaze[currentRoom]->validDoor == -1)
    {
        return -1;
    }
    else
    {
        return bridgeMaze[currentRoom]->validDoor;
    }
}


// Clear maze.
void MazeBridgeDriver::clearMaze()
{
    int i,j;

    for (i = 0, j = bridgeMaze.size(); i < j; i++)
    {
        delete bridgeMaze[i];
    }
    bridgeMaze.clear();
    currentRoom = -1;
}


// Print bridge mediators.
// A bridge spans the initial to the goal room.
void MazeBridgeDriver::printBridges()
{
    int i,n;
    Mona::Mediator *mediator,*mediator2;
    Mona::Receptor *receptor;
    Mona::Motor *motor;
    list<Mona::Neuron *>::iterator neuronItr;

    printf("Bridges detected:\n");
    printf("End-to-end bridges:\n");
    for (i = 0; i < numDoors; i++)
    {
        for (neuronItr = mona->mediators.begin();
            neuronItr != mona->mediators.end(); neuronItr++)
        {
            mediator = (Mona::Mediator *)*neuronItr;
            if (mediator->level != 0) continue;
            if (mediator->causes.size() != 1) continue;
            n = (2 * (numRooms - 3)) + 1;
            if (mediator->intermediates.size() != n) continue;
            if (mediator->causes[0]->type != Mona::RECEPTOR) continue;
            receptor = (Mona::Receptor *)(mediator->causes[0]);
            if (receptor->sensorMask[numDoors + numGoals] !=
                START_ROOM_MARK) continue;
            if (mediator->effect->type != Mona::RECEPTOR) continue;
            receptor = (Mona::Receptor *)(mediator->effect);
            if (receptor->sensorMask[numDoors + numGoals] !=
                GOAL_ROOM_MARK) continue;
            if (mediator->intermediates[0]->type != Mona::MOTOR) continue;
            motor = (Mona::Motor *)(mediator->intermediates[0]);
            if (motor->response != i) continue;
            if (mediator->intermediates[n-1]->type != Mona::MOTOR) continue;
            motor = (Mona::Motor *)(mediator->intermediates[n-1]);
            if (motor->response != i) continue;
            printf("Bridge %d:\n", i); mediator->print();
            break;
        }
    }

    printf("Maze-spanning bridge:\n");
    for (neuronItr = mona->mediators.begin();
        neuronItr != mona->mediators.end(); neuronItr++)
    {
        mediator = (Mona::Mediator *)*neuronItr;
        if (mediator->level != 0) continue;
        if (mediator->causes.size() != 1) continue;
        n = (2 * (numRooms - 5)) + 1;
        if (mediator->intermediates.size() != n) continue;
        if (mediator->causes[0]->type != Mona::RECEPTOR) continue;
        receptor = (Mona::Receptor *)(mediator->causes[0]);
        if (receptor->sensorMask[numDoors + numGoals] !=
            BEGIN_MAZE_MARK) continue;
        if (mediator->effect->type != Mona::RECEPTOR) continue;
        receptor = (Mona::Receptor *)(mediator->effect);
        if (receptor->sensorMask[numDoors + numGoals] !=
            END_MAZE_MARK) continue;
        mediator->print();
        break;
    }

    printf("End-point bridges:\n");
    for (i = 0; i < numDoors; i++)
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
            printf("Bridge %d:\n", i); mediator->print();
            break;
        }
    }
}


// Create SNNS neural network pattern files.
// These are named:
// <snns file prefix>_<doors per room>
//   /ends/<context door>.pat
//   /middle/<bridge length>].pat
//   /complete/<bridge length>_<context door>.pat
void MazeBridgeDriver::createSNNSpatterns(char *snnsPrefix)
{
    int contextDoor;
    char patterndir[1024];
    #ifdef WIN32
    char basedir[1024];
    #endif

    if (snnsPrefix == NULL) snnsPrefix = "snns";

    // Create pattern directory.
    sprintf(patterndir, "%s_%d", snnsPrefix, numDoors);
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

    // Generate maze "ends" and "complete" patterns.
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

    // Create maze "ends" patterns.
    #ifdef WIN32
    if (SetCurrentDirectory("ends") == 0)
    #else
        if (chdir("ends") == -1)
    #endif
    {
        fprintf(stderr, "Cannot change to pattern \"ends\" sub-directory\n");
        exit(1);
    }

    for (contextDoor = 0; contextDoor < numDoors; contextDoor++)
    {
        // Build maze.
        buildMaze(ENDS, contextDoor);

        // Generate pattern.
        createSNNSpatternFile(ENDS);
    }

    // Create "complete" maze patterns.
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

    for (contextDoor = 0; contextDoor < numDoors; contextDoor++)
    {
        // Build maze.
        buildMaze(COMPLETE, contextDoor);

        // Generate pattern.
        createSNNSpatternFile(COMPLETE);
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

    // Generate "middle" maze pattern.
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

    // Build maze.
    buildMaze(MIDDLE);

    // Generate pattern.
    createSNNSpatternFile(MIDDLE);

    #ifdef WIN32
    if (SetCurrentDirectory(basedir) == 0)
    #else
        if (chdir("../..") == -1)
    #endif
    {
        fprintf(stderr, "Cannot change to base directory\n");
        exit(1);
    }
}


// Create an SNNS pattern file.
void MazeBridgeDriver::createSNNSpatternFile(MAZE_TYPE mazeConfig)
{
    FILE *fp;
    time_t clock;
    int i,j,k,c;
    MazeRoom *room;
    char buf[100];

    // Validity check.
    if ((numDoors + 3) > NN_NUM_INPUT)
    {
        fprintf(stderr, "Maximum network inputs (%d) exceeded (%d)\n", NN_NUM_INPUT, numDoors + 3);
        exit(1);
    }
    if (numDoors > NN_NUM_OUTPUT)
    {
        fprintf(stderr, "Maximum network outputs (%d) exceeded (%d)\n", NN_NUM_OUTPUT, numDoors);
        exit(1);
    }

    // Open pattern file.
    for (i = 0; i < numDoors; i++)
    {
        if (i == bridgeMaze[0]->validDoor) break;
    }
    switch(mazeConfig)
    {
        case COMPLETE:
            sprintf(buf, "%d_%d.pat", numRooms - 5, i);
            break;
        case ENDS:
            sprintf(buf, "%d.pat", i);
            break;
        case MIDDLE:
            sprintf(buf, "%d.pat", numRooms - 5);
            break;
    }
    if ((fp = fopen(buf, "w")) == NULL)
    {
        fprintf(stderr, "Cannot open %s for writing\n", buf);
        exit(1);
    }

    fprintf(fp, "SNNS pattern definition file V3.2\n");
    clock = time(0);
    fprintf(fp, "generated at %s\n\n", ctime(&clock));
    switch(mazeConfig)
    {
        case COMPLETE:
            fprintf(fp, "No. of patterns : %d\n", numRooms - 2);
            break;
        case ENDS:
            fprintf(fp, "No. of patterns : 2\n");
            break;
        case MIDDLE:
            fprintf(fp, "No. of patterns : %d\n", numRooms - 4);
            break;
    }
    fprintf(fp, "No. of input units : %d\n", NN_NUM_INPUT);
    fprintf(fp, "No. of output units : %d\n\n", NN_NUM_OUTPUT);

    c = 1;
    if (mazeConfig != MIDDLE)
    {
        fprintf(fp, "# Input pattern %d:\n", c);
        for (i = 0; i < numDoors; i++)
        {
            if (i == bridgeMaze[0]->validDoor)
            {
                fprintf(fp, "1 ");
            } else
            {
                fprintf(fp, "0 ");
            }
        }
        for (; i < NN_NUM_INPUT; i++) fprintf(fp, "0 ");
        fprintf(fp, "\n");
        fprintf(fp, "# Output pattern %d:\n", c); c++;
        for (i = 0; i < numDoors; i++)
        {
            if (i == bridgeMaze[0]->validDoor)
            {
                fprintf(fp, "1 ");
            } else
            {
                fprintf(fp, "0 ");
            }
        }
        for (; i < NN_NUM_OUTPUT; i++)
        {
            fprintf(fp, "0 ");
        }
        fprintf(fp, "\n");
    }
    switch(mazeConfig)
    {
        case COMPLETE:
        case ENDS:
            k = numRooms - 2;
            break;
        case MIDDLE:
            k = numRooms - 3;
            break;
    }
    for (i = 1; i < k; i++)
    {
        if (mazeConfig == ENDS && i < numRooms - 3) continue;
        fprintf(fp, "# Input pattern %d:\n", c);
        room = bridgeMaze[i]->room;
        for (j = 0; j < numDoors; j++) fprintf(fp, "1 ");
        for (; j < NN_NUM_INPUT - 3; j++) fprintf(fp, "0 ");
        switch(room->mark)
        {
            case 0: fprintf(fp, "0 0 0\n"); break;
            case 1: fprintf(fp, "0 0 1\n"); break;
            case 2: fprintf(fp, "0 1 0\n"); break;
            case 3: fprintf(fp, "0 1 1\n"); break;
            case 4: fprintf(fp, "1 0 0\n"); break;
            case 5: fprintf(fp, "1 0 1\n"); break;
        }
        fprintf(fp, "# Output pattern %d:\n", c); c++;
        for (j = 0; j < numDoors; j++)
        {
            if (room->mark == END_MAZE_MARK)
            {
                if (j == bridgeMaze[i]->validDoor)
                {
                    fprintf(fp, "1 ");
                }
                else
                {
                    fprintf(fp, "0 ");
                }
            }
            else
            {
                if (j == bridgeMaze[i]->validDoor)
                {
                    fprintf(fp, "1 ");
                }
                else
                {
                    fprintf(fp, "0 ");
                }
            }
        }
        for (; j < NN_NUM_OUTPUT; j++) fprintf(fp, "0 ");
        fprintf(fp, "\n");
    }
    fclose(fp);
}
