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

#include "maze.hpp"
#include <math.h>
#include <assert.h>

// Version.
void
Maze::version()
{
    const char *mazeVersion = MAZE_VERSION;
    printf("%s\n", &mazeVersion[5]);
}


// Maximum context creation tries.
const int Maze::MAX_CREATE_TRIES = 100;

// Constructors.
Maze::Maze(int numRooms, int numDoors, int numGoals)
{
    // Clear component id dispenser.
    idDispenser = 0;

    // Save dimensions.
    this->numRooms = numRooms;
    this->numDoors = numDoors;
    this->numGoals = numGoals;

    instanceSeed = 0;
}


Maze::Maze(int numRooms, int numDoors, int numGoals,
vector<int> &contextSizes, int effectDelayScale,
RANDOM metaSeed, RANDOM instanceSeed,
bool twoway)
{
    int i,j,k,n,p,q,d;
    MazeRoom *room;
    MazeContext *context,*context2,*context3;
    vector<MazeComponent *> components;

    // Clear component id dispenser.
    idDispenser = 0;

    // Save dimensions.
    this->numRooms = numRooms;
    this->numDoors = numDoors;
    this->numGoals = numGoals;

    // Establish meta-maze random state.
    randomizer->RAND_PUSH();
    randomizer->SRAND(metaSeed);

    // Save instance state.
    this->instanceSeed = instanceSeed;

    // Create rooms.
    if (numRooms == 0)
    {
        currentRoom = -1;
        return;
    }
    for (i = 0; i < numRooms; i++)
    {
        room = new MazeRoom(idDispenser);
        idDispenser++;
        assert(room != NULL);
        room->instanceSeed = instanceSeed;
        rooms.push_back(room);
        room->doors.resize(numDoors);
        for (j = 0; j < numDoors; j++)
        {
            room->doors[j] = false;
        }
        room->goals.resize(numGoals);
        for (j = 0; j < numGoals; j++)
        {
            room->goals[j] = false;
        }
    }
    for (i = 0; i < numGoals; i++)
    {
        room = rooms[randomizer->RAND_CHOICE(numRooms)];
        room->goals[i] = true;
    }
    currentRoom = 0;

    // Create contexts.
    for (i = 0, j = contextSizes.size(); i < j; i++)
    {
        contexts.push_back(vector<MazeContext *>());
        for (k = 0; k < contextSizes[i]; k++)
        {
            for (n = 0; n < MAX_CREATE_TRIES; n++)
            {
                context = new MazeContext(idDispenser++);
                assert(context != NULL);
                if (i == 0)
                {
                    context->cause = rooms[randomizer->RAND_CHOICE(rooms.size())];
                    context->effect = rooms[randomizer->RAND_CHOICE(rooms.size())];
                    if (context->cause == context->effect)
                    {
                        delete context;
                        continue;
                    }
                    room = (MazeRoom *)context->cause;
                    p = randomizer->RAND_CHOICE(numDoors);
                    for (q = 0; q < numDoors; q++)
                    {
                        if (!room->doors[p])
                        {
                            context->door = d = p;
                            room->doors[p] = true;
                            break;
                        }
                        p = (p + 1) % numDoors;
                    }
                    if (q == numDoors)
                    {
                        delete context;
                        continue;
                    }
                    if (twoway)
                    {
                        context3 = new MazeContext(idDispenser++);
                        assert(context3 != NULL);
                        context3->cause = context->effect;
                        context3->effect = context->cause;
                        room = (MazeRoom *)context3->cause;
                        p = randomizer->RAND_CHOICE(numDoors);
                        for (q = 0; q < numDoors; q++)
                        {
                            if (!room->doors[p])
                            {
                                context3->door = p;
                                room->doors[p] = true;
                                break;
                            }
                            p = (p + 1) % numDoors;
                        }
                        if (q == numDoors)
                        {
                            room = (MazeRoom *)context->cause;
                            room->doors[d] = false;
                            delete context;
                            delete context3;
                            continue;
                        }
                    }
                }
                else
                {
                    if (contexts[i-1].size() == 0|| components.size() == 0) break;
                    if (randomizer->RAND_BOOL())
                    {
                        context->cause = contexts[i-1][randomizer->RAND_CHOICE(contexts[i-1].size())];
                        context->effect = components[randomizer->RAND_CHOICE(components.size())];
                    }
                    else
                    {
                        context->effect = contexts[i-1][randomizer->RAND_CHOICE(contexts[i-1].size())];
                        context->cause = components[randomizer->RAND_CHOICE(components.size())];
                    }
                    if (context->cause == context->effect)
                    {
                        delete context;
                        continue;
                    }
                    for (p = 0, q = contexts[i].size(); p < q; p++)
                    {
                        context2 = contexts[i][p];
                        if (context2->cause == context->cause &&
                            context2->effect == context->effect)
                        {
                            break;
                        }
                    }
                    if (p < q)
                    {
                        delete context;
                        continue;
                    }
                }
                context->cause->causeContexts.push_back(context);
                context->effectDelay = randomizer->RAND_CHOICE((i+1)*effectDelayScale) + 1;
                contexts[i].push_back(context);
                if (twoway)
                {
                    context3->cause->causeContexts.push_back(context3);
                    context3->effectDelay = randomizer->RAND_CHOICE((i+1)*effectDelayScale) + 1;
                    contexts[i].push_back(context3);
                }
                break;
            }
        }
        for (p = 0, q = contexts[i].size(); p < q; p++)
        {
            context = contexts[i][p];
            components.push_back((MazeComponent *)context);
        }
    }

    // Restore random state.
    randomizer->RAND_POP();
}


// Destructor.
Maze::~Maze()
{
    int i,j,p,q;
    MazeRoom *room;
    MazeContext *context;

    // Delete maze components.
    for (i = 0, j = rooms.size(); i < j; i++)
    {
        room = rooms[i];
        delete room;
    }
    rooms.clear();

    // Delete maze contexts.
    for (i = 0, j = contexts.size(); i < j; i++)
    {
        for (p = 0, q = contexts[i].size(); p < q; p++)
        {
            context = contexts[i][p];
            delete context;
        }
        contexts[i].clear();
    }
}


// Get current room.
void Maze::getRoom(MazeRoom &roomOut)
{
    int i,j;
    MazeRoom *room;
    ContextSignature *signature;

    roomOut.doors.clear();
    roomOut.goals.clear();
    if (currentRoom == -1) return;
    room = rooms[currentRoom];
    roomOut.id = room->id;
    roomOut.mark = room->mark;
    roomOut.instanceSeed = room->instanceSeed;

    // Copy doors.
    roomOut.doors.resize(room->doors.size());
    for (i = 0, j = room->doors.size(); i < j; i++)
    {
        roomOut.doors[i] = room->doors[i];
    }

    // Have goals in room been consumed in previous visit?
    roomOut.goals.resize(room->goals.size());
    for (i = 0, j = room->goals.size(); i < j; i++)
    {
        roomOut.goals[i] = false;
    }
    signature = room->getSignature();
    if (signature->visited) return;
    signature->visited = true;
    for (i = 0, j = room->goals.size(); i < j; i++)
    {
        roomOut.goals[i] = room->goals[i];
    }
}


// Choose door.
// Return probability of next room access.
bool Maze::chooseDoor(int door, bool force)
{
    ContextSignature *signature;

    return chooseDoor(door, &signature, force);
}


// Choose door.
// Return probability of next room access.
bool Maze::chooseDoor(int door,
ContextSignature **signature, bool force)
{
    int i,j;
    MazeRoom *room;
    MazeContext *context;
    vector<MazeContext *> work;

    // Validate door.
    *signature = NULL;
    if (currentRoom == -1) return false;
    room = rooms[currentRoom];
    if (!room->doors[door]) return false;

    // Find applicable cause context.
    for (i = 0, j = room->causeContexts.size(); i < j; i++)
    {
        context = room->causeContexts[i];
        if (door == context->door) break;
    }
    if (i == j) return false;

    // Fire cause event.
    context->fireCause();

    // Check access probability.
    room = (MazeRoom *)context->effect;
    *signature = room->getSignature();
    if ((*signature)->success || force)
    {
        // Move to new room.
        for (i = 0, j = rooms.size(); i < j; i++)
        {
            if (room == rooms[i]) break;
        }
        currentRoom = i;

        // Advance "clock": expire timed-out contexts.
        expireContexts();

        // Fire effect events.
        for (i = 0, j = room->contexts.size(); i < j; i++)
        {
            context = room->contexts[i];
            context->effectTimer = -1;
            work.push_back(context);
        }
        room->contexts.clear();
        for (i = 0, j = work.size(); i < j; i++)
        {
            context = work[i];
            context->fireEffect();
        }
        work.clear();

        // Accessed room.
        return true;

    }
    else
    {

        // Clear contexts.
        for (i = 0, j = room->contexts.size(); i < j; i++)
        {
            context = room->contexts[i];
            context->effectTimer = -1;
        }
        room->contexts.clear();

        // Failed to access room.
        return false;
    }
}


// Expire timed-out contexts.
void Maze::expireContexts()
{
    int i,j,p,q,x,y;
    MazeRoom *room;
    MazeContext *context,*context2;
    vector<MazeContext *> work;

    for (i = 0, j = rooms.size(); i < j; i++)
    {
        room = rooms[i];
        work.clear();
        for (p = 0, q = room->contexts.size(); p < q; p++)
        {
            context = room->contexts[p];
            context->effectTimer--;
            if (context->effectTimer >= 0)
            {
                work.push_back(context);
            }
        }
        room->contexts.clear();
        room->contexts.resize(work.size());
        copy(work.begin(), work.end(),
            room->contexts.begin());
    }

    for (i = 0, j = contexts.size(); i < j; i++)
    {
        for (p = 0, q = contexts[i].size(); p < q; p++)
        {
            context = contexts[i][p];
            work.clear();
            for (x = 0, y = context->contexts.size(); x < y; x++)
            {
                context2 = context->contexts[x];
                context2->effectTimer--;
                if (context2->effectTimer > 0)
                {
                    work.push_back(context2);
                }
            }
            context->contexts.clear();
            context->contexts.resize(work.size());
            copy(work.begin(), work.end(),
                context->contexts.begin());
        }
    }
}


// Clone maze.
Maze *Maze::clone()
{
    return clone(instanceSeed);
}


Maze *Maze::clone(RANDOM newInstanceSeed)
{
    int i,j,p,q;
    MazeRoom *room;
    MazeContext *context;
    Maze *maze;

    maze = new Maze(numRooms, numDoors, numGoals);
    assert(maze != NULL);
    maze->instanceSeed = newInstanceSeed;

    // Clone components.
    for (i = 0, j = rooms.size(); i < j; i++)
    {
        room = rooms[i];
        maze->rooms.push_back(room->clone(newInstanceSeed));
    }
    maze->currentRoom = currentRoom;
    maze->contexts.resize(contexts.size());
    for (i = 0, j = contexts.size(); i < j; i++)
    {
        for (p = 0, q = contexts[i].size(); p < q; p++)
        {
            context = contexts[i][p];
            maze->contexts[i].push_back(context->clone());
        }
    }

    // Resolve cloned references.
    for (i = 0, j = rooms.size(); i < j; i++)
    {
        room = rooms[i];
        resolveClone(room, maze);
    }
    for (i = 0, j = contexts.size(); i < j; i++)
    {
        for (p = 0, q = contexts[i].size(); p < q; p++)
        {
            context = contexts[i][p];
            resolveClone(context, maze);
        }
    }

    return maze;
}


// Find component by id in given maze.
MazeComponent *Maze::findByID(int id)
{
    int i,j,p,q;
    MazeRoom *room;
    MazeContext *context;

    for (i = 0, j = rooms.size(); i < j; i++)
    {
        room = rooms[i];
        if (room->id == id) return (MazeComponent *)room;
    }
    for (i = 0, j = contexts.size(); i < j; i++)
    {
        for (p = 0, q = contexts[i].size(); p < q; p++)
        {
            context = contexts[i][p];
            if (context->id == id) return (MazeComponent *)context;
        }
    }
    return NULL;
}


// Resolve cloned references.
void Maze::resolveClone(MazeComponent *baseComponent, Maze *maze)
{
    int i,j;
    MazeComponent *component;
    MazeContext *context,*baseContext;

    if ((component = maze->findByID(baseComponent->id)) == NULL) return;

    for (i = 0, j = baseComponent->causeContexts.size(); i < j; i++)
    {
        if ((context = (MazeContext *)maze->findByID(baseComponent->causeContexts[i]->id)) != NULL)
        {
            component->causeContexts.push_back(context);
        }
    }
    for (i = 0, j = baseComponent->contexts.size(); i < j; i++)
    {
        if ((context = (MazeContext *)maze->findByID(baseComponent->contexts[i]->id)) != NULL)
        {
            component->contexts.push_back(context);
        }
    }
    if (baseComponent->type == MazeComponent::CONTEXT)
    {
        baseContext = (MazeContext *)baseComponent;
        context = (MazeContext *)component;
        context->cause = maze->findByID(baseContext->cause->id);
        context->effect = maze->findByID(baseContext->effect->id);
    }
}


// Print maze.
void Maze::print(FILE *fp)
{
    int i,j,p,q;
    MazeRoom *room;
    MazeContext *context;

    // Print rooms.
    fprintf(fp, "Rooms:\n");
    for (i = 0, j = rooms.size(); i < j; i++)
    {
        room = rooms[i];
        room->print();
    }
    fprintf(fp, "Current room id=%d\n", rooms[currentRoom]->id);

    fprintf(fp, "Contexts:\n");
    for (i = 0, j = contexts.size(); i < j; i++)
    {
        fprintf(fp, "Level=%d:\n", i);
        for (p = 0, q = contexts[i].size(); p < q; p++)
        {
            context = contexts[i][p];
            context->print();
        }
    }
}


// Dump maze.
void Maze::dump(FILE *fp)
{
    int i,j,p,q;
    MazeRoom *room;
    MazeContext *context;

    // Print maze rooms.
    fprintf(fp, "Rooms:\n");
    for (i = 0, j = rooms.size(); i < j; i++)
    {
        room = rooms[i];
        room->print();
    }

    // Print maze contexts.
    fprintf(fp, "Contexts:\n");
    for (i = 0, j = contexts.size(); i < j; i++)
    {
        fprintf(fp, "Level %d:\n", i);
        for (p = 0, q = contexts[i].size(); p < q; p++)
        {
            context = contexts[i][p];
            context->print(fp);
        }
    }
}


// Clone connecting room.
RoomDescriptor::ConnectingRoom *
RoomDescriptor::ConnectingRoom::clone()
{
    ConnectingRoom *cr;
    cr = new ConnectingRoom();
    assert(cr != NULL);
    cr->roomIndex = roomIndex;
    cr->door = door;
    cr->roomDesc = roomDesc;
    cr->signature = signature->clone();
    cr->resolved = resolved;
    cr->visited = visited;
    cr->validInstanceCount = validInstanceCount;
    return cr;
}


// Room descriptor constructor.
RoomDescriptor::RoomDescriptor(int id, Maze *maze)
{
    this->id = id;
    this->maze = maze;
    backMaze = NULL;
    searchLength = -1;
}


// Room descriptor destructor.
RoomDescriptor::~RoomDescriptor()
{
    int i,j;
    ConnectingRoom *connectingRoom;

    delete maze;
    if (backMaze != NULL) delete backMaze;
    for (i = 0, j = connectingRooms.size(); i < j; i++)
    {
        connectingRoom = connectingRooms[i];
        delete connectingRoom;
    }
    connectingRooms.clear();
    for (i = 0, j = backConnectingRooms.size(); i < j; i++)
    {
        connectingRoom = backConnectingRooms[i];
        delete connectingRoom;
    }
    backConnectingRooms.clear();
}


// Backup connections.
void RoomDescriptor::backup()
{
    int i,j;
    ConnectingRoom *connectingRoom;

    backMaze = maze->clone();
    for (i = 0, j = connectingRooms.size(); i < j; i++)
    {
        connectingRoom = connectingRooms[i]->clone();
        backConnectingRooms.push_back(connectingRoom);
        connectingRooms[i]->validInstanceCount = -1;
    }
}


// Restore components.
void RoomDescriptor::restore()
{
    int i,j;
    ConnectingRoom *connectingRoom;

    delete maze;
    maze = backMaze;
    backMaze = NULL;
    for (i = 0, j = connectingRooms.size(); i < j; i++)
    {
        connectingRoom = connectingRooms[i];
        delete connectingRoom;
    }
    connectingRooms.clear();
    for (i = 0, j = backConnectingRooms.size(); i < j; i++)
    {
        connectingRoom = backConnectingRooms[i];
        connectingRooms.push_back(connectingRoom);
    }
    backConnectingRooms.clear();
}


// Given room in same context state?
bool RoomDescriptor::dejaVu(RoomDescriptor *roomDesc2)
{
    int i,j,p,q,r,s;
    Maze *maze2;
    MazeRoom *room,*room2;
    ContextSignature *signature,*signature2;

    // Rooms are in same state?
    maze2 = roomDesc2->maze;
    if (maze->currentRoom != maze2->currentRoom) return false;

    // Note: take this shortcut to avoid making sure both
    // mazes are in the identical context state - very expensive!
    else return true;

    for (i = 0, j = maze->rooms.size(); i < j; i++)
    {
        room = maze->rooms[i];
        room2 = maze2->rooms[i];
        q = room->contextSignatures.size();
        s = room2->contextSignatures.size();
        if (q != s) return false;
        for (p = 0; p < q; p++)
        {
            signature = room->contextSignatures[p];
            for (r = 0; r < s; r++)
            {
                signature2 = room2->contextSignatures[r];
                if (signature->match(signature2)) break;
            }
            if (r == s) return false;
        }
    }
    return true;
}


// Number of search passes to make.
const int MazeMap::SEARCH_PASSES = 100;

// Maze map constructor.
MazeMap::MazeMap(MAP_TYPE type)
{
    idDispenser = 0;
    this->type = type;
    mapSet = false;
    currentDescriptor = NULL;
    backCurrentDescriptor = NULL;
}


// Maze map destructor.
MazeMap::~MazeMap()
{
    clear();
}


// Clear maze map(s).
void MazeMap::clear()
{
    int i,j;

    if (mapSet)
    {
        for (i = 0, j = maps.size(); i < j; i++)
        {
            delete maps[i];
        }
        maps.clear();
    }
    else
    {

        idDispenser = 0;
        instanceSeeds.clear();
        instanceFrequencies.clear();
        for (i = 0, j = descriptors.size(); i < j; i++)
        {
            delete descriptors[i];
        }
        descriptors.clear();
        currentDescriptor = NULL;
        backCurrentDescriptor = NULL;
    }
}


// Generate and map a set of mazes.
void MazeMap::mapMaze(Maze *maze, int instanceIndex,
vector<RANDOM> &instanceSeeds, vector<double> &instanceFrequencies)
{
    int i,j;
    MazeMap *map;
    Maze *maze2;

    assert(type == META_MAP);
    assert(instanceSeeds.size() > 0 &&
        instanceSeeds.size() == instanceFrequencies.size());
    mapSet = true;
    this->instanceIndex = instanceIndex;
    for (i = 0, j = instanceSeeds.size(); i < j; i++)
    {
        this->instanceSeeds.push_back(instanceSeeds[i]);
        this->instanceFrequencies.push_back(instanceFrequencies[i]);
        map = new MazeMap(MazeMap::META_MAP);
        assert(map != NULL);
        maps.push_back(map);
        maze2 = maze->clone(instanceSeeds[i]);
        map->mapMaze(maze2);
        delete maze2;
        validMaps.push_back(true);
    }
}


// Generate and map a maze.
void MazeMap::mapMaze(Maze *maze)
{
    int i,j,p,q;
    Maze *maze2;
    RoomDescriptor *roomDesc;
    bool done;

    // Clear map.
    clear();

    // Save maze instance.
    instanceSeeds.push_back(maze->instanceSeed);
    instanceFrequencies.push_back(1.0);

    // Generate map.
    maze2 = maze->clone();
    if (maze2->currentRoom != -1)
    {
        if (type == ROOM_MAP)
        {
            done = false;
            while (!done)
            {
                done = true;
                mapMazeRooms(maze2);

                // All rooms mapped?
                for (i = 0, j = maze2->rooms.size(); i < j; i++)
                {
                    for (p = 0, q = descriptors.size(); p < q; p++)
                    {
                        roomDesc = descriptors[p];
                        if (roomDesc->maze->currentRoom == i) break;
                    }
                    if (p == q)
                    {
                        maze2 = maze->clone();
                        maze2->currentRoom = i;
                        done = false;
                        break;
                    }
                }
            }
        }
        else
        {
            roomDesc = new RoomDescriptor(idDispenser++, maze2);
            assert(roomDesc != NULL);
            descriptors.push_back(roomDesc);
            mapMazeContexts(roomDesc, 0);
        }
        currentDescriptor = descriptors[0];
    }
}


// Map maze: map type == ROOM_MAP.
void MazeMap::mapMazeRooms(Maze *maze)
{
    int i,j,p,q;
    RoomDescriptor *roomDesc;
    MazeRoom *room;
    Maze *maze2;
    ContextSignature *signature;
    RoomDescriptor::ConnectingRoom *connectingRoom;

    // Map visit.
    for (i = 0, j = descriptors.size(); i < j; i++)
    {
        roomDesc = descriptors[i];
        if (roomDesc->maze->currentRoom == maze->currentRoom)
        {
            delete maze;
            maze = roomDesc->maze;
            break;
        }
    }
    if (i == j)
    {
        // Create room descriptor.
        roomDesc = new RoomDescriptor(idDispenser++, maze);
        assert(roomDesc != NULL);
        descriptors.push_back(roomDesc);
    }

    // Visit accessible rooms.
    room = maze->rooms[maze->currentRoom];
    for (i = 0, j = room->doors.size(); i < j; i++)
    {
        if (room->doors[i])
        {
            // Can enter door?
            maze2 = maze->clone();
            if (maze2->chooseDoor(i, &signature, true))
            {
                for (p = 0, q = roomDesc->connectingRooms.size(); p < q; p++)
                {
                    connectingRoom = roomDesc->connectingRooms[p];
                    if (connectingRoom->roomIndex == maze2->currentRoom &&
                        connectingRoom->door == i) break;
                }
                if (p == q)
                {
                    // Map connecting room.
                    connectingRoom = new RoomDescriptor::ConnectingRoom();
                    assert(connectingRoom != NULL);
                    connectingRoom->roomIndex = maze2->currentRoom;
                    connectingRoom->door = i;
                    connectingRoom->signature = signature->clone();
                    roomDesc->connectingRooms.push_back(connectingRoom);
                    mapMazeRooms(maze2);
                }

            }
            else
            {
                delete maze2;
            }
        }
    }
}


// Maximum recursion level.
const int MazeMap::MAX_LEVEL = 50;

// Map maze: type == META_MAP or INSTANCE_MAP
void MazeMap::mapMazeContexts(RoomDescriptor *roomDesc, int level)
{
    int i,j,p,q;
    RoomDescriptor *roomDesc2;
    Maze *maze,*maze2;
    MazeRoom *room;
    ContextSignature *signature;
    RoomDescriptor::ConnectingRoom *connectingRoom;

    // Check recursion limit.
    maze = roomDesc->maze;
    if (level > maze->numRooms || level > MAX_LEVEL) return;

    // Visit accessible rooms.
    room = maze->rooms[maze->currentRoom];
    for (i = 0, j = room->doors.size(); i < j; i++)
    {
        // If door exists.
        if (room->doors[i])
        {
            // Has door been taken?
            for (p = 0, q = roomDesc->connectingRooms.size(); p < q; p++)
            {
                connectingRoom = roomDesc->connectingRooms[p];
                if (connectingRoom->door == i) break;
            }
            if (p < q) continue;

            // Can enter door?
            maze2 = maze->clone();
            if (maze2->chooseDoor(i, &signature,
                (bool)(type != INSTANCE_MAP)))
            {
                signature = signature->clone();

                // Create room descriptor.
                roomDesc2 = new RoomDescriptor(idDispenser++, maze2);
                assert(roomDesc2 != NULL);

                // Check if looping.
                for (p = 0, q = descriptors.size(); p < q; p++)
                {
                    if (roomDesc2->dejaVu(descriptors[p])) break;
                }
                if (p < q)
                {
                    delete roomDesc2;
                    roomDesc2 = descriptors[p];
                    maze2 = roomDesc2->maze;
                }
                else
                {
                    descriptors.push_back(roomDesc2);
                }

                // Connect to room.
                connectingRoom = new RoomDescriptor::ConnectingRoom();
                assert(connectingRoom != NULL);
                if (type == INSTANCE_MAP)
                {
                    connectingRoom->resolved = true;
                }
                connectingRoom->roomIndex = maze2->currentRoom;
                connectingRoom->roomDesc = roomDesc2;
                connectingRoom->door = i;
                connectingRoom->signature = signature;
                if (type == INSTANCE_MAP)
                {
                    connectingRoom->resolved = true;
                }
                roomDesc->connectingRooms.push_back(connectingRoom);

                // Map next room.
                mapMazeContexts(roomDesc2, level + 1);
            }
            else
            {
                delete maze2;
            }
        }
    }
}


// Return best door toward given goal.
// If goal in current room, return doors.size().
// If goal unreachable, return -1.
int MazeMap::gotoGoal(int goal)
{
    int i,j,k,door,numDoors,id,validInstanceCount;
    double d,d2;
    vector<bool> validMapsSave;
    vector<int> ids,doors,instanceCounts;
    vector<double> doorAccum;
    vector<double> pathLengthAccum;
    vector<int> pathCountAccum;

    // Sanity check.
    assert(type != ROOM_MAP);

    // Non-set call?
    if (!mapSet) return gotoGoalSim(goal);

    // Prepare for maze simulation.
    for (i = validInstanceCount = 0, j = maps.size(); i < j; i++)
    {
        validMapsSave.push_back(validMaps[i]);
        if (validMaps[i])
        {
            validInstanceCount++;
            maps[i]->backup();
        }
    }
    id = maps[instanceIndex]->currentDescriptor->maze->rooms[maps[instanceIndex]->currentDescriptor->maze->currentRoom]->id;
    ids.push_back(id);
    instanceCounts.push_back(validInstanceCount);
    numDoors = maps[0]->currentDescriptor->maze->numDoors;
    doorAccum.resize(numDoors);
    pathLengthAccum.resize(numDoors);
    pathCountAccum.resize(numDoors);

    // Simulate a traversal of the maze, recording the path taken.
    while (true)
    {
        // Accumulate information on doors leading to successful
        // goal paths for maze instances.
        for (i = 0; i < numDoors; i++)
        {
            doorAccum[i] = 0.0;
            pathLengthAccum[i] = 0.0;
            pathCountAccum[i] = 0;
        }
        for (i = 0, j = maps.size(); i < j; i++)
        {
            if (validMaps[i])
            {
                k = maps[i]->gotoGoalSub(goal, instanceSeeds[i],
                    instanceFrequencies[i], validInstanceCount,
                    doorAccum, pathLengthAccum, pathCountAccum);
                if (i == instanceIndex) door = k;
            }
        }
        if (door >= numDoors)
        {
            doors.push_back(door);
            break;
        }

        // Choose door with best chance of leading to goal.
        // If tie, prefer shortest average path.
        door = -1;
        d = 0.0;
        for (i = 0; i < numDoors; i++)
        {
            if (doorAccum[i] > d)
            {
                door = i;
                d = doorAccum[i];
            }
        }
        if (door != -1)
        {
            for (i = 0; i < numDoors; i++)
            {
                if (pathCountAccum[i] > 0)
                {
                    pathLengthAccum[i] /= (double)pathCountAccum[i];
                }
            }
            d2 = -1.0;
            for (i = 0; i < numDoors; i++)
            {
                if (doorAccum[i] == d)
                {
                    if (d2 < 0.0 || pathLengthAccum[i] < d2)
                    {
                        door = i;
                        d2 = pathLengthAccum[i];
                    }
                }
            }
        }
        doors.push_back(door);
        if (door < 0) break;
        chooseDoor(door);
        id = maps[instanceIndex]->currentDescriptor->maze->rooms[maps[instanceIndex]->currentDescriptor->maze->currentRoom]->id;
        ids.push_back(id);

        // Recompute valid instance count.
        for (i = validInstanceCount = 0, j = maps.size(); i < j; i++)
        {
            if (validMaps[i]) validInstanceCount++;
        }
        instanceCounts.push_back(validInstanceCount);
    }

    // Restore maps for "live" use.
    for (i = 0, j = maps.size(); i < j; i++)
    {
        validMaps[i] = validMapsSave[i];
        if (validMaps[i])
        {
            maps[i]->restore();
        }
    }

    // Get door to take for this instance.
    door = doors[0];

    // No path or found goal?
    if (door < 0 || door >= numDoors) return door;

    // If maze has possible higher-level contexts do not
    // attempt to eliminate loops, since traversing them
    // may alter the maze "topology".
    if (maps[instanceIndex]->currentDescriptor->maze->contexts.size() > 1) return door;

    // Find optimal door that eliminates looping.
    // This involves "jumping ahead" from the current room using
    // the door used the last time that all instances are in
    // march step.
    for (i = 1, j = ids.size(); i < j; i++)
    {
        if (ids[i] == ids[0] && instanceCounts[i] == instanceCounts[0])
        {
            door = doors[i];
        }
    }
    return door;
}


// Take door leading toward given goal in a meta-maze
// that has unknown specific instances. This is done
// by simulating a set of instances.
int MazeMap::gotoGoalSim(int goal)
{
    int i,j,n,p,q,r,s,door;
    double d,d2;
    vector<double> doorAccum,pathLengthAccum;
    vector<int> pathCountAccum;
    RoomDescriptor *roomDesc;
    MazeRoom *room,*room2;
    RoomDescriptor::ConnectingRoom *connectingRoom;
    stack<int> path;
    bool foundGoal;

    // Sanity checks.
    assert(!mapSet);
    assert(type != ROOM_MAP);

    // Start search for goal from current location.
    if (currentDescriptor == NULL ||
        currentDescriptor->maze->currentRoom == -1) return -1;

    // Current room contains a goal?
    room = currentDescriptor->maze->rooms[currentDescriptor->maze->currentRoom];
    if (room->goals[goal])
    {
        foundGoal = true;
    }
    else
    {
        foundGoal = false;
    }
    for (i = 0, j = currentDescriptor->maze->numGoals; i < j; i++)
    {
        if (room->goals[i]) break;
    }
    if (i < j)
    {
        // Having found them, remove all goals in current room from map.
        for (i = 0, j = descriptors.size(); i < j; i++)
        {
            roomDesc = descriptors[i];
            for (p = 0, q = roomDesc->maze->numRooms; p < q; p++)
            {
                room2 = roomDesc->maze->rooms[p];
                for (r = 0, s = roomDesc->maze->numGoals; r < s; r++)
                {
                    if (room->goals[r])
                    {
                        room2->goals[r] = false;
                    }
                }
            }
        }
    }
    if (foundGoal) return currentDescriptor->maze->numDoors;

    // Accumulate information on doors leading to successful
    // goal paths for maze instances.
    i = currentDescriptor->maze->numDoors;
    doorAccum.resize(i);
    pathLengthAccum.resize(i);
    pathCountAccum.resize(i);
    for (i = 0, j = doorAccum.size(); i < j; i++)
    {
        doorAccum[i] = 0;
        pathLengthAccum[i] = 0.0;
        pathCountAccum[i] = 0;
    }
    for (n = 0; n < SEARCH_PASSES; n++)
    {
        // Try doors.
        for (i = 0, j = currentDescriptor->connectingRooms.size(); i < j; i++)
        {
            connectingRoom = currentDescriptor->connectingRooms[i];
            if (connectingRoom->visited) continue;
            roomDesc = connectingRoom->roomDesc;
            door = connectingRoom->door;
            for (p = 0, q = descriptors.size(); p < q; p++)
            {
                descriptors[p]->searchLength = -1;
            }
            currentDescriptor->searchLength = 0;
            while (path.size() > 0) path.pop();
            path.push(currentDescriptor->maze->rooms[currentDescriptor->maze->currentRoom]->id);
            if (connectingRoom->resolved)
            {
                if (connectingRoom->signature->success)
                {
                    if (MazeMap::searchGoal(goal, roomDesc, path, (RANDOM)n))
                    {
                        doorAccum[door] += 1.0;
                        pathLengthAccum[door] += path.size();
                        pathCountAccum[door]++;
                    }
                }
            }
            else
            {
                if (connectingRoom->signature->getSuccess(n))
                {
                    if (MazeMap::searchGoal(goal, roomDesc, path, (RANDOM)n))
                    {
                        doorAccum[door] += 1.0;
                        pathLengthAccum[door] += path.size();
                        pathCountAccum[door]++;
                    }
                }
            }
        }
    }

    // Choose door with best chance of leading to goal.
    // If tie, prefer shortest average path.
    door = -1;
    d = 0.0;
    for (i = 0, j = doorAccum.size(); i < j; i++)
    {
        if (doorAccum[i] > d)
        {
            door = i;
            d = doorAccum[i];
        }
    }
    if (door != -1)
    {
        for (i = 0, j = doorAccum.size(); i < j; i++)
        {
            if (pathCountAccum[i] > 0)
            {
                pathLengthAccum[i] /= (double)pathCountAccum[i];
            }
        }
        d2 = -1.0;
        for (i = 0, j = doorAccum.size(); i < j; i++)
        {
            if (doorAccum[i] == d)
            {
                if (d2 < 0.0 || pathLengthAccum[i] < d2)
                {
                    door = i;
                    d2 = pathLengthAccum[i];
                }
            }
        }
    }
    return door;
}


// Go to goal subfunction.
// Take door leading toward given goal given
// a set of possible maze instances and their
// relative frequencies of occurrance.
// Return best door toward given goal.
// If goal in current room, return doors.size().
// If goal unreachable, return -1.
// This can be used for meta-maze training where
// there is a specific set of possible maze instances,
// or, by supplying a single instance seed, training
// for that specific instance. It is also possible
// to weight the paths by supplying the appropriate
// instance frequency information.
int MazeMap::gotoGoalSub(int goal,
RANDOM instanceSeed,
double instanceFrequency,
int validInstanceCount,
vector<double> &doorAccum,
vector<double> &pathLengthAccum,
vector<int> &pathCountAccum)
{
    int i,j,p,q,r,s,door;
    RoomDescriptor *roomDesc;
    MazeRoom *room,*room2;
    RoomDescriptor::ConnectingRoom *connectingRoom;
    stack<int> path;
    bool foundGoal;

    // Sanity check.
    assert(!mapSet);
    assert(type != ROOM_MAP);

    // Start search for goal from current location.
    if (currentDescriptor == NULL ||
        currentDescriptor->maze->currentRoom == -1) return -1;

    // Current room contains a goal?
    room = currentDescriptor->maze->rooms[currentDescriptor->maze->currentRoom];
    if (room->goals[goal])
    {
        foundGoal = true;
    }
    else
    {
        foundGoal = false;
    }
    for (i = 0, j = currentDescriptor->maze->numGoals; i < j; i++)
    {
        if (room->goals[i]) break;
    }
    if (i < j)
    {
        // Having found them, remove all goals in current room from map.
        for (i = 0, j = descriptors.size(); i < j; i++)
        {
            roomDesc = descriptors[i];
            for (p = 0, q = roomDesc->maze->numRooms; p < q; p++)
            {
                room2 = roomDesc->maze->rooms[p];
                for (r = 0, s = roomDesc->maze->numGoals; r < s; r++)
                {
                    if (room->goals[r])
                    {
                        room2->goals[r] = false;
                    }
                }
            }
        }
    }
    if (foundGoal) return currentDescriptor->maze->numDoors;

    // Try doors.
    for (i = 0, j = currentDescriptor->connectingRooms.size(); i < j; i++)
    {
        connectingRoom = currentDescriptor->connectingRooms[i];
        if (currentDescriptor->maze->contexts.size() > 1)
        {
            if (connectingRoom->visited) continue;
        }
        else
        {
            if (connectingRoom->validInstanceCount != -1 &&
                connectingRoom->validInstanceCount <= validInstanceCount) continue;
        }
        roomDesc = connectingRoom->roomDesc;
        door = connectingRoom->door;
        for (p = 0, q = descriptors.size(); p < q; p++)
        {
            descriptors[p]->searchLength = -1;
        }
        currentDescriptor->searchLength = 0;
        while (path.size() > 0) path.pop();
        path.push(currentDescriptor->maze->rooms[currentDescriptor->maze->currentRoom]->id);
        if (connectingRoom->resolved)
        {
            if (connectingRoom->signature->success)
            {
                if (MazeMap::searchGoal(goal, roomDesc, path, instanceSeed))
                {
                    doorAccum[door] += instanceFrequency;
                    pathLengthAccum[door] += (double)path.size();
                    pathCountAccum[door]++;
                }
            }
        }
        else
        {
            if (connectingRoom->signature->getSuccess(instanceSeed))
            {
                if (MazeMap::searchGoal(goal, roomDesc, path, instanceSeed))
                {
                    doorAccum[door] += instanceFrequency;
                    pathLengthAccum[door] += (double)path.size();
                    pathCountAccum[door]++;
                }
            }
        }
    }

    return 0;
}


// Search for shortest path to goal starting in given room.
// Probabilities are determined by given seed.
bool MazeMap::searchGoal(int goal, RoomDescriptor *roomDesc,
stack<int> &path, RANDOM randomSeed)
{
    int i,j,door,length;
    MazeRoom *room;
    RoomDescriptor::ConnectingRoom *connectingRoom;
    RoomDescriptor *roomDesc2;
    stack<int> workpath,bestpath;

    // Prevent looping.
    if (roomDesc->searchLength != -1 &&
        roomDesc->searchLength < path.size()) return false;
    roomDesc->searchLength = path.size();

    // This room contains goal?
    room = roomDesc->maze->rooms[roomDesc->maze->currentRoom];
    path.push(room->id);
    if (room->goals[goal]) return true;

    // Try doors.
    length = -1;
    for (i = 0, j = roomDesc->connectingRooms.size(); i < j; i++)
    {
        connectingRoom = roomDesc->connectingRooms[i];
        roomDesc2 = connectingRoom->roomDesc;
        door = connectingRoom->door;
        workpath = path;
        if (connectingRoom->resolved)
        {
            if (connectingRoom->signature->success)
            {
                if (searchGoal(goal, roomDesc2, workpath, randomSeed))
                {
                    if (length == -1 || workpath.size() < length)
                    {
                        length = workpath.size();
                        bestpath = workpath;
                    }
                }
            }
        }
        else
        {
            if (connectingRoom->signature->getSuccess(randomSeed))
            {
                if (searchGoal(goal, roomDesc2, workpath, randomSeed))
                {
                    if (length == -1 || workpath.size() < length)
                    {
                        length = workpath.size();
                        bestpath = workpath;
                    }
                }
            }
        }
    }
    if (length == -1)
    {
        path.pop();
        return false;
    }
    else
    {
        path = bestpath;
        return true;
    }
}


// Update current location in map by choosing door.
// Resolve connections that have same signature.
// A resolved connection means that the learner has tried
// the connection, so the probability is resolved.
void MazeMap::chooseDoor(int door, int validInstanceCount)
{
    int i,j,p,q,mark;
    RoomDescriptor *roomDesc;
    RoomDescriptor::ConnectingRoom *connectingRoom;
    ContextSignature *signature;

    // Map set?
    if (mapSet)
    {
        for (i = validInstanceCount = 0, j = maps.size(); i < j; i++)
        {
            if (validMaps[i]) validInstanceCount++;
        }
        for (i = 0, j = maps.size(); i < j; i++)
        {
            if (validMaps[i])
            {
                maps[i]->chooseDoor(door, validInstanceCount);
            }
        }

        // Invalidate maps that do not go to same room as maze instance.
        mark = maps[instanceIndex]->currentDescriptor->maze->rooms[maps[instanceIndex]->currentDescriptor->maze->currentRoom]->mark;
        for (i = 0, j = maps.size(); i < j; i++)
        {
            if (validMaps[i])
            {
                if (maps[i]->currentDescriptor->maze->rooms[maps[i]->currentDescriptor->maze->currentRoom]->mark != mark)
                {
                    validMaps[i] = false;
                }
            }
        }
        return;
    }

    // Take door and update map.
    assert(currentDescriptor != NULL &&
        currentDescriptor->maze->currentRoom != -1);
    signature = NULL;
    for (i = 0, j = currentDescriptor->connectingRooms.size(); i < j; i++)
    {
        connectingRoom = currentDescriptor->connectingRooms[i];
        if (connectingRoom->door == door)
        {
            if (connectingRoom->signature->success)
            {
                connectingRoom->visited = true;
                connectingRoom->validInstanceCount = validInstanceCount;
                currentDescriptor = connectingRoom->roomDesc;
            }
            signature = connectingRoom->signature;
            break;
        }
    }
    if (signature == NULL) return;

    // Resolve signatures.
    for (i = 0, j = descriptors.size(); i < j; i++)
    {
        roomDesc = descriptors[i];
        for (p = 0, q = roomDesc->connectingRooms.size(); p < q; p++)
        {
            connectingRoom = roomDesc->connectingRooms[p];
            if (connectingRoom->signature->match(signature))
            {
                connectingRoom->signature->success = signature->success;
                connectingRoom->resolved = true;
            }
        }
    }
}


// Backup rooms.
void MazeMap::backup()
{
    int i,j;
    RoomDescriptor *roomDesc;

    if (mapSet)
    {
        for (i = 0, j = maps.size(); i < j; i++)
        {
            maps[i]->backup();
        }
    }
    else
    {
        backCurrentDescriptor = currentDescriptor;
        for (i = 0, j = descriptors.size(); i < j; i++)
        {
            roomDesc = descriptors[i];
            roomDesc->backup();
        }
    }
}


// Restore rooms.
void MazeMap::restore()
{
    int i,j;
    RoomDescriptor *roomDesc;

    if (mapSet)
    {
        for (i = 0, j = maps.size(); i < j; i++)
        {
            maps[i]->restore();
        }
    }
    else
    {
        currentDescriptor = backCurrentDescriptor;
        backCurrentDescriptor = NULL;
        for (i = 0, j = descriptors.size(); i < j; i++)
        {
            roomDesc = descriptors[i];
            roomDesc->restore();
        }
    }
}


// Dump maze map in text or Graphviz "dot" format to stdout.
void MazeMap::dump(DUMP_TYPE dumpType, FILE *fp)
{
    vector<int> dummy;
    dump(dumpType, INVALID_RANDOM, -1, dummy, dummy, fp);
}


// Dump maze map with optional instance annotations, including goal path.
void MazeMap::dump(DUMP_TYPE dumpType, RANDOM instanceSeed,
int goal, vector<int> &ids, vector<int> &doors, FILE *fp)
{
    int i,j,k,p,q,r,s;
    RoomDescriptor *roomDesc;
    MazeRoom *room,*room2;
    RoomDescriptor::ConnectingRoom *connectingRoom;

    // Map set?
    if (mapSet)
    {
        // Dump member map.
        if (instanceSeed == INVALID_RANDOM)
        {
            maps[0]->dump(dumpType, fp);
        }
        else
        {
            for (i = 0, j = maps.size(); i < j; i++)
            {
                if (maps[i]->currentDescriptor->maze->instanceSeed == instanceSeed)
                {
                    maps[i]->dump(dumpType, instanceSeed, goal, ids, doors, fp);
                    break;
                }
            }
        }
        return;
    }

    if (dumpType == TEXT)
    {
        switch(type)
        {
            case ROOM_MAP:
                fprintf(fp, "Maze rooms map:\n");
                break;
            case INSTANCE_MAP:
                fprintf(fp, "Maze instance map:\n");
                break;
            case META_MAP:
                fprintf(fp, "Maze meta map:\n");
                break;
        }
        if (instanceSeed != INVALID_RANDOM)
        {
            fprintf(fp, "Instance=%d, Goal=%d\n", instanceSeed, goal);
        }
        for (i = 0, j = descriptors.size(); i < j; i++)
        {
            roomDesc = descriptors[i];
            room = roomDesc->maze->rooms[roomDesc->maze->currentRoom];
            if (type == ROOM_MAP)
            {
                fprintf(fp, "Room %d:\n", room->id);
            }
            else
            {
                fprintf(fp, "Room %d:", roomDesc->id);
            }
            fprintf(fp, "\tMark %d:\n", room->mark);
            fprintf(fp, "\tDoors:");
            for (p = 0, q = room->doors.size(); p < q; p++)
            {
                if (room->doors[p])
                {
                    fprintf(fp, " %d", p);
                }
            }
            fprintf(fp, "\n");
            fprintf(fp, "\tGoals:");
            for (p = 0, q = room->goals.size(); p < q; p++)
            {
                if (room->goals[p])
                {
                    fprintf(fp, " %d", p);
                }
            }
            fprintf(fp, "\n");
            for (p = k = 0, q = ids.size(); p < q; p++)
            {
                if ((type == ROOM_MAP && ids[p] == room->id) ||
                    (type != ROOM_MAP && ids[p] == roomDesc->id))
                {
                    if (k == 0)
                    {
                        fprintf(fp, "\tSequence: ");
                    }
                    fprintf(fp, "%d ", p);
                    k++;
                }
            }
            if (k > 0)
            {
                fprintf(fp, "\n");
            }
            fprintf(fp, "\tConnections:\n");
            for (p = 0, q = roomDesc->connectingRooms.size(); p < q; p++)
            {
                connectingRoom = roomDesc->connectingRooms[p];
                if (type == ROOM_MAP)
                {
                    room2 = roomDesc->maze->rooms[connectingRoom->roomIndex];
                    if (roomDesc->maze->contexts.size() > 1)
                    {
                        fprintf(fp, "\t\tRoom %d via door %d\n",
                            room2->id, connectingRoom->door);
                    }
                    else
                    {
                        fprintf(fp, "\t\tRoom %d via door %d with probability=%0.2f",
                            room2->id, connectingRoom->door,
                            connectingRoom->signature->probability);
                        if (instanceSeed != INVALID_RANDOM)
                        {
                            if (connectingRoom->signature->getSuccess(instanceSeed))
                            {
                                fprintf(fp, " (open)");
                            }
                            else
                            {
                                fprintf(fp, " (closed)");
                            }
                        }
                        fprintf(fp, "\n");
                    }
                }
                else
                {
                    fprintf(fp, "\t\tRoom %d via door %d with probability=%0.2f",
                        connectingRoom->roomDesc->id, connectingRoom->door,
                        connectingRoom->signature->probability);
                    if (instanceSeed != INVALID_RANDOM)
                    {
                        if (connectingRoom->signature->getSuccess(instanceSeed))
                        {
                            fprintf(fp, " (open)");
                        }
                        else
                        {
                            fprintf(fp, " (closed)");
                        }
                    }
                    fprintf(fp, "\n");
                }
                for (r = k = 0, s = ids.size(); r < s; r++)
                {
                    if ((type == ROOM_MAP && ids[r] == room->id) ||
                        (type != ROOM_MAP && ids[r] == roomDesc->id))
                    {
                        if (r < doors.size() && doors[r] == connectingRoom->door)
                        {
                            if (k == 0)
                            {
                                fprintf(fp, "\t\tSequence: ");
                            }
                            fprintf(fp, "%d ", r);
                            k++;
                        }
                    }
                }
                if (k > 0)
                {
                    fprintf(fp, "\n");
                }
            }
        }
    }                                             // dumpType == GRAPH
    else
    {

        fprintf(fp, "digraph Maze {\n");
        fprintf(fp, "\tgraph [size=\"8.5,11\",fontsize=24];\n");

        for (i = 0, j = descriptors.size(); i < j; i++)
        {
            roomDesc = descriptors[i];
            room = roomDesc->maze->rooms[roomDesc->maze->currentRoom];
            if (type == ROOM_MAP)
            {
                fprintf(fp, "\t\"%d\" [label=\"room %d", room->id, room->id);
            }
            else
            {
                fprintf(fp, "\t\"%d\" [label=\"room %d", roomDesc->id, roomDesc->id);
            }
            fprintf(fp, " mark %d\\n", room->mark);
            fprintf(fp, "goals: ");
            for (p = 0, q = room->goals.size(); p < q; p++)
            {
                if (room->goals[p])
                {
                    fprintf(fp, "%d ", p);
                }
            }
            for (p = k = 0, q = ids.size(); p < q; p++)
            {
                if ((type == ROOM_MAP && ids[p] == room->id) ||
                    (type != ROOM_MAP && ids[p] == roomDesc->id))
                {
                    if (k == 0)
                    {
                        fprintf(fp, "\\nseq: ");
                    }
                    fprintf(fp, "%d ", p);
                    k++;
                }
            }
            fprintf(fp, "\",shape=box];\n");
        }
        for (i = 0, j = descriptors.size(); i < j; i++)
        {
            roomDesc = descriptors[i];
            room = roomDesc->maze->rooms[roomDesc->maze->currentRoom];
            for (p = 0, q = roomDesc->connectingRooms.size(); p < q; p++)
            {
                connectingRoom = roomDesc->connectingRooms[p];
                if (type == ROOM_MAP)
                {
                    room2 = roomDesc->maze->rooms[connectingRoom->roomIndex];
                    if (roomDesc->maze->contexts.size() > 1)
                    {
                        fprintf(fp, "\t\"%d\" -> \"%d\" [label=\"door %d",
                            room->id, room2->id, connectingRoom->door);
                    }
                    else
                    {
                        fprintf(fp, "\t\"%d\" -> \"%d\" [label=\"door %d (%0.2f",
                            room->id, room2->id, connectingRoom->door,
                            connectingRoom->signature->probability);
                        if (instanceSeed != INVALID_RANDOM)
                        {
                            if (connectingRoom->signature->getSuccess(instanceSeed))
                            {
                                fprintf(fp, "/open");
                            }
                            else
                            {
                                fprintf(fp, "/closed");
                            }
                        }
                        fprintf(fp, ")");
                    }
                }
                else
                {
                    fprintf(fp, "\t\"%d\" -> \"%d\" [label=\"door %d (%0.2f",
                        roomDesc->id, connectingRoom->roomDesc->id,
                        connectingRoom->door,
                        connectingRoom->signature->probability);
                    if (instanceSeed != INVALID_RANDOM)
                    {
                        if (connectingRoom->signature->getSuccess(instanceSeed))
                        {
                            fprintf(fp, "/open");
                        }
                        else
                        {
                            fprintf(fp, "/closed");
                        }
                    }
                    fprintf(fp, ")");
                }
                for (r = k = 0, s = ids.size(); r < s; r++)
                {
                    if ((type == ROOM_MAP && ids[r] == room->id) ||
                        (type != ROOM_MAP && ids[r] == roomDesc->id))
                    {
                        if (r < doors.size() && doors[r] == connectingRoom->door)
                        {
                            if (k == 0)
                            {
                                fprintf(fp, " seq: ");
                            }
                            fprintf(fp, "%d ", r);
                            k++;
                        }
                    }
                }
                fprintf(fp, "\",style=solid];\n");
            }
        }
        switch(type)
        {
            case ROOM_MAP:
                fprintf(fp, "\tlabel = \"Maze room map");
                break;
            case INSTANCE_MAP:
                fprintf(fp, "\tlabel = \"Maze instance map");
                break;
            case META_MAP:
                fprintf(fp, "\tlabel = \"Maze meta map");
                break;
        }
        if (instanceSeed != INVALID_RANDOM)
        {
            fprintf(fp, ", instance=%d, goal=%d", instanceSeed, goal);
        }
        fprintf(fp, "\";\n");
        fprintf(fp, "}\n");
    }
}
