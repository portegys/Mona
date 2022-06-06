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

// Learn environment:
// Firing unpredicted effects prompt the creation of mediators.
// Expired predicted effects prompt cause conjunction additions
// and new disabling mediators to explain the failures.
void
Mona::learn()
{
    int i,j,k;
    LearningEvent *learningEvent;
    list<LearningEvent *>::iterator learningEventItr;
    list<LearningEvent *> tmpList;
    Motor *motor;
    Receptor *receptor;
    Mediator *mediator;
    list<Neuron *>::iterator neuronItr;
    SENSOR *sensorValues;

    #ifdef MONA_TRACE
    if (traceLearn)
    {
        printf("***Learn phase***\n");
    }
    #endif

    // Purge expired and absent events.
    for (i = 0, j = learningEvents.size(); i < j; i++)
    {
        tmpList.clear();
        for (learningEventItr = learningEvents[i].begin();
            learningEventItr != learningEvents[i].end(); learningEventItr++)
        {
            learningEvent = *learningEventItr;
            if (learningEvent->strength > 0.0)
            {
                k = i + MAX_MEDIATOR_SKEW;
                if (k >= eventTimers.size()) k = eventTimers.size() - 1;
                if ((eventClock - learningEvent->timestamp) <=
                    (eventTimers[k][eventTimers[k].size() - 1] *
                    (MAX_MEDIATOR_EVENTS - 1)))
                {
                    tmpList.push_back(learningEvent);
                }
                else
                {
                    delete learningEvent;
                }
            }
            else
            {
                delete learningEvent;
            }
        }
        learningEvents[i].clear();
        for (learningEventItr = tmpList.begin();
            learningEventItr != tmpList.end(); learningEventItr++)
        {
            learningEvent = *learningEventItr;
            learningEvents[i].push_back(learningEvent);
        }
    }

    // Create new receptors to match sensors values.
    sensorValues = new SENSOR[numSensors];
    assert(sensorValues != NULL);
    for (i = 0; i < numSensors; i++)
    {
        sensorValues[i] = sensors[i];
    }
    #ifdef NEVER
    for (i = 0; i < numSensors; i++)
    {
        if (sensorValues[i] == 0)
        {
            if (random.RAND_CHANCE(SENSOR_MASK_NEGATIVE_PROBABILITY))
            {
                sensorValues[i] = DONT_CARE;
            }
        }
        else
        {
            if (random.RAND_CHANCE(SENSOR_MASK_POSITIVE_PROBABILITY))
            {
                sensorValues[i] = DONT_CARE;
            }
        }
    }
    #endif
    for (i = j = 0; i < numSensors; i++)
    {
        if (sensorValues[i] != DONT_CARE) j++;
    }
    for (neuronItr = receptors.begin();
        neuronItr != receptors.end() && j > 0; neuronItr++)
    {
        if (((Receptor *)*neuronItr)->isDuplicate(sensorValues))
        {
            break;
        }
    }
    if (j > 0 && neuronItr == receptors.end())
    {
        receptor = newReceptor(sensorValues, NULL);
        receptor->motive = MIN_MOTIVE;
        receptor->firingStrength = (double)j / (double)numSensors;
        #ifdef MONA_TRACE
        if (traceLearn)
        {
            printf("Create receptor: ");
            receptor->print();
            printf("\n");
        }
        #endif
    }
    delete sensorValues;

    // Time-stamp and store significant events.
    for (neuronItr = receptors.begin();
        neuronItr != receptors.end(); neuronItr++)
    {
        receptor = (Receptor *)*neuronItr;
        if (receptor->motive > 0.0 || receptor->firingStrength > 0.0)
        {
            learningEvent = new LearningEvent(receptor, eventClock);
            assert(learningEvent != NULL);
            learningEvents[0].push_back(learningEvent);
        }
    }
    for (neuronItr = motors.begin();
        neuronItr != motors.end(); neuronItr++)
    {
        motor = (Motor *)*neuronItr;
        if (motor->motive > 0.0 || motor->firingStrength > 0.0)
        {
            learningEvent = new LearningEvent(motor, eventClock);
            assert(learningEvent != NULL);
            learningEvents[0].push_back(learningEvent);
        }
    }
    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        if (mediator->motive > 0.0 || mediator->firingStrength > 0.0)
        {
            learningEvent = new LearningEvent(mediator, eventClock);
            assert(learningEvent != NULL);
            learningEvents[mediator->level + 1].push_back(learningEvent);
        }
    }

    // Create new mediators based on observed events.
    for (i = 0, j = learningEvents.size() - 1; i < j; i++)
    {
        for (learningEventItr = learningEvents[i].begin();
            learningEventItr != learningEvents[i].end(); learningEventItr++)
        {
            learningEvent = *learningEventItr;
            if (learningEvent->timestamp == eventClock &&
                learningEvent->motive > 0.0 &&
                learningEvent->neuron->type != MOTOR)
            {
                // Create new mediator.
                createMediator(learningEvent);

                // Create mediator conjunction.
                createConjunction(learningEvent);

                // Extend mediator.
                extendMediator(learningEvent);
            }
        }
    }

    // Update motor goal values.
    for (neuronItr = motors.begin();
        neuronItr != motors.end(); neuronItr++)
    {
        motor = (Motor *)*neuronItr;
        motor->goals.update(oldNeeds, needs, motor->firingStrength);
    }

    // Update receptor goal values.
    for (neuronItr = receptors.begin();
        neuronItr != receptors.end(); neuronItr++)
    {
        receptor = (Receptor *)*neuronItr;
        receptor->goals.update(oldNeeds, needs, receptor->firingStrength);
    }

    // Accumulate receptor and mediator enablings.
    for (neuronItr = receptors.begin();
        neuronItr != receptors.end(); neuronItr++)
    {
        receptor = (Receptor *)*neuronItr;
        receptor->accumEnablings();
    }
    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        mediator->accumEnablings();
    }

    // Mark parasite wagers.
    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        mediator->markParasiticWagers();
    }

    // Pay wagers.
    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        mediator->payWagers();
    }

    // Expire timed-out enablings.
    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        mediator->expireEnablings();
    }

    // Dump enablings.
    #ifdef MONA_TRACE
    if (traceLearn)
    {
        dumpEnablings();
    }
    #endif

    // Delete excess mediators.
    while (mediators.size() > MAX_MEDIATORS)
    {
        if ((mediator = getWorstMediator()) == NULL) break;
        deleteNeuron(mediator);
    }

    // Increment event clock.
    eventClock++;
}


// Create a new mediator for given effect.
void
Mona::createMediator(LearningEvent *effectEvent)
{
    int i,j,k,n,level;
    LearningEvent *causeEvent,*responseEvent,*learningEvent;
    list<LearningEvent *>::iterator causeEventItr,
        responseEventItr;
    vector<LearningEvent *> tmpVector;
    Mediator *mediator;
    list<Neuron *>::iterator neuronItr;
    bool enabler;
    double probability;
    #ifdef VERBOSE_DESCRIPTION
    char *description;
    #endif
    char description[50];

    // Effect fired?
    if (effectEvent->strength > 0.0)
    {
        // Determine probability of creating an enabling mediator.
        probability =
            pow((effectEvent->motive / maxMotive), MOTIVE_LEARNING_DAMPER) *
            pow((effectEvent->strength / MAX_ENABLEMENT), STRENGTH_LEARNING_DAMPER);
        if (!random.RAND_CHANCE(probability)) return;
        enabler = true;

    }                                             // Effect misfired.
    else
    {

        // Create a disabler only if event predicted.
        if (effectEvent->enablement <= 0.0) return;

        probability =
            pow((effectEvent->motive / maxMotive), MOTIVE_LEARNING_DAMPER) *
            pow((effectEvent->enablement / MAX_ENABLEMENT), ENABLEMENT_LEARNING_DAMPER);
        if (!random.RAND_CHANCE(probability)) return;
        enabler = false;
    }

    // Find a cause event.
    if (effectEvent->neuron->type != MEDIATOR)
    {
        level = 0;
    }
    else
    {
        level = ((Mediator *)effectEvent->neuron)->level + 1;
    }
    n = random.RAND_CHOICE(MAX_MEDIATOR_SKEW + 1);
    if (random.RAND_BOOL()) n += level; else n = level - n;
    for (i = 0, j = (MAX_MEDIATOR_SKEW * 2) + 1; i < j; i++)
    {
        if (n >= 0 && n < learningEvents.size())
        {
            for (causeEventItr = learningEvents[n].begin();
                causeEventItr != learningEvents[n].end(); causeEventItr++)
            {
                causeEvent = *causeEventItr;
                if (causeEvent->strength <= 0.0) continue;
                if (causeEvent->neuron->type == MOTOR) continue;
                if (causeEvent->timestamp >= effectEvent->begin) continue;
                if (causeEvent->neuron->type != MEDIATOR)
                {
                    k = 0;
                }
                else
                {
                    k = ((Mediator *)causeEvent->neuron)->level + 1;
                }
                if (level > k) k = level;
                if ((effectEvent->begin - causeEvent->timestamp) >
                    eventTimers[k][eventTimers[k].size() - 1]) continue;

                // Add a response?
                responseEvent = NULL;
                if (effectEvent->neuron->type == RECEPTOR)
                {
                    tmpVector.clear();
                    for (responseEventItr = learningEvents[0].begin();
                        responseEventItr != learningEvents[0].end(); responseEventItr++)
                    {
                        responseEvent = *responseEventItr;
                        if (responseEvent->neuron->type == MOTOR &&
                            responseEvent->strength > 0.0 &&
                            responseEvent->timestamp == effectEvent->timestamp)
                        {
                            tmpVector.push_back(responseEvent);
                        }
                    }
                    if (tmpVector.size() > 0)
                    {
                        responseEvent = tmpVector[random.RAND_CHOICE(tmpVector.size())];
                    }
                    else
                    {
                        continue;
                    }
                }

                // Create the mediator.
                #ifdef VERBOSE_DESCRIPTION
                if (responseEvent != NULL)
                {
                    description = new char[strlen(causeEvent->neuron->id.description) +
                        strlen(responseEvent->neuron->id.description) +
                        strlen(effectEvent->neuron->id.description) + 10];
                    sprintf(description, "(%s)-(%s)->(%s)", causeEvent->neuron->id.description,
                        responseEvent->neuron->id.description,
                        effectEvent->neuron->id.description);
                }
                else
                {
                    description = new char[strlen(causeEvent->neuron->id.description) +
                        strlen(effectEvent->neuron->id.description) + 10];
                    sprintf(description, "(%s)->(%s)", causeEvent->neuron->id.description,
                        effectEvent->neuron->id.description);
                }
                mediator = newMediator(NEW_ENABLEMENT, enabler, NEW_ENABLEMENT, description);
                delete description;
                #else
                mediator = newMediator(NEW_ENABLEMENT, enabler, NEW_ENABLEMENT, NULL);
                sprintf(description, "Mediator %d", mediator->id.value);
                mediator->id.setDescription(description);
                #endif
                mediator->addEvent(CAUSE_EVENT, causeEvent->neuron);
                if (responseEvent != NULL)
                {
                    mediator->addEvent(INTERMEDIATE_EVENT, responseEvent->neuron);
                }
                mediator->addEvent(EFFECT_EVENT, effectEvent->neuron);

                // Duplicate?
                for (neuronItr = mediators.begin();
                    neuronItr != mediators.end(); neuronItr++)
                {
                    if (((Mediator *)*neuronItr)->isDuplicate(mediator))
                    {
                        deleteNeuron(mediator);
                        break;
                    }
                }

                // Make new mediator available for learning.
                if (neuronItr == mediators.end() && enabler &&
                    mediator->level < MAX_MEDIATOR_LEVEL)
                {
                    mediator->causeBegin = causeEvent->begin;
                    mediator->firingStrength = causeEvent->strength *
                        effectEvent->strength;
                    learningEvent = new LearningEvent(mediator, eventClock);
                    assert(learningEvent != NULL);
                    learningEvents[mediator->level + 1].push_back(learningEvent);
                }

                #ifdef MONA_TRACE
                if (traceLearn)
                {
                    if (neuronItr == mediators.end())
                    {
                        printf("Create:\n");
                        mediator->print();
                    }
                }
                #endif
                #ifdef NEVER
                // Throttle new mediators.
                if (neuronItr == mediators.end()) return;
                #endif
            }
        }
        n--;
        if (n < (level - MAX_MEDIATOR_SKEW))
        {
            n = level + MAX_MEDIATOR_SKEW;
        }
    }
}


// Create a new mediator conjunction for given effect.
void
Mona::createConjunction(LearningEvent *effectEvent)
{
    int i,j,k,n,p,q,r,s,t,level;
    LearningEvent *causeEvent;
    list<LearningEvent *>::iterator causeEventItr;
    Mediator *mediator,*baseMediator;
    struct Notify *notify;
    Enabling *enabling;
    list<Enabling *>::iterator listItr;
    list<Neuron *>::iterator neuronItr;
    ENABLEMENT disablement,expiringEnablement;
    double probability;
    bool enabler;
    #ifdef VERBOSE_DESCRIPTION
    char *description;
    #endif
    char description[50];

    if ((j = effectEvent->neuron->notifyList.size()) > 0)
    {
        k = random.RAND_CHOICE(j);
    }
    for (i = 0; i < j; i++)
    {
        k = (k + 1) % j;
        notify = effectEvent->neuron->notifyList[k];
        if (notify->eventType != EFFECT_EVENT) continue;
        baseMediator = notify->mediator;
        if (baseMediator->causes.size() == MAX_MEDIATOR_CAUSES) continue;
        disablement = expiringEnablement = 0.0;
        for (listItr = baseMediator->wageredEnablings.enablings.begin();
            listItr != baseMediator->wageredEnablings.enablings.end(); listItr++)
        {
            enabling = *listItr;
            if (baseMediator->enabler)
            {
                if (enabling->age >=
                    eventTimers[baseMediator->level][enabling->timerIndex])
                {
                    expiringEnablement += enabling->value;
                }
            }
            else
            {
                disablement += enabling->value;
            }
        }

        if (effectEvent->strength == 0.0 &&
            effectEvent->enablement > 0.0 &&
            expiringEnablement > 0.0)
        {
            // Determine probability of enabling conjunction.
            probability =
                pow((effectEvent->motive / maxMotive), MOTIVE_LEARNING_DAMPER) *
                pow((expiringEnablement / MAX_ENABLEMENT), EXPIRING_ENABLEMENT_LEARNING_DAMPER);
            if (!random.RAND_CHANCE(probability)) continue;
            enabler = true;

        } else if (effectEvent->strength > 0.0 &&
            effectEvent->enablement < 0.0 &&
            disablement > 0.0)
        {
            // Determine probability of disabling conjunction.
            probability =
                pow((effectEvent->motive / maxMotive), MOTIVE_LEARNING_DAMPER) *
                pow((effectEvent->strength / MAX_ENABLEMENT), STRENGTH_LEARNING_DAMPER) *
                pow((disablement / MAX_ENABLEMENT), ENABLEMENT_LEARNING_DAMPER);
            if (!random.RAND_CHANCE(probability)) continue;
            enabler = false;

        } else continue;

        n = random.RAND_CHOICE(MAX_MEDIATOR_SKEW + 1);
        if (effectEvent->neuron->type != MEDIATOR)
        {
            level = 0;
        }
        else
        {
            level = ((Mediator *)effectEvent->neuron)->level + 1;
        }
        if (random.RAND_BOOL()) n += level; else n = level - n;
        for (p = 0, q = (MAX_MEDIATOR_SKEW * 2) + 1; p < q; p++)
        {
            if (n >= 0 && n < learningEvents.size() &&
                n < baseMediator->level + 1)
            {
                for (causeEventItr = learningEvents[n].begin();
                    causeEventItr != learningEvents[n].end(); causeEventItr++)
                {
                    causeEvent = *causeEventItr;
                    if (causeEvent->neuron->type == MOTOR) continue;
                    if (causeEvent->timestamp >= effectEvent->begin) continue;
                    if ((effectEvent->begin - causeEvent->timestamp) >
                        eventTimers[baseMediator->level][eventTimers[baseMediator->level].size() - 1]) continue;
                    if (baseMediator->causes.size() >= MAX_MEDIATOR_CAUSES) continue;
                    for (r = 0, s = baseMediator->causes.size(); r < s; r++)
                    {
                        if (baseMediator->causes[r] == causeEvent->neuron) break;
                    }
                    if (r < s) continue;
                    if (causeEvent->neuron->type == RECEPTOR)
                    {
                        for (r = 0, s = baseMediator->causes.size(); r < s; r++)
                        {
                            if (baseMediator->causes[r]->type == RECEPTOR)
                            {
                                if (((Receptor *)(causeEvent->neuron))->subsumes((Receptor *)(baseMediator->causes[r])) ||
                                    ((Receptor *)(baseMediator->causes[r]))->subsumes((Receptor *)(causeEvent->neuron)))
                                {
                                    break;
                                }
                            }
                        }
                        if (r < s) continue;
                    }

                    // Create the mediator.
                    #ifdef VERBOSE_DESCRIPTION
                    for (r = t = 0, s = baseMediator->causes.size(); r < s; r++)
                    {
                        t += strlen(baseMediator->causes[r]->id.description) + 1;
                    }
                    t += strlen(causeEvent->neuron->id.description) + 1;
                    if (baseMediator->response != NULL)
                    {
                        t += strlen(baseMediator->response->id.description) + 1;
                    }
                    t += strlen(effectEvent->neuron->id.description) + 10;
                    description = new char[t];
                    strcpy(description,"(");
                    for (r = t = 0, s = baseMediator->causes.size(); r < s; r++)
                    {
                        strcat(description, baseMediator->causes[r]->id.description);
                        strcat(description, ",");
                    }
                    strcat(description, causeEvent->neuron->id.description);
                    if (baseMediator->response != NULL)
                    {
                        strcat(description, ")-(");
                        strcat(description, baseMediator->response->id.description);
                    }
                    strcat(description, ")->(");
                    strcat(description, effectEvent->neuron->id.description);
                    strcat(description, ")");
                    mediator = newMediator(baseMediator->getBaseEnablement(),
                        enabler, baseMediator->utility, description);
                    delete description;
                    #else
                    mediator = newMediator(baseMediator->getBaseEnablement(),
                        enabler, baseMediator->utility, NULL);
                    sprintf(description, "Mediator %d", mediator->id.value);
                    mediator->id.setDescription(description);
                    #endif
                    for (r = 0, s = baseMediator->causes.size(); r < s; r++)
                    {
                        mediator->addEvent(CAUSE_EVENT, baseMediator->causes[r]);
                    }
                    mediator->addEvent(CAUSE_EVENT, causeEvent->neuron);
                    for (r = 0, s = baseMediator->intermediates.size(); r < s; r++)
                    {
                        mediator->addEvent(INTERMEDIATE_EVENT, baseMediator->intermediates[r]);
                    }
                    mediator->addEvent(EFFECT_EVENT, baseMediator->effect);

                    // Duplicate?
                    for (neuronItr = mediators.begin();
                        neuronItr != mediators.end(); neuronItr++)
                    {
                        if (((Mediator *)*neuronItr)->isDuplicate(mediator))
                        {
                            deleteNeuron(mediator);
                            break;
                        }
                    }

                    #ifdef MONA_TRACE
                    if (traceLearn)
                    {
                        if (neuronItr == mediators.end())
                        {
                            printf("Create:\n");
                            mediator->print();
                        }
                    }
                    #endif

                    // Throttle new mediators.
                    if (neuronItr == mediators.end()) return;
                }
            }
            n--;
            if (n < (level - MAX_MEDIATOR_SKEW))
            {
                n = level + MAX_MEDIATOR_SKEW;
            }
        }
    }
}


// Extend a mediator to incorporate given effect.
void
Mona::extendMediator(LearningEvent *effectEvent)
{
    int i,j,n,level;
    LearningEvent *causeEvent,*responseEvent,*learningEvent;
    list<LearningEvent *>::reverse_iterator causeEventItr;
    list<LearningEvent *>::iterator responseEventItr;
    vector<LearningEvent *> tmpVector;
    Mediator *mediator,*baseMediator;
    list<Neuron *>::iterator neuronItr;
    bool enabler;
    double probability;
    #ifdef VERBOSE_DESCRIPTION
    char *description;
    #endif
    char description[50];

    // Effect fired?
    if (effectEvent->strength > 0.0)
    {
        // Determine probability of extending an enabling mediator.
        probability =
            pow((effectEvent->motive / maxMotive), MOTIVE_LEARNING_DAMPER) *
            pow((effectEvent->strength / MAX_ENABLEMENT), STRENGTH_LEARNING_DAMPER);
        if (!random.RAND_CHANCE(probability)) return;
        enabler = true;

    }                                             // Effect misfired.
    else
    {

        // Create a disabler only if event predicted.
        if (effectEvent->enablement <= 0.0) return;

        probability =
            pow((effectEvent->motive / maxMotive), MOTIVE_LEARNING_DAMPER) *
            pow((effectEvent->enablement / MAX_ENABLEMENT), ENABLEMENT_LEARNING_DAMPER);
        if (!random.RAND_CHANCE(probability)) return;
        enabler = false;
    }

    // Phase I: incorporate effect into previous mediator.

    // Find a cause event.
    if (effectEvent->neuron->type != MEDIATOR)
    {
        level = 0;
    }
    else
    {
        level = ((Mediator *)effectEvent->neuron)->level + 1;
    }
    for (n = level + 1; n <= (level + MAX_MEDIATOR_SKEW + 1) &&
        n < learningEvents.size(); n++)
    {
        for (causeEventItr = learningEvents[n].rbegin();
            causeEventItr != learningEvents[n].rend(); causeEventItr++)
        {
            causeEvent = *causeEventItr;
            if (causeEvent->strength <= 0.0) continue;
            baseMediator = (Mediator *)(causeEvent->neuron);
            if (!baseMediator->enabler) continue;
            if (causeEvent->timestamp >= effectEvent->begin) continue;
            if ((effectEvent->begin - causeEvent->timestamp) >
                eventTimers[n - 1][eventTimers[n - 1].size() - 1]) continue;
            if (effectEvent->neuron->type == RECEPTOR) i = 2; else i = 1;
            if (baseMediator->intermediates.size() + i + 2 > MAX_MEDIATOR_EVENTS) continue;

            // Add a response?
            responseEvent = NULL;
            if (effectEvent->neuron->type == RECEPTOR)
            {
                tmpVector.clear();
                for (responseEventItr = learningEvents[0].begin();
                    responseEventItr != learningEvents[0].end(); responseEventItr++)
                {
                    responseEvent = *responseEventItr;
                    if (responseEvent->neuron->type == MOTOR &&
                        responseEvent->strength > 0.0 &&
                        responseEvent->timestamp == effectEvent->timestamp)
                    {
                        tmpVector.push_back(responseEvent);
                    }
                }
                if (tmpVector.size() > 0)
                {
                    responseEvent = tmpVector[random.RAND_CHOICE(tmpVector.size())];
                }
                else
                {
                    continue;
                }
            }

            // Create the mediator.
            #ifdef VERBOSE_DESCRIPTION
            if (responseEvent != NULL)
            {
                description = new char[strlen(causeEvent->neuron->id.description) +
                    strlen(responseEvent->neuron->id.description) +
                    strlen(effectEvent->neuron->id.description) + 10];
                sprintf(description, "%s-(%s)->(%s)", causeEvent->neuron->id.description,
                    responseEvent->neuron->id.description,
                    effectEvent->neuron->id.description);
            }
            else
            {
                description = new char[strlen(causeEvent->neuron->id.description) +
                    strlen(effectEvent->neuron->id.description) + 10];
                sprintf(description, "%s->(%s)", causeEvent->neuron->id.description,
                    effectEvent->neuron->id.description);
            }
            mediator = newMediator(NEW_ENABLEMENT, enabler, NEW_ENABLEMENT, description);
            delete description;
            #else
            mediator = newMediator(NEW_ENABLEMENT, enabler, NEW_ENABLEMENT, NULL);
            sprintf(description, "Mediator %d", mediator->id.value);
            mediator->id.setDescription(description);
            #endif
            for (i = 0, j = baseMediator->causes.size(); i < j; i++)
            {
                mediator->addEvent(CAUSE_EVENT, baseMediator->causes[i]);
            }
            for (i = 0, j = baseMediator->intermediates.size(); i < j; i++)
            {
                mediator->addEvent(INTERMEDIATE_EVENT, baseMediator->intermediates[i]);
            }
            mediator->addEvent(INTERMEDIATE_EVENT, baseMediator->effect);
            if (responseEvent != NULL)
            {
                mediator->addEvent(INTERMEDIATE_EVENT, responseEvent->neuron);
            }
            mediator->addEvent(EFFECT_EVENT, effectEvent->neuron);

            // Duplicate?
            for (neuronItr = mediators.begin();
                neuronItr != mediators.end(); neuronItr++)
            {
                if (((Mediator *)*neuronItr)->isDuplicate(mediator))
                {
                    deleteNeuron(mediator);
                    break;
                }
            }

            // Make new mediator available for learning.
            if (neuronItr == mediators.end() && enabler &&
                mediator->level < MAX_MEDIATOR_LEVEL)
            {
                mediator->causeBegin = causeEvent->begin;
                mediator->firingStrength = causeEvent->strength *
                    effectEvent->strength;
                learningEvent = new LearningEvent(mediator, eventClock);
                assert(learningEvent != NULL);
                learningEvents[mediator->level + 1].push_back(learningEvent);
            }

            #ifdef MONA_TRACE
            if (traceLearn)
            {
                if (neuronItr == mediators.end())
                {
                    printf("Create:\n");
                    mediator->print();
                }
            }
            #endif
            // Throttle new mediators.
            if (neuronItr == mediators.end()) return;
        }
    }

    // Phase II: incorporate a previous cause into mediator.

    // Find a new cause event.
    if (!enabler) return;
    if (effectEvent->neuron->type != MEDIATOR) return;
    baseMediator = (Mediator *)(effectEvent->neuron);
    if (baseMediator->causes.size() > 1) return;
    level = baseMediator->level + 1;
    for (n = level - 1; n >= (level - MAX_MEDIATOR_SKEW - 1) &&
        n >= 0; n--)
    {
        for (causeEventItr = learningEvents[n].rbegin();
            causeEventItr != learningEvents[n].rend(); causeEventItr++)
        {
            causeEvent = *causeEventItr;
            if (causeEvent->neuron->type == MOTOR) continue;
            if (causeEvent->strength <= 0.0) continue;
            if (causeEvent->timestamp >= effectEvent->begin) continue;
            if ((effectEvent->begin - causeEvent->timestamp) >
                eventTimers[baseMediator->level][eventTimers[baseMediator->level].size() - 1]) continue;
            if (baseMediator->causes[0]->type == RECEPTOR) i = 2; else i = 1;
            if (baseMediator->intermediates.size() + i + 2 > MAX_MEDIATOR_EVENTS) continue;

            // Add a response?
            responseEvent = NULL;
            if (baseMediator->causes[0]->type == RECEPTOR)
            {
                tmpVector.clear();
                for (responseEventItr = learningEvents[0].begin();
                    responseEventItr != learningEvents[0].end(); responseEventItr++)
                {
                    responseEvent = *responseEventItr;
                    if (responseEvent->neuron->type == MOTOR &&
                        responseEvent->strength > 0.0 &&
                        responseEvent->timestamp == effectEvent->begin)
                    {
                        tmpVector.push_back(responseEvent);
                    }
                }
                if (tmpVector.size() > 0)
                {
                    responseEvent = tmpVector[random.RAND_CHOICE(tmpVector.size())];
                }
                else
                {
                    continue;
                }
            }

            // Create the mediator.
            #ifdef VERBOSE_DESCRIPTION
            if (responseEvent != NULL)
            {
                description = new char[strlen(causeEvent->neuron->id.description) +
                    strlen(responseEvent->neuron->id.description) +
                    strlen(effectEvent->neuron->id.description) + 10];
                sprintf(description, "(%s)-(%s)->%s", causeEvent->neuron->id.description,
                    responseEvent->neuron->id.description,
                    effectEvent->neuron->id.description);
            }
            else
            {
                description = new char[strlen(causeEvent->neuron->id.description) +
                    strlen(effectEvent->neuron->id.description) + 10];
                sprintf(description, "(%s)->%s", causeEvent->neuron->id.description,
                    effectEvent->neuron->id.description);
            }
            mediator = newMediator(NEW_ENABLEMENT, enabler, NEW_ENABLEMENT, description);
            delete description;
            #else
            mediator = newMediator(NEW_ENABLEMENT, enabler, NEW_ENABLEMENT, NULL);
            sprintf(description, "Mediator %d", mediator->id.value);
            mediator->id.setDescription(description);
            #endif
            mediator->addEvent(CAUSE_EVENT, causeEvent->neuron);
            if (responseEvent != NULL)
            {
                mediator->addEvent(INTERMEDIATE_EVENT, responseEvent->neuron);
            }
            mediator->addEvent(INTERMEDIATE_EVENT, baseMediator->causes[0]);
            for (i = 0, j = baseMediator->intermediates.size(); i < j; i++)
            {
                mediator->addEvent(INTERMEDIATE_EVENT, baseMediator->intermediates[i]);
            }
            mediator->addEvent(EFFECT_EVENT, baseMediator->effect);

            // Duplicate?
            for (neuronItr = mediators.begin();
                neuronItr != mediators.end(); neuronItr++)
            {
                if (((Mediator *)*neuronItr)->isDuplicate(mediator))
                {
                    deleteNeuron(mediator);
                    break;
                }
            }

            #ifdef MONA_TRACE
            if (traceLearn)
            {
                if (neuronItr == mediators.end())
                {
                    printf("Create:\n");
                    mediator->print();
                }
            }
            #endif
            // Throttle new mediators.
            if (neuronItr == mediators.end()) return;
        }
    }
}


// Accumulated enablings.
void
Mona::Neuron::accumEnablings()
{
    int i,j;
    struct Notify *notify;
    Mediator *mediator;
    Enabling *enabling;
    list<Enabling *>::iterator listItr;

    enablingAccum = disablingAccum = 0.0;
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
                enablingAccum += enabling->value;
            }
            else
            {
                disablingAccum += enabling->value;
            }
        }
    }
}


// Mark parasitic wagers. See isParasite() for definition
// of a parasite.
void Mona::Mediator::markParasiticWagers()
{
    Enabling *enabling;
    list<Enabling *>::iterator enablingItr;
    ENABLEMENT payableEnablement,wageredEnablement,enablement;
    WEIGHT transfer;

    if (!enabler) return;
    payableEnablement = firingStrength;
    wageredEnablement = 0.0;
    for (enablingItr = wageredEnablings.enablings.begin();
        enablingItr != wageredEnablings.enablings.end(); enablingItr++)
    {
        enabling = *enablingItr;
        if (enabling->newInSet) continue;
        if (enabling->effectWager)
        {
            wageredEnablement += enabling->value;
        }
    }
    if (payableEnablement >= wageredEnablement)
    {
        transfer = 1.0;
    }
    else
    {
        if (wageredEnablement > 0.0)
        {
            transfer = payableEnablement / wageredEnablement;
        }
        else
        {
            transfer = 0.0;
        }
    }
    for (enablingItr = wageredEnablings.enablings.begin();
        enablingItr != wageredEnablings.enablings.end(); enablingItr++)
    {
        enabling = *enablingItr;
        if (enabling->newInSet) continue;
        if (!enabling->effectWager) continue;

        enablement = enabling->value * transfer;
        if (enablement > 0.0)
        {
            markParasite(enabling);
        }
    }
}


// Is wager a parasite?
// There are two conditions:
// A. A parasite mediator's effect event is part of a
//    stronger over-arching mediator's event stream.
// B. A parasite has an elementary event stream that is
//    a subset of another of greater enablement.
void
Mona::Mediator::markParasite(Enabling *wager)
{
    int i,j;
    struct Notify *notify;
    Mediator *mediator;
    Enabling *enabling;
    list<Enabling *>::iterator enablingItr;
    ENABLEMENT enablement,enablement2;
    TIME begin,begin2;
    bool gotBegin;
    vector<struct ElemEvent> emptyEvents;

    // Default to negative condition.
    wager->parasite = false;

    // Disregard disabler.
    if (!enabler) return;

    // Test for condition A:

    // Check for a longer mediator with a stronger enabling.
    enablement = wager->value;
    begin = wager->causeBegin;
    for (i = 0, j = effect->notifyList.size(); i < j; i++)
    {
        notify = effect->notifyList[i];
        mediator = notify->mediator;
        if (mediator == this) continue;
        if (!mediator->enabler) continue;
        gotBegin = false;
        begin2 = 0;
        enablement2 = 0.0;
        switch(notify->eventType)
        {
            case INTERMEDIATE_EVENT:
                for (enablingItr = mediator->intermediateEnablings[notify->eventIndex]->enablings.begin();
                    enablingItr != mediator->intermediateEnablings[notify->eventIndex]->enablings.end(); enablingItr++)
                {
                    enabling = *enablingItr;
                    enablement2 += enabling->value;
                    if (!gotBegin || enabling->causeBegin < begin2)
                    {
                        begin2 = enabling->causeBegin;
                        gotBegin = true;
                    }
                }
                if (enablement2 > enablement && begin2 < begin)
                {
                    wager->parasite = true;
                    return;
                }
                break;
            case EFFECT_EVENT:
                for (enablingItr = mediator->effectEnablings.enablings.begin();
                    enablingItr != mediator->effectEnablings.enablings.end(); enablingItr++)
                {
                    enabling = *enablingItr;
                    enablement2 += enabling->value;
                    if (!gotBegin || enabling->causeBegin < begin2)
                    {
                        begin2 = enabling->causeBegin;
                        gotBegin = true;
                    }
                }
                for (enablingItr = mediator->wageredEnablings.enablings.begin();
                    enablingItr != mediator->wageredEnablings.enablings.end(); enablingItr++)
                {
                    enabling = *enablingItr;
                    enablement2 += enabling->value;
                    if (!gotBegin || enabling->causeBegin < begin2)
                    {
                        begin2 = enabling->causeBegin;
                        gotBegin = true;
                    }
                }
                if (enablement2 > enablement && begin2 < begin)
                {
                    wager->parasite = true;
                    return;
                }
                break;
        }
    }

    // Check for parasitic condition B.
    wager->parasite = isParasite(wager, emptyEvents);
}


// A parasite has an elementary event stream that is
// a subset of another of greater enablement.
bool
Mona::Mediator::isParasite(Enabling *wager,
vector<struct ElemEvent> events, bool trueWager)
{
    int i,j,p,q;
    list<Neuron *>::iterator neuronItr;
    Receptor *receptor;
    Mediator *mediator;
    Enabling *enabling;
    list<Enabling *>::iterator enablingItr;
    vector<struct ElemEvent> accumEvents,emptyEvents;
    struct Notify *notify;
    int currentID;

    // Check for subsuming event stream.
    if (wager != NULL)
    {
        // Find the current receptor event id.
        if (events.size() > 0)
        {
            currentID = events[events.size() - 1].id;
        } else if (wager->events.size() > 0)
        {
            currentID = wager->events[wager->events.size() - 1].id;
        }
        else
        {
            return false;
        }
        bool found = false;
        for (neuronItr = mona->mediators.begin();
            neuronItr != mona->mediators.end(); neuronItr++)
        {
            mediator = (Mediator *)*neuronItr;
            if (mediator == this || !mediator->enabler) continue;
            if (mediator->effect->type != RECEPTOR) continue;
            receptor = (Receptor *)mediator->effect;
            if (receptor->id.value != currentID) continue;
            for (i = 0, j = mediator->intermediateEnablings.size(); i < j; i++)
            {
                for (enablingItr = mediator->intermediateEnablings[i]->enablings.begin();
                    enablingItr != mediator->intermediateEnablings[i]->enablings.end(); enablingItr++)
                {
                    enabling = *enablingItr;
                    if (enabling->newInSet) continue;
                    found = testEventStream(wager, events, mediator, enabling, emptyEvents, false);
                    if (found) break;
                }
            }
            if (found) break;
            for (enablingItr = mediator->effectEnablings.enablings.begin();
                enablingItr != mediator->effectEnablings.enablings.end(); enablingItr++)
            {
                enabling = *enablingItr;
                if (enabling->newInSet) continue;
                found = testEventStream(wager, events, mediator, enabling, emptyEvents, false);
                if (found) break;
            }
            if (found) break;
            for (enablingItr = mediator->wageredEnablings.enablings.begin();
                enablingItr != mediator->wageredEnablings.enablings.end(); enablingItr++)
            {
                enabling = *enablingItr;
                if (enabling->newInSet) continue;
                found = testEventStream(wager, events, mediator, enabling, emptyEvents, true);
                if (found) break;
            }
            if (found) break;
        }
        if (!found) return false;
    }

    // Check parent event streams.
    for (i = 0, j = notifyList.size(); i < j; i++)
    {
        notify = notifyList[i];
        mediator = notify->mediator;
        if (!mediator->enabler) continue;
        switch(notify->eventType)
        {
            case CAUSE_EVENT:
                if (trueWager)
                {
                    if (mediator->intermediates.size() > 0)
                    {
                        for (enablingItr = mediator->intermediateEnablings[0]->enablings.begin();
                            enablingItr != mediator->intermediateEnablings[0]->enablings.end(); enablingItr++)
                        {
                            enabling = *enablingItr;
                            if (enabling->newInSet) continue;

                            // Check enabling containing this event stream.
                            if (containsEventStream(enabling->events, wager->events) == 0)
                            {
                                if (!mediator->isParasite(enabling, emptyEvents, false)) return false;
                            }
                        }
                    }
                    else
                    {
                        for (enablingItr = mediator->effectEnablings.enablings.begin();
                            enablingItr != mediator->effectEnablings.enablings.end(); enablingItr++)
                        {
                            enabling = *enablingItr;
                            if (enabling->newInSet) continue;

                            // Check enabling containing this event stream.
                            if (containsEventStream(enabling->events, wager->events) == 0)
                            {
                                if (!mediator->isParasite(enabling, emptyEvents, false)) return false;
                            }
                        }
                    }
                }
                else
                {
                    // Check levels above the cause.
                    accumEvents.clear();
                    if (wager != NULL)
                    {
                        for (p = 0, q = wager->events.size(); p < q; p++)
                        {
                            accumEvents.push_back(wager->events[p]);
                        }
                    }
                    for (p = 0, q = events.size(); p < q; p++)
                    {
                        accumEvents.push_back(events[p]);
                    }
                    if (!mediator->isParasite(NULL, accumEvents, false)) return false;
                }
                break;

            case INTERMEDIATE_EVENT:
                if (trueWager)
                {
                    if (mediator->intermediates.size() > notify->eventIndex + 1)
                    {
                        for (enablingItr = mediator->intermediateEnablings[notify->eventIndex + 1]->enablings.begin();
                            enablingItr != mediator->intermediateEnablings[notify->eventIndex + 1]->enablings.end(); enablingItr++)
                        {
                            enabling = *enablingItr;
                            if (enabling->newInSet) continue;

                            // Check enabling containing this event stream.
                            if (containsEventStream(enabling->events, wager->events) == 1)
                            {
                                if (!mediator->isParasite(enabling, emptyEvents, false)) return false;
                            }
                        }
                    }
                    else
                    {
                        for (enablingItr = mediator->effectEnablings.enablings.begin();
                            enablingItr != mediator->effectEnablings.enablings.end(); enablingItr++)
                        {
                            enabling = *enablingItr;
                            if (enabling->newInSet) continue;

                            // Check enabling containing this event stream.
                            if (containsEventStream(enabling->events, wager->events) == 1)
                            {
                                if (!mediator->isParasite(enabling, emptyEvents, false)) return false;
                            }
                        }
                    }
                }
                else
                {
                    for (enablingItr = mediator->intermediateEnablings[notify->eventIndex]->enablings.begin();
                        enablingItr != mediator->intermediateEnablings[notify->eventIndex]->enablings.end(); enablingItr++)
                    {
                        enabling = *enablingItr;
                        if (enabling->newInSet) continue;
                        accumEvents.clear();
                        if (wager != NULL)
                        {
                            for (p = 0, q = wager->events.size(); p < q; p++)
                            {
                                accumEvents.push_back(wager->events[p]);
                            }
                        }
                        for (p = 0, p = events.size(); p < q; p++)
                        {
                            accumEvents.push_back(events[p]);
                        }
                        if (!mediator->isParasite(enabling, accumEvents, false)) return false;
                    }
                }
                break;

            case EFFECT_EVENT:
                if (trueWager)
                {
                    for (enablingItr = mediator->wageredEnablings.enablings.begin();
                        enablingItr != mediator->wageredEnablings.enablings.end(); enablingItr++)
                    {
                        enabling = *enablingItr;
                        if (enabling->newInSet) continue;

                        // Check enabling containing this event stream.
                        if (containsEventStream(enabling->events, wager->events) == 1)
                        {
                            if (!mediator->isParasite(enabling, emptyEvents, true)) return false;
                        }
                    }
                }
                else
                {
                    for (enablingItr = mediator->effectEnablings.enablings.begin();
                        enablingItr != mediator->effectEnablings.enablings.end(); enablingItr++)
                    {
                        enabling = *enablingItr;
                        if (enabling->newInSet) continue;
                        accumEvents.clear();
                        if (wager != NULL)
                        {
                            for (p = 0, q = wager->events.size(); p < q; p++)
                            {
                                accumEvents.push_back(wager->events[p]);
                            }
                        }
                        for (p = 0, q = events.size(); p < q; p++)
                        {
                            accumEvents.push_back(events[p]);
                        }
                        if (!mediator->isParasite(enabling, accumEvents, false)) return false;
                    }
                }
                break;
        }
    }

    return true;
}


// Is the elementary event stream for the source event
// a subset of the target event stream? Target event stream
// must also have greater enablement.
// Recurse on target mediator events.
bool
Mona::Mediator::testEventStream(Enabling *sourceEnabling, vector<struct ElemEvent> sourceEvents,
Mediator *targetMediator, Enabling *targetEnabling, vector<struct ElemEvent> targetEvents,
bool trueWager)
{
    int i,j,p,q;
    vector<struct ElemEvent> sourceAccumEvents,targetAccumEvents;
    Mediator *mediator;
    Enabling *enabling;
    list<Enabling *>::iterator enablingItr;
    vector<struct ElemEvent> accumEvents,emptyEvents;
    struct Notify *notify;

    // Cannot self-compare.
    if (targetMediator == this) return false;

    // Compare streams.
    if (targetEnabling != NULL)
    {
        if (targetEnabling->value > sourceEnabling->value)
        {
            sourceAccumEvents.clear();
            for (i = 0, j = sourceEnabling->events.size(); i < j; i++)
            {
                sourceAccumEvents.push_back(sourceEnabling->events[i]);
            }
            for (i = 0, j = sourceEvents.size(); i < j; i++)
            {
                sourceAccumEvents.push_back(sourceEvents[i]);
            }
            targetAccumEvents.clear();
            if (targetEnabling != NULL)
            {
                for (i = 0, j = targetEnabling->events.size(); i < j; i++)
                {
                    targetAccumEvents.push_back(targetEnabling->events[i]);
                }
            }
            for (i = 0, j = targetEvents.size(); i < j; i++)
            {
                targetAccumEvents.push_back(targetEvents[i]);
            }
            switch(containsEventStream(targetAccumEvents, sourceAccumEvents))
            {
                case 1: return true;
                case -2: return false;
            }
        }
    }

    // Check target parent event streams.
    for (i = 0, j = targetMediator->notifyList.size(); i < j; i++)
    {
        notify = targetMediator->notifyList[i];
        mediator = notify->mediator;
        if (!mediator->enabler) continue;
        switch(notify->eventType)
        {
            case CAUSE_EVENT:
                if (trueWager)
                {
                    if (mediator->intermediates.size() > 0)
                    {
                        for (enablingItr = mediator->intermediateEnablings[0]->enablings.begin();
                            enablingItr != mediator->intermediateEnablings[0]->enablings.end(); enablingItr++)
                        {
                            enabling = *enablingItr;
                            if (enabling->newInSet) continue;

                            // Compare event streams.
                            if (testEventStream(sourceEnabling, sourceEvents,
                                mediator, enabling, emptyEvents, false)) return true;
                        }
                    }
                    else
                    {
                        for (enablingItr = mediator->effectEnablings.enablings.begin();
                            enablingItr != mediator->effectEnablings.enablings.end(); enablingItr++)
                        {
                            enabling = *enablingItr;
                            if (enabling->newInSet) continue;

                            // Compare event streams.
                            if (testEventStream(sourceEnabling, sourceEvents,
                                mediator, enabling, emptyEvents, false)) return true;
                        }
                    }
                }
                else
                {
                    // Check levels above the cause.
                    accumEvents.clear();
                    if (targetEnabling != NULL)
                    {
                        for (p = 0, q = targetEnabling->events.size(); p < q; p++)
                        {
                            accumEvents.push_back(targetEnabling->events[p]);
                        }
                    }
                    for (p = 0, q = targetEvents.size(); p < q; p++)
                    {
                        accumEvents.push_back(targetEvents[p]);
                    }
                    if (testEventStream(sourceEnabling, sourceEvents,
                        mediator, NULL, accumEvents, false)) return true;
                }
                break;

            case INTERMEDIATE_EVENT:
                if (trueWager)
                {
                    if (mediator->intermediates.size() > notify->eventIndex + 1)
                    {
                        for (enablingItr = mediator->intermediateEnablings[notify->eventIndex + 1]->enablings.begin();
                            enablingItr != mediator->intermediateEnablings[notify->eventIndex + 1]->enablings.end(); enablingItr++)
                        {
                            enabling = *enablingItr;
                            if (enabling->newInSet) continue;

                            // Compare event streams.
                            if (testEventStream(sourceEnabling, sourceEvents,
                                mediator, enabling, emptyEvents, false)) return true;
                        }
                    }
                    else
                    {
                        for (enablingItr = mediator->effectEnablings.enablings.begin();
                            enablingItr != mediator->effectEnablings.enablings.end(); enablingItr++)
                        {
                            enabling = *enablingItr;
                            if (enabling->newInSet) continue;

                            // Compare event streams.
                            if (testEventStream(sourceEnabling, sourceEvents,
                                mediator, enabling, emptyEvents, false)) return true;
                        }
                    }
                }
                else
                {
                    for (enablingItr = mediator->intermediateEnablings[notify->eventIndex]->enablings.begin();
                        enablingItr != mediator->intermediateEnablings[notify->eventIndex]->enablings.end(); enablingItr++)
                    {
                        enabling = *enablingItr;
                        if (enabling->newInSet) continue;
                        accumEvents.clear();
                        if (targetEnabling != NULL)
                        {
                            for (p = 0, q = targetEnabling->events.size(); p < q; p++)
                            {
                                accumEvents.push_back(targetEnabling->events[p]);
                            }
                        }
                        for (p = 0, q = targetEvents.size(); p < q; p++)
                        {
                            accumEvents.push_back(targetEvents[p]);
                        }

                        // Compare event streams.
                        if (testEventStream(sourceEnabling, sourceEvents,
                            mediator, enabling, accumEvents, false)) return true;
                    }
                }
                break;

            case EFFECT_EVENT:
                if (trueWager)
                {
                    for (enablingItr = mediator->wageredEnablings.enablings.begin();
                        enablingItr != mediator->wageredEnablings.enablings.end(); enablingItr++)
                    {
                        enabling = *enablingItr;
                        if (enabling->newInSet) continue;

                        // Compare event streams.
                        if (testEventStream(sourceEnabling, sourceEvents,
                            mediator, enabling, emptyEvents, true)) return true;
                    }
                }
                else
                {
                    for (enablingItr = mediator->effectEnablings.enablings.begin();
                        enablingItr != mediator->effectEnablings.enablings.end(); enablingItr++)
                    {
                        enabling = *enablingItr;
                        if (enabling->newInSet) continue;
                        accumEvents.clear();
                        if (targetEnabling != NULL)
                        {
                            for (p = 0, q = targetEnabling->events.size(); p < q; p++)
                            {
                                accumEvents.push_back(targetEnabling->events[p]);
                            }
                        }
                        for (p = 0, q = targetEvents.size(); p < q; p++)
                        {
                            accumEvents.push_back(targetEvents[p]);
                        }

                        // Compare event streams.
                        if (testEventStream(sourceEnabling, sourceEvents,
                            mediator, enabling, accumEvents, false)) return true;
                    }
                }
                break;
        }
    }

    return false;
}


// Event id stream s1 contains s2?
// Returns: -2=mismatch, -1=subset, 0=equal, 1=superset
int
Mona::Mediator::containsEventStream(vector<struct ElemEvent> &s1,
vector<struct ElemEvent> &s2)
{
    int i,j;

    for (i = s1.size() - 1, j = s2.size() - 1; i >= 0 && j >= 0; i--, j--)
    {
        if (s1[i].id != s2[j].id ||
            s1[i].timestamp != s2[j].timestamp) return -2;
    }
    if (i < 0)
    {
        if (j < 0) return 0; else return -1;
    }
    else
    {
        return 1;
    }
}


// Pay enabling wagers.
void
Mona::Mediator::payWagers()
{
    int i,j,k;
    Enabling *enabling;
    list<Enabling *>::iterator enablingItr;
    ENABLEMENT payableEnablement,wageredEnablement,
        enablement,earliestEnablement,enablementUpdate;
    int earliestTimerIndex;
    WEIGHT transfer,motiveFactor;

    // Pay in proportion to firing strength.
    payableEnablement = firingStrength;
    earliestTimerIndex = -1;
    wageredEnablement = 0.0;
    for (enablingItr = wageredEnablings.enablings.begin();
        enablingItr != wageredEnablings.enablings.end(); enablingItr++)
    {
        enabling = *enablingItr;
        if (enabling->newInSet) continue;
        if (enabling->effectWager)
        {
            wageredEnablement += enabling->value;
        }
    }
    if (payableEnablement >= wageredEnablement)
    {
        transfer = 1.0;
    }
    else
    {
        if (wageredEnablement > 0.0)
        {
            transfer = payableEnablement / wageredEnablement;
        }
        else
        {
            transfer = 0.0;
        }
    }
    if (enabler)
    {
        for (enablingItr = wageredEnablings.enablings.begin();
            enablingItr != wageredEnablings.enablings.end(); enablingItr++)
        {
            enabling = *enablingItr;
            if (enabling->newInSet) continue;
            if (!enabling->effectWager) continue;

            // Update goal value.
            if (enabling->motive <= 0.0) continue;
            if (enabling->motive < mona->maxMotive)
            {
                motiveFactor = enabling->motive / mona->maxMotive;
            }
            else
            {
                motiveFactor = 1.0;
            }
            goals.update(enabling->needSave,
                mona->needs, firingStrength * motiveFactor);

            // Reward fired enablings.
            enablement = enabling->value * transfer;
            if (enablement > 0.0 && !enabling->parasite)
            {
                enabling->value -= enablement;
                baseEnablement += enablement;
                if (earliestTimerIndex == -1 ||
                    enabling->timerIndex < earliestTimerIndex)
                {
                    earliestTimerIndex = enabling->timerIndex;
                    earliestEnablement = enablement;
                }
                enablementUpdate = wagerHistory.getEnablement(FIRE,
                    getBaseEnablement(), enablement);
                baseEnablement = enablementUpdate - getBaseEnablementOut();

                // Update intermediate and effect base enablements.
                for (i = 0, j = intermediateEnablements.size(); i < j; i++)
                {
                    intermediateEnablements[i] = enablementUpdate;
                }
                effectEnablement = enablementUpdate;
            }

            // Punish expired enablings.
            if (enabling->value > 0.0)
            {
                if (enabling->age >= mona->eventTimers[level][enabling->timerIndex])
                {
                    baseEnablement += enabling->value;
                    enablement = enabling->value;
                    enabling->value = 0.0;
                    enablementUpdate = wagerHistory.getEnablement(EXPIRE,
                        getBaseEnablement(), enablement * motiveFactor * EXPIRATION_WEIGHT);
                    baseEnablement = enablementUpdate - getBaseEnablementOut();

                    // Update intermediate and effect base enablements.
                    for (i = 0, j = intermediateEnablements.size(); i < j; i++)
                    {
                        intermediateEnablements[i] = enablementUpdate;
                    }
                    effectEnablement = enablementUpdate;
                }
            }
        }

    }                                             // disabler
    else
    {

        for (enablingItr = wageredEnablings.enablings.begin();
            enablingItr != wageredEnablings.enablings.end(); enablingItr++)
        {
            enabling = *enablingItr;
            if (enabling->newInSet) continue;
            if (!enabling->effectWager) continue;

            // Get motive factor.
            if (enabling->motive <= 0.0) continue;
            if (enabling->motive < mona->maxMotive)
            {
                motiveFactor = enabling->motive / mona->maxMotive;
            }
            else
            {
                motiveFactor = 1.0;
            }
            if (motiveFactor <= MIN_MOTIVE) motiveFactor = 0.0;

            // Punish fired disablings.
            enablement = enabling->value * transfer;
            if (enablement > 0.0)
            {
                enabling->value -= enablement;
                baseEnablement += enablement;
                enablementUpdate = wagerHistory.getEnablement(EXPIRE,
                    getBaseEnablement(), enablement * EXPIRATION_WEIGHT);
                baseEnablement = enablementUpdate - getBaseEnablementOut();

                // Update intermediate and effect base enablements.
                for (i = 0, j = intermediateEnablements.size(); i < j; i++)
                {
                    intermediateEnablements[i] = enablementUpdate;
                }
                effectEnablement = enablementUpdate;
            }

            // Pay expired disablings.
            if (enabling->value > 0.0)
            {
                if (enabling->age >= mona->eventTimers[level][enabling->timerIndex])
                {
                    if (earliestTimerIndex == -1 ||
                        enabling->timerIndex < earliestTimerIndex)
                    {
                        earliestTimerIndex = enabling->timerIndex;
                        earliestEnablement = enabling->value;
                    }
                    baseEnablement += enabling->value;
                    enablement = enabling->value;
                    enabling->value = 0.0;
                    enablementUpdate = wagerHistory.getEnablement(FIRE,
                        getBaseEnablement(), enablement * motiveFactor);
                    baseEnablement = enablementUpdate - getBaseEnablementOut();

                    // Update intermediate and effect base enablements.
                    for (i = 0, j = intermediateEnablements.size(); i < j; i++)
                    {
                        intermediateEnablements[i] = enablementUpdate;
                    }
                    effectEnablement = enablementUpdate;
                }
            }
        }
    }

    // Punish expired intermediate enablings.
    for (i = 0, j = intermediates.size(); i < j; i++)
    {
        for (enablingItr = intermediateEnablings[i]->enablings.begin();
            enablingItr != intermediateEnablings[i]->enablings.end(); enablingItr++)
        {
            enabling = *enablingItr;
            if (enabling->newInSet) continue;

            if (enabling->motive <= 0.0) continue;
            if (enabling->motive < mona->maxMotive)
            {
                motiveFactor = enabling->motive / mona->maxMotive;
            }
            else
            {
                motiveFactor = 1.0;
            }

            if (enabling->value > 0.0)
            {
                if (enabling->age >= mona->eventTimers[level][enabling->timerIndex])
                {
                    baseEnablement += enabling->value;
                    enablement = enabling->value;
                    enabling->value = 0.0;
                    if (intermediates[i]->type == MOTOR)
                    {
                        enablementUpdate = wagerHistory.getEnablement(EXPIRE,
                            getBaseEnablement(), enablement * motiveFactor *
                            RESPONSE_EXPIRATION_WEIGHT);
                    }
                    else
                    {
                        enablementUpdate = wagerHistory.getEnablement(EXPIRE,
                            getBaseEnablement(), enablement * motiveFactor *
                            EXPIRATION_WEIGHT);
                    }
                    baseEnablement = enablementUpdate - getBaseEnablementOut();

                    // Update intermediate base enablements.
                    for (k = 0; k <= i; k++)
                    {
                        intermediateEnablements[k] = enablementUpdate;
                    }
                }
            }
        }
    }

    // Punish expired effect enablings.
    for (enablingItr = effectEnablings.enablings.begin();
        enablingItr != effectEnablings.enablings.end(); enablingItr++)
    {
        enabling = *enablingItr;
        if (enabling->newInSet) continue;

        if (enabling->motive <= 0.0) continue;
        if (enabling->motive < mona->maxMotive)
        {
            motiveFactor = enabling->motive / mona->maxMotive;
        }
        else
        {
            motiveFactor = 1.0;
        }

        if (enabling->value > 0.0)
        {
            if (enabling->age >= mona->eventTimers[level][enabling->timerIndex])
            {
                baseEnablement += enabling->value;
                enablement = enabling->value;
                enabling->value = 0.0;
                enablementUpdate = wagerHistory.getEnablement(EXPIRE,
                    getBaseEnablement(), enablement * motiveFactor *
                    EXPIRATION_WEIGHT);
                baseEnablement = enablementUpdate - getBaseEnablementOut();

                // Update intermediate and effect base enablements.
                for (i = 0, j = intermediateEnablements.size(); i < j; i++)
                {
                    intermediateEnablements[i] = enablementUpdate;
                }
                effectEnablement = enablementUpdate;
            }
        }
    }

    // Set mediator utility.
    utility = getBaseEnablement();

    // Update and normalize timed wager weights.
    if (earliestTimerIndex != -1)
    {
        timedWagerWeights.values[earliestTimerIndex] += earliestEnablement;
        for (i = 0, enablement = 0.0; i < mona->numEventTimers; i++)
        {
            if (timedWagerWeights.values[i] < MIN_WAGER_WEIGHT)
            {
                timedWagerWeights.values[i] = MIN_WAGER_WEIGHT;
            }
            enablement += timedWagerWeights.values[i];
        }
        for (i = 0; i < mona->numEventTimers; i++)
        {
            timedWagerWeights.values[i] /= enablement;
        }
    }
}


// Expired enablings.
void
Mona::Mediator::expireEnablings(bool force)
{
    int i,j;
    Enabling *enabling;
    list<Enabling *> tmpList;
    list<Enabling *>::iterator enablingItr;
    struct PendingEnabling *penabling;
    list<PendingEnabling *> pendingList;
    list<struct PendingEnabling *>::iterator pendingEnablingItr;

    // Age and expire intermediate enablings.
    for (i = 0, j = intermediateEnablings.size(); i < j; i++)
    {
        tmpList.clear();
        for (enablingItr = intermediateEnablings[i]->enablings.begin();
            enablingItr != intermediateEnablings[i]->enablings.end(); enablingItr++)
        {
            enabling = *enablingItr;
            enabling->age++;
            if (force || enabling->age > mona->eventTimers[level][enabling->timerIndex])
            {
                baseEnablement += enabling->value;
                delete enabling;
            }
            else
            {
                tmpList.push_back(enabling);
            }
        }
        intermediateEnablings[i]->enablings.clear();
        for (enablingItr = tmpList.begin();
            enablingItr != tmpList.end(); enablingItr++)
        {
            enabling = *enablingItr;
            intermediateEnablings[i]->enablings.push_back(enabling);
        }
    }

    // Age and expire effect enablings.
    tmpList.clear();
    for (enablingItr = effectEnablings.enablings.begin();
        enablingItr != effectEnablings.enablings.end(); enablingItr++)
    {
        enabling = *enablingItr;
        enabling->age++;
        if (force || enabling->age > mona->eventTimers[level][enabling->timerIndex])
        {
            baseEnablement += enabling->value;
            delete enabling;
        }
        else
        {
            tmpList.push_back(enabling);
        }
    }
    effectEnablings.enablings.clear();
    for (enablingItr = tmpList.begin();
        enablingItr != tmpList.end(); enablingItr++)
    {
        enabling = *enablingItr;
        effectEnablings.enablings.push_back(enabling);
    }

    // Age and expire wagered enablings.
    tmpList.clear();
    for (enablingItr = wageredEnablings.enablings.begin();
        enablingItr != wageredEnablings.enablings.end(); enablingItr++)
    {
        enabling = *enablingItr;
        enabling->age++;
        if (force || enabling->age > mona->eventTimers[level][enabling->timerIndex])
        {
            baseEnablement += enabling->value;
            delete enabling;
        }
        else
        {
            tmpList.push_back(enabling);
        }
    }
    wageredEnablings.enablings.clear();
    for (enablingItr = tmpList.begin();
        enablingItr != tmpList.end(); enablingItr++)
    {
        enabling = *enablingItr;
        wageredEnablings.enablings.push_back(enabling);
    }

    // Age and expire pending enablings.
    for (i = 0, j = pendingEnablings.size(); i < j; i++)
    {
        pendingList.clear();
        for (pendingEnablingItr = pendingEnablings[i].begin();
            pendingEnablingItr != pendingEnablings[i].end(); pendingEnablingItr++)
        {
            penabling = *pendingEnablingItr;
            penabling->age++;
            if (force || penabling->age > mona->eventTimers[level][mona->numEventTimers - 1])
            {
                delete penabling;
            }
            else
            {
                pendingList.push_back(penabling);
            }
        }
        pendingEnablings[i].clear();
        for (pendingEnablingItr = pendingList.begin();
            pendingEnablingItr != pendingList.end(); pendingEnablingItr++)
        {
            penabling = *pendingEnablingItr;
            pendingEnablings[i].push_back(penabling);
        }
    }
}


// Expire enabling on motor.
void
Mona::Motor::expireMotorEnabling()
{
    int i,j,k,p;
    struct Notify *notify;
    Mediator *mediator;
    Enabling *enabling;
    list<Enabling *>::iterator enablingItr;
    WEIGHT motiveFactor;
    ENABLEMENT enablement,enablementUpdate;
    WEIGHT expireStrength;

    for (i = 0, j = notifyList.size(); i < j; i++)
    {
        notify = notifyList[i];
        mediator = notify->mediator;
        k = notify->eventIndex;

        // Determine expiration strength.
        enablement = 0.0;
        for (enablingItr = mediator->intermediateEnablings[k]->enablings.begin();
            enablingItr != mediator->intermediateEnablings[k]->enablings.end(); enablingItr++)
        {
            enabling = *enablingItr;
            enablement += enabling->value;
        }
        if (enablement > 0.0)
        {
            expireStrength = enablement / mediator->getBaseEnablement();
        }
        else
        {
            expireStrength = 0.0;
        }

        // Expire motor enablings.
        for (enablingItr = mediator->intermediateEnablings[k]->enablings.begin();
            enablingItr != mediator->intermediateEnablings[k]->enablings.end(); enablingItr++)
        {
            enabling = *enablingItr;
            if (enabling->motive < mona->maxMotive)
            {
                motiveFactor = enabling->motive / mona->maxMotive;
            }
            else
            {
                motiveFactor = 1.0;
            }
            if (enabling->value > 0.0)
            {
                mediator->baseEnablement += enabling->value;
                enablement = enabling->value;
                enabling->value = 0.0;
                enablementUpdate = mediator->wagerHistory.getEnablement(EXPIRE,
                    mediator->getBaseEnablement(), enablement * motiveFactor *
                    RESPONSE_EXPIRATION_WEIGHT);
                mediator->baseEnablement = enablementUpdate - mediator->getBaseEnablementOut();

                // Update intermediate base enablements.
                for (p = 0; p <= k; p++)
                {
                    mediator->intermediateEnablements[p] = enablementUpdate;
                }

                // Set mediator utility.
                mediator->utility = mediator->getBaseEnablement();
            }
            delete enabling;
        }
        mediator->intermediateEnablings[k]->enablings.clear();

        // Expire higher level wagers on mediator.
        if (expireStrength > 0.0)
        {
            mediator->expireResponseWagers(expireStrength);
        }
    }
}


// Expire wagers due to response abandonment.
void
Mona::Mediator::expireResponseWagers(WEIGHT strength)
{
    int i,j;
    struct Notify *notify;
    Mediator *mediator;
    Enabling *enabling;
    list<Enabling *>::iterator enablingItr;
    WEIGHT motiveFactor;
    ENABLEMENT enablement,enablementUpdate;
    WEIGHT expireStrength;

    for (i = 0, j = notifyList.size(); i < j; i++)
    {
        notify = notifyList[i];
        if (notify->eventType != EFFECT_EVENT) continue;
        mediator = notify->mediator;

        // Determine expiration strength.
        enablement = 0.0;
        for (enablingItr = mediator->wageredEnablings.enablings.begin();
            enablingItr != mediator->wageredEnablings.enablings.end(); enablingItr++)
        {
            enabling = *enablingItr;
            enablement += enabling->value * strength;
        }
        if (enablement > 0.0)
        {
            expireStrength = enablement / mediator->getBaseEnablement();
        }
        else
        {
            expireStrength = 0.0;
        }

        // Expire wagers.
        for (enablingItr = mediator->wageredEnablings.enablings.begin();
            enablingItr != mediator->wageredEnablings.enablings.end(); enablingItr++)
        {
            enabling = *enablingItr;
            if (enabling->motive < mona->maxMotive)
            {
                motiveFactor = enabling->motive / mona->maxMotive;
            }
            else
            {
                motiveFactor = 1.0;
            }
            if (enabling->value > 0.0)
            {
                enablement = enabling->value * strength;
                mediator->baseEnablement += enablement;
                enabling->value -= enablement;
                enablementUpdate = mediator->wagerHistory.getEnablement(EXPIRE,
                    mediator->getBaseEnablement(), enablement * motiveFactor *
                    RESPONSE_EXPIRATION_WEIGHT);
                mediator->baseEnablement = enablementUpdate - mediator->getBaseEnablementOut();

                // Update intermediate and effect base enablements.
                for (i = 0, j = mediator->intermediateEnablements.size(); i < j; i++)
                {
                    mediator->intermediateEnablements[i] = enablementUpdate;
                }
                mediator->effectEnablement = enablementUpdate;

                // Set mediator utility.
                mediator->utility = mediator->getBaseEnablement();
            }
        }

        // Expire higher level wagers on mediator.
        if (expireStrength > 0.0)
        {
            mediator->expireResponseWagers(expireStrength);
        }
    }
}


// Clear working memory.
void Mona::clearWorkingMemory()
{
    int i,j;
    Receptor *receptor;
    Motor *motor;
    Mediator *mediator;
    list<Neuron *>::iterator neuronItr;
    LearningEvent *learningEvent;
    list<LearningEvent *>::iterator learningEventItr;

    for (neuronItr = receptors.begin();
        neuronItr != receptors.end(); neuronItr++)
    {
        receptor = (Receptor *)*neuronItr;
        receptor->firingStrength = 0.0;
        receptor->motive = MIN_MOTIVE;
        #ifdef ACTIVITY_TRACKING
        receptor->tracker.fire = false;
        receptor->tracker.enable = false;
        receptor->tracker.drive = false;
        #endif
    }
    for (neuronItr = motors.begin();
        neuronItr != motors.end(); neuronItr++)
    {
        motor = (Motor *)*neuronItr;
        motor->firingStrength = 0.0;
        motor->motive = MIN_MOTIVE;
        #ifdef ACTIVITY_TRACKING
        motor->tracker.fire = false;
        motor->tracker.enable = false;
        motor->tracker.drive = false;
        #endif
    }
    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        mediator->firingStrength = 0.0;
        mediator->motive = MIN_MOTIVE;
        mediator->expireEnablings(true);
        #ifdef ACTIVITY_TRACKING
        mediator->tracker.fire = false;
        mediator->tracker.enable = false;
        mediator->tracker.drive = false;
        #endif
    }
    for (i = 0, j = learningEvents.size(); i < j; i++)
    {
        for (learningEventItr = learningEvents[i].begin();
            learningEventItr != learningEvents[i].end(); learningEventItr++)
        {
            learningEvent = *learningEventItr;
            delete learningEvent;
        }
        learningEvents[i].clear();
    }
    eventClock = 0;
}


// Dump mediators and enablings.
void
Mona::dumpEnablings()
{
    int i,j;
    Mediator *mediator;
    list<Neuron *>::iterator neuronItr;
    Enabling *enabling;
    list<Enabling *>::iterator enablingItr;

    printf("Enabling dump:\n");
    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        printf("Mediator %d (%s), base=%f, enablement=%f\n", mediator->id.value,
            mediator->id.description, mediator->getBaseEnablement(), mediator->getEnablement());
        printf("Intermediate enablings:\n");
        for (i = 0, j = mediator->intermediateEnablings.size(); i < j; i++)
        {
            printf("Event %d:\n", i);
            for (enablingItr = mediator->intermediateEnablings[i]->enablings.begin();
                enablingItr != mediator->intermediateEnablings[i]->enablings.end(); enablingItr++)
            {
                enabling = *enablingItr;
                printf("enabling value=%f,age=%d,timerIndex=%d,causeBegin=%d,motive=%f,",
                    enabling->value, enabling->age, enabling->timerIndex, enabling->causeBegin,enabling->motive);
                if (enabling->newInSet) printf("new=true\n"); else printf("new=false\n");
            }
        }
        printf("Effect enablings:\n");
        for (enablingItr = mediator->effectEnablings.enablings.begin();
            enablingItr != mediator->effectEnablings.enablings.end(); enablingItr++)
        {
            enabling = *enablingItr;
            printf("enabling value=%f,age=%d,timerIndex=%d,causeBegin=%d,motive=%f,",
                enabling->value, enabling->age, enabling->timerIndex, enabling->causeBegin, enabling->motive);
            if (enabling->newInSet) printf("new=true\n"); else printf("new=false\n");
        }
        printf("Wagered enablings:\n");
        for (enablingItr = mediator->wageredEnablings.enablings.begin();
            enablingItr != mediator->wageredEnablings.enablings.end(); enablingItr++)
        {
            enabling = *enablingItr;
            printf("enabling value=%f,age=%d,timerIndex=%d,causeBegin=%d,motive=%f,",
                enabling->value, enabling->age, enabling->timerIndex, enabling->causeBegin, enabling->motive);
            if (enabling->newInSet) printf("new=true,"); else printf("new=false,");
            if (enabling->effectWager) printf("effect wager=true\n"); else printf("effect wager=false\n");
        }
    }
}
