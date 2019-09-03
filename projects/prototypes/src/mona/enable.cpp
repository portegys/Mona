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
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 */

// Mona implementation.
#include "mona.hpp"
#include <math.h>

// Enablement processing.
void
Mona::enable()
{
  Receptor *receptor;
  Motor *motor;
  Mediator *mediator;
  struct Notify *notify;
  int i,j,p,q;

#ifdef MONA_TRACE
  printf("***Enable phase***\n");
#endif

  // Distribute and pay wagers on firing motor events.
  for (i = 0, j = motors.getsize(); i < j; i++)
    {
      motor = (Motor *)motors.index(i);
      if (motor->firing)
		{
		  for (p = 0, q = motor->notifyList.getsize(); p < q; p++)
			{
			  notify = (struct Notify *)motor->notifyList.index(p);
			  (notify->mediator)->eventFiring(notify->eventType, notify->causeIndex,
				  MAX_FIRING, &(motor->wagerSet), &needs);
			}
		  motor->goals.update(&oldNeeds, &needs, MAX_FIRING);
		  motor->payWagers(&needs);
		}
    }

  // Distribute and pay wagers on firing receptor events.
  for (i = 0, j = receptors.getsize(); i < j; i++)
    {
      receptor = (Receptor *)receptors.index(i);
      if (receptor->firing)
		{
		  for (p = 0, q = receptor->notifyList.getsize(); p < q; p++)
			{
			  notify = (struct Notify *)receptor->notifyList.index(p);
			  (notify->mediator)->eventFiring(notify->eventType, notify->causeIndex,
				  MAX_FIRING, &(receptor->wagerSet), &needs);
			}
		  receptor->goals.update(&oldNeeds, &needs, MAX_FIRING);
		  receptor->payWagers(&needs);
		}
    }

  // Clean-up migrated wagers for firing mediator events.
  for (i = 0, j = mediators.getsize(); i < j; i++)
    {
      mediator = (Mediator *)mediators.index(i);
      if (mediator->firing)
		{
		  mediator->cleanMigratedWagers();
		}
    }

  // Age pending wagers.
  for (i = 0, j = mediators.getsize(); i < j; i++)
    {
      mediator = (Mediator *)mediators.index(i);
      mediator->agePendingWagers();
    }

  // Process expired enablement wagers.
  for (i = 0, j = mediators.getsize(); i < j; i++)
    {
      mediator = (Mediator *)mediators.index(i);
      mediator->updateExpiredWagers();
    }
  for (i = 0, j = mediators.getsize(); i < j; i++)
    {
      mediator = (Mediator *)mediators.index(i);
      mediator->deleteExpiredWagers();
    }

  // Dump wagers.
#ifdef MONA_TRACE
  dumpWagers();
#endif
}

// Dump mediators and wagers.
void
Mona::dumpWagers()
{
	int i,j,p,q;
	Mediator *mediator;
	Wager *wager;

	printf("Wager dump:\n");
	for (i = 0, j = mediators.getsize(); i < j; i++)
    {
		mediator = (Mediator *)mediators.index(i);
		printf("Mediator %d (%s), base=%f, enablement=%f\n", mediator->id.value,
			mediator->id.description, mediator->baseEnablement, mediator->getEnablement());
		for (p = 0, q = mediator->wagersOut.getsize(); p < q; p++)
		{
			wager = (Wager *)mediator->wagersOut.index(p);
			printf("wager value=%f,timer=%d,effect=%d\n", wager->value,
				wager->timer, wager->wagerSet->neuron->id.value);
		}
	}
}

// Detect firing of mediator event.
void
Mona::Mediator::eventFiring(EVENT_TYPE eventType, int causeIndex,
	WEIGHT strength, WagerSet *wagerSet, VALUE_SET *needs)
{
  struct Notify *notify;
  int i,j;
  WEIGHT w;
  ENABLEMENT e;

  // Cause firing?
  if (eventType == CAUSE)
    {
      // Wager enablement on effect.
      wagerEnablement(causeIndex, strength, needs);
      return;
    }

  // Firing strength is the input strength times enablement wagered out.
  e = enablementOut + ((Neuron *)this)->wagerSet.getValueOut();
  if ((w = e * strength) > MAX_FIRING)
  {
	  w = MAX_FIRING;
  }
  if (w <= 0.0) return;

  // Effect firing, so fire mediator.
#ifdef MONA_TRACE
  printf("Mediator firing: %s\n", id.description);
#endif
  firing = true;

#ifdef ACTIVITY_TRACKING
  tracker.fire = true;
#endif

#ifdef LEARN
  // Learn from event.
  learn(this, w, wagerSet, needs);
#endif

  // Notify super-mediators of firing.
  for (i = 0, j = notifyList.getsize(); i < j; i++)
    {
      notify = (struct Notify *)notifyList.index(i);
      (notify->mediator)->eventFiring(notify->eventType, notify->causeIndex,
		  w, wagerSet, needs);
    }
}

// Pay enablement wagers.
void
Mona::Neuron::payWagers(VALUE_SET * needs)
{
  Wager *wager;
  Mediator *mediator;
  int i,j;
#if !ANT_ENV && !NEST_ENV
  int k;
  WEIGHT w;
#endif

  for (i = 0, j = wagerSet.wagers.getsize(); i < j; i++)
    {
      wager = (Wager *)wagerSet.wagers.index(i);
	  mediator = wager->source;

      // Restore base enablement.
	  if (mediator->enabler)
	  {
		  mediator->baseEnablement += wager->value;
		  mediator->enablementOut -= wager->value;
	  } else {
		  mediator->baseEnablement -= wager->value;
		  mediator->enablementOut += wager->value;
	  }

#if !ANT_ENV && !NEST_ENV
      // Update and normalize timed wager rates.
      k = wager->rateIndex;
      mediator->timedWagerWeights[k] += wager->value;
      if (mediator->timedWagerWeights[k] < MIN_WAGER_RATE_WEIGHT)
		{
		  mediator->timedWagerWeights[k] = MIN_WAGER_RATE_WEIGHT;
		}
      for (k = 0, w = 0.0; k < NUM_WAGER_RATES; k++)
		{
		  w += mediator->timedWagerWeights[k];
		}
      for (k = 0; k < NUM_WAGER_RATES; k++)
		{
		  mediator->timedWagerWeights[k] /= w;
		}

      // Update base enablement using weighted average.
	  w = wager->weightByContext();
	  mediator->baseEnablement =
		  mediator->wagerHistory.getEnablement(w, FIRE, mediator->enabler) -
		  mediator->enablementOut;

	  // Update goal value.
	  if (mediator->enabler)
	  {
		mediator->goals.update(&(wager->needSave), needs, w);
	  }
#endif
    }

  // Delete wagers.
  while (wagerSet.wagers.getsize() > 0)
    {
      wager = (Wager *)wagerSet.wagers.index(0);
	  mediator = wager->source;
      wagerSet.remove(wager);
	  mediator->wagersOut.remove((char *)wager);
      delete wager;
    }
}

// Wager enablement.
void
Mona::Mediator::wagerEnablement(int causeIndex,
	WEIGHT strength, VALUE_SET *needs)
{
	int i,j;
	struct PendingWager *pwager,*pwager2;

	// Add pending wager.
	pwager = new struct PendingWager;
	assert(pwager != NULL);
	pwager->causeIndex = causeIndex;
	pwager->strength = strength;
	pwager->age = 0;
	pwager->next = (struct PendingWager *)pendingWagers.index(causeIndex);
	pendingWagers.insert(causeIndex, (char *)pwager);

	// All causes have fired?
	for (i = 0, j = causes.getsize(); i < j; i++)
	{
		if ((struct PendingWager *)pendingWagers.index(i) == NULL) return;
	}

	// Create wagers from combinations of cause events.
	pwager = (struct PendingWager *)pendingWagers.index(0);
	while (pwager != NULL)
	{
		wagerEnablementSub(1, pwager->strength, pwager->age, needs);
		pwager2 = pwager->next;
		delete pwager;
		pwager = pwager2;
	}
	pendingWagers.insert(0, NULL);
}

// Wager enablement subroutine.
void
Mona::Mediator::wagerEnablementSub(int causeIndex, WEIGHT strength,
	TIMER age, VALUE_SET *needs)
{
	struct PendingWager *pwager,*pwager2;

	// Create wager?
	if (causeIndex == causes.getsize())
	{
		wagerEnablement(strength, age, needs);
		causeFiringStrength += strength;
		if (causeFiringStrength > MAX_FIRING)
		{
			causeFiringStrength = MAX_FIRING;
		}
		return;
	}

	// Continue building combinations.
	pwager = (struct PendingWager *)pendingWagers.index(causeIndex);
	while (pwager != NULL)
	{
		strength *= pwager->strength;
		if (age < pwager->age) age = pwager->age;
		wagerEnablementSub(causeIndex+1, strength, age, needs);
		pwager2 = pwager->next;
		delete pwager;
		pwager = pwager2;
	}
	pendingWagers.insert(causeIndex, NULL);
}

// Wager enablement given a cause age.
void
Mona::Mediator::wagerEnablement(WEIGHT strength, TIMER age, VALUE_SET *needs)
{
	int i,j,t;
	Wager *wager;
	WEIGHT w;
	ENABLEMENT d,e;

	// Determine fraction of base enablement to wager.
	if ((w = strength / MAX_FIRING) == 0.0) return;
	d = getBaseEnablement() * w;
	if (d > baseEnablement) d = baseEnablement;
	baseEnablement -= d;
	enablementOut += d;
	if (!enabler) d = -d;
#ifdef ACTIVITY_TRACKING
	tracker.enable = true;
#endif

	// Distribute enablement to effect neuron.
	for (i = 0; i < NUM_WAGER_RATES && d != 0.0; i++)
	{
		t = Mona::wagerRateTimers[i] - age;
		if (t < 0) continue;
		e = d * timedWagerWeights[i];
		if (e == 0.0) continue;
		wager = new Wager(needs, e, t, i, this);
		assert(wager != NULL);
		wagersOut.insert((char *)wager);
		effect->wagerSet.insert(wager);
#ifdef ACTIVITY_TRACKING
		effect->tracker.enable = true;
#endif
	}

	// Split and migrate external wagers to effect neuron.
	for (i = 0, j = wagerSet.wagers.getsize(); i < j; i++)
	{
		wager = (Wager *)wagerSet.wagers.index(i);
		if (wager->value == 0.0) continue;
		d = (wager->value + wager->valueOut) * w;
		if (fabs(d) > fabs(wager->value)) d = wager->value;
		wager->value -= d;
		wager->valueOut += d;
		wager = wager->clone();
		assert(wager != NULL);
		wager->value = d;
		wager->valueOut = 0.0;
		wager->source->wagersOut.insert((char *)wager);
		effect->wagerSet.insert(wager);
#ifdef ACTIVITY_TRACKING
		wager->source->tracker.enable = true;
		effect->tracker.enable = true;
#endif
	}
}

// Update expired enablement wagers.
void
Mona::Mediator::updateExpiredWagers()
{
  Wager *wager;
  int i,j;
#if !ANT_ENV && !NEST_ENV
  int k;
  WEIGHT w;
#endif

  for (i = 0, j = wagersOut.getsize(); i < j; i++)
    {
      wager = (Wager *)wagersOut.index(i);
      wager->timer--;
      if (wager->timer >= 0) continue;

      // Restore base enablement.
	  if (enabler)
	  {
		  baseEnablement += wager->value;
		  enablementOut -= wager->value;
	  } else {
		  baseEnablement -= wager->value;
		  enablementOut += wager->value;
	  }

#if !ANT_ENV && !NEST_ENV
      // Update and normalize timed wager rates.
      k = wager->rateIndex;
      timedWagerWeights[k] -= wager->value;
      if (timedWagerWeights[k] < MIN_WAGER_RATE_WEIGHT)
		{
		  timedWagerWeights[k] = MIN_WAGER_RATE_WEIGHT;
		}
      for (k = 0, w = 0.0; k < NUM_WAGER_RATES; k++)
		{
		  w += timedWagerWeights[k];
		}
      for (k = 0; k < NUM_WAGER_RATES; k++)
		{
		  timedWagerWeights[k] /= w;
		}

      // Update base enablement using weighted average.
	  w = wager->weightByContext();
	  baseEnablement =
		  wagerHistory.getEnablement(w, EXPIRE, enabler) - enablementOut;
#endif

#ifdef LEARN
      // Learn from absence of event.
      learn(this, wager);
#endif
    }
}

// Delete expired enablement wagers.
void
Mona::Mediator::deleteExpiredWagers()
{
  Wager *wager;
  int i,j;

  for (i = 0, j = wagersOut.getsize(); i < j;)
    {
      wager = (Wager *)wagersOut.index(i);
      if (wager->timer >= 0) { i++; continue; }
      wager->wagerSet->remove(wager);
	  wagersOut.remove(i);
	  j--;
      delete wager;
    }
}

// Clean migrated wagers.
void
Mona::Mediator::cleanMigratedWagers()
{
  Wager *wager;
  int i,j;

  for (i = 0, j = wagersOut.getsize(); i < j;)
    {
      wager = (Wager *)wagersOut.index(i);
      if (wager->value != 0.0) { i++; continue; }
      wager->wagerSet->remove(wager);
	  wagersOut.remove(i);
	  j--;
      delete wager;
    }
}

// Age pending enablement wagers.
void
Mona::Mediator::agePendingWagers()
{
  struct PendingWager *pwager,*pwager2,*pwager3;
  int i,j;

  for (i = 0, j = pendingWagers.getsize(); i < j; i++)
  {
      pwager = (struct PendingWager *)pendingWagers.index(i);
	  pwager2 = NULL;
	  while (pwager != NULL)
	  {
		  pwager->age++;

		  // Delete pending wager if age exceeds maximum possible cause duration.
		  if ((Mona::wagerRateTimers[NUM_WAGER_RATES-1] - pwager->age) < 0)
		  {
			  pwager3 = pwager->next;
			  if (pwager2 == NULL)
			  {
				  pendingWagers.insert(i, (char *)pwager3);
			  } else {
				  pwager2->next = pwager3;
			  }
			  delete pwager;
			  pwager = pwager3;
		  } else {
			  pwager2 = pwager;
			  pwager = pwager->next;
		  }
	  }
  }
}

/*
 * Weight wager in context with other wagers.
 * Factors are:
 * (1) The wager value.
 * (2) Wager value relative to the total enablement of
 *     its wager set and other wagers of like sign. If the
 *     wager represents a high proportion of the total
 *     enablement, then the weight is higher.
 * (3) Wager motive is also a factor in the weight. The more
 *     the wager is needed, the higher the weight.
 */
Mona::ENABLEMENT
Mona::Wager::weightByContext()
{
  ENABLEMENT b,v;

  // If wager un-motivated, no weight.
  if (motive <= 0.0) return(0.0);

  // Get fraction of base enablement.
  b = source->getBaseEnablement();
  if (b == 0.0) return(0.0);
  b = value / b;
  if (b < 0.0) b = -b;

  // Get fraction of wager set.
  v = 0.0;
  if (value > 0.0)
    {
      v = (value / wagerSet->getEnablingValue());
    }
  if (value < 0.0)
    {
      v = (value / wagerSet->getDisablingValue());
    }

  // If wager is "dominant", boost weight to enhance learning.
  if (v > 0.9) v *= 2.0;

#ifdef NEVER
  // Factor in motive.
  NEED f = (double)needSave.size * MAX_NEED;
  if (motive < f) v *= (motive / f);
#endif
  return(v*b);
}

/*
 * Get enablement using weighted average of wager outcomes.
 * The function is updated based on the inclusion of the given
 * wager weight and outcome in the history stream. For example,
 * it might be observed that any expiring wager signals the onset
 * of a long stream of failing wagers. Conversely, it may be
 * observed that an expiration or firing is just a "blip" and
 * not worth causing a change to the enablement.
 */
Mona::ENABLEMENT
Mona::WagerHistory::getEnablement(WEIGHT wagerWeight,
								  WAGER_OUTCOME type, bool enabler)
{
  Event *event;
  ENABLEMENT e;
  WEIGHT sum;

  // Store event.
  event = new Event(wagerWeight, type);
  assert(event != NULL);
  if (newest == NULL)
    {
      newest = oldest = event;
    } else {
      newest->next = event;
	  newest = event;
    }
  eventCount++;
  if (eventCount > MAX_WAGER_HISTORY_EVENTS)
    {
      event = oldest;
      oldest = oldest->next;
      delete event;
      eventCount--;
    }

  // Get weighted average.
  for (event = oldest, e = sum = 0.0; event != NULL; event = event->next)
  {
	  if (enabler)
	  {
		if (event->type == FIRE) e += event->weight;
	  } else {
		// Disabler rewarded for expired wager.
		if (event->type == EXPIRE) e += event->weight;
	  }
	  sum += event->weight;
  }
  if (sum > 0.0) e /= sum;
  e *= MAX_ENABLEMENT;
  if (e < MIN_ENABLEMENT) e = MIN_ENABLEMENT;
  return(e);
}

// Set expected history.
void
Mona::WagerHistory::setHistory(double rate)
{
  int i;
  Event *event;
  WAGER_OUTCOME type;

  for (i = 0; i < MAX_WAGER_HISTORY_EVENTS; i++)
  {
	  if (RAND_PROB < rate) type = FIRE; else type = EXPIRE;

	  event = new Event(1.0, type);
	  assert(event != NULL);
	  if (newest == NULL)
		{
		  newest = oldest = event;
		} else {
		  newest->next = event;
		  newest = event;
		}
	  eventCount++;
	  if (eventCount > MAX_WAGER_HISTORY_EVENTS)
		{
		  event = oldest;
		  oldest = oldest->next;
		  delete event;
		  eventCount--;
		}
  }
}
