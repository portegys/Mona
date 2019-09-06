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
#include <math.h>

// Enablement processing.
void
Mona::enable()
{
    int i,j;
    Receptor *receptor;
    Motor *motor;
    Mediator *mediator;
    list<Neuron *>::iterator neuronItr;
    EnablingSet *enablingSet;
    struct Notify *notify;
    struct ElemEvent event;
    vector<struct ElemEvent> events;

    #ifdef MONA_TRACE
    if (traceEnable)
    {
        printf("***Enable phase***\n");
    }
    #endif

    // Clear mediators.
    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        mediator->firingStrength = 0.0;
        for (i = 0, j = mediator->intermediateEnablings.size(); i < j; i++)
        {
            enablingSet = mediator->intermediateEnablings[i];
            enablingSet->clearFlags();
        }
        mediator->effectEnablings.clearFlags();
        mediator->wageredEnablings.clearFlags();
    }

    // Notify mediators of motor firing events.
    for (neuronItr = motors.begin();
        neuronItr != motors.end(); neuronItr++)
    {
        motor = (Motor *)*neuronItr;
        if (motor->firingStrength > 0.0)
        {
            events.clear();
            event.id = motor->id.value;
            event.timestamp = eventClock;
            events.push_back(event);
            for (i = 0, j = motor->notifyList.size(); i < j; i++)
            {
                notify = motor->notifyList[i];

                (notify->mediator)->eventFiring(notify->eventType,
                    motor->firingStrength, notify->eventIndex,
                    eventClock, events);
            }
        }
    }

    // Notify mediators of receptor firing events.
    for (neuronItr = receptors.begin();
        neuronItr != receptors.end(); neuronItr++)
    {
        receptor = (Receptor *)*neuronItr;
        events.clear();
        if (receptor->firingStrength > 0.0)
        {
            event.id = receptor->id.value;
            event.timestamp = eventClock;
            events.push_back(event);
        }
        for (i = 0, j = receptor->notifyList.size(); i < j; i++)
        {
            notify = receptor->notifyList[i];
            if (notify->eventType == EFFECT_EVENT ||
                receptor->firingStrength > 0.0)
            {
                (notify->mediator)->eventFiring(notify->eventType,
                    receptor->firingStrength, notify->eventIndex,
                    eventClock, events);
            }
        }
    }
}


// Detect firing of mediator event.
void
Mona::Mediator::eventFiring(EVENT_TYPE eventType, WEIGHT strength,
int eventIndex, TIME eventBegin, vector<struct ElemEvent> &events)
{
    int i,j;
    struct Notify *notify;
    Enabling *enabling,*enabling2;
    list<Enabling *>::iterator enablingItr;
    ENABLEMENT enablement;
    WEIGHT transfer,notifyStrength;
    list<Enabling *> tmpList;
    vector<struct ElemEvent> emptyEvents;

    switch(eventType)
    {
        case CAUSE_EVENT:

            // Enable next event.
            enableEvent(strength, eventIndex, eventBegin, events);
            return;

        case INTERMEDIATE_EVENT:

            // Transfer enablings to next event.
            enablement = 0.0;
            for (enablingItr = intermediateEnablings[eventIndex]->enablings.begin();
                enablingItr != intermediateEnablings[eventIndex]->enablings.end(); enablingItr++)
            {
                enabling = *enablingItr;
                if (enabling->newInSet) continue;
                enablement += enabling->value;
            }
            if (strength >= (enablement / MAX_ENABLEMENT))
            {
                transfer = 1.0;
            }
            else
            {
                if (enablement > 0.0)
                {
                    transfer = (MAX_ENABLEMENT * strength) / enablement;
                }
                else
                {
                    transfer = 0.0;
                }
            }
            if (transfer > 0.0 && enablement > 0.0)
            {
                for (enablingItr = intermediateEnablings[eventIndex]->enablings.begin();
                    enablingItr != intermediateEnablings[eventIndex]->enablings.end(); enablingItr++)
                {
                    enabling = *enablingItr;
                    if (enabling->newInSet || enabling->value == 0.0) continue;
                    enabling2 = enabling->clone();
                    enabling2->appendEvents(events);
                    if (intermediates[eventIndex]->type == MOTOR)
                    {
                        enabling2->age = 1;
                    }
                    else
                    {
                        enabling2->age = 0;
                    }
                    enabling2->value *= transfer;
                    enabling->value *= (1.0 - transfer);
                    if (eventIndex == intermediateEnablings.size() - 1)
                    {
                        if (effect->type == MEDIATOR)
                        {
                            effectEnablings.insert(enabling2);
                        }
                        else
                        {
                            enabling2->effectBegin = mona->eventClock;
                            wageredEnablings.insert(enabling2);
                        }
                    }
                    else
                    {
                        intermediateEnablings[eventIndex+1]->insert(enabling2);
                    }
                    if (intermediates[eventIndex]->type == MOTOR)
                    {
                        enabling2->newInSet = false;
                    }
                }
            }
            return;

        case WAGER_EVENT:

            // If wagering event is re-firing, assume that
            // wagered enablings are timing-out.
            enablement = 0.0;
            for (enablingItr = wageredEnablings.enablings.begin();
                enablingItr != wageredEnablings.enablings.end(); enablingItr++)
            {
                enabling = *enablingItr;
                if (enabling->newInSet) continue;
                enablement += enabling->value;
            }
            if (strength >= (enablement / MAX_ENABLEMENT))
            {
                transfer = 1.0;
            }
            else
            {
                if (enablement > 0.0)
                {
                    transfer = (MAX_ENABLEMENT * strength) / enablement;
                }
                else
                {
                    transfer = 0.0;
                }
            }
            if (transfer > 0.0 && enablement > 0.0)
            {
                tmpList.clear();
                for (enablingItr = wageredEnablings.enablings.begin();
                    enablingItr != wageredEnablings.enablings.end(); enablingItr++)
                {
                    enabling = *enablingItr;
                    if (enabling->newInSet || enabling->value == 0.0) continue;
                    enabling2 = enabling->clone();
                    enabling2->value *= transfer;
                    enabling->value *= (1.0 - transfer);
                    tmpList.push_back(enabling2);
                }
                for (enablingItr = tmpList.begin();
                    enablingItr != tmpList.end(); enablingItr++)
                {
                    enabling = *enablingItr;
                    enabling->age = mona->eventTimers[level][enabling->timerIndex];
                    wageredEnablings.insert(enabling);
                    enabling->newInSet = false;
                }
            }

            // Wager enabling.
            enablement = 0.0;
            for (enablingItr = effectEnablings.enablings.begin();
                enablingItr != effectEnablings.enablings.end(); enablingItr++)
            {
                enabling = *enablingItr;
                if (enabling->newInSet) continue;
                enablement += enabling->value;
            }
            if (strength >= (enablement / MAX_ENABLEMENT))
            {
                transfer = 1.0;
            }
            else
            {
                if (enablement > 0.0)
                {
                    transfer = (MAX_ENABLEMENT * strength) / enablement;
                }
                else
                {
                    transfer = 0.0;
                }
            }
            if (transfer > 0.0 && enablement > 0.0)
            {
                for (enablingItr = effectEnablings.enablings.begin();
                    enablingItr != effectEnablings.enablings.end(); enablingItr++)
                {
                    enabling = *enablingItr;
                    if (enabling->newInSet || enabling->value == 0.0) continue;
                    enabling2 = enabling->clone();
                    enabling2->value *= transfer;
                    enabling->value *= (1.0 - transfer);
                    enabling2->effectBegin = eventBegin;
                    wageredEnablings.insert(enabling2);
                }
            }
            return;

        case EFFECT_EVENT:

            // Determine upward firing strength.
            causeBegin = 0;
            enablement = 0.0;
            enabling = NULL;
            if (enabler)
            {
                // Find best set of enablings that have a common event stream.
                for (enablingItr = wageredEnablings.enablings.begin();
                    enablingItr != wageredEnablings.enablings.end(); enablingItr++)
                {
                    enabling2 = *enablingItr;
                    if (enabling2->newInSet) continue;
                    if (enabling2->value > enablement)
                    {
                        enablement = enabling2->value;
                        enabling = enabling2;
                        causeBegin = enabling->causeBegin;
                    }
                }
                if (enablement > 0.0)
                {
                    enabling->effectWager = true;
                    enabling->appendEvents(events);
                    for (enablingItr = wageredEnablings.enablings.begin();
                        enablingItr != wageredEnablings.enablings.end(); enablingItr++)
                    {
                        enabling2 = *enablingItr;
                        if (enabling2->newInSet) continue;
                        if (enabling2 == enabling)continue;
                        if ((j = enabling2->events.size()) != enabling->events.size()) continue;
                        for (i = 0; i < j; i++)
                        {
                            if (enabling2->events[i].id != enabling->events[i].id ||
                                enabling2->events[i].timestamp != enabling->events[i].timestamp) break;
                        }
                        if (i < j) continue;
                        enablement += enabling2->value;
                        enabling2->effectWager = true;
                        enabling2->appendEvents(events);
                    }
                }
                if (strength >= (enablement / MAX_ENABLEMENT))
                {
                    transfer = 1.0;
                }
                else
                {
                    if (enablement > 0.0)
                    {
                        transfer = (MAX_ENABLEMENT * strength) / enablement;
                    }
                    else
                    {
                        transfer = 0.0;
                    }
                }
                firingStrength = transfer * enablement;
                notifyStrength = firingStrength / getBaseEnablement();

            }                                     // disabler
            else
            {

                for (enablingItr = wageredEnablings.enablings.begin();
                    enablingItr != wageredEnablings.enablings.end(); enablingItr++)
                {
                    enabling2 = *enablingItr;
                    if (enabling2->newInSet) continue;

                    // Timed out wager?
                    if (enabling2->age > mona->eventTimers[level][enabling2->timerIndex])
                    {
                        enablement += enabling2->value;
                        enabling2->effectWager = true;
                        enabling2->appendEvents(events);
                        if (enabling == NULL ||
                            enabling->causeBegin < enabling2->causeBegin)
                        {
                            enabling = enabling2;
                            causeBegin = enabling->causeBegin;
                        }
                    }
                }
                if ((1.0 - strength) >= (enablement / MAX_ENABLEMENT))
                {
                    transfer = 1.0;
                }
                else
                {
                    if (enablement > 0.0)
                    {
                        transfer = (MAX_ENABLEMENT * (1.0 - strength)) / enablement;
                    }
                    else
                    {
                        transfer = 0.0;
                    }
                }
                firingStrength = transfer * enablement;
                notifyStrength = firingStrength / getBaseEnablement();
            }

        #ifdef MONA_TRACE
            if (firingStrength > 0.0 && mona->traceEnable)
            {
                printf("Mediator firing: %s\n", id.description);
            }
        #endif

        #ifdef ACTIVITY_TRACKING
            if (firingStrength > 0.0)
            {
                tracker.fire = true;
            }
        #endif

            // Notify super-mediators.
            for (i = 0, j = notifyList.size(); i < j; i++)
            {
                notify = notifyList[i];
                if (notify->eventType == EFFECT_EVENT || firingStrength > 0.0)
                {
                    if (enabling != NULL)
                    {
                        (notify->mediator)->eventFiring(notify->eventType, notifyStrength,
                            notify->eventIndex, causeBegin, enabling->events);
                    }
                    else
                    {
                        (notify->mediator)->eventFiring(notify->eventType, notifyStrength,
                            notify->eventIndex, causeBegin, emptyEvents);
                    }

                }
            }
            return;
    }
}


// Cause firing: enable next event.
void
Mona::Mediator::enableEvent(WEIGHT strength,
int causeIndex, TIME causeBegin, vector<struct ElemEvent> &events)
{
    int i,j;
    struct PendingEnabling *penabling;
    list<struct PendingEnabling *>::iterator pendingEnablingItr;
    vector<struct PendingEnabling *> accumList;
    list<struct PendingEnabling *> tmpList;

    // Add pending enabling.
    penabling = new struct PendingEnabling;
    assert(penabling != NULL);
    penabling->causeIndex = causeIndex;
    penabling->strength = strength;
    penabling->age = 0;
    penabling->causeBegin = causeBegin;
    penabling->events = events;
    pendingEnablings[causeIndex].push_back(penabling);

    // All causes have fired?
    for (i = 0, j = pendingEnablings.size(); i < j; i++)
    {
        if (pendingEnablings[i].size() == 0) return;
    }

    // Accumulate enablings from combinations of cause events.
    accumList.resize(pendingEnablings.size());
    for (pendingEnablingItr = pendingEnablings[0].begin();
        pendingEnablingItr != pendingEnablings[0].end(); pendingEnablingItr++)
    {
        penabling = *pendingEnablingItr;
        accumList[0] = penabling;
        accumEnabling(1, accumList);
    }

    // Delete depleted pending enablings.
    for (i = 0, j = pendingEnablings.size(); i < j; i++)
    {
        tmpList.clear();
        for (pendingEnablingItr = pendingEnablings[i].begin();
            pendingEnablingItr != pendingEnablings[i].end(); pendingEnablingItr++)
        {
            penabling = *pendingEnablingItr;
            if (penabling->strength <= 0.0)
            {
                delete penabling;
            }
            else
            {
                tmpList.push_back(penabling);
            }
        }
        pendingEnablings[i].clear();
        for (pendingEnablingItr = tmpList.begin();
            pendingEnablingItr != tmpList.end(); pendingEnablingItr++)
        {
            penabling = *pendingEnablingItr;
            pendingEnablings[i].push_back(penabling);
        }
    }
}


// Accumulate pending enablings.
void
Mona::Mediator::accumEnabling(int causeIndex,
vector<struct PendingEnabling *> &accumList)
{
    struct PendingEnabling *penabling;
    list<struct PendingEnabling *>::iterator pendingEnablingItr;

    // Attach enabling to next event?
    if (causeIndex == causes.size())
    {
        attachEnabling(accumList);
        return;
    }

    // Continue building combinations.
    for (pendingEnablingItr = pendingEnablings[causeIndex].begin();
        pendingEnablingItr != pendingEnablings[causeIndex].end(); pendingEnablingItr++)
    {
        penabling = *pendingEnablingItr;
        accumList[causeIndex] = penabling;
        accumEnabling(causeIndex + 1, accumList);
    }
}


// Attach enabling to event.
void
Mona::Mediator::attachEnabling(vector<struct PendingEnabling *> &accumList)
{
    int i,j,p,q,r,s;
    Enabling *enabling;
    ENABLEMENT delta,enablement;
    struct PendingEnabling *penabling;
    WEIGHT strength,notifyStrength;
    TIME age,causeBegin;
    struct Notify *notify;
    vector<struct ElemEvent> emptyEvents;
    list<struct ElemEvent> sortedEvents;
    list<struct ElemEvent>::iterator eventItr,earlyEventItr;
    struct ElemEvent event,earlyEvent;

    // Determine enabling properties.
    strength = 0.0;
    age = 0;
    causeBegin = mona->eventClock;
    for (i = 0, j = accumList.size(); i < j; i++)
    {
        penabling = accumList[i];
        if (penabling->strength <= 0.0) return;
        strength += penabling->strength;
        if (age < penabling->age) age = penabling->age;
        if (causeBegin > penabling->causeBegin) causeBegin = penabling->causeBegin;
    }
    strength /= (double)i;

    // Deplete strength of pending enablings.
    for (i = 0, j = accumList.size(); i < j; i++)
    {
        penabling = accumList[i];
        penabling->strength -= strength;
        if (penabling->strength < 0.0)
        {
            penabling->strength = 0.0;
        }
    }

    // Determine fraction of base enablement to attach.
    if (strength >= (baseEnablement / MAX_ENABLEMENT))
    {
        delta = baseEnablement;
    }
    else
    {
        delta = MAX_ENABLEMENT * strength;
    }
    notifyStrength = delta / getBaseEnablement();
    baseEnablement -= delta;
    #ifdef ACTIVITY_TRACKING
    tracker.enable = true;
    #endif

    // Distribute enablement to next neuron.
    for (i = 0; i < mona->numEventTimers && delta > 0.0; i++)
    {
        enablement = delta * timedWagerWeights.get(i);
        if (age < mona->eventTimers[level][i])
        {
            enabling = new Enabling(enablement, motive,
                mona->needs, age, i, causeBegin, INVALID_TIME);
            assert(enabling != NULL);

            // Add events to enabling, sorted by time and id.
            sortedEvents.clear();
            for (p = 0, q = accumList.size(); p < q; p++)
            {
                penabling = accumList[p];
                for (r = 0, s = penabling->events.size(); r < s; r++)
                {
                    sortedEvents.push_back(penabling->events[r]);
                }
            }
            while (sortedEvents.size() > 0)
            {
                earlyEvent.id = -1;
                for (eventItr = sortedEvents.begin();
                    eventItr != sortedEvents.end(); eventItr++)
                {
                    event = *eventItr;
                    if (earlyEvent.id == -1 ||
                        earlyEvent.timestamp > event.timestamp ||
                        (earlyEvent.timestamp == event.timestamp &&
                        earlyEvent.id > event.id))
                    {
                        earlyEvent = event;
                        earlyEventItr = eventItr;
                    }
                }
                sortedEvents.erase(earlyEventItr);
                enabling->appendEvent(earlyEvent);
            }

            if (intermediates.size() > 0)
            {
                intermediateEnablings[0]->insert(enabling);
                #ifdef ACTIVITY_TRACKING
                intermediates[0]->tracker.enable = true;
                #endif
            }
            else
            {
                if (effect->type == MEDIATOR)
                {
                    effectEnablings.insert(enabling);
                }
                else
                {
                    enabling->effectBegin = mona->eventClock;
                    wageredEnablings.insert(enabling);
                }
                #ifdef ACTIVITY_TRACKING
                effect->tracker.enable = true;
                #endif
            }
        }
        else
        {
            baseEnablement += enablement;
        }
    }

    // Notify super-mediators of wagering condition.
    if (notifyStrength > 0.0)
    {
        for (i = 0, j = notifyList.size(); i < j; i++)
        {
            notify = notifyList[i];
            if (notify->eventType == EFFECT_EVENT)
            {
                (notify->mediator)->eventFiring(WAGER_EVENT, notifyStrength, -1, causeBegin, emptyEvents);
            }
        }
    }
}


// Get mediating enablement.
Mona::ENABLEMENT
Mona::Neuron::getMediatingEnablement()
{
    int i,j;
    ENABLEMENT enablement;
    struct Notify *notify;
    Mediator *mediator;

    enablement = 0.0;
    for (i = 0, j = notifyList.size(); i < j; i++)
    {
        notify = notifyList[i];
        if (notify->eventType != EFFECT_EVENT) continue;
        mediator = notify->mediator;
        if (mediator->enabler)
        {
            enablement += mediator->effectEnablings.getValue();
        }
        else
        {
            enablement -= mediator->effectEnablings.getValue();
        }
    }
    if (enablement > MAX_ENABLEMENT) enablement = MAX_ENABLEMENT;
    if (enablement < -MAX_ENABLEMENT) enablement = -MAX_ENABLEMENT;
    if (enablement > -NEARLY_ZERO && enablement < NEARLY_ZERO) enablement = 0.0;
    return enablement;
}


// Get mediating wagered enablement.
Mona::ENABLEMENT
Mona::Neuron::getMediatingWageredEnablement()
{
    int i,j;
    ENABLEMENT enablement;
    struct Notify *notify;
    Mediator *mediator;
    Enabling *enabling;
    list<Enabling *>::iterator listItr;

    enablement = 0.0;
    for (i = 0, j = notifyList.size(); i < j; i++)
    {
        notify = notifyList[i];
        if (notify->eventType != EFFECT_EVENT) continue;
        mediator = notify->mediator;
        for (listItr = mediator->wageredEnablings.enablings.begin();
            listItr != mediator->wageredEnablings.enablings.end(); listItr++)
        {
            enabling = *listItr;
            if (mediator->enabler)
            {
                enablement += enabling->value;
            }
            else
            {
                enablement -= enabling->value;
            }
        }
    }
    if (enablement > MAX_ENABLEMENT) enablement = MAX_ENABLEMENT;
    if (enablement < -MAX_ENABLEMENT) enablement = -MAX_ENABLEMENT;
    if (enablement > -NEARLY_ZERO && enablement < NEARLY_ZERO) enablement = 0.0;
    return enablement;
}
