/*
 * This software is provided under the terms of the GNU General
 * Public License as published by the Free Software Foundation.
 *
 * Copyright (c) 2003 Tom Portegys, All Rights Reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "../common/common.h"
#ifdef ACTIVITY_TRACKING
#include <vector>
#endif

#define MONA_VERSION "@(#) Mona version 2.2"

// Mona: sensory/response, neural network, and needs.
class Mona
{
public:

  // Version.
  static void version();

  // Construct/destruct.
  Mona(int numSensors, int maxResponse, int numNeeds);
  ~Mona();

  // Data types.
  typedef int SENSOR;
  typedef int RESPONSE;
  typedef double RESPONSE_POTENTIAL;
  typedef bool FIRING_STATE;
  typedef double NEED;
  typedef double GOAL;
  typedef double MOTIVE;
  typedef double ENABLEMENT;
  typedef int TIMER;
  typedef double WEIGHT;
  enum NEURON_TYPE { RECEPTOR, MOTOR, MEDIATOR };
  enum EVENT_TYPE { CAUSE, EFFECT };
  enum WAGER_OUTCOME { EXPIRE, FIRE };

  // Parameters.
  static const NEED MAX_NEED;
  static const ENABLEMENT MAX_ENABLEMENT;
  static const ENABLEMENT MIN_ENABLEMENT;
  static const WEIGHT DRIVE_ATTENUATION;
  static const WEIGHT MAX_FIRING;
  static const int MAX_WAGER_HISTORY_EVENTS;
  static const int NUM_WAGER_RATES;
  static const WEIGHT MIN_WAGER_RATE_WEIGHT;

  // Sensors.
  SENSOR *sensors;
  int numSensors;

  // Response.
  // Response 0 defined as "wait/noop".
  RESPONSE response;
  RESPONSE maxResponse;
  RESPONSE_POTENTIAL *responsePotentials;
  int *responseWork;

  // Needs.
  VALUE_SET needs;
  VALUE_SET oldNeeds;
  char **needDescriptions;
  int numNeeds;

  // Initialize and set needs.
  void initNeed(int index, NEED value, char *description);
  void setNeed(int index, NEED value);
  NEED getNeed(int index);

  // Wager rate timers.
  static int *wagerRateTimers;
  static int *initWagerRateTimers();

  // Sense/respond cycle.
  RESPONSE cycle(SENSOR *);
  void enable();
  void drive();
  void sense();
  void respond();

  // Response mode.
  enum RESPONSE_MODE { PROBABILISTIC, DETERMINISTIC } responseMode;

  // Sub-functions.
  void clearMotiveWork();
  void accumMotive();
  void dumpWagers();

  // Forward declarations
  class Neuron;
  class Receptor;
  class Motor;
  class Mediator;
  class Wager;
  class WagerSet;
  class WagerHistory;
  class GoalValue;
  class MotiveAccum;

  // Identification.
  class Identifier
  {
  public:
    int value;
    Neuron *neuron;
	char *description;

	// Constructor.
    Identifier()
      {
		value = dispenser;
		dispenser++;
		neuron = NULL;
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
		if (n == neuron && n->id.value == value)
		  return(true);
		else
		  return(false);
      }

	// Clear.
	void clear()
	{
		if (description != NULL) delete description;
		description = NULL;
		neuron = NULL;
	}

  private:
    static int dispenser;
  };

  // Goal values.
  class GoalValue
  {
  public:
    VALUE_SET goalSet;
    WEIGHT weight;

	// Constructors.
	GoalValue(int numGoals)
	{
		goalSet.alloc(numGoals);
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
		return(goalSet.sum());
      }

    // Get specific goal value.
    NEED getValue(int index)
      {
		return(goalSet.get(index));
      }

	// Set goals.
	void setGoals(VALUE_SET *goals, WEIGHT weight)
	{
		goalSet.load(goals);
		this->weight = weight;
	}

	// Get number of goals.
	int getNumGoals()
	{
		return(goalSet.size);
	}

	// Set number of goals.
	void setNumGoals(int numGoals)
	{
		goalSet.alloc(numGoals);
	}

	// Update goal value.
    void update(VALUE_SET *needBefore, VALUE_SET *needAfter, WEIGHT weightIn)
      {
      }

	// Clear.
	void clear()
	{
		goalSet.clear();
		weight = 0.0;
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
#ifdef ACTIVITY_TRACKING
	std::vector<Neuron *> drivers;
#endif

	// Constructors.
	MotiveAccum(int numNeeds)
	{
		base.alloc(numNeeds);
		delta.alloc(numNeeds);
		weight = 0.0;
		enabler = true;
	}
	MotiveAccum()
	{
		weight = 0.0;
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
		register int i;
		double b,d,v;

		v = 0.0;
		for (i = 0; i < delta.size; i++)
		{
			b = base.get(i);
			d = delta.get(i);
			if (d < 0.0)
			{
				if ((b + d) < 0.0) v += b; else v -= d;
			} else {
				if ((b + d) > MAX_NEED) v -= (MAX_NEED - b); else v -= d;
			}
		}
		return(v);
    }

	// Configure accumulator
	void config(VALUE_SET *needs)
	{
		base.load(needs);
		delta.alloc(base.size);
		weight = 1.0;
		enabler = true;
	}

	// Load need delta values.
    void loadNeeds(MotiveAccum *motiveAccum)
    {
		delta.load(&motiveAccum->delta);
		weight = motiveAccum->weight;
	}

	// Accumulate need delta values.
    void accumNeeds(MotiveAccum *motiveAccum)
    {
		delta.add(&motiveAccum->delta);
	}

	// Accumulate goal values.
    void accumGoals(GoalValue *goals)
    {
		VALUE_SET v;

		v.load(&goals->goalSet);
		v.scale(weight);
		delta.subtract(&v);
	}

	// Scale accumulator.
	void scale(WEIGHT weight)
	{
		delta.scale(weight);
	}

	// Reset accumulator.
	void reset()
	{
		delta.zero();
		enabler = true;
	}

	// Clear accumulator.
	void clear()
	{
		base.clear();
		delta.clear();
		enabler = true;
	}

	// Print
	void print()
	{
		register int i;

		printf("base: ");
		for (i = 0; i < base.size; i++)
		{
			printf("%f ",base.get(i));
		}
		printf("\ndelta: ");
		for (i = 0; i < delta.size; i++)
		{
			printf("%f ",delta.get(i));
		}
		printf("weight=%f\n", weight);
		if (enabler) printf("enabler\n");
		else printf("disabler\n");
		printf("value=%f\n", getValue());
	}
  };

  // Enablement wager.
  class Wager
  {
  public:
    ENABLEMENT value;
	ENABLEMENT valueOut;
    MOTIVE motive;
    MOTIVE accum[2],work[2];
    VALUE_SET needSave;
    TIMER timer;
    int rateIndex;
    Mediator *source;
    WagerSet *wagerSet;

	// Constructor.
    Wager(VALUE_SET *needs, ENABLEMENT value, TIMER duration,
			int rateIndex, Mediator *source)
	{
	  clear();
	  needSave.load(needs);
	  this->value = value;
	  this->timer = duration;
	  this->rateIndex = rateIndex;
	  this->source = source;
	}

	// Destructor.
    ~Wager()
	{
	  clear();
	}

	// Clone.
    Wager *clone()
	{
	  return(new Wager(&needSave, value, timer, rateIndex, source));
	}

	// Weight wager in context.
	ENABLEMENT weightByContext();

	// Clear wager.
	void clear()
	{
	  value = valueOut = 0.0;
	  motive = 0.0;
	  accum[0] = accum[1] = 0.0;
	  work[0] = work[1] = 0.0;
	  timer = 0;
	  rateIndex = -1;
	  source = NULL;
	  wagerSet = NULL;
	  needSave.clear();
	}
  };

  // Set of wagers.
  class WagerSet
  {
  public:
    POINTER_LIST wagers;
    Neuron *neuron;

	// Constructor.
    WagerSet()
      {
		clear();
      }

	// Destructor.
    ~WagerSet()
      {
		clear();
      }

	// Insert a wager.
    void insert(Wager *wager)
      {
		wager->wagerSet = this;
		wagers.insert((char *)wager);
      }

	// Remove a wager.
    void remove(Wager *wager)
      {
		wagers.remove((char *)wager);
      }

    // Get value.
    ENABLEMENT getValue()
      {
		Wager *wager;
		int i,j;
		ENABLEMENT e;

		for (i = 0, j = wagers.getsize(), e = 0.0; i < j; i++)
		  {
		    wager = (Wager *)wagers.index(i);
		    e += wager->value;
          }
        return(e);
      }

    // Get value wagered out.
    ENABLEMENT getValueOut()
      {
		Wager *wager;
		int i,j;
		ENABLEMENT e;

		for (i = 0, j = wagers.getsize(), e = 0.0; i < j; i++)
		  {
		    wager = (Wager *)wagers.index(i);
		    e += wager->valueOut;
          }
        return(e);
      }

    // Get enabling value.
    ENABLEMENT getEnablingValue()
      {
		Wager *wager;
		int i,j;
		ENABLEMENT e;

		for (i = 0, j = wagers.getsize(), e = 0.0; i < j; i++)
		  {
		    wager = (Wager *)wagers.index(i);
		    if (wager->value > 0.0) e += wager->value;
          }
        return(e);
      }

    // Get enabling value wagered out.
    ENABLEMENT getEnablingValueOut()
      {
		Wager *wager;
		int i,j;
		ENABLEMENT e;

		for (i = 0, j = wagers.getsize(), e = 0.0; i < j; i++)
		  {
		    wager = (Wager *)wagers.index(i);
		    if (wager->valueOut > 0.0) e += wager->valueOut;
          }
        return(e);
      }

    // Get disabling value.
    ENABLEMENT getDisablingValue()
      {
		Wager *wager;
		int i,j;
		ENABLEMENT e;

		for (i = 0, j = wagers.getsize(), e = 0.0; i < j; i++)
		  {
		    wager = (Wager *)wagers.index(i);
		    if (wager->value < 0.0) e += wager->value;
          }
        return(e);
      }

    // Get disabling value wagered out.
    ENABLEMENT getDisablingValueOut()
      {
		Wager *wager;
		int i,j;
		ENABLEMENT e;

		for (i = 0, j = wagers.getsize(), e = 0.0; i < j; i++)
		  {
		    wager = (Wager *)wagers.index(i);
		    if (wager->valueOut < 0.0) e += wager->valueOut;
          }
        return(e);
      }

	// Clear.
	void clear()
	{
		int i,j;

	    neuron = NULL;
		for (i = 0, j = wagers.getsize(); i < j; i++)
		{
			wagers.remove(0);
		}
		wagers.clear();
	}
  };

  // Wager history.
  class WagerHistory
  {
  public:

	// Wager outcome event.
    class Event
    {
	public:
      WEIGHT weight;
      WAGER_OUTCOME type;
      Event *next;

	  // Constructor.
	  Event(WEIGHT weight, WAGER_OUTCOME type)
	  {
		  this->weight = weight;
		  this->type = type;
		  next = NULL;
	  }
    } *newest, *oldest;  // oldest is head, newest is tail.
    int eventCount;

	// Constructor.
    WagerHistory()
	{
	  newest = oldest = NULL;
	  eventCount = 0;
	}

	// Destructor.
    ~WagerHistory()
	{
	  clear();
	}

	// Get enablement by wager history.
	ENABLEMENT getEnablement(WEIGHT, WAGER_OUTCOME, bool enabler);

	// Set expected history.
	void setHistory(double rate);

	// Clear.
	void clear()
	{
		Event *e,*e2;

		for (e = oldest; e != NULL; e = e2)
		{
			e2 = e->next;
			delete e;
		}
		newest = oldest = NULL;
		eventCount = 0;
	}
  };

  // Neuron.
  class Neuron
  {
  public:

    // Initialize/clear.
	void init(int numNeeds, char *description);
	void clear();

    // Identifier.
    Identifier id;

    // Neuron type.
    NEURON_TYPE type;

    // Firing state.
    FIRING_STATE firing;

    // Goal value.
    GoalValue goals;

    // Drive.
    void drive(MotiveAccum *);
    MotiveAccum work[2];

    // Enablement wagers.
    WagerSet wagerSet;

	// Pay wagers.
	void payWagers(VALUE_SET *needs);

    // Event notification.
    POINTER_LIST notifyList;

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
    Receptor(SENSOR *sensorMask, int numSensors, int numNeeds, char *description);
    ~Receptor();

    // Sense.
    SENSOR *sensorMask;
    void sense(SENSOR *, int numSensors);

    // Motive for response selection.
    MOTIVE motive;
  };

  // Motor neuron.
  class Motor : public Neuron
  {
  public:

    // Construct/destruct.
    Motor(RESPONSE, int numNeeds, char *description);
    ~Motor();

    // Response.
    RESPONSE response;

    // Motive for response selection.
    MOTIVE motive;
    MotiveAccum accum[2];

#ifdef ACTIVITY_TRACKING
	std::vector<Neuron *> driverWork;
	std::vector<Neuron *> drivers;
#endif
  };

  // Mediator neuron.
  class Mediator : public Neuron
  {
  public:

    // Construct/destruct.
	Mediator(ENABLEMENT, bool enabler, int numNeeds, char *description);
    ~Mediator();

    // Events.
    POINTER_LIST causes;	// Cause conjunction.
    Neuron *effect;
    void addEvent(EVENT_TYPE, Neuron *);
    void eventFiring(EVENT_TYPE, int causeIndex, WEIGHT strength,
		WagerSet *, VALUE_SET *needs);

    // Base enablement.
    ENABLEMENT baseEnablement;

	// Get base enablement including portion wagered out.
    ENABLEMENT getBaseEnablement()
      {
		return(baseEnablement + enablementOut);
      }

	// How enabled is this mediator.
    ENABLEMENT getEnablement()
      {
		ENABLEMENT e;

		if ((e = baseEnablement + wagerSet.getValue()) > MAX_ENABLEMENT)
			return(MAX_ENABLEMENT);
		else if (e < -MAX_ENABLEMENT)
			return(-MAX_ENABLEMENT);
		else
			return(e);
      }

	// Get enablement wagered out.
	ENABLEMENT getEnablementOut()
      {
		ENABLEMENT e;

		e = enablementOut + wagerSet.getValueOut();
		if (e > MAX_ENABLEMENT)
			return(MAX_ENABLEMENT);
		else if (e < -MAX_ENABLEMENT)
			return(-MAX_ENABLEMENT);
		else
			return(e);
      }

	// Get total enablement.
	ENABLEMENT getTotalEnablement()
      {
		ENABLEMENT e;

		e = getEnablement() + getEnablementOut();
		if (e > MAX_ENABLEMENT)
			return(MAX_ENABLEMENT);
		else if (e < -MAX_ENABLEMENT)
			return(-MAX_ENABLEMENT);
		else
			return(e);
      }

	// Wager enablement.
	void wagerEnablement(int causeIndex, WEIGHT strength, VALUE_SET *needs);
	void wagerEnablementSub(int causeIndex, WEIGHT strength, TIMER duration, VALUE_SET *needs);
	void wagerEnablement(WEIGHT strength, TIMER duration, VALUE_SET *needs);

	// Clean migrated wagers.
	void cleanMigratedWagers();

	// Expire wagers.
	void updateExpiredWagers();
	void deleteExpiredWagers();

	// Age pending wagers.
	void agePendingWagers();

    // Enabler?
    bool enabler;

	// Repeater?
	// Effect by repetitive cause.
	bool repeater;
	void setRepeater(bool r) { repeater = r; }

    // Enablement wager weights of varying durations.
    WEIGHT *timedWagerWeights;
	void createTimedWagerWeights();

	// Pending wagers.
	struct PendingWager
	{
		int causeIndex;
		WEIGHT strength;
		TIMER age;
		struct PendingWager *next;
	};
	POINTER_LIST pendingWagers;
	WEIGHT causeFiringStrength;

    // Outstanding wagers.
    POINTER_LIST wagersOut;
	ENABLEMENT enablementOut;

    // Wager history.
    WagerHistory wagerHistory;

    // Drive cause.
    void driveCause(MotiveAccum *);
  };

  // Mediator event notification.
  struct Notify
  {
    Mediator *mediator;
    EVENT_TYPE eventType;
	int causeIndex;
  };

  // Network.
  POINTER_LIST receptors;
  POINTER_LIST motors;
  POINTER_LIST mediators;

  // Add/delete neurons to/from network.
  Receptor *newReceptor(SENSOR *sensorMask, char *description);
  Motor *newMotor(RESPONSE response, char *description);
  Mediator *newMediator(ENABLEMENT enablement, bool enabler, char *description);
  void deleteNeuron(Neuron *);

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
};
#endif

