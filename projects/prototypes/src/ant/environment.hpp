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

#ifndef __ENVIRONMENT__
#define __ENVIRONMENT__

// The Mona foraging ant environment definitions

#ifdef UNIX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#else
#include <cstdio>
#include <cstdlib>
#include <string>
#endif
#include <iostream>
#include <assert.h>
#include "../common/common.h"
using namespace std;

#define ANT_ENVIRONMENT_VERSION "@(#) Mona foraging ant environment - version 1.2"

class Environment
{

public:

	// Version
	static void version();

	// Inputs
	enum INPUT { FORWARD=0, BACKWARD=1, ORIENT=2, GRAB=3, DROP=4 };

	// Cell
	struct Cell
	{
		int nest;
		int mark;
		int food;
	};

	// Output
	typedef struct Cell OUTPUT;

	// Set output
	static void setOutput(OUTPUT *o, int n, int m, int f)
	{
		o->nest = n;
		o->mark = m;
		o->food = f;
	}

	// Constructor
	Environment(int, int, int, double);

	// Functions
	void input(INPUT);
	OUTPUT output();

	// Print input/output to stdout
	void printInput(INPUT);
	void printOutput(OUTPUT);

private:

	// Data
	struct Cell **field;	// foraging field
	int width,height;	// dimensions of field
	int x,y;		// current location and direction of ant
	enum DIR { RIGHT=0, LEFT=1, UP=2, DOWN=3 } dir;
	bool hasFood;		// ant has food?
	double trailDirParm;	// probability of changing trail direction
};
#endif
