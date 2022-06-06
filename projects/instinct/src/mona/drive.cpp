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
#include <math.h>

// Drive need changes caused by goal values through network.
void
Mona::drive()
{
    int i,j;
    Receptor *receptor;
    Motor *motor;
    Mediator *mediator;
    list<Neuron *>::iterator neuronItr;
    MotiveAccum motiveAccum(numNeeds);
    Enabling *enabling;
    list<Enabling *>::iterator enablingItr;

    #ifdef MONA_TRACE
    if (traceDrive)
    {
        printf("***Drive phase***\n");
        printf("Needs: "); needs.print();
    }
    #endif

    // Reset.
    for (neuronItr = receptors.begin();
        neuronItr != receptors.end(); neuronItr++)
    {
        receptor = (Receptor *)*neuronItr;
        receptor->motive = 0.0;
        receptor->motiveValid = false;
        receptor->accum[0].init(needs);
        receptor->accum[1].init(needs);
    }
    for (neuronItr = motors.begin();
        neuronItr != motors.end(); neuronItr++)
    {
        motor = (Motor *)*neuronItr;
        motor->motive = 0.0;
        motor->motiveValid = false;
        motor->accum[0].init(needs);
        motor->accum[1].init(needs);
        #ifdef ACTIVITY_TRACKING
        motor->drivers.clear();
        #endif
    }
    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        mediator->motive = 0.0;
        mediator->motiveValid = false;
        mediator->accum[0].init(needs);
        mediator->accum[1].init(needs);
    }

    // Drive.
    for (neuronItr = receptors.begin();
        neuronItr != receptors.end(); neuronItr++)
    {
        receptor = (Receptor *)*neuronItr;
        if (receptor->goals.getValue() != 0.0)
        {
            motiveAccum.init(needs);
            clearMotiveAccum();
            #ifdef ACTIVITY_TRACKING
            motiveAccum.drivers.clear();
            #endif
            receptor->drive(motiveAccum);
            setMotive();
        }
    }
    for (neuronItr = motors.begin();
        neuronItr != motors.end(); neuronItr++)
    {
        motor = (Motor *)*neuronItr;
        if (motor->goals.getValue() != 0.0)
        {
            motiveAccum.init(needs);
            clearMotiveAccum();
            #ifdef ACTIVITY_TRACKING
            motiveAccum.drivers.clear();
            #endif
            motor->drive(motiveAccum);
            setMotive();
        }
    }
    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        if (mediator->goals.getValue() != 0.0)
        {
            motiveAccum.init(needs);
            clearMotiveAccum();
            #ifdef ACTIVITY_TRACKING
            motiveAccum.drivers.clear();
            #endif
            mediator->drive(motiveAccum);
            setMotive();
        }
    }

    // Calculate final motives.
    for (neuronItr = receptors.begin();
        neuronItr != receptors.end(); neuronItr++)
    {
        receptor = (Receptor *)*neuronItr;
        if (receptor->motive < MIN_MOTIVE)
        {
            receptor->motive = MIN_MOTIVE;
        }
    }
    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        if (mediator->motive < MIN_MOTIVE)
        {
            mediator->motive = MIN_MOTIVE;
        }

        // Adjust enabling motives.
        for (i = 0, j = mediator->intermediateEnablings.size(); i < j; i++)
        {
            for (enablingItr = mediator->intermediateEnablings[i]->enablings.begin();
                enablingItr != mediator->intermediateEnablings[i]->enablings.end(); enablingItr++)
            {
                enabling = *enablingItr;
                if (mediator->motive > enabling->motive)
                {
                    enabling->motive = mediator->motive;
                }
            }
        }
        for (enablingItr = mediator->effectEnablings.enablings.begin();
            enablingItr != mediator->effectEnablings.enablings.end(); enablingItr++)
        {
            enabling = *enablingItr;
            if (mediator->motive > enabling->motive)
            {
                enabling->motive = mediator->motive;
            }
        }
        for (enablingItr = mediator->wageredEnablings.enablings.begin();
            enablingItr != mediator->wageredEnablings.enablings.end(); enablingItr++)
        {
            enabling = *enablingItr;
            if (mediator->motive > enabling->motive)
            {
                enabling->motive = mediator->motive;
            }
        }
    }
}


// Clear motive accumulators.
void
Mona::clearMotiveAccum()
{
    Receptor *receptor;
    Motor *motor;
    Mediator *mediator;
    list<Neuron *>::iterator neuronItr;

    for (neuronItr = receptors.begin();
        neuronItr != receptors.end(); neuronItr++)
    {
        receptor = (Receptor *)*neuronItr;
        receptor->accum[0].reset();
        receptor->accum[1].reset();
    }
    for (neuronItr = motors.begin();
        neuronItr != motors.end(); neuronItr++)
    {
        motor = (Motor *)*neuronItr;
        motor->accum[0].reset();
        motor->accum[1].reset();
        #ifdef ACTIVITY_TRACKING
        motor->driverWork.clear();
        #endif
    }
    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        mediator->accum[0].reset();
        mediator->accum[1].reset();
    }
}


// Set motives.
void
Mona::setMotive()
{
    int i,j;
    Receptor *receptor;
    Motor *motor;
    Mediator *mediator;
    list<Neuron *>::iterator neuronItr;
    MOTIVE motive;

    for (neuronItr = receptors.begin();
        neuronItr != receptors.end(); neuronItr++)
    {
        receptor = (Receptor *)*neuronItr;
        motive = receptor->accum[0].getValue();
        motive -= receptor->accum[1].getValue();
        if (!receptor->motiveValid || receptor->motive < motive)
        {
            receptor->motive = motive;
            receptor->motiveValid = true;
        }
    }
    for (neuronItr = motors.begin();
        neuronItr != motors.end(); neuronItr++)
    {
        motor = (Motor *)*neuronItr;
        motive = motor->accum[0].getValue();
        motive -= motor->accum[1].getValue();
        if (!motor->motiveValid || motor->motive < motive)
        {
            motor->motive = motive;
            motor->motiveValid = true;
            #ifdef ACTIVITY_TRACKING
            motor->drivers.clear();
            for (i = 0, j = motor->driverWork.size(); i < j; i++)
            {
                motor->drivers.push_back(motor->driverWork[i]);
            }
            #endif
        }
    }
    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        motive = mediator->accum[0].getValue();
        motive -= mediator->accum[1].getValue();
        if (!mediator->motiveValid || mediator->motive < motive)
        {
            mediator->motive = motive;
            mediator->motiveValid = true;
        }
    }
}


// Neuron drive.
void
Mona::Neuron::drive(MotiveAccum &motiveAccum)
{
    Mediator *mediator;
    Neuron *cause;
    struct Notify *notify;
    int i,j;
    MOTIVE m;
    WEIGHT eventWeight,eventWeightSum,superWeight;
    MotiveAccum a;
    #ifdef ACTIVITY_TRACKING
    int p,q;
    #endif

    // Validate mediator drive.
    if (type == MEDIATOR)
    {
        mediator = (Mediator *)this;
        if (mediator->enabler && !motiveAccum.enabler) return;
        if (!mediator->enabler && motiveAccum.enabler) return;
    }

    // Accumulate need change due to goal value.
    if (motiveAccum.enabler) motiveAccum.accumGoals(goals);

    // Store maximum motive.
    m = motiveAccum.getValue();
    if (fabs(m) < MIN_MOTIVE) return;
    if (motiveAccum.enabler)
    {
        if ((m >= 0.0 && accum[0].getValue() < m) ||
            (m < 0.0 && accum[0].getValue() > m))
        {
            accum[0].loadNeeds(motiveAccum);
        } else return;
    }
    else
    {
        if ((m >= 0.0 && accum[1].getValue() < m) ||
            (m < 0.0 && accum[1].getValue() > m))
        {
            accum[1].loadNeeds(motiveAccum);
        } else return;
    }
    #ifdef MONA_TRACE
    if (mona->traceDrive)
    {
        printf("Drive %d (%s), motive=%f\n",id.value, id.description, m);
    }
    #endif
    #ifdef ACTIVITY_TRACKING
    if (type == MOTOR)
    {
        Motor *motor = (Motor *)this;
        motor->driverWork.clear();
        for (p = 0, q = motiveAccum.drivers.size(); p < q; p++)
        {
            motor->driverWork.push_back(motiveAccum.drivers[p]);
        }
    }
    for (p = 0, q = motiveAccum.drivers.size(); p < q; p++)
    {
        a.drivers.push_back(motiveAccum.drivers[p]);
    }
    a.drivers.push_back(this);
    #endif

    // Drive terminates on motor neurons.
    if (type == MOTOR) return;

    if (type == MEDIATOR)
    {
        eventWeightSum = 0.0;
        mediator = (Mediator *)this;

        // Distribute motive to cause events in proportion to
        // enablement. This represents enablement waiting for the
        // cause to fire.
        eventWeight = mediator->getDriveWeight(CAUSE_EVENT);
        if (eventWeight > 0.0)
        {
            eventWeightSum += eventWeight;
            a.config(motiveAccum, eventWeight * DRIVE_ATTENUATION);
            a.enabler = motiveAccum.enabler;
            for (i = 0, j = mediator->causes.size(); i < j; i++)
            {
                // Drive causes that have not fired.
                if (mediator->pendingEnablings[i].size() == 0)
                {
                    cause = mediator->causes[i];
                    cause->drive(a);
                }
            }
        }

        // Distribute motive to intermediate event in proportion to
        // enablement distributed by previous event firings.
        for (i = 0, j = mediator->intermediateEnablings.size(); i < j; i++)
        {
            // Proportionally drive intermediate event.
            eventWeight = mediator->getDriveWeight(INTERMEDIATE_EVENT, i);
            if (eventWeight > 0.0)
            {
                eventWeightSum += eventWeight;
                a.config(motiveAccum, eventWeight * DRIVE_ATTENUATION);
                a.enabler = motiveAccum.enabler;
                mediator->intermediates[i]->drive(a);
            }
        }

        // Distribute motive to effect event in proportion to
        // enablement distributed by previous event firings.
        eventWeight = mediator->getDriveWeight(EFFECT_EVENT);
        if (eventWeight > 0.0)
        {
            eventWeightSum += eventWeight;
            a.config(motiveAccum, eventWeight * DRIVE_ATTENUATION);
            a.enabler = motiveAccum.enabler;
            mediator->effect->drive(a);
        }

    }
    else
    {

        // Neuron is not mediator.
        eventWeightSum = 0.0;
    }

    // Distribute motive to super-mediators.
    superWeight = 1.0 - eventWeightSum;
    if (superWeight > 0.0)
    {
        superWeight *= DRIVE_ATTENUATION;
        for (i = 0, j = notifyList.size(); i < j; i++)
        {
            notify = notifyList[i];
            if (notify->eventType != EFFECT_EVENT) continue;
            mediator = notify->mediator;

            // Reverse sense of enabler for disabling mediators.
            a.config(motiveAccum, superWeight);
            if (mediator->enabler)
            {
                a.enabler = motiveAccum.enabler;
            }
            else
            {
                a.enabler = !motiveAccum.enabler;
            }
            mediator->driveCause(a);
        }
    }
}


// Drive mediator cause.
void
Mona::Mediator::driveCause(MotiveAccum &motiveAccum)
{
    Mediator *mediator;
    Neuron *cause;
    struct Notify *notify;
    int i,j;
    MOTIVE m;
    WEIGHT eventWeight,eventWeightSum,superWeight;
    MotiveAccum a;
    #ifdef ACTIVITY_TRACKING
    int p,q;
    #endif

    #ifdef ACTIVITY_TRACKING
    for (p = 0, q = motiveAccum.drivers.size(); p < q; p++)
    {
        accum.drivers.push_back(motiveAccum.drivers[p]);
    }
    accum.drivers.push_back(this);
    #endif

    // Store maximum motive.
    m = motiveAccum.getValue();
    if (fabs(m) < MIN_MOTIVE) return;
    if (motiveAccum.enabler)
    {
        if ((m >= 0.0 && accum[0].getValue() < m) ||
            (m < 0.0 && accum[0].getValue() > m))
        {
            accum[0].loadNeeds(motiveAccum);
        } else return;
    }
    else
    {
        if ((m >= 0.0 && accum[1].getValue() < m) ||
            (m < 0.0 && accum[1].getValue() > m))
        {
            accum[1].loadNeeds(motiveAccum);
        } else return;
    }
    #ifdef MONA_TRACE
    if (mona->traceDrive)
    {
        printf("Drive cause %d (%s), motive=%f\n",id.value, id.description, m);
    }
    #endif

    // Distribute motive to cause events in proportion to maximum
    // enablement. This represents enablement waiting for the
    // cause to fire.
    if (motiveAccum.enabler)
    {
        eventWeightSum = 0.0;
        eventWeight = getDriveWeight(CAUSE_EVENT);
        if (eventWeight > 0.0)
        {
            eventWeightSum += eventWeight;
            a.config(motiveAccum, eventWeight * DRIVE_ATTENUATION);
            a.enabler = motiveAccum.enabler;
            for (i = 0, j = causes.size(); i < j; i++)
            {
                // Drive causes that have not fired.
                if (pendingEnablings[i].size() == 0)
                {
                    cause = causes[i];
                    cause->drive(a);
                }
            }
        }

        for (i = 0, j = intermediateEnablings.size(); i < j; i++)
        {
            // Proportionally drive intermediate event.
            eventWeight = getDriveWeight(INTERMEDIATE_EVENT, i);
            if (eventWeight > 0.0)
            {
                eventWeightSum += eventWeight;
                a.config(motiveAccum, eventWeight * DRIVE_ATTENUATION);
                a.enabler = motiveAccum.enabler;
                intermediates[i]->drive(a);
            }
        }

        superWeight = 1.0 - eventWeightSum;

    }
    else
    {

        // Disabling.
        superWeight = 1.0;
    }

    // Distribute motive to super-mediators.
    if (superWeight > 0.0)
    {
        superWeight *= DRIVE_ATTENUATION;
        for (i = 0, j = notifyList.size(); i < j; i++)
        {
            notify = notifyList[i];
            if (notify->eventType != EFFECT_EVENT) continue;
            mediator = notify->mediator;

            // Reverse sense of enabler for disabling mediators.
            a.config(motiveAccum, superWeight);
            if (mediator->enabler)
            {
                a.enabler = motiveAccum.enabler;
            }
            else
            {
                a.enabler = !motiveAccum.enabler;
            }
            mediator->driveCause(a);
        }
    }
}


// Get drive weight.
Mona::WEIGHT Mona::Mediator::getDriveWeight(EVENT_TYPE eventType, int eventIndex)
{
    int i,j;
    ENABLEMENT enablement,eventEnablement,posProb,negProb,e,e2;
    double eventFraction;
    bool useWageredSet;
    struct Notify *notify;
    Mediator *mediator;
    WEIGHT driveWeight;

    enablement = getBaseEnablement();
    switch(eventType)
    {
        case CAUSE_EVENT:
            eventEnablement = baseEnablement;
            if (enablement > 0.0)
            {
                eventFraction = eventEnablement / enablement;
            }
            else
            {
                eventFraction = 0.0;
            }
            useWageredSet = false;
            break;
        case INTERMEDIATE_EVENT:
            eventEnablement = intermediateEnablings[eventIndex]->getValue();
            if (enablement > 0.0)
            {
                eventEnablement *= (intermediateEnablements[eventIndex] / enablement);
                eventFraction = eventEnablement / enablement;
            }
            else
            {
                eventFraction = 0.0;
            }
            useWageredSet = true;
            break;
        case EFFECT_EVENT:
            eventEnablement = effectEnablings.getValue();
            if (enablement > 0.0)
            {
                eventEnablement *= (effectEnablement / enablement);
                eventFraction = eventEnablement / enablement;
            }
            else
            {
                eventFraction = 0.0;
            }
            useWageredSet = true;
            break;
    }

    posProb = 1.0 - (eventEnablement / MAX_ENABLEMENT);
    negProb = -1.0;
    for (i = 0, j = notifyList.size(); i < j; i++)
    {
        notify = notifyList[i];
        if (notify->eventType != EFFECT_EVENT) continue;
        mediator = notify->mediator;
        if (useWageredSet)
        {
            e = mediator->wageredEnablings.getValue();
        }
        else
        {
            e = mediator->effectEnablings.getValue();
        }
        e2 = mediator->getBaseEnablement();
        if (e2 > 0.0)
        {
            e *= mediator->effectEnablement / e2;
        }
        e /= MAX_ENABLEMENT;
        if (e < 0.0) e = 0.0;
        if (e > 1.0) e = 1.0;
        e = 1.0 - e;
        if (mediator->enabler)
        {
            posProb *= e;
        }
        else
        {
            if (negProb < 0.0) negProb = 1.0;
            negProb *= e;
        }
    }
    posProb = 1.0 - posProb;
    if (negProb < 0.0)
    {
        driveWeight = posProb * eventFraction;
    }
    else
    {
        driveWeight = posProb * negProb * eventFraction;
    }
    return driveWeight;
}
