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

// Mona auxiliary classes.

#ifndef __MONA_AUX__
#define __MONA_AUX__

class LearningEvent;

// Identification.
class Identifier
{
    public:
        int value;
        char *description;

        // Constructor.
        Identifier()
        {
            description = NULL;
        }

        // Destructor.
        ~Identifier()
        {
            clear();
        }

        // Equality test.
        bool equals(Neuron *n)
        {
            return(n->id.value == value);
        }

        // Clear.
        void clear()
        {
            if (description != NULL) delete description;
            description = NULL;
        }

        // Set description.
        void setDescription(char *description)
        {
            if (description == NULL) description = "<no_description>";
            if (this->description != NULL) delete this->description;
            this->description = new char[strlen(description)+1];
            assert(this->description != NULL);
            strcpy(this->description, description);
        }

        // Load identifier.
        void load(FILE *fp)
        {
            int size;

            clear();
            FREAD_INT(&value, fp);
            FREAD_INT(&size, fp);
            if (size > 0)
            {
                description = new char[size];
                assert(description != NULL);
                FREAD_STRING(description, size, fp);
            }
            else
            {
                description = NULL;
            }
        }

        // Save identifier.
        void save(FILE *fp)
        {
            int size;

            FWRITE_INT(&value, fp);
            if (description != NULL)
            {
                size = strlen(description) + 1;
            }
            else
            {
                size = 0;
            }
            FWRITE_INT(&size, fp);
            if (description != NULL)
            {
                FWRITE_STRING(description, size, fp);
            }
        }
};

// Goal values.
class GoalValue
{
    public:
        VALUE_SET goals;
        WEIGHT weight;

        // Need change event.
        class Event
        {
            public:
                VALUE_SET needBefore;
                VALUE_SET needAfter;
                WEIGHT weight;

                // Constructor.
                Event(VALUE_SET &needBefore, VALUE_SET &needAfter,
                    WEIGHT weight)
                {
                    this->needBefore.load(needBefore);
                    this->needAfter.load(needAfter);
                    this->weight = weight;
                }
                Event() {}

                // Load.
                void load(FILE *fp)
                {
                    needBefore.load(fp);
                    needAfter.load(fp);
                    FREAD_DOUBLE(&weight, fp);
                }

                // Save.
                void save(FILE *fp)
                {
                    needBefore.save(fp);
                    needAfter.save(fp);
                    FWRITE_DOUBLE(&weight, fp);
                }
        };

        // List of need change events.
        list<Event *> events;

        // Constructors.
        GoalValue(int numGoals)
        {
            goals.alloc(numGoals);
            weight = 0.0;
        }
        GoalValue()
        {
            weight = 0.0;
        }

        // Destructor.
        ~GoalValue()
        {
            clear();
        }

        // Get total goal value.
        NEED getValue()
        {
            return(goals.sum());
        }

        // Get specific goal value.
        NEED getValue(int index)
        {
            return(goals.get(index));
        }

        // Set initial goals.
        void setGoals(VALUE_SET &goals, WEIGHT weight)
        {
            int i,j;
            Event *event;
            list<Event *>::iterator eventItr;
            VALUE_SET needBefore, needAfter;

            this->goals.load(goals);
            this->weight = weight;

            // Clear history.
            for (eventItr = events.begin();
                eventItr != events.end(); eventItr++)
            {
                event = *eventItr;
                delete event;
            }
            events.clear();

            // Create supporting history event.
            needBefore.alloc(goals.size());
            needAfter.alloc(goals.size());
            for (i = 0, j = goals.size(); i < j; i++)
            {
                needBefore.set(i, goals.get(i));
                needAfter.set(i, 0.0);
            }
            update(needBefore, needAfter, 1.0);
        }

        // Get number of goals.
        int getNumGoals()
        {
            return(goals.size());
        }

        // Set number of goals.
        void setNumGoals(int numGoals)
        {
            goals.alloc(numGoals);
        }

        // Update goal value.
        void update(VALUE_SET &needBefore,
            VALUE_SET &needAfter, WEIGHT weight)
        {
        #ifdef NEVER
            int i,j;
            Event *event;
            list<Event *>::iterator eventItr;
            NEED delta;
            WEIGHT sum;

            // Store event.
            if (weight == 0.0) return;
            event = new Event(needBefore, needAfter, weight);
            assert(event != NULL);
            events.push_back(event);
            if (events.size() > MAX_GOAL_VALUE_EVENTS)
            {
                event = events.front();
                events.pop_front();
                delete event;
            }

            // Update goal value.
            for (i = 0, j = goals.size(); i < j; i++)
            {
                delta = sum = 0.0;
                for (eventItr = events.begin();
                    eventItr != events.end(); eventItr++)
                {
                    event = *eventItr;
                    delta +=
                        (event->needBefore.get(i) - event->needAfter.get(i)) *
                        event->weight;
                    sum += event->weight;
                }
                if (sum > 0.0)
                {
                    goals.set(i, delta / sum);
                }
                else
                {
                    goals.set(i, 0.0);
                }
            }
        #endif
        }

        // Clear.
        void clear()
        {
            Event *event;
            list<Event *>::iterator listItr;

            goals.clear();
            weight = 0.0;

            for (listItr = events.begin();
                listItr != events.end(); listItr++)
            {
                event = *listItr;
                delete event;
            }
            events.clear();
        }

        // Load.
        void load(FILE *fp)
        {
            int size;
            Event *event;

            clear();
            goals.load(fp);
            FREAD_DOUBLE(&weight, fp);

            events.clear();
            FREAD_INT(&size, fp);
            for (int i = 0; i < size; i++)
            {
                event = new Event();
                assert(event != NULL);
                event->load(fp);
                events.push_back(event);
            }
        }

        // Save.
        void save(FILE *fp)
        {
            int size;
            Event *event;
            list<Event *>::iterator listItr;

            goals.save(fp);
            FWRITE_DOUBLE(&weight, fp);

            size = events.size();
            FWRITE_INT(&size, fp);
            for (listItr = events.begin();
                listItr != events.end(); listItr++)
            {
                event = *listItr;
                event->save(fp);
            }
        }
};

// Motive accumulator.
// Accumulates need change caused by goals.
class MotiveAccum
{
    public:
        VALUE_SET base;
        VALUE_SET delta;
        WEIGHT weight;
        bool enabler;
        vector<int> tracker;
    #ifdef ACTIVITY_TRACKING
        vector<Neuron *> drivers;
    #endif

        // Constructors.
        MotiveAccum(int numNeeds)
        {
            base.alloc(numNeeds);
            delta.alloc(numNeeds);
            weight = 1.0;
            enabler = true;
        }
        MotiveAccum()
        {
            weight = 1.0;
            enabler = true;
        }

        // Destructor.
        ~MotiveAccum()
        {
            clear();
        }

        // Get value of accumulator.
        // This is the sum of the delta values
        // bounded by the base need values.
        MOTIVE getValue()
        {
            int i;
            double b,d,v;

            v = 0.0;
            for (i = 0; i < delta.size(); i++)
            {
                b = base.get(i);
                d = delta.get(i);
                if (d < 0.0)
                {
                    if ((b + d) < 0.0) v += b; else v -= d;
                }
                else
                {
                    if ((b + d) > MAX_NEED) v -= (MAX_NEED - b); else v -= d;
                }
            }
            return(v);
        }

        // Configure accumulator.
        void config(MotiveAccum &accum, WEIGHT weight)
        {
            int i,j;

            init(accum.base);
            loadNeeds(accum);
            scale(weight);
            this->weight *= weight;
            tracker.clear();
            for (i = 0, j = accum.tracker.size(); i < j; i++)
            {
                tracker.push_back(accum.tracker[i]);
            }
        }

        // Initialize accumulator.
        void init(VALUE_SET &needs)
        {
            base.load(needs);
            delta.alloc(base.size());
            weight = 1.0;
            enabler = true;
            tracker.clear();
        }

        // Load need delta values.
        void loadNeeds(MotiveAccum &motiveAccum)
        {
            delta.load(motiveAccum.delta);
        }

        // Accumulate need delta values.
        void accumNeeds(MotiveAccum &motiveAccum)
        {
            delta.add(motiveAccum.delta);
        }

        // Accumulate goal values.
        void accumGoals(GoalValue &goals)
        {
            VALUE_SET v;

            v.load(goals.goals);
            v.multiply(weight);
            delta.subtract(v);
        }

        // Add an id to the tracker.
        // Return false if duplicate.
        bool addTracker(int id)
        {
            int i,j;

            for (i = 0, j = tracker.size(); i < j; i++)
            {
                if (tracker[i] == id) return false;
            }
            tracker.push_back(id);
            return true;
        }

        // Scale accumulator.
        void scale(WEIGHT weight)
        {
            delta.multiply(weight);
        }

        // Reset accumulator.
        void reset()
        {
            delta.zero();
            enabler = true;
            tracker.clear();
        }

        // Clear accumulator.
        void clear()
        {
            base.clear();
            delta.clear();
            enabler = true;
            tracker.clear();
        }

        // Print
        void print()
        {
            int i,j;

            printf("base: ");
            for (i = 0, j = base.size(); i < j; i++)
            {
                printf("%f ",base.get(i));
            }
            printf("\ndelta: ");
            for (i = 0, j = delta.size(); i < j; i++)
            {
                printf("%f ",delta.get(i));
            }
            printf("weight=%f\n", weight);
            if (enabler) printf("enabler\n");
            else printf("disabler\n");
            printf("value=%f\n", getValue());
            printf("tracker ids: ");
            for (i = 0, j = tracker.size(); i < j; i++)
            {
                printf("%d ", tracker[i]);
            }
            printf("\n");
        }

        // Load.
        void load(FILE *fp)
        {
            int i,j;

            clear();
            base.load(fp);
            delta.load(fp);
            FREAD_DOUBLE(&weight, fp);
            FREAD_BOOL(&enabler, fp);
            tracker.clear();
            FREAD_INT(&i, fp);
            tracker.resize(i);
            for (i = 0, j = tracker.size(); i < j; i++)
            {
                FREAD_INT(&tracker[i], fp);
            }
        }

        // Save.
        void save(FILE *fp)
        {
            int i,j;

            base.save(fp);
            delta.save(fp);
            FWRITE_DOUBLE(&weight, fp);
            FWRITE_BOOL(&enabler, fp);
            i = tracker.size();
            FWRITE_INT(&i, fp);
            for (i = 0, j = tracker.size(); i < j; i++)
            {
                FWRITE_INT(&tracker[i], fp);
            }
        }
};

// Elementary event: non-mediator neuron id and timestamp.
struct ElemEvent
{
    int id;
    TIME timestamp;
};

// Event enabling.
class Enabling
{
    public:
        ENABLEMENT value;
        MOTIVE motive;
        VALUE_SET needSave;
        TIME age;
        int timerIndex;
        EnablingSet *set;
        bool newInSet;
        bool effectWager;
        bool parasite;
        TIME causeBegin;
        TIME effectBegin;

        // Elementary event stream.
        vector<struct ElemEvent> events;

        // Constructor.
        Enabling(ENABLEMENT value, MOTIVE motive,
            VALUE_SET &needs, TIME age, int timerIndex,
            TIME causeBegin, TIME effectBegin)
        {
            clear();
            this->value = value;
            this->motive = motive;
            needSave.load(needs);
            this->age = age;
            this->timerIndex = timerIndex;
            this->causeBegin = causeBegin;
            this->effectBegin = effectBegin;
        }
        Enabling()
        {
            clear();
        }

        // Destructor.
        ~Enabling()
        {
            clear();
        }

        // Clone.
        Enabling *clone()
        {
            int i,j;
            Enabling *enabling;
            enabling = new Enabling(value, motive, needSave,
                age, timerIndex, causeBegin, effectBegin);
            assert(enabling != NULL);
            enabling->newInSet = newInSet;
            enabling->effectWager = effectWager;
            enabling->parasite = parasite;
            enabling->events.resize(events.size());
            for (i = 0, j = events.size(); i < j; i++)
            {
                enabling->events[i] = events[i];
            }
            return enabling;
        }

        // Clear enabling.
        void clear()
        {
            value = 0.0;
            motive = 0.0;
            age = 0;
            timerIndex = -1;
            set = NULL;
            newInSet = effectWager = parasite = false;
            causeBegin = effectBegin = 0;
            needSave.clear();
            events.clear();
        }

        // Append event to stream.
        void appendEvent(struct ElemEvent event)
        {
            events.push_back(event);
        }
        void appendEvents(vector<struct ElemEvent> &events)
        {
            int i,j;
            for (i = 0, j = events.size(); i < j; i++)
            {
                this->events.push_back(events[i]);
            }
        }

        // Load.
        void load(FILE *fp)
        {
            int i,j;
            struct ElemEvent event;

            clear();
            FREAD_DOUBLE(&value, fp);
            FREAD_DOUBLE(&motive, fp);
            needSave.load(fp);
            FREAD_LONG(&age, fp);
            FREAD_INT(&timerIndex, fp);
            set = NULL;
            FREAD_BOOL(&newInSet, fp);
            FREAD_BOOL(&effectWager, fp);
            FREAD_BOOL(&parasite, fp);
            FREAD_LONG(&causeBegin, fp);
            FREAD_LONG(&effectBegin, fp);
            FREAD_INT(&j, fp);
            for (i = 0; i < j; i++)
            {
                FREAD_INT(&event.id, fp);
                FREAD_LONG(&event.timestamp, fp);
                events.push_back(event);
            }
        }

        // Save.
        void save(FILE *fp)
        {
            int i,j;
            ElemEvent event;

            FWRITE_DOUBLE(&value, fp);
            FWRITE_DOUBLE(&motive, fp);
            needSave.save(fp);
            FWRITE_LONG(&age, fp);
            FWRITE_INT(&timerIndex, fp);
            FWRITE_BOOL(&newInSet, fp);
            FWRITE_BOOL(&effectWager, fp);
            FWRITE_BOOL(&parasite, fp);
            FWRITE_LONG(&causeBegin, fp);
            FWRITE_LONG(&effectBegin, fp);
            j = events.size();
            FWRITE_INT(&j, fp);
            for (i = 0; i < j; i++)
            {
                event = events[i];
                FWRITE_INT(&event.id, fp);
                FWRITE_LONG(&event.timestamp, fp);
            }
        }
};

// Set of event enablings.
class EnablingSet
{
    public:
        list<Enabling *> enablings;

        // Constructor.
        EnablingSet()
        {
            clear();
        }

        // Destructor.
        ~EnablingSet()
        {
            clear();
        }

        // Insert an enabling.
        void insert(Enabling *enabling)
        {
            enabling->set = this;
            enabling->newInSet = true;
            enablings.push_back(enabling);
        }

        // Remove an enabling.
        void remove(Enabling *enabling)
        {
            enablings.remove(enabling);
        }

        // Get enabling value.
        ENABLEMENT getValue()
        {
            Enabling *enabling;
            ENABLEMENT e;
            list<Enabling *>::iterator listItr;

            e = 0.0;
            for (listItr = enablings.begin();
                listItr != enablings.end(); listItr++)
            {
                enabling = *listItr;
                e += enabling->value;
            }
            return(e);
        }

        // Clear special flags.
        void clearFlags()
        {
            Enabling *enabling;
            list<Enabling *>::iterator listItr;

            for (listItr = enablings.begin();
                listItr != enablings.end(); listItr++)
            {
                enabling = *listItr;
                enabling->newInSet = false;
                enabling->effectWager = false;
                enabling->parasite = false;
            }
        }

        // Clear.
        void clear()
        {
            Enabling *enabling;
            list<Enabling *>::iterator listItr;

            for (listItr = enablings.begin();
                listItr != enablings.end(); listItr++)
            {
                enabling = *listItr;
                delete enabling;
            }
            enablings.clear();
        }

        // Load.
        void load(FILE *fp)
        {
            int size;
            Enabling *enabling;

            clear();
            FREAD_INT(&size, fp);
            for (int i = 0; i < size; i++)
            {
                enabling = new Enabling();
                assert(enabling != NULL);
                enabling->load(fp);
                enabling->set = this;
                enablings.push_back(enabling);
            }
        }

        // Save.
        void save(FILE *fp)
        {
            int size;
            Enabling *enabling;
            list<Enabling *>::iterator listItr;

            size = enablings.size();
            FWRITE_INT(&size, fp);
            for (listItr = enablings.begin();
                listItr != enablings.end(); listItr++)
            {
                enabling = *listItr;
                enabling->save(fp);
            }
        }
};

// Wager history.
class WagerHistory
{
    public:

        // Get enablement history of wager outcomes.
        ENABLEMENT getEnablement(EVENT_OUTCOME type, ENABLEMENT baseEnablement,
            WEIGHT wagerWeight)
        {
            ENABLEMENT enablement;

            wagerWeight /= MAX_ENABLEMENT;
            if (type == FIRE)
            {
                enablement = baseEnablement +
                    ((MAX_ENABLEMENT - baseEnablement) *
                    wagerWeight * WAGER_HISTORY_UPDATE_VELOCITY);
            }
            else
            {
                enablement = baseEnablement +
                    ((MIN_ENABLEMENT - baseEnablement) *
                    wagerWeight * WAGER_HISTORY_UPDATE_VELOCITY);
            }
            return(enablement);
        }

        // Constructor.
        WagerHistory() {}

        // Set history.
        void setHistory(ENABLEMENT enablement)
        {
        }

        // Load.
        void load(FILE *fp)
        {
        }

        // Save.
        void save(FILE *fp)
        {
        }
};

// Mediator event notification.
struct Notify
{
    Mediator *mediator;
    EVENT_TYPE eventType;
    int eventIndex;
};

// Learning events.
class LearningEvent
{
    public:
        Neuron *neuron;
        WEIGHT strength;
        ENABLEMENT enablement;
        MOTIVE motive;
        TIME begin;
        TIME timestamp;

        LearningEvent(Neuron *neuron, TIME eventTimer)
        {
            this->neuron = neuron;
            strength = neuron->firingStrength;
            enablement = neuron->getMediatingWageredEnablement();
            motive = neuron->motive;
            if (neuron->type == MEDIATOR)
            {
                begin = ((Mediator *)neuron)->causeBegin;
            }
            else
            {
                begin = eventTimer;
            }
            timestamp = eventTimer;
        }

        LearningEvent()
        {
            neuron = NULL;
            strength = enablement = motive = 0.0;
            begin = timestamp = 0;
        }

        // Load.
        void load(FILE *fp)
        {
            int id;

            FREAD_INT(&id, fp);
            neuron = (Neuron *)id;
            FREAD_DOUBLE(&strength, fp);
            FREAD_DOUBLE(&enablement, fp);
            FREAD_DOUBLE(&motive, fp);
            FREAD_LONG(&begin, fp);
            FREAD_LONG(&timestamp, fp);
        }

        // Save.
        void save(FILE *fp)
        {
            FWRITE_INT(&neuron->id.value, fp);
            FWRITE_DOUBLE(&strength, fp);
            FWRITE_DOUBLE(&enablement, fp);
            FWRITE_DOUBLE(&motive, fp);
            FWRITE_LONG(&begin, fp);
            FWRITE_LONG(&timestamp, fp);
        }

        // Print.
        void print()
        {
            printf("Learning event:\n");
            if (neuron != NULL)
            {
                printf("neuron:\n");
                switch(neuron->type)
                {
                    case RECEPTOR:
                        ((Receptor *)neuron)->print(); printf("\n");
                        break;
                    case MOTOR:
                        ((Motor *)neuron)->print(); printf("\n");
                        break;
                    case MEDIATOR:
                        ((Mediator *)neuron)->print();
                        break;
                }
            }
            else
            {
                printf("neuron: NULL\n");
            }
            printf("strength=%f,", strength);
            printf("enablement=%f,", enablement);
            printf("motive=%f,", motive);
            printf("begin=%d,", begin);
            printf("timestamp=%d\n", timestamp);
        }
};
#endif
