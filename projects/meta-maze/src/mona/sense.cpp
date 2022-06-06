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

#include "mona.hpp"

// Sense environment.
void
Mona::sense()
{
    Receptor *receptor;
    list<Neuron *>::iterator listItr;

    #ifdef MONA_TRACE
    if (traceSense)
    {
        int i;
        printf("***Sense phase***\n");
        printf("Sensors: ");
        for (i = 0; i < numSensors; i++)
        {
            printf("%d ", sensors[i]);
        }
        printf("\n");
    }
    #endif

    for (listItr = receptors.begin();
        listItr != receptors.end(); listItr++)
    {
        receptor = (Receptor *)*listItr;
        receptor->sense(sensors, numSensors);
    }
}


// Sense firing of receptor neuron.
void
Mona::Receptor::sense(SENSOR *sensors, int numSensors)
{
    int i;

    firingStrength = 0.0;
    for (i = 0; i < numSensors; i++)
    {
        if (sensors[i] != sensorMask[i] && sensorMask[i] != DONT_CARE)
        {
            firingStrength = 0.0;
            return;
        }
        if (sensors[i] == sensorMask[i])
        {
            firingStrength += 1.0 / (double)numSensors;
        }
    }

    #ifdef MONA_TRACE
    if (mona->traceSense)
    {
        printf("Receptor firing: %s\n", id.description);
    }
    #endif

    #ifdef ACTIVITY_TRACKING
    tracker.fire = true;
    #endif
}
