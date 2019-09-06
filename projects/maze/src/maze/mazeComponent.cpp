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

// Maze element.

#include "mazeComponent.hpp"

// Maze room constructors.
MazeRoom::MazeRoom(int id, int mark)
{
    this->id = id;
    type = ROOM;
    this->mark = mark;
    instanceSeed = 0;
}


MazeRoom::MazeRoom()
{
    id = -1;
    type = ROOM;
    mark = -1;
    instanceSeed = 0;
}


// Maze room destructor.
MazeRoom::~MazeRoom()
{
    int i,j;
    ContextSignature *signature;

    // Delete signatures.
    for (i = 0, j = contextSignatures.size(); i < j; i++)
    {
        signature = contextSignatures[i];
        delete signature;
    }
    contextSignatures.clear();
}


// Maze context constructor.
MazeContext::MazeContext(int id)
{
    this->id = id;
    this->type = CONTEXT;
    door = -1;
    probability = randomizer->RAND_PROB();
    effectDelay = 0;
    effectTimer = -1;
    cause = effect = NULL;
}


// Get current room signature.
ContextSignature *MazeRoom::getSignature()
{
    int i,j;
    MazeComponent *context;
    vector<int> ids;
    ContextSignature *contextSignature;

    // Signature matches previous context set?
    ids.push_back(id);
    for (i = 0, j = contexts.size(); i < j; i++)
    {
        context = contexts[i];
        ids.push_back(context->id);
    }
    for (i = 0, j = contextSignatures.size(); i < j; i++)
    {
        contextSignature = contextSignatures[i];
        if (contextSignature->match(ids))
        {
            contextSignature->visited = true;
            return contextSignature;
        }
    }

    // Create signature.
    contextSignature = new ContextSignature(ids);
    contextSignatures.push_back(contextSignature);

    // Determine success.
    contextSignature->probability = getAccessProbability();
    contextSignature->success =
        contextSignature->getSuccess(instanceSeed);

    return contextSignature;
}


// Get probability of access.
double MazeRoom::getAccessProbability()
{
    int i,j;
    double psum;
    MazeContext *context;

    // Accumulate active context probability.
    psum = 0.0;
    for (i = 0, j = contexts.size(); i < j; i++)
    {
        context = contexts[i];
        psum += context->probability;
    }
    if (psum < 0.0) psum = 0.0;
    if (psum > 1.0) psum = 1.0;

    return psum;
}


// Activate context based on cause event.
void MazeContext::fireCause()
{
    int i,j;
    MazeContext *context;

    // Effect event in progress?
    if (effectTimer >= 0) return;

    // Add this context to effect.
    effectTimer = effectDelay;
    effect->contexts.push_back(this);

    // Propagate pending effect probabilities.
    for (i = 0, j = contexts.size(); i < j; i++)
    {
        context = contexts[i];
        context->effectTimer += effectTimer;
        effect->contexts.push_back(context);
    }
    contexts.clear();

    // Boost pending cause timers.
    for (i = 0, j = causeContexts.size(); i < j; i++)
    {
        context = causeContexts[i];
        context->boostTimer(effectTimer);
    }
}


// Boost timers for pending contexts.
void MazeContext::boostTimer(int increment)
{
    int i,j;
    MazeContext *context;

    // Effect event in progress?
    if (effectTimer >= 0) return;

    // Boost pending contexts.
    for (i = 0, j = contexts.size(); i < j; i++)
    {
        context = contexts[i];
        context->effectTimer += increment;
    }

    // Continue boosting.
    for (i = 0, j = causeContexts.size(); i < j; i++)
    {
        context = causeContexts[i];
        context->boostTimer(increment);
    }
}


// Activate context based on effect event.
void MazeContext::fireEffect()
{
    int i,j;
    MazeContext *context;

    // Fire parent cause events.
    for (i = 0, j = causeContexts.size(); i < j; i++)
    {
        context = causeContexts[i];
        context->fireCause();
    }
}


// Clone room.
MazeRoom *MazeRoom::clone()
{
    return clone(instanceSeed);
}


MazeRoom *MazeRoom::clone(RANDOM newInstanceSeed)
{
    int i,j;
    MazeRoom *room;
    ContextSignature *signature;

    room = new MazeRoom(id, mark);
    assert(room != NULL);
    room->instanceSeed = newInstanceSeed;
    room->doors.resize(doors.size());
    for (i = 0, j = room->doors.size(); i < j; i++)
    {
        room->doors[i] = doors[i];
    }
    room->goals.resize(goals.size());
    for (i = 0, j = room->goals.size(); i < j; i++)
    {
        room->goals[i] = goals[i];
    }
    room->contextSignatures.resize(contextSignatures.size());
    for (i = 0, j = room->contextSignatures.size(); i < j; i++)
    {
        signature = contextSignatures[i];
        room->contextSignatures[i] = signature->clone();
    }
    return room;
}


// Clone context.
MazeContext *MazeContext::clone()
{
    MazeContext *context;

    context = new MazeContext(id);
    assert(context!= NULL);
    context->door = door;
    context->probability = probability;
    context->effectDelay = effectDelay;
    context->effectTimer = effectTimer;
    context->cause = context->effect = NULL;
    return context;
}


// Print room.
void MazeRoom::print(FILE *fp)
{
    int i,j;
    MazeContext *context;

    fprintf(fp, "Room id=%d mark=%d doors: ", id, mark);
    for (i = 0, j = doors.size(); i < j; i++)
    {
        fprintf(fp, "%d ", (int)doors[i]);
    }
    fprintf(fp, "goals: ");
    for (i = 0, j = goals.size(); i < j; i++)
    {
        fprintf(fp, "%d ", (int)goals[i]);
    }
    fprintf(fp, "instanceSeed=%d ", instanceSeed);
    fprintf(fp, "contexts: ");
    for (i = 0, j = contexts.size(); i < j; i++)
    {
        context = contexts[i];
        fprintf(fp, "id=%d ", context->id);
    }
    fprintf(fp, "\n");
}


// Print context.
void MazeContext::print(FILE *fp)
{
    int i,j;
    MazeContext *context;

    fprintf(fp, "Context id=%d, ", id);
    fprintf(fp, "cause id=%d, effect id=%d, ",
        cause->id, effect->id);
    fprintf(fp, "door=%d, effect delay=%d, timer=%d, ",
        door, effectDelay, effectTimer);
    fprintf(fp, "probability=%f ", probability);
    fprintf(fp, "contexts: ");
    for (i = 0, j = contexts.size(); i < j; i++)
    {
        context = contexts[i];
        fprintf(fp, "id=%d ", context->id);
    }
    fprintf(fp, "\n");
}


// Context signature constructor.
ContextSignature::ContextSignature(vector<int> &ids)
{
    int i,j;

    this->ids.clear();
    this->ids.resize(ids.size());
    for (i = 0, j = ids.size(); i < j; i++)
    {
        this->ids[i] = ids[i];
    }
    sort();
    probability = 0.0;
    success = visited = false;
}


// Context signature destructor.
ContextSignature::~ContextSignature()
{
    ids.clear();
}


// Get signature success.
bool ContextSignature::getSuccess(RANDOM randomSeed)
{
    int i,j;
    bool ret;

    // Seed random numbers based on context and given seed.
    randomizer->RAND_PUSH();
    randomizer->SRAND((RANDOM)ids[0]);
    for (i = 1, j = ids.size(); i < j; i++)
    {
        randomizer->SRAND(randomizer->RAND() + (RANDOM)ids[i]);
    }
    randomizer->SRAND(randomizer->RAND() + randomSeed);

    // Determine success.
    ret = randomizer->RAND_CHANCE(probability);
    randomizer->RAND_POP();

    return ret;
}


// Context signatures match?
bool ContextSignature::match(ContextSignature *signature)
{
    int i,j;

    if (ids.size() != signature->ids.size()) return false;

    for (i = 0, j = ids.size(); i < j; i++)
    {
        if (ids[i] != signature->ids[i]) return false;
    }
    return true;
}


// Context signature matches ids?
bool ContextSignature::match(vector<int> ids)
{
    int i,j,p,q;

    if (this->ids.size() != ids.size()) return false;

    for (i = 0, j = ids.size(); i < j; i++)
    {
        for (p = 0, q = ids.size(); p < q; p++)
        {
            if (ids[p] == this->ids[i]) break;
        }
        if (p == q) return false;
    }
    return true;
}


// Clone signature.
ContextSignature *ContextSignature::clone()
{
    ContextSignature *signature;

    signature = new ContextSignature(ids);
    assert(signature != NULL);
    signature->probability = probability;
    signature->success = success;
    signature->visited = visited;

    return signature;
}


// Sort ids.
void ContextSignature::sort()
{
    int i,j,k,n;

    for (i = 0, j = ids.size(); i < j; i++)
    {
        for (k = i + 1; k < j; k++)
        {
            if (ids[i] > ids[k])
            {
                n = ids[i];
                ids[i] = ids[k];
                ids[k] = n;
            }
        }
    }
}


// Print context signature.
void ContextSignature::print(FILE *fp)
{
    int i,j;

    fprintf(fp, "ids: ");
    for (i = 0, j = ids.size(); i < j; i++)
    {
        fprintf(fp, "%d ", ids[i]);
    }
    fprintf(fp, "probability=%f ", probability);
    if (success)
    {
        fprintf(fp, "success=true");
    }
    else
    {
        fprintf(fp, "success=false");
    }
}
