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

// The Mona foraging ant program

/*

This program illustrates how a neural-network can be used to simulate
a foraging ant.  The ant must following an erratic trail of marks from
its nest to a piece of food, which it must then carry back to the nest.

*/

#include "monami.hpp"
#ifdef UNIX
#include <getopt.h>
#else
#include "../common/XGetopt.h"
#endif

// Type definitions.
typedef MonAmi::OUTPUT OUTPUT;
typedef Mona::RESPONSE RESPONSE;

// Parameters
const int FieldWidth = 50;
const int FieldHeight = 50;
const int TrailLength = 20;
const double TrailDirParm = .1;
const int NumSensors = 3;
const int MaxResponse = 4;
const int NumNeeds = 1;
const long DefaultRandomSeed = 15;
long RandomSeed = -1;

char *Usemsg = "Usage: %s -i <iterations> [-d <dump frequency>] [-r <random seed>] | -v (version)\n";

int
main(int argc, char *argv[])
{
	OUTPUT output;
	RESPONSE response;
	int i,iterations,dumpFrequency;
	char buf[100];
	Environment *environment;
	Mona *mona;
	MonAmi *monAmi;

	iterations = dumpFrequency = -1;
	while ((i = getopt(argc,argv,"i:d:r:v")) != EOF)
	{
		switch(i)
		{
			case 'i':	if (iterations >= 0)
						{
							fprintf(stderr,Usemsg,argv[0]);
							exit(1);
						}
						if ((iterations = atoi(optarg)) < 0)
						{
							fprintf(stderr,Usemsg,argv[0]);
							exit(1);
						}
						break;

			case 'd':	if (dumpFrequency > 0)
						{
							fprintf(stderr,Usemsg,argv[0]);
							exit(1);
						}
						if ((dumpFrequency = atoi(optarg)) <= 0)
						{
							fprintf(stderr,"%s: dump frequency must be > 0\n",argv[0]);
							exit(1);
						}
						break;

			case 'r':	if (RandomSeed != -1)
						{
							fprintf(stderr,Usemsg,argv[0]);
							exit(1);
						}
						if ((RandomSeed = atoi(optarg)) < 0)
						{
							fprintf(stderr,Usemsg,argv[0]);
							exit(1);
						}
						break;

			case 'v':	Mona::version();
						Environment::version();
						exit(0);

			default:	fprintf(stderr,Usemsg,argv[0]);
						exit(1);
		}
	}
	if (iterations < 0)
	{
		fprintf(stderr,Usemsg,argv[0]);
		exit(1);
	}
	if (dumpFrequency > iterations)
	{
		fprintf(stderr,"%s: dump frequency must be <= iterations\n",argv[0]);
		exit(1);
	}
	if (RandomSeed == -1)
	{
		RandomSeed = DefaultRandomSeed;
	}
	srand(RandomSeed);

	// Create objects.
	environment = new Environment(FieldWidth, FieldHeight, TrailLength, TrailDirParm);
	assert(environment != NULL);
	mona = new Mona(NumSensors, MaxResponse, NumNeeds);
	assert(mona != NULL);
	monAmi = new MonAmi(mona);
	assert(monAmi != NULL);

	if (dumpFrequency != -1)
	{
		mona->dumpNetwork(Mona::GRAPH, "Foraging ant network", true);
	}

	for (i = 0; i < iterations; i++)
	{
		printf("\nIteration=%d\n", i);
		output = environment->output();
		printf("Sensors: ( ");
		environment->printOutput(output);
		printf(")\n");
		monAmi->setNeed(output);
		printf("Need: %f\n", mona->getNeed(0));
		response = mona->cycle(monAmi->outputToSensors(output));
		printf("Response: ( ");
		environment->printInput(monAmi->responseToInput(response));
		printf(" )\n");
		environment->input(monAmi->responseToInput(response));

		if (dumpFrequency != -1 && (i+1)%dumpFrequency == 0)
		{
			sprintf(buf,"Foraging ant network - iteration = %d",i);
			mona->dumpNetwork(Mona::GRAPH, buf, true);
		}
	}

	delete monAmi;
	delete mona;
	delete environment;

	exit(0);
}
