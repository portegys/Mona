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

// Version.
void
Mona::version()
{
    const char *monaVersion = MONA_VERSION;
    printf("%s\n", &monaVersion[5]);
}


// Parameters.
const Mona::NEED Mona::MAX_NEED = 10.0;
const Mona::MOTIVE Mona::MIN_MOTIVE = 0.001;
const int Mona::MAX_GOAL_VALUE_EVENTS = 20;
const Mona::ENABLEMENT Mona::MAX_ENABLEMENT = 1.0;
const Mona::ENABLEMENT Mona::MIN_ENABLEMENT = 0.05;
const Mona::ENABLEMENT Mona::NEW_ENABLEMENT = 0.075;
const Mona::WEIGHT Mona::DRIVE_ATTENUATION = 0.99;
const Mona::WEIGHT Mona::WAGER_HISTORY_UPDATE_VELOCITY = 0.25;
const int Mona::DEFAULT_MAX_MEDIATORS = 400;
const int Mona::MAX_MEDIATOR_LEVEL = 1;
const int Mona::MAX_MEDIATOR_CAUSES = 3;
const int Mona::MAX_MEDIATOR_EVENTS = 40;
const int Mona::MAX_MEDIATOR_SKEW = 0;
const Mona::WEIGHT Mona::MIN_WAGER_WEIGHT = 0.01;
const Mona::WEIGHT Mona::MOTIVE_LEARNING_DAMPER = 0.001;
const Mona::WEIGHT Mona::STRENGTH_LEARNING_DAMPER = 0.1;
const Mona::WEIGHT Mona::ENABLEMENT_LEARNING_DAMPER = 0.1;
const Mona::WEIGHT Mona::EXPIRING_ENABLEMENT_LEARNING_DAMPER = 1.0;
const double Mona::SENSOR_MASK_NEGATIVE_PROBABILITY = 1.0;
const double Mona::SENSOR_MASK_POSITIVE_PROBABILITY = 0.1;
const Mona::WEIGHT Mona::EXPIRATION_WEIGHT = 1.0;
const Mona::WEIGHT Mona::RESPONSE_EXPIRATION_WEIGHT = 1.0;
const double Mona::RESIDUAL_RESPONSE_RANDOMNESS = 0.001;
const RANDOM Mona::DEFAULT_RANDOM_SEED = 4517;

// Behavior cycle.
Mona::RESPONSE
Mona::cycle(Mona::SENSOR *sensors)
{
    memcpy(this->sensors, sensors, sizeof(SENSOR) * numSensors);

    #ifdef ACTIVITY_TRACKING
    // Reset tracking.
    Receptor *receptor;
    list<Neuron *>::iterator listItr;

    for (listItr = receptors.begin();
        listItr != receptors.end(); listItr++)
    {
        receptor = (Receptor *)*listItr;
        receptor->tracker.fire = false;
        receptor->tracker.enable = false;
        receptor->tracker.drive = false;
    }
    Motor *motor;
    for (listItr = motors.begin();
        listItr != motors.end(); listItr++)
    {
        motor = (Motor *)*listItr;
        motor->tracker.fire = false;
        motor->tracker.enable = false;
        motor->tracker.drive = false;
    }
    Mediator *mediator;
    for (listItr = mediators.begin();
        listItr != mediators.end(); listItr++)
    {
        mediator = (Mediator *)*listItr;
        mediator->tracker.fire = false;
        mediator->tracker.enable = false;
        mediator->tracker.drive = false;
    }
    #endif

    sense();
    enable();
    learn();
    drive();
    respond();

    // To detect need changes.
    oldNeeds.load(needs);

    return(response);
}


// Mona constructor.
Mona::Mona(int numSensors, int maxResponse,
int numNeeds, RANDOM randomSeed)
{
    random.SRAND(randomSeed);
    init(numSensors, maxResponse, numNeeds, 1);
}


Mona::Mona(int numSensors, int maxResponse,
int numNeeds, int numEventTimers, RANDOM randomSeed)
{
    random.SRAND(randomSeed);
    init(numSensors, maxResponse, numNeeds, numEventTimers);
}


void
Mona::init(int numSensors, int maxResponse,
int numNeeds, int numEventTimers)
{
    int i;

    // Set I/O capabilities
    this->numSensors = numSensors;
    sensors = new SENSOR[numSensors];
    assert(sensors != NULL);
    this->maxResponse = maxResponse;
    responsePotentials = new RESPONSE_POTENTIAL[maxResponse+1];
    assert(responsePotentials != NULL);
    responseWork = new int[maxResponse+1];
    assert(responseWork != NULL);
    responseRandomness = RESIDUAL_RESPONSE_RANDOMNESS;
    responseOverride = -1;
    responseOverridePotentials = new RESPONSE_POTENTIAL[maxResponse+1];
    assert(responseOverridePotentials != NULL);
    for (i = 0; i <= maxResponse; i++)
    {
        responseOverridePotentials[i] = 0.0;
    }
    responseInhibitors = new bool[maxResponse+1];
    assert(responseInhibitors != NULL);
    for (i = 0; i <= maxResponse; i++)
    {
        responseInhibitors[i] = false;
    }

    // Create needs.
    this->numNeeds = numNeeds;
    needs.alloc(numNeeds);
    oldNeeds.alloc(numNeeds);
    needDescriptions.resize(numNeeds);
    maxMotive = (double)needs.size() * MAX_NEED;

    // Allocate event delay timers.
    this->numEventTimers = numEventTimers;
    allocEventTimers(numEventTimers);

    // Learning initialization.
    MAX_MEDIATORS = DEFAULT_MAX_MEDIATORS;
    idDispenser = 0;
    eventClock = 0;
    learningEvents.resize(MAX_MEDIATOR_LEVEL + 2);

    #ifdef MONA_TRACE
    // Tracing.
    traceSense = false;
    traceEnable = false;
    traceLearn = false;
    traceDrive = false;
    traceRespond = false;
    #endif
}


// Allocate event delay timers.
void Mona::allocEventTimers(int numTimers)
{
    int i,j;

    eventTimers.resize(MAX_MEDIATOR_LEVEL + 1);
    for (i = 0; i <= MAX_MEDIATOR_LEVEL; i++)
    {
        eventTimers[i].resize(numTimers);
        eventTimers[i][0] = (int)(pow(2.0, i));
        for (j = 1; j < numTimers; j++)
        {
            eventTimers[i][j] = eventTimers[i][0];
        }
    }
}


// Mona destructor.
Mona::~Mona()
{
    random.RAND_CLEAR();
    clear();
}


void
Mona::clear()
{
    int i,j;
    Neuron *neuron;
    Motor *motor;
    list<Neuron *> tmpList;
    list<Neuron *>::iterator neuronItr;
    char *description;
    vector<char *>::iterator descriptionItr;
    LearningEvent *learningEvent;
    list<LearningEvent *>::iterator learningEventItr;

    delete sensors;
    sensors = NULL;
    delete responsePotentials;
    responsePotentials = NULL;
    delete responseWork;
    responseWork = NULL;
    delete responseOverridePotentials;
    responseOverridePotentials = NULL;
    delete responseInhibitors;
    responseInhibitors = NULL;
    tmpList.clear();
    for (neuronItr = receptors.begin();
        neuronItr != receptors.end(); neuronItr++)
    {
        tmpList.push_back(*neuronItr);
    }

    // Deleting receptors will delete mediators.
    for (neuronItr = tmpList.begin();
        neuronItr != tmpList.end(); neuronItr++)
    {
        neuron = *neuronItr;
        deleteNeuron(neuron);
    }
    receptors.clear();
    for (neuronItr = motors.begin();
        neuronItr != motors.end(); neuronItr++)
    {
        motor = (Motor *)*neuronItr;
        delete motor;
    }
    motors.clear();
    needs.clear();
    oldNeeds.clear();
    for (descriptionItr = needDescriptions.begin();
        descriptionItr != needDescriptions.end(); descriptionItr++)
    {
        description = *descriptionItr;
        if (description != NULL)
        {
            delete description;
        }
    }
    needDescriptions.clear();
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
    learningEvents.clear();
    eventTimers.clear();
}


// Initialize neuron.
void
Mona::Neuron::init(int numNeeds, char *description, Mona *mona)
{
    id.setDescription(description);
    goals.setNumGoals(numNeeds);
    firingStrength = 0.0;
    this->mona = mona;
    #ifdef ACTIVITY_TRACKING
    tracker.fire = false;
    tracker.enable = false;
    tracker.drive = false;
    #endif
}


// Clear neuron.
void
Mona::Neuron::clear()
{
    int i,j;

    id.clear();
    goals.clear();
    motive = 0.0;
    motiveValid = false;
    for (i = 0; i < MAX_DRIVE_SOURCES; i++)
    {
        accum[i][0].clear();
        accum[i][1].clear();
    }

    for (i = 0, j = notifyList.size(); i < j; i++)
    {
        delete notifyList[i];
    }
    notifyList.clear();
}


// Load neuron.
void Mona::Neuron::load(FILE *fp)
{
    int i,j,k;
    struct Notify *notify;

    clear();
    id.load(fp);
    FREAD_INT(&i, fp);
    type = (NEURON_TYPE)i;
    FREAD_DOUBLE(&firingStrength, fp);
    goals.load(fp);
    FREAD_DOUBLE(&motive, fp);
    FREAD_BOOL(&motiveValid, fp);
    for (i = 0; i < MAX_DRIVE_SOURCES; i++)
    {
        accum[i][0].load(fp);
        accum[i][1].load(fp);
    }
    notifyList.clear();
    FREAD_INT(&i, fp);
    notifyList.resize(i);
    for (i = 0, j = notifyList.size(); i < j; i++)
    {
        notify = new struct Notify;
        assert(notify != NULL);
        FREAD_INT(&k, fp);
        notify->mediator = (Mediator *)k;
        FREAD_INT(&k, fp);
        notify->eventType = (EVENT_TYPE)k;
        FREAD_INT(&notify->eventIndex, fp);
        notifyList[i] = notify;
    }
}


// Save neuron.
void Mona::Neuron::save(FILE *fp)
{
    int i,j,k;
    struct Notify *notify;

    id.save(fp);
    i = (int)type;
    FWRITE_INT(&i, fp);
    FWRITE_DOUBLE(&firingStrength, fp);
    goals.save(fp);
    FWRITE_DOUBLE(&motive, fp);
    FWRITE_BOOL(&motiveValid, fp);
    for (i = 0; i < MAX_DRIVE_SOURCES; i++)
    {
        accum[i][0].save(fp);
        accum[i][1].save(fp);
    }
    i = notifyList.size();
    FWRITE_INT(&i, fp);
    for (i = 0, j = notifyList.size(); i < j; i++)
    {
        notify = notifyList[i];
        FWRITE_INT(&notify->mediator->id.value, fp);
        k = (int)notify->eventType;
        FWRITE_INT(&k, fp);
        FWRITE_INT(&notify->eventIndex, fp);
    }
}


// Create receptor and add to network.
Mona::Receptor *
Mona::newReceptor(SENSOR *sensorMask, char *description)
{
    Receptor *r;

    r = new Receptor(sensorMask, numSensors, numNeeds,
        description, this);
    assert(r != NULL);
    r->id.value = idDispenser;
    idDispenser++;
    receptors.push_back(r);
    return(r);
}


// Receptor constructor.
Mona::Receptor::Receptor(SENSOR *sensorMask, int numSensors,
int numNeeds, char *description, Mona *mona)
{
    init(numNeeds, description, mona);
    type = RECEPTOR;
    this->sensorMask = new SENSOR[numSensors];
    assert(this->sensorMask != NULL);
    memcpy(this->sensorMask, sensorMask, numSensors*sizeof(SENSOR));
    motive = MIN_MOTIVE;
}


// Is given receptor a duplicate of this?
bool Mona::Receptor::isDuplicate(Receptor *receptor)
{
    if (id.value == receptor->id.value) return false;
    for (int i = 0; i < mona->numSensors; i++)
    {
        if (sensorMask[i] != receptor->sensorMask[i]) return false;
    }
    return true;
}


bool Mona::Receptor::isDuplicate(SENSOR *sensorValues)
{
    for (int i = 0; i < mona->numSensors; i++)
    {
        if (sensorMask[i] != sensorValues[i]) return false;
    }
    return true;
}


// Does given receptor subsume this?
bool Mona::Receptor::subsumes(Receptor *receptor)
{
    if (id.value == receptor->id.value) return false;
    bool ret = false;
    for (int i = 0; i < mona->numSensors; i++)
    {
        if (sensorMask[i] != receptor->sensorMask[i])
        {
            if (sensorMask[i] == DONT_CARE) return false;
            if (receptor->sensorMask[i] == DONT_CARE)
            {
                ret = true;
            }
            else
            {
                return false;
            }
        }
    }
    return ret;
}


// Receptor destructor.
Mona::Receptor::~Receptor()
{
    clear();
    delete sensorMask;
    sensorMask = NULL;
}


// Print receptor.
void Mona::Receptor::print()
{
    int i;

    printf("Receptor={id=%d (%s),sensors=[", id.value, id.description);
    if (mona != NULL)
    {
        for (i = 0; i < mona->numSensors - 1; i++)
        {
            printf("%d,", sensorMask[i]);
        }
        printf("%d", sensorMask[i]);
    }
    printf("]}");
}


// Load receptor.
void Mona::Receptor::load(FILE *fp)
{
    int size;

    clear();
    delete sensorMask;
    ((Neuron *)this)->load(fp);
    FREAD_INT(&size, fp);
    sensorMask = new SENSOR[size];
    assert(sensorMask != NULL);
    for (int i = 0; i < size; i++)
    {
        FREAD_INT(&sensorMask[i], fp);
    }
}


// Save receptor.
void Mona::Receptor::save(FILE *fp)
{
    ((Neuron *)this)->save(fp);
    FWRITE_INT(&mona->numSensors, fp);
    for (int i = 0; i < mona->numSensors; i++)
    {
        FWRITE_INT(&sensorMask[i], fp);
    }
}


// Create motor and add to network.
Mona::Motor *
Mona::newMotor(RESPONSE response, char *description)
{
    Motor *m;

    m = new Motor(response, numNeeds, description, this);
    assert(m != NULL);
    m->id.value = idDispenser;
    idDispenser++;
    motors.push_back(m);
    return(m);
}


// Motor constructor.
Mona::Motor::Motor(RESPONSE response, int numNeeds,
char *description, Mona *mona)
{
    init(numNeeds, description, mona);
    type = MOTOR;
    this->response = response;
    motive = MIN_MOTIVE;
}


// Motor destructor.
Mona::Motor::~Motor()
{
    clear();
}


// Print motor.
void Mona::Motor::print()
{
    printf("Motor={id=%d (%s),response=%d}", id.value, id.description, response);
}


// Load motor.
void Mona::Motor::load(FILE *fp)
{
    clear();
    ((Neuron *)this)->load(fp);
    FREAD_INT(&response, fp);
}


// Save motor.
void Mona::Motor::save(FILE *fp)
{
    ((Neuron *)this)->save(fp);
    FWRITE_INT(&response, fp);
}


// Create mediator and add to network.
Mona::Mediator *
Mona::newMediator(ENABLEMENT baseEnablement,
bool enabler, WEIGHT utility, char *description)
{
    Mediator *m;

    m = new Mediator(baseEnablement, enabler,
        utility, numNeeds, description, this);
    assert(m != NULL);
    m->id.value = idDispenser;
    idDispenser++;
    mediators.push_back(m);
    return(m);
}


// Mediator constructor.
Mona::Mediator::Mediator(ENABLEMENT baseEnablement,
bool enabler, WEIGHT utility, int numNeeds,
char *description, Mona *mona)
{
    init(numNeeds, description, mona);
    type = MEDIATOR;
    instinct = false;
    effect = NULL;
    this->baseEnablement = effectEnablement = baseEnablement;
    wagerHistory.setHistory(baseEnablement / MAX_ENABLEMENT);
    this->enabler = enabler;
    createTimedWagerWeights();
    level = 0;
    motive = MIN_MOTIVE;
    this->utility = utility;
    causeBegin = 0;
}


// Is given mediator a duplicate of this?
bool Mona::Mediator::isDuplicate(Mediator *mediator)
{
    int i,j,p,q;

    if (id.value == mediator->id.value) return false;
    if (enabler != mediator->enabler) return false;
    if (causes.size() != mediator->causes.size()) return false;
    for (i = 0, j = causes.size(); i < j; i++)
    {
        for (p = 0, q = mediator->causes.size(); p < q; p++)
        {
            if (causes[i] == mediator->causes[p]) break;
        }
        if (p == q) return false;
    }
    if (intermediates.size() != mediator->intermediates.size()) return false;
    for (i = 0, j = intermediates.size(); i < j; i++)
    {
        if (intermediates[i] != mediator->intermediates[i]) return false;
    }
    if (effect != mediator->effect) return false;
    return true;
}


// Mediator destructor.
Mona::Mediator::~Mediator()
{
    int i,j;
    Neuron *neuron;
    struct Notify *notify;
    vector<struct Notify *>::iterator notifyItr;
    EnablingSet *enablingSet;
    struct PendingEnabling *pendingEnabling;
    list<struct PendingEnabling *>::iterator pendingEnablingItr;

    for (i = 0, j = causes.size(); i < j; i++)
    {
        neuron = causes[i];
        for (notifyItr = neuron->notifyList.begin();
            notifyItr != neuron->notifyList.end(); notifyItr++)
        {
            notify = *notifyItr;
            if (notify->mediator == this)
            {
                neuron->notifyList.erase(notifyItr);
                delete notify;
                break;
            }
        }
    }
    for (i = 0, j = intermediates.size(); i < j; i++)
    {
        neuron = intermediates[i];
        for (notifyItr = neuron->notifyList.begin();
            notifyItr != neuron->notifyList.end(); notifyItr++)
        {
            notify = *notifyItr;
            if (notify->mediator == this)
            {
                neuron->notifyList.erase(notifyItr);
                delete notify;
                break;
            }
        }
    }
    for (notifyItr = effect->notifyList.begin();
        notifyItr != effect->notifyList.end(); notifyItr++)
    {
        notify = *notifyItr;
        if (notify->mediator == this)
        {
            effect->notifyList.erase(notifyItr);
            delete notify;
            break;
        }
    }
    causes.clear();
    intermediates.clear();
    for (i = 0, j = intermediateEnablings.size(); i < j; i++)
    {
        enablingSet = intermediateEnablings[i];
        delete enablingSet;
    }
    intermediateEnablings.clear();
    effectEnablings.clear();
    wageredEnablings.clear();
    timedWagerWeights.clear();
    for (i = 0, j = pendingEnablings.size(); i < j; i++)
    {
        for (pendingEnablingItr = pendingEnablings[i].begin();
            pendingEnablingItr != pendingEnablings[i].end(); pendingEnablingItr++)
        {
            pendingEnabling = *pendingEnablingItr;
            delete pendingEnabling;
        }
        pendingEnablings[i].clear();
    }
    clear();
}


// Print mediator.
void Mona::Mediator::print(bool brief, int level)
{
    int i,j;

    if (!brief)
    {
        for (j = 0; j < level; j++) printf("    ");
    }
    printf("Mediator={");
    if (brief)
    {
        printf("id=%d (%s),instinct=");
        if (instinct) printf("true"); else printf("false");
        printf(",baseEnablement=%f,", id.value, id.description, getBaseEnablement());
        if (enabler)
        {
            printf("enabler=true,");
        }
        else
        {
            printf("enabler=false,");
        }
        printf("level=%d,", this->level);
        printf("utility=%f", utility);
        printf(",causes={");
    }
    else
    {
        printf("\n");
        for (j = 0; j < level; j++) printf("    ");
        printf("  id=%d (%s)\n", id.value, id.description);
        for (j = 0; j < level; j++) printf("    ");
        printf("  instinct=");
        if (instinct) printf("true"); else printf("false");
        printf("\n");
        for (j = 0; j < level; j++) printf("    ");
        printf("  baseEnablement=%f\n", getBaseEnablement());
        for (j = 0; j < level; j++) printf("    ");
        if (enabler)
        {
            printf("  enabler=true\n");
        }
        else
        {
            printf("  enabler=false\n");
        }
        for (j = 0; j < level; j++) printf("    ");
        printf("  level=%d\n", this->level);
        for (j = 0; j < level; j++) printf("    ");
        printf("  utility=%f\n", utility);
        for (j = 0; j < level; j++) printf("    ");
        printf("  causes={\n");
    }
    for (i = 0; i < causes.size(); i++)
    {
        if (causes[i]->type == MEDIATOR)
        {
            if (brief)
            {
                printf("id=%d (%s)",causes[i]->id.value,causes[i]->id.description);
            }
            else
            {
                ((Mediator *)causes[i])->print(false, level + 1);
            }
        }
        else
        {
            if (!brief)
            {
                for (j = 0; j <= level; j++) printf("    ");
            }
            ((Receptor *)causes[i])->print();
        }
        if (i < causes.size() - 1)
        {
            if (brief) printf(","); else printf("\n");
        }
    }
    if (brief)
    {
        printf("},");
    }
    else
    {
        printf("\n");
        for (j = 0; j < level; j++) printf("    ");
        printf("  }\n");
    }
    if (brief)
    {
        printf("intermediates={");
    }
    else
    {
        for (j = 0; j < level; j++) printf("    ");
        printf("  intermediates={\n");
    }
    for (i = 0; i < intermediates.size(); i++)
    {
        if (intermediates[i]->type == MEDIATOR)
        {
            if (brief)
            {
                printf("id=%d (%s)",intermediates[i]->id.value,intermediates[i]->id.description);
            }
            else
            {
                ((Mediator *)intermediates[i])->print(false, level + 1);
                printf("\n");
            }
        }
        else
        {
            if (!brief)
            {
                for (j = 0; j <= level; j++) printf("    ");
            }
            if (intermediates[i]->type == RECEPTOR)
            {
                ((Receptor *)intermediates[i])->print();
            }
            else
            {
                ((Motor *)intermediates[i])->print();
            }
            if (!brief)
            {
                printf("\n");
                for (j = 0; j <= level; j++) printf("    ");
                printf("intermediate enablement=%f\n", intermediateEnablements[i]);
            }
        }
        if (brief)
        {
            if (i < intermediates.size() - 1) printf(",");
        }
    }
    if (brief)
    {
        printf("},");
    }
    else
    {
        for (j = 0; j < level; j++) printf("    ");
        printf("  }\n");
    }
    if (brief)
    {
        printf("effect={");
    }
    else
    {
        for (j = 0; j < level; j++) printf("    ");
        printf("  effect={\n");
    }
    if (effect->type == MEDIATOR)
    {
        if (brief)
        {
            printf("id=%d (%s)",effect->id.value,effect->id.description);
        }
        else
        {
            ((Mediator *)effect)->print(false, level + 1);
        }
    }
    else
    {
        if (!brief)
        {
            for (j = 0; j <= level; j++) printf("    ");
        }
        ((Receptor *)effect)->print();
    }
    if (brief)
    {
        printf("}}\n");
    }
    else
    {
        printf("\n");
        for (j = 0; j <= level; j++) printf("    ");
        printf("effect enablement=%f\n", effectEnablement);
        for (j = 0; j < level; j++) printf("    ");
        printf("  }\n");
        for (j = 0; j < level; j++) printf("    ");
        printf("}");
        if (level == 0) printf("\n");
    }
}


// Load mediator.
void Mona::Mediator::load(FILE *fp)
{
    int i,j,k,p,q;
    double d;
    EnablingSet *enablingSet;
    struct PendingEnabling *pendingEnabling;

    clear();
    ((Neuron *)this)->load(fp);
    FREAD_INT(&i, fp);
    causes.resize(i);
    for (i = 0, j = causes.size(); i < j; i++)
    {
        FREAD_INT(&k, fp);
        causes[i] = (Neuron *)k;
    }
    FREAD_INT(&i, fp);
    intermediates.resize(i);
    for (i = 0, j = intermediates.size(); i < j; i++)
    {
        FREAD_INT(&k, fp);
        intermediates[i] = (Neuron *)k;
    }
    FREAD_INT(&k, fp);
    effect = (Neuron *)k;
    FREAD_BOOL(&enabler, fp);
    FREAD_BOOL(&instinct, fp);
    FREAD_DOUBLE(&baseEnablement, fp);
    FREAD_INT(&i, fp);
    intermediateEnablements.resize(i);
    for (i = 0, j = intermediateEnablements.size(); i < j; i++)
    {
        FREAD_DOUBLE(&d, fp);
        intermediateEnablements[i] = d;
    }
    FREAD_DOUBLE(&effectEnablement, fp);
    FREAD_INT(&i, fp);
    intermediateEnablings.resize(i);
    for (i = 0, j = intermediateEnablings.size(); i < j; i++)
    {
        enablingSet = new EnablingSet();
        assert(enablingSet != NULL);
        enablingSet->load(fp);
        intermediateEnablings[i] = enablingSet;
    }
    effectEnablings.load(fp);
    wageredEnablings.load(fp);
    FREAD_INT(&i, fp);
    pendingEnablings.resize(i);
    for (i = 0, j = pendingEnablings.size(); i < j; i++)
    {
        FREAD_INT(&q, fp);
        for (p = 0; p < q; p++)
        {
            pendingEnabling = new struct PendingEnabling;
            assert(pendingEnabling != NULL);
            FREAD_INT(&pendingEnabling->causeIndex, fp);
            FREAD_DOUBLE(&pendingEnabling->strength, fp);
            FREAD_LONG(&pendingEnabling->age, fp);
            FREAD_LONG(&pendingEnabling->causeBegin, fp);
            pendingEnablings[i].push_back(pendingEnabling);
        }
    }
    timedWagerWeights.load(fp);
    wagerHistory.load(fp);
    FREAD_INT(&level, fp);
    FREAD_DOUBLE(&utility, fp);
    FREAD_LONG(&causeBegin, fp);
}


// Save mediator.
void Mona::Mediator::save(FILE *fp)
{
    int i,j,k;
    double d;
    EnablingSet *enablingSet;
    struct PendingEnabling *pendingEnabling;
    list<struct PendingEnabling *>::iterator pendingEnablingItr;

    ((Neuron *)this)->save(fp);
    i = causes.size();
    FWRITE_INT(&i, fp);
    for (i = 0, j = causes.size(); i < j; i++)
    {
        k = causes[i]->id.value;
        FWRITE_INT(&k, fp);
    }
    i = intermediates.size();
    FWRITE_INT(&i, fp);
    for (i = 0, j = intermediates.size(); i < j; i++)
    {
        k = intermediates[i]->id.value;
        FWRITE_INT(&k, fp);
    }
    k = effect->id.value;
    FWRITE_INT(&k, fp);
    FWRITE_BOOL(&enabler, fp);
    FWRITE_BOOL(&instinct, fp);
    FWRITE_DOUBLE(&baseEnablement, fp);
    i = intermediateEnablements.size();
    FWRITE_INT(&i, fp);
    for (i = 0, j = intermediateEnablements.size(); i < j; i++)
    {
        d = intermediateEnablements[i];
        FWRITE_DOUBLE(&d, fp);
    }
    FWRITE_DOUBLE(&effectEnablement, fp);
    i = intermediateEnablings.size();
    FWRITE_INT(&i, fp);
    for (i = 0, j = intermediateEnablings.size(); i < j; i++)
    {
        enablingSet = intermediateEnablings[i];
        enablingSet->save(fp);
    }
    effectEnablings.save(fp);
    wageredEnablings.save(fp);
    k = pendingEnablings.size();
    FWRITE_INT(&k, fp);
    for (i = 0, j = pendingEnablings.size(); i < j; i++)
    {
        k = pendingEnablings[i].size();
        FWRITE_INT(&k, fp);
        for (pendingEnablingItr = pendingEnablings[i].begin();
            pendingEnablingItr != pendingEnablings[i].end(); pendingEnablingItr++)
        {
            pendingEnabling = *pendingEnablingItr;
            FWRITE_INT(&pendingEnabling->causeIndex, fp);
            FWRITE_DOUBLE(&pendingEnabling->strength, fp);
            FWRITE_LONG(&pendingEnabling->age, fp);
            FWRITE_LONG(&pendingEnabling->causeBegin, fp);
        }
    }
    timedWagerWeights.save(fp);
    wagerHistory.save(fp);
    FWRITE_INT(&level, fp);
    FWRITE_DOUBLE(&utility, fp);
    FWRITE_LONG(&causeBegin, fp);
}


// Create timed wager weights array.
void
Mona::Mediator::createTimedWagerWeights()
{
    timedWagerWeights.alloc(mona->numEventTimers);
    for (int i = 0; i < mona->numEventTimers; i++)
    {
        timedWagerWeights.set(i, 1.0 / mona->numEventTimers);
    }
}


// Add mediator event.
void
Mona::Mediator::addEvent(EVENT_TYPE type, Neuron *neuron)
{
    int i,j,k;
    struct Notify *notify;
    Mediator *mediator;
    EnablingSet *enablingSet;

    switch(type)
    {
        case CAUSE_EVENT:
            causes.push_back(neuron);
            pendingEnablings.resize(causes.size());
            assert(causes.size() <= MAX_MEDIATOR_CAUSES);
            break;
        case INTERMEDIATE_EVENT:
            intermediates.push_back(neuron);
            intermediateEnablements.push_back(baseEnablement);
            enablingSet = new EnablingSet();
            assert(enablingSet != NULL);
            intermediateEnablings.push_back(enablingSet);
            assert(intermediates.size() <= MAX_MEDIATOR_EVENTS - 2);
            break;
        case EFFECT_EVENT:
            effect = neuron;
            break;
    }
    if (neuron->type == MEDIATOR)
    {
        mediator = (Mediator *)neuron;
        if (mediator->level + 1 > level)
        {
            level = mediator->level + 1;
            assert(level <= MAX_MEDIATOR_LEVEL);
        }
    }
    notify = new struct Notify;
    assert(notify != NULL);
    notify->mediator = this;
    notify->eventType = type;
    switch(type)
    {
        case CAUSE_EVENT:
            notify->eventIndex = causes.size() - 1;
            break;
        case INTERMEDIATE_EVENT:
            notify->eventIndex = intermediates.size() - 1;
            break;
        case EFFECT_EVENT:
            notify->eventIndex = -1;
            break;
    }
    neuron->notifyList.push_back(notify);

    // Sort by type: effect, intermediate, cause.
    for (i = 0, j = neuron->notifyList.size(); i < j; i++)
    {
        for (k = i + 1; k < j; k++)
        {
            switch(neuron->notifyList[k]->eventType)
            {
                case EFFECT_EVENT:
                    switch(neuron->notifyList[i]->eventType)
                    {
                        case EFFECT_EVENT: break;
                        case INTERMEDIATE_EVENT:
                        case CAUSE_EVENT:
                            notify = neuron->notifyList[i];
                            neuron->notifyList[i] = neuron->notifyList[k];
                            neuron->notifyList[k] = notify;
                            break;
                    }
                    break;
                case INTERMEDIATE_EVENT:
                    switch(neuron->notifyList[i]->eventType)
                    {
                        case EFFECT_EVENT: break;
                        case INTERMEDIATE_EVENT: break;
                        case CAUSE_EVENT:
                            notify = neuron->notifyList[i];
                            neuron->notifyList[i] = neuron->notifyList[k];
                            neuron->notifyList[k] = notify;
                            break;
                    }
                    break;
                case CAUSE_EVENT:
                    break;
            }
        }
    }
}


// Get mediator with worst utility.
// Skip instinctive mediators.
Mona::Mediator *
Mona::getWorstMediator()
{
    Mediator *mediator;
    list<Neuron *>::iterator neuronItr;
    vector<Mediator *>worstMediators;
    WEIGHT utility,worstUtility;

    worstUtility = 0.0;
    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        if (mediator->isInstinct()) continue;
        utility = mediator->getUtility();
        if (worstMediators.size() == 0 || utility < worstUtility)
        {
            worstMediators.clear();
            worstMediators.push_back(mediator);
            worstUtility = utility;
        } else if (fabs(utility - worstUtility) <= NEARLY_ZERO)
        {
            worstMediators.push_back(mediator);
        }
    }
    if (worstMediators.size() == 0)
    {
        return NULL;
    }
    else
    {
        // Return a random choice of worst.
        return worstMediators[random.RAND_CHOICE(worstMediators.size())];
    }
}


// Get mediator with best utility.
Mona::Mediator *
Mona::getBestMediator()
{
    Mediator *mediator;
    list<Neuron *>::iterator neuronItr;
    vector<Mediator *>bestMediators;
    WEIGHT utility,bestUtility;

    bestUtility = 0.0;
    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        utility = mediator->getUtility();
        if (bestMediators.size() == 0 || utility > bestUtility)
        {
            bestMediators.clear();
            bestMediators.push_back(mediator);
            bestUtility = utility;
        } else if (fabs(utility - bestUtility) <= NEARLY_ZERO)
        {
            bestMediators.push_back(mediator);
        }
    }
    if (bestMediators.size() == 0)
    {
        return NULL;
    }
    else
    {
        // Return a random choice.
        return bestMediators[random.RAND_CHOICE(bestMediators.size())];
    }
}


// Get mediator utility.
Mona::WEIGHT
Mona::Mediator::getUtility()
{
    int i,j;
    Mediator *mediator;
    WEIGHT bestUtility,utilityWork;

    bestUtility = utility;
    for (i = 0, j = notifyList.size(); i < j; i++)
    {
        mediator = notifyList[i]->mediator;
        utilityWork = mediator->getUtility();
        if (utilityWork > bestUtility)
        {
            bestUtility = utilityWork;
        }
    }
    return bestUtility;
}


// Is mediator an instinct?
bool
Mona::Mediator::isInstinct()
{
    int i,j;
    Mediator *mediator;

    if (instinct) return true;
    for (i = 0, j = notifyList.size(); i < j; i++)
    {
        mediator = notifyList[i]->mediator;
        if (mediator->isInstinct()) return true;
    }
    return false;
}


// Remove neuron from network.
void
Mona::deleteNeuron(Neuron *neuron)
{
    int i,j;
    struct Notify *notify;
    Mediator *parent;
    list<Mediator *> parents;
    list<Mediator *>::iterator parentsItr;
    bool duplicate;
    LearningEvent *learningEvent;
    list<LearningEvent *>::iterator learningEventItr;

    // Delete parents.
    for (i = 0, j = neuron->notifyList.size(); i < j; i++)
    {
        notify = neuron->notifyList[i];
        duplicate = false;
        for (parentsItr = parents.begin();
            parentsItr != parents.end(); parentsItr++)
        {
            parent = *parentsItr;
            if (parent == notify->mediator)
            {
                duplicate = true;
                break;
            }
        }
        if (!duplicate) parents.push_back(notify->mediator);
    }
    for (parentsItr = parents.begin();
        parentsItr != parents.end(); parentsItr++)
    {
        parent = *parentsItr;
        deleteNeuron((Neuron *)parent);
    }

    // Remove from learning space.
    if (neuron->type != MEDIATOR)
    {
        i = 0;
    }
    else
    {
        i = ((Mediator *)neuron)->level + 1;
    }
    if (i < learningEvents.size())
    {
        for (learningEventItr = learningEvents[i].begin();
            learningEventItr != learningEvents[i].end();)
        {
            learningEvent = *learningEventItr;
            if (learningEvent->neuron == neuron)
            {
                learningEvents[i].erase(learningEventItr);
                delete learningEvent;
                learningEventItr = learningEvents[i].begin();
            }
            else
            {
                learningEventItr++;
            }
        }
    }

    // Delete neuron.
    switch(neuron->type)
    {
        case RECEPTOR:
            receptors.remove(neuron);
            delete (Receptor *)neuron;
            break;
        case MOTOR:
            motors.remove(neuron);
            delete (Motor *)neuron;
            break;
        case MEDIATOR:
            mediators.remove(neuron);
            delete (Mediator *)neuron;
            break;
    }
}


// Initialize need.
void
Mona::initNeed(int index, NEED value, char *description)
{
    needs.set(index, value);
    oldNeeds.set(index, value);
    if (description == NULL) description = "<no_description>";
    needDescriptions[index] = new char[strlen(description)+1];
    assert(needDescriptions[index] != NULL);
    strcpy(needDescriptions[index], description);
}


// Set need.
void
Mona::setNeed(int index, NEED value)
{
    assert(index >= 0 && index < numNeeds);
    needs.set(index, value);
}


// Get need.
Mona::NEED
Mona::getNeed(int index)
{
    assert(index >= 0 && index < numNeeds);
    return(needs.get(index));
}


// Load network.
Mona *
Mona::load(char *filename)
{
    FILE *fp;
    Mona *mona;

    if ((fp = fopen(filename, "r")) == NULL)
    {
        return NULL;
    }
    mona = load(fp);
    fclose(fp);
    return mona;
}


Mona *
Mona::load(FILE *fp)
{
    int numSensors, maxResponse, numNeeds, numEventTimers;
    Mona *mona;

    // Create mona.
    FREAD_INT(&numSensors, fp);
    FREAD_INT(&maxResponse, fp);
    FREAD_INT(&numNeeds, fp);
    FREAD_INT(&numEventTimers, fp);
    mona = new Mona(numSensors, maxResponse,
        numNeeds, numEventTimers);
    assert(mona != NULL);

    // Load instance.
    mona->loadInstance(fp);
    return mona;
}


void
Mona::loadInstance(FILE *fp)
{
    int i,j,p,q;
    LearningEvent *learningEvent;
    list<LearningEvent *>::iterator learningEventItr;
    Receptor *receptor;
    Motor *motor;
    Mediator *mediator;
    list<Neuron *>::iterator neuronItr;
    struct Notify *notify;

    clear();
    random.RAND_LOAD(fp);
    init(numSensors, maxResponse, numNeeds, numEventTimers);
    for (i = 0; i < numSensors; i++)
    {
        FREAD_INT(&sensors[i], fp);
    }
    FREAD_INT(&response, fp);
    needs.load(fp);
    oldNeeds.load(fp);
    for (i = 0; i < numNeeds; i++)
    {
        FREAD_INT(&j, fp);
        if (j > 0)
        {
            needDescriptions[i] = new char[j];
            assert(needDescriptions[i] != NULL);
            FREAD_STRING(needDescriptions[i], j, fp);
        }
    }
    for (i = 0; i <= MAX_MEDIATOR_LEVEL; i++)
    {
        for (j = 0; j < numEventTimers; j++)
        {
            FREAD_LONG(&eventTimers[i][j], fp);
        }
    }
    FREAD_LONG(&eventClock, fp);
    for (i = 0, j = learningEvents.size(); i < j; i++)
    {
        FREAD_INT(&q, fp);
        for (p = 0; p < q; p++)
        {
            learningEvent = new LearningEvent();
            assert(learningEvent != NULL);
            learningEvent->load(fp);
            learningEvents[i].push_back(learningEvent);
        }
    }
    idDispenser = 0;
    FREAD_INT(&j, fp);
    for (i = 0; i < j; i++)
    {
        receptor =
            new Receptor(sensors, numSensors, numNeeds,
            NULL, this);
        assert(receptor != NULL);
        receptor->load(fp);
        receptor->mona = this;
        receptors.push_back(receptor);
        if (receptor->id.value > idDispenser)
        {
            idDispenser = receptor->id.value + 1;
        }
    }
    FREAD_INT(&j, fp);
    for (i = 0; i < j; i++)
    {
        motor = new Motor(0, numNeeds, NULL, this);
        assert(motor != NULL);
        motor->load(fp);
        motor->mona = this;
        motors.push_back(motor);
        if (motor->id.value > idDispenser)
        {
            idDispenser = motor->id.value + 1;
        }
    }
    FREAD_INT(&j, fp);
    for (i = 0; i < j; i++)
    {
        mediator = new Mediator(0.0, true, 0.0,
            numNeeds, NULL, this);
        assert(mediator != NULL);
        mediator->load(fp);
        mediator->mona = this;
        mediators.push_back(mediator);
        if (mediator->id.value > idDispenser)
        {
            idDispenser = mediator->id.value + 1;
        }
    }

    // Resolve neuron addresses using id.
    for (neuronItr = receptors.begin();
        neuronItr != receptors.end(); neuronItr++)
    {
        receptor = (Receptor *)*neuronItr;
        for (i = 0, j = receptor->notifyList.size(); i < j; i++)
        {
            notify = receptor->notifyList[i];
            notify->mediator =
                (Mediator *)findByID((unsigned long)(notify->mediator));
        }
    }
    for (neuronItr = motors.begin();
        neuronItr != motors.end(); neuronItr++)
    {
        motor = (Motor *)*neuronItr;
        for (i = 0, j = motor->notifyList.size(); i < j; i++)
        {
            notify = motor->notifyList[i];
            notify->mediator =
                (Mediator *)findByID((unsigned long)(notify->mediator));
        }
    }
    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        for (i = 0, j = mediator->causes.size(); i < j; i++)
        {
            mediator->causes[i] =
                findByID((unsigned long)(mediator->causes[i]));
        }
        for (i = 0, j = mediator->intermediates.size(); i < j; i++)
        {
            mediator->intermediates[i] =
                findByID((unsigned long)(mediator->intermediates[i]));
        }
        mediator->effect =
            findByID((unsigned long)(mediator->effect));
        for (i = 0, j = mediator->notifyList.size(); i < j; i++)
        {
            notify = mediator->notifyList[i];
            notify->mediator =
                (Mediator *)findByID((unsigned long)(notify->mediator));
        }
    }
    for (i = 0, j = learningEvents.size(); i < j; i++)
    {
        for (learningEventItr = learningEvents[i].begin();
            learningEventItr != learningEvents[i].end(); learningEventItr++)
        {
            learningEvent = *learningEventItr;
            learningEvent->neuron =
                findByID((unsigned long)(learningEvent->neuron));
        }
    }
}


// Find neuron by id.
Mona::Neuron *
Mona::findByID(unsigned long id)
{
    Receptor *receptor;
    Motor *motor;
    Mediator *mediator;
    list<Neuron *>::iterator neuronItr;

    for (neuronItr = receptors.begin();
        neuronItr != receptors.end(); neuronItr++)
    {
        receptor = (Receptor *)*neuronItr;
        if (receptor->id.value == id) return (Neuron *)receptor;
    }
    for (neuronItr = motors.begin();
        neuronItr != motors.end(); neuronItr++)
    {
        motor = (Motor *)*neuronItr;
        if (motor->id.value == id) return (Neuron *)motor;
    }
    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        if (mediator->id.value == id) return (Neuron *)mediator;
    }
    return NULL;
}


// Save network.
bool
Mona::save(char *filename)
{
    FILE *fp;

    if ((fp = fopen(filename, "w")) == NULL)
    {
        return false;
    }
    save(fp);
    fclose(fp);
    return true;
}


void
Mona::save(FILE *fp)
{
    int i,j,k;
    LearningEvent *learningEvent;
    list<LearningEvent *>::iterator learningEventItr;
    Receptor *receptor;
    Motor *motor;
    Mediator *mediator;
    list<Neuron *>::iterator neuronItr;

    FWRITE_INT(&numSensors, fp);
    FWRITE_INT(&maxResponse, fp);
    FWRITE_INT(&numNeeds, fp);
    FWRITE_INT(&numEventTimers, fp);
    random.RAND_SAVE(fp);
    for (i = 0; i < numSensors; i++)
    {
        FWRITE_INT(&sensors[i], fp);
    }
    FWRITE_INT(&response, fp);
    needs.save(fp);
    oldNeeds.save(fp);
    for (i = 0; i < numNeeds; i++)
    {
        if (needDescriptions[i] != NULL)
        {
            j = strlen(needDescriptions[i]) + 1;
            FWRITE_INT(&j, fp);
            FWRITE_STRING(needDescriptions[i], j, fp);
        }
        else
        {
            j = 0;
            FWRITE_INT(&j, fp);
        }
    }
    for (i = 0; i <= MAX_MEDIATOR_LEVEL; i++)
    {
        for (j = 0; j < numEventTimers; j++)
        {
            FWRITE_LONG(&eventTimers[i][j], fp);
        }
    }
    FWRITE_LONG(&eventClock, fp);
    for (i = 0, j = learningEvents.size(); i < j; i++)
    {
        k = learningEvents[i].size();
        FWRITE_INT(&k, fp);
        for (learningEventItr = learningEvents[i].begin();
            learningEventItr != learningEvents[i].end(); learningEventItr++)
        {
            learningEvent = *learningEventItr;
            learningEvent->save(fp);
        }
    }
    i = receptors.size();
    FWRITE_INT(&i, fp);
    for (neuronItr = receptors.begin();
        neuronItr != receptors.end(); neuronItr++)
    {
        receptor = (Receptor *)*neuronItr;
        receptor->save(fp);
    }
    i = motors.size();
    FWRITE_INT(&i, fp);
    for (neuronItr = motors.begin();
        neuronItr != motors.end(); neuronItr++)
    {
        motor = (Motor *)*neuronItr;
        motor->save(fp);
    }
    i = mediators.size();
    FWRITE_INT(&i, fp);
    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        mediator->save(fp);
    }
}
