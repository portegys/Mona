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

// Mona definitions.

#ifndef __MONA__
#define __MONA__

#include "../common/common.h"

#define MONA_VERSION "@(#) Mona version 3.1.1"

// Mona: sensory/response, neural network, and needs.
class Mona
{
    public:

        // Version.
        static void version();

        // Construct/destruct.
        Mona(int numSensors, int maxResponse, int numNeeds,
            RANDOM randomSeed=DEFAULT_RANDOM_SEED);
        Mona(int numSensors, int maxResponse,
            int numNeeds, int numEventTimers,
            RANDOM randomSeed=DEFAULT_RANDOM_SEED);
        void init(int numSensors, int maxResponse,
            int numNeeds, int numEventTimers);
        ~Mona();
        void clear();

        // Data types.
        typedef int SENSOR;
        typedef int RESPONSE;
        typedef double RESPONSE_POTENTIAL;
        typedef double MOTIVE;
        typedef double NEED;
        typedef double ENABLEMENT;
        typedef double WEIGHT;
        enum NEURON_TYPE { RECEPTOR, MOTOR, MEDIATOR };
        enum EVENT_TYPE
        {
            CAUSE_EVENT, RESPONSE_EVENT, INTERMEDIATE_EVENT,
            EFFECT_EVENT, WAGER_EVENT
        };
        enum EVENT_OUTCOME { EXPIRE, FIRE };

        // Forward declarations
        class Neuron;
        class Receptor;
        class Motor;
        class Mediator;
        class Enabling;
        class EnablingSet;
        class WagerHistory;
        class GoalValue;
        class MotiveAccum;

        // Parameters.
        static const NEED MAX_NEED;
        static const MOTIVE MIN_MOTIVE;
        static const int MAX_GOAL_VALUE_EVENTS;
        static const ENABLEMENT MAX_ENABLEMENT;
        static const ENABLEMENT MIN_ENABLEMENT;
        static const ENABLEMENT NEW_ENABLEMENT;
        static const WEIGHT DRIVE_ATTENUATION;
        enum { MAX_DRIVE_SOURCES=3 }
        MaxDriveSources;
        enum { MAX_DRIVE_ENABLERS=1 }
        MaxDriveEnablers;
        static const WEIGHT WAGER_HISTORY_UPDATE_VELOCITY;
        int MAX_MEDIATORS;
        static const int DEFAULT_MAX_MEDIATORS;
        static const int MAX_MEDIATOR_LEVEL;
        static const int MAX_MEDIATOR_CAUSES;
        static const int MAX_MEDIATOR_EVENTS;
        static const int MAX_MEDIATOR_SKEW;
        static const WEIGHT MIN_WAGER_WEIGHT;
        static const WEIGHT MOTIVE_LEARNING_DAMPER;
        static const WEIGHT STRENGTH_LEARNING_DAMPER;
        static const WEIGHT ENABLEMENT_LEARNING_DAMPER;
        static const WEIGHT EXPIRING_ENABLEMENT_LEARNING_DAMPER;
        static const double SENSOR_MASK_NEGATIVE_PROBABILITY;
        static const double SENSOR_MASK_POSITIVE_PROBABILITY;
        static const WEIGHT EXPIRATION_WEIGHT;
        static const WEIGHT RESPONSE_EXPIRATION_WEIGHT;
        static const double RESIDUAL_RESPONSE_RANDOMNESS;
        static const RANDOM DEFAULT_RANDOM_SEED;

    #ifdef MONA_TRACE
        // Tracing.
        bool traceSense;
        bool traceEnable;
        bool traceLearn;
        bool traceDrive;
        bool traceRespond;
    #endif

        // Include auxiliary classes.
    #include "mona-aux.hpp"

        // Sensors.
        SENSOR *sensors;
        int numSensors;

        // Response.
        RESPONSE response;
        RESPONSE maxResponse;
        WEIGHT responseRandomness;
        RESPONSE_POTENTIAL *responsePotentials;
        int *responseWork;
        RESPONSE responseOverride;
        RESPONSE_POTENTIAL *responseOverridePotentials;
        bool *responseInhibitors;

        // Needs.
        VALUE_SET needs;
        VALUE_SET oldNeeds;
        vector<char *> needDescriptions;
        int numNeeds;
        MOTIVE maxMotive;

        // Initialize and set needs.
        void initNeed(int index, NEED value, char *description);
        void setNeed(int index, NEED value);
        NEED getNeed(int index);

        // Event delay timers.
        int numEventTimers;
        vector<vector<TIME> > eventTimers;
        void allocEventTimers(int numTimers);

        // Behavior cycle.
        RESPONSE cycle(SENSOR *);
        void sense();
        void enable();
        void learn();
        void drive();
        void respond();

        // Sub-functions.
        void clearMotiveAccum();
        void setMotive();
        void dumpEnablings();

        // Event clock.
        TIME eventClock;

        // Learning event list.
        vector<list<LearningEvent *> > learningEvents;

        // Mediator creation functions.
        void createMediator(LearningEvent *effectEvent);
        void createConjunction(LearningEvent *effectEvent);
        void extendMediator(LearningEvent *effectEvent);

        // Random numbers.
        Random random;

        // Neuron.
        class Neuron
        {
            public:

                // Initialize/clear.
                void init(int numNeeds, char *description, Mona *mona);
                void clear();

                // Identifier.
                Identifier id;

                // Neuron type.
                NEURON_TYPE type;

                // Firing strength.
                ENABLEMENT firingStrength;

                // Goal value.
                GoalValue goals;

                // Get mediating enablement.
                ENABLEMENT getMediatingEnablement();

                // Get mediating wagered enablement.
                ENABLEMENT getMediatingWageredEnablement();

                // Accumulated enablings.
                ENABLEMENT enablingAccum,disablingAccum;
                void accumEnablings();

                // Drive.
                void drive(MotiveAccum &);
                MOTIVE motive;
                bool motiveValid;
                MotiveAccum accum[MAX_DRIVE_SOURCES][2];

                // Event notification.
                vector<struct Notify *> notifyList;

                // Global data.
                Mona *mona;

                // Load.
                void load(FILE *fp);

                // Save.
                void save(FILE *fp);

        #ifdef ACTIVITY_TRACKING
                // Track activity that neuron engages in.
                struct Activation
                {
                    bool fire;
                    bool enable;
                    bool drive;
                } tracker;
        #endif
        };

        // Receptor neuron.
        class Receptor : public Neuron
        {
            public:

                // Construct/destruct.
                Receptor(SENSOR *sensorMask, int numSensors,
                    int numNeeds, char *description, Mona *mona);
                ~Receptor();

                // Sense.
                SENSOR *sensorMask;
                void sense(SENSOR *, int numSensors);

                // Is given receptor a duplicate of this?
                bool isDuplicate(Receptor *);
                bool isDuplicate(SENSOR *);
                bool subsumes(Receptor *);

                // Print receptor.
                void print();

                // Load receptor.
                void load(FILE *fp);

                // Save receptor.
                void save(FILE *fp);
        };

        // Motor neuron.
        class Motor : public Neuron
        {
            public:

                // Construct/destruct.
                Motor(RESPONSE, int numNeeds,
                    char *description, Mona *mona);
                ~Motor();

                // Response.
                RESPONSE response;

                // Expire enabling on motor.
                void expireMotorEnabling();

                // Print motor.
                void print();

                // Load motor.
                void load(FILE *fp);

                // Save motor.
                void save(FILE *fp);

        #ifdef ACTIVITY_TRACKING
                vector<Neuron *> driverWork;
                vector<Neuron *> drivers;
        #endif
        };

        // Mediator neuron.
        // Mediates a sequence of neuron events.
        class Mediator : public Neuron
        {
            public:

                // Construct/destruct.
                Mediator(ENABLEMENT, bool enabler, WEIGHT utility,
                    int numNeeds, char *description, Mona *mona);
                ~Mediator();

                // Events.
                vector<Neuron *> causes;
                vector<Neuron *> intermediates;
                Neuron *effect;
                void addEvent(EVENT_TYPE, Neuron *);
                void eventFiring(EVENT_TYPE, WEIGHT strength,
                    int eventIndex, TIME eventBegin,
                    vector<struct ElemEvent> &events);

                // Instinct?
                bool instinct;
                bool isInstinct();

                // Mediator is enabler?
                bool enabler;

                // Base enablement.
                ENABLEMENT baseEnablement;

                // Intermediate and effect base enablements.
                // Each represent the base enablement of an
                // non-cause event given that prior events
                // have occurred.
                vector<ENABLEMENT> intermediateEnablements;
                ENABLEMENT effectEnablement;

                // Intermediate enablings.
                vector<EnablingSet *> intermediateEnablings;

                // Effect enablings and wagers.
                EnablingSet effectEnablings;
                EnablingSet wageredEnablings;

                // Get base enablement including portions in enablings.
                ENABLEMENT getBaseEnablement()
                {
                    return(baseEnablement + getBaseEnablementOut());
                }

                // Get base enablement out for enabling.
                ENABLEMENT getBaseEnablementOut()
                {
                    int i,j;
                    EnablingSet *enablingSet;
                    ENABLEMENT enablement;

                    enablement = 0.0;
                    for (i = 0, j = intermediateEnablings.size(); i < j; i++)
                    {
                        enablingSet = intermediateEnablings[i];
                        enablement += enablingSet->getValue();
                    }

                    return(enablement +
                        effectEnablings.getValue() +
                        wageredEnablings.getValue());
                }

                // How enabled is this mediator.
                ENABLEMENT getEnablement()
                {
                    return baseEnablement + getMediatingEnablement();
                }

                // Pending enablings.
                struct PendingEnabling
                {
                    int causeIndex;
                    WEIGHT strength;
                    TIME age;
                    TIME causeBegin;
                    vector<struct ElemEvent> events;
                };
                vector<list<struct PendingEnabling *> > pendingEnablings;

                // Event enabling.
                void enableEvent(WEIGHT strength, int causeIndex,
                    TIME causeBegin, vector<struct ElemEvent> &events);
                void accumEnabling(int causeIndex,
                    vector<struct PendingEnabling *> &);
                void attachEnabling(vector<struct PendingEnabling *> &);
                void wagerEnabling(WEIGHT strength, TIME causeBegin);

                // Enabling wager weights of varying durations.
                VALUE_SET timedWagerWeights;
                void createTimedWagerWeights();

                // Mark parasitic wagers.
                void markParasiticWagers();
                void markParasite(Enabling *wager);
                bool isParasite(Enabling *wager, vector<struct ElemEvent> events, bool trueWager=true);
                bool testEventStream(Enabling *sourceEnabling, vector<struct ElemEvent> sourceEvents,
                    Mediator *targetMediator, Enabling *targetEnabling,
                    vector<struct ElemEvent> targetEvents, bool trueWager);
                int containsEventStream(vector<struct ElemEvent> &s1, vector<struct ElemEvent> &s2);

                // Pay wagers.
                void payWagers();

                // Expire enablings.
                void expireEnablings(bool force=false);
                void expireResponseWagers(WEIGHT strength);

                // Wager history.
                WagerHistory wagerHistory;

                // Drive cause.
                void driveCause(MotiveAccum &);

                // Get drive weight.
                WEIGHT getDriveWeight(EVENT_TYPE, int eventIndex=0);

                // Learning variables:

                // Composition level.
                // Level 0 mediator is composed of non-mediator neurons.
                // Level n mediator is composed of at least one
                // level n-1 neuron.
                int level;

                // Mediator utility.
                WEIGHT  utility;
                WEIGHT getUtility();

                // Time of causation.
                TIME causeBegin;

                // Is given mediator a duplicate of this?
                bool isDuplicate(Mediator *);

                // Print mediator.
                void print(bool brief=false, int level=0);

                // Load mediator.
                void load(FILE *fp);

                // Save mediator.
                void save(FILE *fp);
        };

        // Unique identifier dispenser.
        int idDispenser;

        // Network.
        list<Neuron *> receptors;
        list<Neuron *> motors;
        list<Neuron *> mediators;

        // Add/delete neurons to/from network.
        Receptor *newReceptor(SENSOR *sensorMask, char *description);
        Motor *newMotor(RESPONSE response, char *description);
        Mediator *newMediator(ENABLEMENT enablement, bool enabler,
            WEIGHT utility, char *description);
        void deleteNeuron(Neuron *);

        // Clear working memory.
        void clearWorkingMemory();

        // Get mediators with worst and best utilities.
        Mediator *getWorstMediator();
        Mediator *getBestMediator();

        // Load network.
        static Mona *load(char *filename);
        static Mona *load(FILE *fp);
        void loadInstance(FILE *fp);
        Neuron *findByID(unsigned long id);

        // Save network.
        bool save(char *filename);
        void save(FILE *fp);

        // Dump network in textual or graphical (dot) format.
        enum DUMP_TYPE { TEXT, GRAPH };
    #ifdef ACTIVITY_TRACKING
        void dumpNetwork(enum DUMP_TYPE, char *title,
        bool enablementDump, bool driveDump,
        bool delimiter, FILE *out=stdout);
    #else
        void dumpNetwork(enum DUMP_TYPE, char *title,
        bool delimiter, FILE *out=stdout);
    #endif
        void dumpMediator(int id, enum DUMP_TYPE, char *title,
        FILE *out=stdout);
        void dumpNeuron(int id, FILE *out=stdout);
};
#endif
