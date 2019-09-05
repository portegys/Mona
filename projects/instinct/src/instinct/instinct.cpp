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

#include "instinct.hpp"

// Instinct constructor.
Instinct::Instinct()
{
    needIndex = 0;
    needValue = 0.0;
    needFrequency = 0;
    freqTimer = 0;
    needDuration = 0;
    durTimer = 0;
    goalValue = 0.0;
    mediator = NULL;
}


// Fire instinct: when associated mediator fires, modify needs.
void Instinct::fire()
{
    Mona::NEED need;

    if (needIndex == -1 || mediator == NULL) return;
    if (mediator->firingStrength > 0.0)
    {
        need = mediator->mona->getNeed(needIndex);
        need -= goalValue;
        if (need < 0.0) need = 0.0;
        mediator->mona->setNeed(needIndex, need);
    }
}


// Induce instinctive need.
void Instinct::induce()
{
    if (needIndex == -1 || mediator == NULL) return;
    if (durTimer > 0)
    {
        durTimer--;
        if (durTimer == 0)
        {
            mediator->mona->setNeed(needIndex, 0.0);
        }
    }
    if (needFrequency == 0) return;
    freqTimer++;
    if (freqTimer >= needFrequency)
    {
        freqTimer = 0;
        mediator->mona->setNeed(needIndex, needValue);
        if (needDuration > 0)
        {
            durTimer = needDuration;
        }
    }
}


// Is instinct a duplicate?
bool Instinct::isDuplicate(Instinct *instinct)
{
    int i,j,k;

    if (events.size() != instinct->events.size()) return false;
    for (i = 0, j = events.size(); i < j; i++)
    {
        for (k = 0; k < NUM_INSTINCT_STIMULI; k++)
        {
            if (events[i].stimuli[k] !=
                instinct->events[i].stimuli[k]) return false;
        }
        if (events[i].response !=
            instinct->events[i].response) return false;
    }
    if (needIndex != instinct->needIndex) return false;
    if (needValue != instinct->needValue) return false;
    if (needFrequency != instinct->needFrequency) return false;
    if (needDuration != instinct->needDuration) return false;
    if (goalValue != instinct->goalValue) return false;

    return true;
}


// Clone instinct.
Instinct *Instinct::clone()
{
    int i,j,k;
    Instinct *instinct;
    EVENT event;

    instinct = new Instinct();
    assert(instinct != NULL);
    for (i = 0, j = events.size(); i < j; i++)
    {
        for (k = 0; k < NUM_INSTINCT_STIMULI; k++)
        {
            event.stimuli[k] = events[i].stimuli[k];
        }
        event.response = events[i].response;
        instinct->events.push_back(event);
    }
    instinct->needIndex = needIndex;
    instinct->needValue = needValue;
    instinct->needFrequency = needFrequency;
    instinct->freqTimer = freqTimer;
    instinct->needDuration = needDuration;
    instinct->durTimer = durTimer;
    instinct->goalValue = goalValue;
    instinct->mediator = mediator;
    return instinct;
}


// Load instinct.
void Instinct::load(char *filename)
{
    FILE *fp;

    if ((fp = fopen(filename, "r")) == NULL)
    {
        fprintf(stderr, "Cannot load instinct from file %s\n", filename);
        exit(1);
    }
    load(fp);
}


void Instinct::load(FILE *fp)
{
    int i,j,k;
    EVENT event;

    FREAD_INT(&j, fp);
    events.clear();
    for (i = 0; i < j; i++)
    {
        for (k = 0; k < NUM_INSTINCT_STIMULI; k++)
        {
            FREAD_INT(&event.stimuli[k], fp);
        }
        FREAD_INT(&event.response, fp);
        events.push_back(event);
    }
    FREAD_INT(&needIndex, fp);
    FREAD_DOUBLE(&needValue, fp);
    FREAD_INT(&needFrequency, fp);
    FREAD_INT(&freqTimer, fp);
    FREAD_INT(&needDuration, fp);
    FREAD_INT(&durTimer, fp);
    FREAD_DOUBLE(&goalValue, fp);
}


// Save instinct.
void Instinct::save(char *filename)
{
    FILE *fp;

    if ((fp = fopen(filename, "w")) == NULL)
    {
        fprintf(stderr, "Cannot save instinct to file %s\n", filename);
        exit(1);
    }
    save(fp);
}


void Instinct::save(FILE *fp)
{
    int i,j,k;

    j = events.size();
    FWRITE_INT(&j, fp);
    for (i = 0; i < j; i++)
    {
        for (k = 0; k < NUM_INSTINCT_STIMULI; k++)
        {
            FWRITE_INT(&events[i].stimuli[k], fp);
        }
        FWRITE_INT(&events[i].response, fp);
    }
    FWRITE_INT(&needIndex, fp);
    FWRITE_DOUBLE(&needValue, fp);
    FWRITE_INT(&needFrequency, fp);
    FWRITE_INT(&freqTimer, fp);
    FWRITE_INT(&needDuration, fp);
    FWRITE_INT(&durTimer, fp);
    FWRITE_DOUBLE(&goalValue, fp);
}


// Print instinct.
// Application supplies getStimulusDesc() and getResponseDesc() functions.
void Instinct::print()
{
    int i,j,k;
    extern char *getStimulusDesc(int, int), *getResponseDesc(int);

    printf("events={");
    for (i = 0, j = events.size(); i < j; i++)
    {
        printf("stimuli=[");
        for (k = 0; k < NUM_INSTINCT_STIMULI; k++)
        {
            printf("%s", getStimulusDesc(k, events[i].stimuli[k]));
            if (k < NUM_INSTINCT_STIMULI - 1) printf(",");
        }
        printf("]");
        printf(",response=%s", getResponseDesc(events[i].response));
        if (i < j - 1) printf(",");
    }
    printf("}");
    printf(",needIndex=%d", needIndex);
    printf(",needValue=%f", needValue);
    printf(",needFrequency=%d", needFrequency);
    printf(",needDuration=%d", needDuration);
    printf(",goalValue=%f", goalValue);
    printf(",mediator: ");
    if (mediator != NULL)
    {
        printf("\n"); mediator->print();
    }
    else
    {
        printf("NA");
    }
    printf("\n");
}
