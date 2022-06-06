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

// The Mona foraging ant utility functions

#include "monami.hpp"

// Constant initialization.
const MonAmi::INPUT MonAmi::FORWARD = Environment::FORWARD;
const MonAmi::INPUT MonAmi::BACKWARD = Environment::BACKWARD;
const MonAmi::INPUT MonAmi::ORIENT = Environment::ORIENT;
const MonAmi::INPUT MonAmi::GRAB = Environment::GRAB;
const MonAmi::INPUT MonAmi::DROP = Environment::DROP;

// Build Mona
MonAmi::MonAmi(Mona *mona)
{
	Mona::Mediator *forage,*grabFood,*getFood,
		*dropFood,*bringFood,
		*disableBringFood,*toFood,*toNest,
		*stepTrail,*travelTrail,*disableStepTrail,
		*backOnTrail,*orientToTrail,*trailOrient,
		*trailSearch,*enableStepTrail;
	Mona::Receptor *nest,*mark,*nomark,*food,*nofood,*nestFood;
	Mona::Motor *forward,*backward,*orient,*grab,*drop;
	VALUE_SET goals;

	assert(mona != NULL);
	this->mona = mona;

	// Need.
	mona->initNeed(0, Mona::MAX_NEED, "Food at nest");

	// Receptors
	nest = mona->newReceptor(setSensors(TRUE, DONT_CARE, DONT_CARE), "Nest");
	mark = mona->newReceptor(setSensors(DONT_CARE, TRUE, DONT_CARE), "Mark");
	nomark = mona->newReceptor(setSensors(DONT_CARE, FALSE, DONT_CARE), "No mark");
	food = mona->newReceptor(setSensors(DONT_CARE, DONT_CARE, TRUE), "Food");
	nofood = mona->newReceptor(setSensors(DONT_CARE, DONT_CARE, FALSE), "No food");
	nestFood = mona->newReceptor(setSensors(TRUE, DONT_CARE, TRUE), "Food at nest");

	// Motors
	forward = mona->newMotor(FORWARD, "Forward");
	backward = mona->newMotor(BACKWARD, "Backward");
	orient = mona->newMotor(ORIENT, "Orient");
	grab = mona->newMotor(GRAB, "Grab");
	drop = mona->newMotor(DROP, "Drop");

	// Mediators

	// grabFood: (food -> grab)
	grabFood = mona->newMediator(Mona::MAX_ENABLEMENT, true, "Grab food");
	grabFood->addEvent(Mona::CAUSE, (Mona::Neuron *)food);
	grabFood->addEvent(Mona::EFFECT, (Mona::Neuron *)grab);

	// getFood: (grabFood -> nofood)
	getFood = mona->newMediator(Mona::MAX_ENABLEMENT, true, "Get food");
	getFood->addEvent(Mona::CAUSE, (Mona::Neuron *)grabFood);
	getFood->addEvent(Mona::EFFECT, (Mona::Neuron *)nofood);

	// dropFood: (nest -> drop)
	dropFood = mona->newMediator(Mona::MAX_ENABLEMENT, true, "Drop food");
	dropFood->addEvent(Mona::CAUSE, (Mona::Neuron *)nest);
	dropFood->addEvent(Mona::EFFECT, (Mona::Neuron *)drop);

	// bringFood: -(dropFood -> nestFood)
	bringFood = mona->newMediator(Mona::MIN_ENABLEMENT, true, "Bring food");
	bringFood->addEvent(Mona::CAUSE, (Mona::Neuron *)dropFood);
	bringFood->addEvent(Mona::EFFECT, (Mona::Neuron *)nestFood);

	// disableBringFood: (nestFood -> -bringFood)
	disableBringFood = mona->newMediator(Mona::MAX_ENABLEMENT, false, "Disable bring food");
	disableBringFood->addEvent(Mona::CAUSE, (Mona::Neuron *)nestFood);
	disableBringFood->addEvent(Mona::EFFECT, (Mona::Neuron *)bringFood);

	// forage: (grabFood -> bringFood)
	forage = mona->newMediator(Mona::MAX_ENABLEMENT, true, "Forage");
	forage->addEvent(Mona::CAUSE, (Mona::Neuron *)grabFood);
	forage->addEvent(Mona::EFFECT, (Mona::Neuron *)bringFood);
	forage->timedWagerWeights[0] = 0.0;
	forage->timedWagerWeights[1] = 0.0;
	forage->timedWagerWeights[2] = 1.0;

	// stepTrail: (mark -> forward)
	stepTrail = mona->newMediator(Mona::MAX_ENABLEMENT, true, "Step trail");
	stepTrail->addEvent(Mona::CAUSE, (Mona::Neuron *)mark);
	stepTrail->addEvent(Mona::EFFECT, (Mona::Neuron *)forward);

	// travelTrail: (stepTrail -> mark)
	travelTrail = mona->newMediator(Mona::MAX_ENABLEMENT, true, "Travel trail");
	travelTrail->addEvent(Mona::CAUSE, (Mona::Neuron *)stepTrail);
	travelTrail->addEvent(Mona::EFFECT, (Mona::Neuron *)mark);

	// toFood: (travelTrail -> food)
	toFood = mona->newMediator(Mona::MAX_ENABLEMENT, true, "To food");
	toFood->addEvent(Mona::CAUSE, (Mona::Neuron *)travelTrail);
	toFood->addEvent(Mona::EFFECT, (Mona::Neuron *)food);
	toFood->timedWagerWeights[0] = 0.0;
	toFood->timedWagerWeights[1] = 0.0;
	toFood->timedWagerWeights[2] = 1.0;
	toFood->setRepeater(true);

	// toNest: (travelTrail -> nest)
	toNest = mona->newMediator(Mona::MAX_ENABLEMENT, true, "To nest");
	toNest->addEvent(Mona::CAUSE, (Mona::Neuron *)travelTrail);
	toNest->addEvent(Mona::EFFECT, (Mona::Neuron *)nest);
	toNest->timedWagerWeights[0] = 0.0;
	toNest->timedWagerWeights[1] = 0.0;
	toNest->timedWagerWeights[2] = 1.0;
	toNest->setRepeater(true);

	// disableStepTrail: (nomark -> -travelTrail)
	disableStepTrail = mona->newMediator(Mona::MAX_ENABLEMENT, false, "Disable step trail");
	disableStepTrail->addEvent(Mona::CAUSE, (Mona::Neuron *)nomark);
	disableStepTrail->addEvent(Mona::EFFECT, (Mona::Neuron *)stepTrail);
	disableStepTrail->timedWagerWeights[0] = 0.0;
	disableStepTrail->timedWagerWeights[1] = 1.0;

	// backOnTrail: (nomark -> backward)
	backOnTrail = mona->newMediator(Mona::MAX_ENABLEMENT, true, "Back on trail");
	backOnTrail->addEvent(Mona::CAUSE, (Mona::Neuron *)nomark);
	backOnTrail->addEvent(Mona::EFFECT, (Mona::Neuron *)backward);

	// orientToTrail: (mark -> orient)
	orientToTrail = mona->newMediator(Mona::MAX_ENABLEMENT, true, "Orient to trail");
	orientToTrail->addEvent(Mona::CAUSE, (Mona::Neuron *)mark);
	orientToTrail->addEvent(Mona::EFFECT, (Mona::Neuron *)orient);

	// trailOrient: (backOnTrail -> orientToTrail)
	trailOrient = mona->newMediator(Mona::MAX_ENABLEMENT, true, "Trail orient");
	trailOrient->addEvent(Mona::CAUSE, (Mona::Neuron *)backOnTrail);
	trailOrient->addEvent(Mona::EFFECT, (Mona::Neuron *)orientToTrail);

	// trailSearch: (trailOrient -> mark)
	trailSearch = mona->newMediator(Mona::MAX_ENABLEMENT, true, "Trail search");
	trailSearch->addEvent(Mona::CAUSE, (Mona::Neuron *)trailOrient);
	trailSearch->addEvent(Mona::EFFECT, (Mona::Neuron *)mark);

	// enableStepTrail: (trailSearch -> travelTrail)
	enableStepTrail = mona->newMediator(Mona::MAX_ENABLEMENT, true, "Enable step trail");
	enableStepTrail->addEvent(Mona::CAUSE, (Mona::Neuron *)trailSearch);
	enableStepTrail->addEvent(Mona::EFFECT, (Mona::Neuron *)stepTrail);
	enableStepTrail->timedWagerWeights[0] = 0.0;
	enableStepTrail->timedWagerWeights[1] = 1.0;

	// Needs/goals
	goals.alloc(mona->numNeeds);
	goals.set(0, Mona::MAX_NEED);
	nestFood->goals.setGoals(&goals, 1.0);
}

// Mona/translators
Mona::SENSOR *
MonAmi::outputToSensors(OUTPUT output)
{
	static Mona::SENSOR sensors[3];

	sensors[0] = output.nest;
	sensors[1] = output.mark;
	sensors[2] = output.food;

	return(sensors);
}

Environment::INPUT
MonAmi::responseToInput(Mona::RESPONSE response)
{
	switch(response)
	{
	case 0: return(FORWARD);
	case 1: return(BACKWARD);
	case 2: return(ORIENT);
	case 3: return(GRAB);
	case 4: return(DROP);
	}
	assert(0);
	return(FORWARD);
}

// Set sensors
Mona::SENSOR *
MonAmi::setSensors(int n, int m, int f)
{
	static OUTPUT o;

	Environment::setOutput(&o, n, m, f);
	return(outputToSensors(o));
}

// Set need.
void
MonAmi::setNeed(OUTPUT output)
{
	// If food at nest, reduce need.
	if (output.nest == TRUE && output.food == TRUE)
	{
		mona->setNeed(0, 0.0);
	}
}

