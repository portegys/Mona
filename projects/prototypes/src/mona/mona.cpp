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

// Mona implementation.
#include "mona.hpp"

// Version.
void
Mona::version()
{
  const char *monaVersion = MONA_VERSION;
  printf("%s\n", &monaVersion[5]);
}

// Parameters.
#if ANT_ENV
const Mona::NEED Mona::MAX_NEED = 10.0;
#endif
#if NEST_ENV
const Mona::NEED Mona::MAX_NEED = 100.0;
#endif
const Mona::ENABLEMENT Mona::MAX_ENABLEMENT = 1.0;
#if ANT_ENV
const Mona::ENABLEMENT Mona::MIN_ENABLEMENT = 0.0001;
#endif
#if NEST_ENV
const Mona::ENABLEMENT Mona::MIN_ENABLEMENT = 0.0;
#endif
const Mona::WEIGHT Mona::DRIVE_ATTENUATION = 0.95;
const Mona::WEIGHT Mona::MAX_FIRING = MAX_ENABLEMENT;
const int Mona::MAX_WAGER_HISTORY_EVENTS = 50;
#ifdef ANT_ENV
const int Mona::NUM_WAGER_RATES = 3;
#endif
#ifdef NEST_ENV
const int Mona::NUM_WAGER_RATES = 2;
#endif
const Mona::WEIGHT Mona::MIN_WAGER_RATE_WEIGHT = 0.01;

// Wager rate timers.
int *Mona::wagerRateTimers = initWagerRateTimers();
int *Mona::initWagerRateTimers()
{
	int *timers;

	timers = new int[NUM_WAGER_RATES];
#ifdef ANT_ENV
	timers[0] = 1;
	timers[1] = 3;
	timers[2] = 30;
#endif
#ifdef NEST_ENV
	timers[0] = 5;
	timers[1] = 300;
#endif
	return(timers);
}

// Unique identifier dispenser.
int Mona::Identifier::dispenser = 0;

// Sense/respond cycle.
Mona::RESPONSE
Mona::cycle(Mona::SENSOR *sensors)
{
#ifdef MONA_TRACE
  printf("***Mona sense/respond cycle***\n");
#endif

  memcpy(this->sensors, sensors, sizeof(SENSOR) * numSensors);

#ifdef ACTIVITY_TRACKING
  // Reset tracking.
  Receptor *receptor;
  int i,j;
  for (i = 0, j = receptors.getsize(); i < j; i++)
    {
      receptor = (Receptor *)receptors.index(i);
	  receptor->tracker.fire = false;
	  receptor->tracker.enable = false;
	  receptor->tracker.drive = false;
    }
  Motor *motor;
  for (i = 0, j = motors.getsize(); i < j; i++)
    {
      motor = (Motor *)motors.index(i);
	  motor->tracker.fire = false;
	  motor->tracker.enable = false;
	  motor->tracker.drive = false;
    }
  Mediator *mediator;
  for (i = 0, j = mediators.getsize(); i < j; i++)
    {
      mediator = (Mediator *)mediators.index(i);
	  mediator->tracker.fire = false;
	  mediator->tracker.enable = false;
	  mediator->tracker.drive = false;
	}
#endif

  sense();
  enable();
  drive();
  respond();

  // To detect need changes.
  oldNeeds.load(&needs);

  return(response);
}

// Mona constructor.
Mona::Mona(int numSensors, int maxResponse, int numNeeds)
{
  // Set I/O capabilities
  this->numSensors = numSensors;
  sensors = new SENSOR[numSensors];
  assert(sensors != NULL);
  this->maxResponse = maxResponse;
  responsePotentials = new RESPONSE_POTENTIAL[maxResponse+1];
  assert(responsePotentials != NULL);
  responseWork = new int[maxResponse+1];
  assert(responseWork != NULL);
#if ANT_ENV || NEST_ENV
  responseMode = DETERMINISTIC;
#else
  responseMode = PROBABILISTIC;
#endif

  // Create needs.
  this->numNeeds = numNeeds;
  needs.alloc(numNeeds);
  oldNeeds.alloc(numNeeds);
  needDescriptions = new char *[numNeeds];
  assert(needDescriptions != NULL);
  memset(needDescriptions, 0, numNeeds * sizeof(char *));
}

// Mona destructor.
Mona::~Mona()
{
  int i,j;
  Receptor *receptor;
  Motor *motor;
  Mediator *mediator;

  delete sensors;
  sensors = NULL;
  delete responsePotentials;
  responsePotentials = NULL;
  delete responseWork;
  responseWork = NULL;
  for (i = 0, j = receptors.getsize(); i < j; i++)
    {
      receptor = (Receptor *)receptors.index(0);
	  delete receptor;
	  receptors.remove(0);
    }
  receptors.clear();
  for (i = 0, j = motors.getsize(); i < j; i++)
    {
      motor = (Motor *)motors.index(0);
	  delete motor;
	  motors.remove(0);
    }
  motors.clear();
  for (i = 0, j = mediators.getsize(); i < j; i++)
    {
      mediator = (Mediator *)mediators.index(0);
	  delete mediator;
	  mediators.remove(0);
    }
  mediators.clear();
  needs.clear();
  oldNeeds.clear();
  for (i = 0; i < numNeeds; i++)
  {
	  if (needDescriptions[i] != NULL)
	  {
		 delete needDescriptions[i];
		 needDescriptions[i] = NULL;
	  }
  }
  delete needDescriptions;
}

// Initialize neuron.
void
Mona::Neuron::init(int numNeeds, char *description)
{
  id.neuron = this;
  if (description == NULL) description = (char *)"<no_description>";
  id.description = new char[strlen(description)+1];
  assert(id.description != NULL);
  strcpy(id.description, description);
  goals.setNumGoals(numNeeds);
  wagerSet.neuron = this;
  firing = false;
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
  id.clear();
  goals.clear();
  work[0].clear();
  work[1].clear();
  notifyList.clear();
}

// Create receptor and add to network.
Mona::Receptor *
Mona::newReceptor(SENSOR *sensorMask, char *description)
{
  Receptor *r;

  r = new Receptor(sensorMask, numSensors, numNeeds, description);
  assert(r != NULL);
  receptors.insert((char *)r);
  return(r);
}

// Receptor constructor.
Mona::Receptor::Receptor(SENSOR *sensorMask, int numSensors, int numNeeds, char *description)
{
  init(numNeeds, description);
  type = RECEPTOR;
  this->sensorMask = new SENSOR[numSensors];
  assert(this->sensorMask != NULL);
  memcpy(this->sensorMask, sensorMask, numSensors*sizeof(SENSOR));
  motive = 0.0;
}

// Receptor destructor.
Mona::Receptor::~Receptor()
{
  clear();
  delete sensorMask;
  sensorMask = NULL;
}

// Create motor and add to network.
Mona::Motor *
Mona::newMotor(RESPONSE response, char *description)
{
  Motor *m;

  m = new Motor(response, numNeeds, description);
  assert(m != NULL);
  motors.insert((char *)m);
  return(m);
}

// Motor constructor.
Mona::Motor::Motor(RESPONSE response, int numNeeds, char *description)
{
  init(numNeeds, description);
  type = MOTOR;
  this->response = response;
  motive = 0.0;
}

// Motor destructor.
Mona::Motor::~Motor()
{
  clear();
  accum[0].clear();
  accum[1].clear();
}

// Create mediator and add to network.
Mona::Mediator *
Mona::newMediator(ENABLEMENT baseEnablement, bool enabler, char *description)
{
  Mediator *m;

  m = new Mediator(baseEnablement, enabler, numNeeds, description);
  assert(m != NULL);
  mediators.insert((char *)m);
  return(m);
}

// Mediator constructor.
Mona::Mediator::Mediator(ENABLEMENT baseEnablement, bool enabler, int numNeeds, char *description)
{
  init(numNeeds, description);
  type = MEDIATOR;
  this->baseEnablement = baseEnablement;
  enablementOut = 0.0;
  this->enabler = enabler;
  repeater = false;
  createTimedWagerWeights();
  causeFiringStrength = 0.0;
}

// Mediator destructor.
Mona::Mediator::~Mediator()
{
  int i,j;
  struct PendingWager *pwager,*pwager2;
  Wager *wager;

  clear();
  delete timedWagerWeights;
  causes.zero();
  causes.clear();
  for (i = 0, j = pendingWagers.getsize(); i < j; i++)
    {
      pwager = (struct PendingWager *)pendingWagers.index(i);
	  while (pwager != NULL)
	  {
		  pwager2 = pwager;
		  pwager = pwager->next;
		  delete pwager2;
	  }
	  pendingWagers.insert(i, NULL);
    }
  pendingWagers.clear();
  for (i = 0, j = wagersOut.getsize(); i < j; i++)
    {
      wager = (Wager *)wagersOut.index(0);
	  delete wager;
	  wagersOut.remove(0);
    }
  wagersOut.clear();
  wagerHistory.clear();
}

// Create timed wager weights array.
void
Mona::Mediator::createTimedWagerWeights()
{
  timedWagerWeights = new WEIGHT[NUM_WAGER_RATES];
  assert(timedWagerWeights != NULL);
#ifdef ANT_ENV
  timedWagerWeights[0] = 1.0;
  timedWagerWeights[1] = 0.0;
  timedWagerWeights[2] = 0.0;
#endif
#ifdef NEST_ENV
  timedWagerWeights[0] = 1.0;
  timedWagerWeights[1] = 0.0;
#endif
}

// Add mediator event.
void
Mona::Mediator::addEvent(EVENT_TYPE type, Neuron *neuron)
{
	struct Notify *notify;

	if (type == CAUSE)
	{
		causes.insert((char *)neuron);
	} else {
		effect = neuron;
	}
	notify = new struct Notify;
	notify->mediator = this;
	notify->eventType = type;
	if (type == CAUSE)
	{
		notify->causeIndex = causes.getsize()-1;
	} else {
		notify->causeIndex = -1;
	}
	neuron->notifyList.insert((char *)notify);
}

// Remove neuron from network.
void
Mona::deleteNeuron(Neuron *neuron)
{
  switch(neuron->type)
    {
    case RECEPTOR:
      delete neuron;
      receptors.remove((char *)neuron);
      break;
    case MOTOR:
      delete neuron;
      motors.remove((char *)neuron);
      break;
    case MEDIATOR:
	  delete neuron;
      mediators.remove((char *)neuron);
      break;
    }
}

// Initialize need.
void
Mona::initNeed(int index, NEED value, char *description)
{
  setNeed(index, value);
  if (description == NULL) description = (char *)"<no_description>";
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
  oldNeeds.set(index, value);
}

// Get need.
Mona::NEED
Mona::getNeed(int index)
{
  assert(index >= 0 && index < numNeeds);
  return(needs.get(index));
}

