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

#ifndef __MONAMI__
#define __MONAMI__

// The Mona foraging ant utility definitions

#include "../mona/mona.hpp"
#include "environment.hpp"

class MonAmi
{

public:

	// Constructor
	MonAmi(Mona *mona);

	// Mona/Environment translators
	Mona::SENSOR *outputToSensors(Environment::OUTPUT);
	Environment::INPUT responseToInput(Mona::RESPONSE);

	// Shorthands for environment symbols
	typedef Environment::INPUT INPUT;
	typedef Environment::OUTPUT OUTPUT;
	static const INPUT FORWARD;
	static const INPUT BACKWARD;
	static const INPUT ORIENT;
	static const INPUT GRAB;
	static const INPUT DROP;

	// Set need.
	void setNeed(Environment::OUTPUT);

	// Set sensors
	Mona::SENSOR *setSensors(int, int, int);

private:

	Mona *mona;
};
#endif


