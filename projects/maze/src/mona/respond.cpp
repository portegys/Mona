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

#include "mona.hpp"

// Respond.
void
Mona::respond()
{
    Motor *motor;
    list<Neuron *>::iterator neuronItr;
    RESPONSE_POTENTIAL sum,max,min;
    int i;
    #ifdef ACTIVITY_TRACKING
    int p,q;
    Neuron *neuron;
    #endif

    #ifdef MONA_TRACE
    if (traceRespond)
    {
        printf("***Respond phase***\n");
    }
    #endif

    // Get response potentials from motor motives.
    for (i = 0; i <= maxResponse; i++)
    {
        responsePotentials[i] = 0.0;
    }
    for (neuronItr = motors.begin();
        neuronItr != motors.end(); neuronItr++)
    {
        motor = (Motor *)*neuronItr;
        if (!responseInhibitors[motor->response])
        {
            responsePotentials[motor->response] += motor->motive;
        }
    }

    // Incorporate override potentials.
    // These are primarily used for training.
    for (i = 0; i <= maxResponse; i++)
    {
        if (!responseInhibitors[i])
        {
            responsePotentials[i] += responseOverridePotentials[i];
        }
    }

    // Normalize response potentials.
    min = 0.0;
    for (i = 0; i <= maxResponse; i++)
    {
        if (!responseInhibitors[i])
        {
            if (responsePotentials[i] < min)
            {
                min = responsePotentials[i];
            }
        }
    }
    sum = 0.0;
    for (i = 0; i <= maxResponse; i++)
    {
        if (!responseInhibitors[i])
        {
            responsePotentials[i] -= min;
            sum += responsePotentials[i];
        }
    }
    if (sum > 0.0)
    {
        for (i = 0; i <= maxResponse; i++)
        {
            responsePotentials[i] /= sum;
        }
    }

    // Incorporate randomness.
    for (i = 0; i <= maxResponse; i++)
    {
        if (!responseInhibitors[i])
        {
            responsePotentials[i] =
                (responsePotentials[i] * (1.0 - responseRandomness)) +
                (random.RAND_PROB() * responseRandomness);
        }
    }

    // Select maximum response potential.
    for (i = 0, response = -1; i <= maxResponse; i++)
    {
        if (!responseInhibitors[i])
        {
            if (response == -1 || responsePotentials[i] > max)
            {
                response = i;
                max = responsePotentials[i];
            }
        }
    }

    // Response overridden?
    if (responseOverride != -1)
    {
        response = responseOverride;
    }

    // Firing responding motor.
    for (neuronItr = motors.begin();
        neuronItr != motors.end(); neuronItr++)
    {
        motor = (Motor *)*neuronItr;
        if (motor->response == response)
        {
            motor->firingStrength = 1.0;

            #ifdef MONA_TRACE
            if (traceRespond)
            {
                printf("Motor firing: %s\n", motor->id.description);
            }
            #endif

            #ifdef ACTIVITY_TRACKING
            motor->tracker.fire = true;
            motor->tracker.drive = true;
            for (p = 0, q = motor->drivers.size(); p < q; p++)
            {
                neuron = motor->drivers[p];
                neuron->tracker.drive = true;
            }
            #endif
        }
        else
        {
            motor->firingStrength = 0.0;

            // Expire enabling on non-firing motor.
            motor->expireMotorEnabling();
        }
    }

    #ifdef MONA_TRACE
    if (traceRespond)
    {
        printf("Response = %d\n", response);
    }
    #endif
}
