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

// Sense environment.
void
Mona::sense()
{
  Receptor *receptor;
  int i,j;

#ifdef MONA_TRACE
  printf("***Sense phase***\n");
  printf("Sensors: ");
  for (i = 0; i < numSensors; i++)
    {
      printf("%d ", sensors[i]);
    }
  printf("\n");
#endif

  for (i = 0, j = receptors.getsize(); i < j; i++)
    {
      receptor = (Receptor *)receptors.index(i);
      receptor->sense(sensors, numSensors);
    }
}

// Sense firing of receptor neuron.
void
Mona::Receptor::sense(SENSOR *sensors, int numSensors)
{
  int i;

  for (i = 0; i < numSensors; i++)
    {
      if (sensors[i] != sensorMask[i] && sensorMask[i] != DONT_CARE)
		{
		  firing = false;
		  return;
		}
    }

#ifdef MONA_TRACE
  printf("Receptor firing: %s\n", id.description);
#endif
  firing = true;

#ifdef ACTIVITY_TRACKING
  tracker.fire = true;
#endif

#ifdef LEARN
  // Learn from event.
  learn(this);
#endif
}
