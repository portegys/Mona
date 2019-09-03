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

// Mona utility functions

#include "mona.hpp"

// Dump network description in textual or graphical source (Graphviz dot) format.
// The textual dump is a static snapshot - does not contain dynamic data
// such as firing states, etc.
void
#ifdef ACTIVITY_TRACKING
Mona::dumpNetwork(enum DUMP_TYPE dumpType, char *title,
	bool enablementDump, bool driveDump, bool delimiter, FILE *out)
#else
Mona::dumpNetwork(enum DUMP_TYPE dumpType, char *title,
		bool delimiter, FILE *out)
#endif
{
	Receptor *receptor;
	Motor *motor;
	Mediator *mediator;
	Neuron *event;
	int i,j,p,q,r,s;
	double e,g;
	char desc[102];
	int linesz = 12;

	if (delimiter) fprintf(out, "%%%_Begin_network_dump_%%%\n");

	// Textual dump?
	if (dumpType == TEXT)
	{
		for (i = 0, j = receptors.getsize(); i < j; i++)
		{
			receptor = (Receptor *)receptors.index(i);
#ifdef ACTIVITY_TRACKING
			if ((enablementDump && (receptor->tracker.fire ||
				receptor->tracker.enable)) ||
			   (driveDump && receptor->tracker.drive))
			   ; else continue;
#endif
			fprintf(out, "Receptor{\"%s\" (", receptor->id.description);
			for (i = 0, j = numSensors-1; i < j; i++)
			{
				fprintf(out, "%d,", receptor->sensorMask[i]);
			}
			fprintf(out, "%d)", receptor->sensorMask[i]);
			if ((g = receptor->goals.getValue()) > 0.0)
			{
				fprintf(out, " goal value=%f", g);
			}
			fprintf(out, "}\n");
		}

		for (i = 0, j = motors.getsize(); i < j; i++)
		{
			motor = (Motor *)motors.index(i);
#ifdef ACTIVITY_TRACKING
			if ((enablementDump && (motor->tracker.fire ||
				motor->tracker.enable)) ||
			   (driveDump && motor->tracker.drive))
			   ; else continue;
#endif
			fprintf(out, "Motor{\"%s\" (%d) motive=%f", motor->id.description,
				motor->response, motor->motive);
			if ((g = motor->goals.getValue()) > 0.0)
			{
				fprintf(out, " goal value=%f", g);
			}
			fprintf(out, "}\n");
		}

		for (i = 0, j = mediators.getsize(); i < j; i++)
		{
			mediator = (Mediator *)mediators.index(i);
#ifdef ACTIVITY_TRACKING
			if ((enablementDump && (mediator->tracker.fire ||
				mediator->tracker.enable)) ||
			   (driveDump && mediator->tracker.drive))
			   ; else continue;
#endif
			fprintf(out, "Mediator{\"%s\" ", mediator->id.description);
			e = mediator->getEnablement();
			r = (int)((e / MAX_ENABLEMENT) * 100.0);
			if (e >= 0.0)
			{
				fprintf(out, "enabled (%d%%)/",r);
			} else {
				fprintf(out, "disabled (%d%%)/",r);
			}
			if (mediator->enabler)
			{
				fprintf(out, "enabling (");
			} else {
				fprintf(out, "disabling (");
			}
			for (p = 0, q = mediator->causes.getsize(); p < q; p++)
			{
				event = (Neuron *)mediator->causes.index(p);
#ifdef ACTIVITY_TRACKING
				if ((enablementDump && (event->tracker.fire ||
					event->tracker.enable)) ||
					(driveDump && event->tracker.drive))
					 ; else continue;
#endif
				fprintf(out, "\"%s\"", event->id.description);
				if (p < q-1) fprintf(out, ",");
			}
			event = mediator->effect;
			fprintf(out, "->\"%s\"",  event->id.description);
			fprintf(out, ")");
			if ((g = mediator->goals.getValue()) > 0.0)
			{
				fprintf(out, " goal value=%f", g);
			}
			fprintf(out, "}\n");
		}

		if (delimiter) fprintf(out, "%%%_End_network_dump_%%%\n");

		return;
	}

	// Graphical dump
	fprintf(out, "digraph Mona {\n");
	fprintf(out, "\tgraph [size=\"8.5,11\",fontsize=24];\n");

	fprintf(out, "\tsubgraph cluster_0 {\n");
	fprintf(out, "\tlabel=\"Receptors\";\n");
	for (p = 0, q = receptors.getsize(); p < q; p++)
	{
		receptor = (Receptor *)receptors.index(p);
#ifdef ACTIVITY_TRACKING
		if ((enablementDump && (receptor->tracker.fire ||
			receptor->tracker.enable)) ||
			(driveDump && receptor->tracker.drive))
			; else continue;
#endif
		fprintf(out, "\t\"%p\" [label=\"", (void *)receptor);
		for (i = j = r = 0; i < 100 && receptor->id.description[j] != '\0'; i++, j++, r++)
		{
			if ((desc[i] = receptor->id.description[j]) == ' ')
			{
				if (r >= linesz)
				{
					desc[i] = '\\'; i++;
					desc[i] = 'n';
					r = 0;
				}
			}
		}
		desc[i] = '\0';
		if ((g = receptor->goals.getValue()) > 0.0)
		{
			fprintf(out, "%s\\nGoal value=%.2f\",shape=triangle", desc, g);
		} else {
			fprintf(out, "%s\",shape=triangle", desc);
		}
#ifdef ACTIVITY_TRACKING
		if (receptor->tracker.fire)
#else
		if (receptor->firing)
#endif
		{
			fprintf(out, ",peripheries=2");
		} else {
			fprintf(out, ",peripheries=1");
		}
		if (g > 0.0)
		{
			fprintf(out, ",style=filled,color=lightgray");
		}
		fprintf(out, "];\n");
	}
	fprintf(out, "\t};\n");

	fprintf(out, "\tsubgraph cluster_1 {\n");
	fprintf(out, "\tlabel=\"Motors\";\n");
	for (p = 0, q = motors.getsize(); p < q; p++)
	{
		motor = (Motor *)motors.index(p);
#ifdef ACTIVITY_TRACKING
		if ((enablementDump && (motor->tracker.fire ||
			motor->tracker.enable)) ||
			(driveDump && motor->tracker.drive))
			; else continue;
#endif
		fprintf(out, "\t\"%p\" [label=\"", (void *)motor);
		for (i = j = r = 0; i < 100 && motor->id.description[j] != '\0'; i++, j++, r++)
		{
			if ((desc[i] = motor->id.description[j]) == ' ')
			{
				if (r >= linesz)
				{
					desc[i] = '\\'; i++;
					desc[i] = 'n';
					r = 0;
				}
			}
		}
		desc[i] = '\0';
		fprintf(out, "%s", desc);
		if ((g = motor->goals.getValue()) > 0.0)
		{
			fprintf(out, "\\nGoal value=%.2f", g);
		}
		fprintf(out, "\\nMotive=%.2f\",shape=triangle,orientation=180", motor->motive);
#ifdef ACTIVITY_TRACKING
		if (motor->tracker.fire)
#else
		if (motor->firing)
#endif
		{
			fprintf(out, ",peripheries=2");
		} else {
			fprintf(out, ",peripheries=1");
		}
		if (g > 0.0)
		{
			fprintf(out, ",style=filled,color=lightgray");
		}
		fprintf(out, "];\n");
	}
	fprintf(out, "\t};\n");

	fprintf(out, "\tsubgraph cluster_2 {\n");
	fprintf(out, "\tlabel=\"Mediators\";\n");
	for (p = 0, q = mediators.getsize(); p < q; p++)
	{
		mediator = (Mediator *)mediators.index(p);
#ifdef ACTIVITY_TRACKING
		if ((enablementDump && (mediator->tracker.fire ||
			mediator->tracker.enable)) ||
			(driveDump && mediator->tracker.drive))
			 ; else continue;
#endif
		fprintf(out, "\t\"%p\" [label=\"", (void *)mediator);
		for (i = j = r = 0; i < 100 && mediator->id.description[j] != '\0'; i++, j++, r++)
		{
			if ((desc[i] = mediator->id.description[j]) == ' ')
			{
				if (r >= linesz)
				{
					desc[i] = '\\'; i++;
					desc[i] = 'n';
					r = 0;
				}
			}
		}
		desc[i] = '\0';
		fprintf(out, "%s", desc);
		if ((g = mediator->goals.getValue()) > 0.0)
		{
			fprintf(out, "\\nGoal value=%.2f", g);
		}
		e = mediator->getEnablement();
		r = (int)((e / MAX_ENABLEMENT) * 100.0);
		if (e >= 0.0)
		{
			if (mediator->enabler)
			{
				fprintf(out, "\\nEnabled (%d%%)\",shape=ellipse", r);
			} else {
				fprintf(out, "\\nEnabled (%d%%)\",shape=box", r);
			}
		} else {
			if (mediator->enabler)
			{
				fprintf(out, "\\nDisabled (%d%%)\",shape=ellipse", r);
			} else {
				fprintf(out, "\\nDisabled (%d%%)\",shape=box", r);
			}
		}
#ifdef ACTIVITY_TRACKING
		if (mediator->tracker.fire)
#else
		if (mediator->firing)
#endif
		{
			fprintf(out, ",peripheries=2");
		} else {
			fprintf(out, ",peripheries=1");
		}
		if (g > 0.0)
		{
			fprintf(out, ",style=filled,color=lightgray");
		}
		fprintf(out, "];\n");
	}
	fprintf(out, "\t};\n");

	for (p = 0, q = mediators.getsize(); p < q; p++)
	{
		mediator = (Mediator *)mediators.index(p);
#ifdef ACTIVITY_TRACKING
		if ((enablementDump && (mediator->tracker.fire ||
			mediator->tracker.enable)) ||
			(driveDump && mediator->tracker.drive))
			 ; else continue;
#endif
		for (r = 0, s = mediator->causes.getsize(); r < s; r++)
		{
			event = (Neuron *)mediator->causes.index(r);
#ifdef ACTIVITY_TRACKING
			if ((enablementDump && (event->tracker.fire ||
				event->tracker.enable)) ||
				(driveDump && event->tracker.drive))
				 ; else continue;
#endif
			fprintf(out, "\t\"%p\" -> \"%p\"", (void *)event, (void *)mediator);
#ifdef ACTIVITY_TRACKING
			if (driveDump && mediator->tracker.drive &&
				event->tracker.drive)
			{
				fprintf(out, " [label=cause,style=solid,arrowhead=dot,arrowtail=inv];\n");
			} else {
				fprintf(out, " [label=cause,style=solid,arrowhead=dot,arrowtail=inv];\n");
			}
#else
			fprintf(out, " [label=cause,style=solid,arrowhead=dot,arrowtail=inv];\n");
#endif
		}
		event = mediator->effect;
#ifdef ACTIVITY_TRACKING
		if ((enablementDump && (event->tracker.fire ||
			event->tracker.enable)) ||
			(driveDump && event->tracker.drive))
			 ; else continue;
#endif
		fprintf(out, "\t\"%p\" -> \"%p\"", (void *)mediator, (void *)event);
#ifdef ACTIVITY_TRACKING
		if (driveDump && mediator->tracker.drive &&
			event->tracker.drive)
		{
			fprintf(out, " [label=effect,style=solid,arrowtail=odot];\n");
		} else {
			fprintf(out, " [label=effect,style=solid,arrowtail=odot];\n");
		}
#else
		fprintf(out, " [label=effect,style=solid,arrowtail=odot];\n");
#endif
	}

	fprintf(out, "\tlabel = \"%s\";\n", title);

	fprintf(out, "}\n");

	if (delimiter) fprintf(out, "%%%_End_network_dump_%%%\n");

	return;
}
