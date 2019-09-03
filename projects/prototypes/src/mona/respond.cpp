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

// Respond.
void
Mona::respond()
{
  Motor *motor;
  RESPONSE_POTENTIAL max;
  int i,j;
#ifdef ACTIVITY_TRACKING
  int p,q;
  Neuron *neuron;
#endif

#ifdef MONA_TRACE
  printf("***Respond phase***\n");
#endif

  for (i = 0; i <= maxResponse; i++)
    {
      responsePotentials[i] = 0.0;
    }

  for (i = 0, j = motors.getsize(); i < j; i++)
    {
      motor = (Motor *)motors.index(i);
      responsePotentials[motor->response] += motor->motive;
    }

  if (responseMode == PROBABILISTIC)
  {
	  // Probabilistically select response.
	  for (i = 0; i <= maxResponse; i++)
	  {
		  responseWork[i] = (int)(responsePotentials[i] * 1000.0);
		  if (i > 0) responseWork[i] += responseWork[i-1];
	  }
	  if (responseWork[maxResponse] > 1)
	  {
		  j = RAND_CHOICE(responseWork[maxResponse]);
		  for (i = 0; i <= maxResponse; i++)
		  {
			  if (j < responseWork[i])
			  {
				  response = i;
				  break;
			  }
		  }
	  } else {
		  response = RAND_CHOICE(maxResponse + 1);
	  }

  } else {	// DETERMINISTIC

	// Select maximum response potential.
	for (i = 0, response = -1; i <= maxResponse; i++)
	{
		if (response == -1 || responsePotentials[i] > max)
		{
			response = i;
			max = responsePotentials[i];
		}
	}
  }

  // Firing responding motor.
  for (i = 0, j = motors.getsize(); i < j; i++)
    {
      motor = (Motor *)motors.index(i);
      if (motor->response == response)
		{
		   motor->firing = true;

#ifdef MONA_TRACE
		   printf("Motor firing: %s\n", motor->id.description);
#endif

#ifdef ACTIVITY_TRACKING
		   motor->tracker.fire = true;
		   motor->tracker.drive = true;
		   for (p = 0, q = motor->drivers.size(); p < q; p++)
		   {
			   neuron = motor->drivers[p];
			   neuron->tracker.drive = true;
		   }
#endif

#ifdef LEARN
		   // Learn from event.
		   learn(motor);
#endif

		}
    }

#ifdef MONA_TRACE
  printf("Response = %d\n", response);
#endif
}
