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
#include <math.h>

// Drive need changes caused by goal values through network.
void
Mona::drive()
{
  int i,j,p,q;
  Receptor *receptor;
  Motor *motor;
  Mediator *mediator;
  Wager *wager;
  MotiveAccum motiveAccum(numNeeds);

#ifdef MONA_TRACE
  printf("***Drive phase***\n");
  printf("Needs: "); needs.print();
#endif

  // Reset.
  for (i = 0, j = receptors.getsize(); i < j; i++)
    {
      receptor = (Receptor *)receptors.index(i);
      receptor->firing = false;
	  receptor->motive = 0.0;
      receptor->work[0].config(&needs);
	  receptor->work[1].config(&needs);
    }
  for (i = 0, j = motors.getsize(); i < j; i++)
    {
      motor = (Motor *)motors.index(i);
      motor->firing = false;
      motor->motive = 0.0;
      motor->accum[0].config(&needs);
	  motor->accum[1].config(&needs);
      motor->work[0].config(&needs);
	  motor->work[1].config(&needs);
#ifdef ACTIVITY_TRACKING
	  motor->drivers.clear();
#endif
    }
  for (i = 0, j = mediators.getsize(); i < j; i++)
    {
      mediator = (Mediator *)mediators.index(i);
      mediator->firing = false;
      mediator->causeFiringStrength = 0.0;
      mediator->work[0].config(&needs);
	  mediator->work[1].config(&needs);
      for (p = 0, q = mediator->wagersOut.getsize(); p < q; p++)
		{
		  wager = (Wager *)mediator->wagersOut.index(p);
		  wager->motive = 0.0;
		  wager->accum[0] = wager->accum[1] = 0.0;
		  wager->work[0] = wager->work[1] = 0.0;
		}
	}

  // Drive.
  for (i = 0, j = receptors.getsize(); i < j; i++)
    {
      receptor = (Receptor *)receptors.index(i);
      motiveAccum.config(&needs);
      if (receptor->goals.getValue() > 0.0)
		{
		  clearMotiveWork();
#ifdef ACTIVITY_TRACKING
		  motiveAccum.drivers.clear();
#endif
		  receptor->drive(&motiveAccum);
		  accumMotive();
		}
    }
  for (i = 0, j = motors.getsize(); i < j; i++)
    {
      motor = (Motor *)motors.index(i);
      motiveAccum.config(&needs);
      if (motor->goals.getValue() > 0.0)
		{
		  clearMotiveWork();
#ifdef ACTIVITY_TRACKING
		  motiveAccum.drivers.clear();
#endif
		  motor->drive(&motiveAccum);
		  accumMotive();
		}
    }
  for (i = 0, j = mediators.getsize(); i < j; i++)
    {
      mediator = (Mediator *)mediators.index(i);
      motiveAccum.config(&needs);
      if (mediator->goals.getValue() > 0.0)
		{
		  clearMotiveWork();
#ifdef ACTIVITY_TRACKING
		  motiveAccum.drivers.clear();
#endif
		  mediator->drive(&motiveAccum);
		  accumMotive();
		}
    }

  // Calculate final motives.
  for (i = 0, j = motors.getsize(); i < j; i++)
    {
      motor = (Motor *)motors.index(i);
	  motor->motive = motor->accum[0].getValue();
	  motor->motive -= motor->accum[1].getValue();
    }
  for (i = 0, j = mediators.getsize(); i < j; i++)
    {
      mediator = (Mediator *)mediators.index(i);
      for (p = 0, q = mediator->wagersOut.getsize(); p < q; p++)
		{
		  wager = (Wager *)mediator->wagersOut.index(p);
		  wager->motive = wager->accum[0] - wager->accum[1];
		}
    }
}

// Clear working motives.
void
Mona::clearMotiveWork()
{
  int i,j,p,q;
  Receptor *receptor;
  Motor *motor;
  Mediator *mediator;
  Wager *wager;

  for (i = 0, j = receptors.getsize(); i < j; i++)
    {
      receptor = (Receptor *)receptors.index(i);
      receptor->work[0].reset();
	  receptor->work[1].reset();
    }
  for (i = 0, j = motors.getsize(); i < j; i++)
    {
      motor = (Motor *)motors.index(i);
      motor->work[0].reset();
	  motor->work[1].reset();
#ifdef ACTIVITY_TRACKING
	  motor->driverWork.clear();
#endif
    }
  for (i = 0, j = mediators.getsize(); i < j; i++)
    {
      mediator = (Mediator *)mediators.index(i);
      mediator->work[0].reset();
	  mediator->work[1].reset();
      for (p = 0, q = mediator->wagersOut.getsize(); p < q; p++)
		{
		  wager = (Wager *)mediator->wagersOut.index(p);
		  wager->work[0] = wager->work[1] = 0.0;
		}
    }
}

// Accumulate motives.
void
Mona::accumMotive()
{
  int i,j,p,q;
  Motor *motor;
  Mediator *mediator;
  Wager *wager;

  for (i = 0, j = motors.getsize(); i < j; i++)
    {
      motor = (Motor *)motors.index(i);
      motor->accum[0].accumNeeds(&motor->work[0]);
      motor->accum[1].accumNeeds(&motor->work[1]);
#ifdef ACTIVITY_TRACKING
      for (p = 0, q = motor->driverWork.size(); p < q; p++)
		{
		  motor->drivers.push_back(motor->driverWork[p]);
		}
#endif
    }
  for (i = 0, j = mediators.getsize(); i < j; i++)
    {
      mediator = (Mediator *)mediators.index(i);
      for (p = 0, q = mediator->wagersOut.getsize(); p < q; p++)
		{
		  wager = (Wager *)mediator->wagersOut.index(p);
		  wager->accum[0] += wager->work[0];
		  wager->accum[1] += wager->work[1];
		}
    }
}

// Neuron drive.
void
Mona::Neuron::drive(MotiveAccum *motiveAccum)
{
  Mediator *mediator;
  Wager *wager;
  Neuron *cause;
  struct Notify *notify;
  int i,j;
  MOTIVE motive;
  WEIGHT weight,superweight;
  MotiveAccum accum;
#ifdef ACTIVITY_TRACKING
  int p,q;
#endif

#ifdef MONA_TRACE
  printf("Drive %d (%s), motive=%f\n",id.value, id.description, motiveAccum->getValue());
#endif

  // Validate mediator drive.
  if (type == MEDIATOR)
  {
      mediator = (Mediator *)this;
	  if (mediator->enabler && !motiveAccum->enabler) return;
	  if (!mediator->enabler && motiveAccum->enabler) return;
  }

  // Accumulate need change due to goal value.
  if (motiveAccum->enabler) motiveAccum->accumGoals(&goals);

  // Store maximum motive.
  motive = motiveAccum->getValue();
  if (motiveAccum->enabler)
  {
	if (work[0].getValue() < motive) work[0].loadNeeds(motiveAccum); else return;
  } else {
	if (work[1].getValue() < motive) work[1].loadNeeds(motiveAccum); else return;
  }
#ifdef ACTIVITY_TRACKING
  if (type == MOTOR)
  {
	  Motor *motor = (Motor *)this;
	  motor->driverWork.clear();
	  for (p = 0, q = motiveAccum->drivers.size(); p < q; p++)
	  {
		  motor->driverWork.push_back(motiveAccum->drivers[p]);
	  }
  }
  for (p = 0, q = motiveAccum->drivers.size(); p < q; p++)
  {
	  accum.drivers.push_back(motiveAccum->drivers[p]);
  }
  accum.drivers.push_back(this);
#endif
  for (i = 0, j = wagerSet.wagers.getsize(); i < j; i++)
  {
	  wager = (Wager *)wagerSet.wagers.index(i);
	  if (motiveAccum->enabler)
	  {
		  if (wager->work[0] < motive) wager->work[0] = motive;
	  } else {
		  if (wager->work[1] < motive) wager->work[1] = motive;
	  }
  }

  // Drive terminates on motor neurons.
  if (type == MOTOR) return;

  if (type == MEDIATOR)
  {
      mediator = (Mediator *)this;

	  // Distribute motive to cause events in proportion to maximum
	  // enablement. This represents enablement waiting for the
	  // cause to fire.
	  if (mediator->repeater)
	  {
		// Drive cause repetitively.
		weight = (mediator->getTotalEnablement() / MAX_ENABLEMENT);
	  } else {
		weight = (mediator->getEnablement() / MAX_ENABLEMENT);
	  }
      if (weight > 1.0) weight = 1.0;
	  if (weight < 0.0) weight = 0.0;
      if (weight > 0.0)
	  {
		  accum.config(&(motiveAccum->base));
		  accum.loadNeeds(motiveAccum);
		  weight *= DRIVE_ATTENUATION;
		  accum.scale(weight);
		  accum.weight *= weight;
		  accum.enabler = motiveAccum->enabler;
		  for (i = 0, j = mediator->causes.getsize(); i < j; i++)
		  {
			// Drive causes that have not fired.
			if ((struct PendingWager *)mediator->pendingWagers.index(i) == NULL)
			{
			  cause = (Neuron *)mediator->causes.index(i);
			  cause->drive(&accum);
			}
		  }
	  }

	  // Set weight for super-mediator drive.
	  superweight = 1.0 - weight;

	  // Distribute motive to effect event in proportion to maximum
	  // enablement wagered out by a previous cause firing.
	  weight = (mediator->getEnablementOut() / MAX_ENABLEMENT);
      if (weight > 1.0) weight = 1.0;
	  if (weight < 0.0) weight = 0.0;
      if (weight > 0.0)
	  {
		  accum.config(&(motiveAccum->base));
		  accum.loadNeeds(motiveAccum);
		  weight *= DRIVE_ATTENUATION;
		  accum.scale(weight);
		  accum.weight *= weight;
		  accum.enabler = motiveAccum->enabler;
		  mediator->effect->drive(&accum);
	  }

	  // Set minimum weight for super-mediator drive.
	  if (superweight > 1.0 - weight)
	  {
		  superweight = 1.0 - weight;
	  }

  } else {

	  // Neuron is not mediator.
	  superweight = 1.0;
  }

  // Distribute motive to super-mediators.
  if (superweight > 0.0)
  {
	superweight *= DRIVE_ATTENUATION;
	for (i = 0, j = notifyList.getsize(); i < j; i++)
    {
      notify = (struct Notify *)notifyList.index(i);
	  if (notify->eventType != EFFECT) continue;
      mediator = notify->mediator;

      // Reverse sense of enabler for disabling mediators.
	  accum.config(&(motiveAccum->base));
	  accum.loadNeeds(motiveAccum);
	  accum.scale(superweight);
	  accum.weight *= superweight;
      if (mediator->enabler)
	  {
		  accum.enabler = motiveAccum->enabler;
	  } else {
		  accum.enabler = !motiveAccum->enabler;
	  }
      mediator->driveCause(&accum);
    }
  }
}

// Drive mediator cause.
void
Mona::Mediator::driveCause(MotiveAccum *motiveAccum)
{
  Mediator *mediator;
  Neuron *cause;
  struct Notify *notify;
  int i,j;
  WEIGHT weight;
  MotiveAccum accum;
#ifdef ACTIVITY_TRACKING
  int p,q;
#endif

#ifdef MONA_TRACE
  printf("Drive cause %d (%s)\n",id.value, id.description);
#endif

#ifdef ACTIVITY_TRACKING
  for (p = 0, q = motiveAccum->drivers.size(); p < q; p++)
  {
	  accum.drivers.push_back(motiveAccum->drivers[p]);
  }
  accum.drivers.push_back(this);
#endif

  // Distribute motive to cause events in proportion to maximum
  // enablement. This represents enablement waiting for the
  // cause to fire.
  if (motiveAccum->enabler)
  {
	  if (repeater)
	  {
		// Drive cause repetitively.
		weight = (getTotalEnablement() / MAX_ENABLEMENT);
	  } else {
		weight = (getEnablement() / MAX_ENABLEMENT);
	  }
      if (weight > 1.0) weight = 1.0;
	  if (weight < 0.0) weight = 0.0;
      if (weight > 0.0)
	  {
		  accum.config(&(motiveAccum->base));
		  accum.loadNeeds(motiveAccum);
		  weight *= DRIVE_ATTENUATION;
		  accum.scale(weight);
		  accum.weight *= weight;
		  accum.enabler = motiveAccum->enabler;
		  for (i = 0, j = causes.getsize(); i < j; i++)
		  {
			// Drive causes that have not fired.
			if ((struct PendingWager *)pendingWagers.index(i) == NULL)
			{
			  cause = (Neuron *)causes.index(i);
			  cause->drive(&accum);
			}
		  }
	  }

	  // Set weight for super-mediator.
	  weight = 1.0 - weight;

  } else {

	  // Disabling.
	  weight = 1.0;
  }

  // Distribute motive to super-mediators.
  if (weight > 0.0)
  {
	weight *= DRIVE_ATTENUATION;
	for (i = 0, j = notifyList.getsize(); i < j; i++)
    {
      notify = (struct Notify *)notifyList.index(i);
	  if (notify->eventType != EFFECT) continue;
      mediator = notify->mediator;

      // Reverse sense of enabler for disabling mediators.
	  accum.config(&(motiveAccum->base));
	  accum.loadNeeds(motiveAccum);
	  accum.scale(weight);
	  accum.weight *= weight;
      if (mediator->enabler)
	  {
		  accum.enabler = motiveAccum->enabler;
	  } else {
		  accum.enabler = !motiveAccum->enabler;
	  }
      mediator->driveCause(&accum);
    }
  }
}
