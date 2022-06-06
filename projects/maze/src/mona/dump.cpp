/*
 * This software is provided under the terms of the GNU General
 * Public License as published by the Free Software Foundation.
 *
 * Copyright (c) 2003-2005 Tom Portegys, All Rights Reserved.
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

// Mona network dump.

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
    list<Neuron *>::iterator neuronItr;
    Neuron *event;
    int i,j,r;
    double e,g;
    char desc[102];
    int linesz = 12;

    if (delimiter) fprintf(out, "%%%_Begin_network_dump_%%%\n");

    // Textual dump?
    if (dumpType == TEXT)
    {
        for (neuronItr = receptors.begin();
            neuronItr != receptors.end(); neuronItr++)
        {
            receptor = (Receptor *)*neuronItr;
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

        for (neuronItr = motors.begin();
            neuronItr != motors.end(); neuronItr++)
        {
            motor = (Motor *)*neuronItr;
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

        for (neuronItr = mediators.begin();
            neuronItr != mediators.end(); neuronItr++)
        {
            mediator = (Mediator *)*neuronItr;
            #ifdef ACTIVITY_TRACKING
            if ((enablementDump && (mediator->tracker.fire ||
                mediator->tracker.enable)) ||
                (driveDump && mediator->tracker.drive))
                ; else continue;
            #endif
            fprintf(out, "Mediator{\"%s\" ", mediator->id.description);
            e = mediator->getBaseEnablement() + mediator->getMediatingEnablement();
            r = (int)((e / MAX_ENABLEMENT) * 100.0);
            if (r > 100) r = 100; else if (r < -100) r = -100;
            if (e >= 0.0)
            {
                fprintf(out, "enabled (%d%%)/",r);
            }
            else
            {
                fprintf(out, "disabled (%d%%)/",r);
            }
            if (mediator->enabler)
            {
                fprintf(out, "enabling (");
            }
            else
            {
                fprintf(out, "disabling (");
            }
            for (i = 0, j = mediator->causes.size(); i < j; i++)
            {
                event = (Neuron *)mediator->causes[i];
                #ifdef ACTIVITY_TRACKING
                if ((enablementDump && (event->tracker.fire ||
                    event->tracker.enable)) ||
                    (driveDump && event->tracker.drive))
                    ; else continue;
                #endif
                fprintf(out, "\"%s\"", event->id.description);
                if (i < j-1) fprintf(out, ",");
            }
            for (i = 0, j = mediator->intermediates.size(); i < j; i++)
            {
                event = (Neuron *)mediator->intermediates[i];
                #ifdef ACTIVITY_TRACKING
                if ((enablementDump && (event->tracker.fire ||
                    event->tracker.enable)) ||
                    (driveDump && event->tracker.drive))
                    ; else continue;
                #endif
                fprintf(out, "->\"%s\"", event->id.description);
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
    for (neuronItr = receptors.begin();
        neuronItr != receptors.end(); neuronItr++)
    {
        receptor = (Receptor *)*neuronItr;
        #ifdef ACTIVITY_TRACKING
        if ((enablementDump && (receptor->tracker.fire ||
            receptor->tracker.enable)) ||
            (driveDump && receptor->tracker.drive))
            ; else continue;
        #endif
        fprintf(out, "\t\"%x\" [label=\"", receptor);
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
        }
        else
        {
            fprintf(out, "%s\",shape=triangle", desc);
        }
        #ifdef ACTIVITY_TRACKING
        if (receptor->tracker.fire)
        #else
            if (receptor->firingStrength > 0.0)
        #endif
        {
            fprintf(out, ",peripheries=2");
        }
        else
        {
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
    for (neuronItr = motors.begin();
        neuronItr != motors.end(); neuronItr++)
    {
        motor = (Motor *)*neuronItr;
        #ifdef ACTIVITY_TRACKING
        if ((enablementDump && (motor->tracker.fire ||
            motor->tracker.enable)) ||
            (driveDump && motor->tracker.drive))
            ; else continue;
        #endif
        fprintf(out, "\t\"%x\" [label=\"", motor);
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
            if (motor->firingStrength > 0.0)
        #endif
        {
            fprintf(out, ",peripheries=2");
        }
        else
        {
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
    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        #ifdef ACTIVITY_TRACKING
        if ((enablementDump && (mediator->tracker.fire ||
            mediator->tracker.enable)) ||
            (driveDump && mediator->tracker.drive))
            ; else continue;
        #endif
        fprintf(out, "\t\"%x\" [label=\"", mediator);
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
        e = mediator->getBaseEnablement() + mediator->getMediatingEnablement();
        r = (int)((e / MAX_ENABLEMENT) * 100.0);
        if (r > 100) r = 100; else if (r < -100) r = -100;
        if (e >= 0.0)
        {
            if (mediator->enabler)
            {
                fprintf(out, "\\nEnabled (%d%%)\",shape=ellipse", r);
            }
            else
            {
                fprintf(out, "\\nEnabled (%d%%)\",shape=box", r);
            }
        }
        else
        {
            if (mediator->enabler)
            {
                fprintf(out, "\\nDisabled (%d%%)\",shape=ellipse", r);
            }
            else
            {
                fprintf(out, "\\nDisabled (%d%%)\",shape=box", r);
            }
        }
        #ifdef ACTIVITY_TRACKING
        if (mediator->tracker.fire)
        #else
            if (mediator->firingStrength > 0.0)
        #endif
        {
            fprintf(out, ",peripheries=2");
        }
        else
        {
            fprintf(out, ",peripheries=1");
        }
        if (g > 0.0)
        {
            fprintf(out, ",style=filled,color=lightgray");
        }
        fprintf(out, "];\n");
    }
    fprintf(out, "\t};\n");

    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        #ifdef ACTIVITY_TRACKING
        if ((enablementDump && (mediator->tracker.fire ||
            mediator->tracker.enable)) ||
            (driveDump && mediator->tracker.drive))
            ; else continue;
        #endif
        for (i = 0, j = mediator->causes.size(); i < j; i++)
        {
            event = (Neuron *)mediator->causes[i];
            #ifdef ACTIVITY_TRACKING
            if ((enablementDump && (event->tracker.fire ||
                event->tracker.enable)) ||
                (driveDump && event->tracker.drive))
                ; else continue;
            #endif
            fprintf(out, "\t\"%x\" -> \"%x\"", event, mediator);
            #ifdef ACTIVITY_TRACKING
            if (driveDump && mediator->tracker.drive &&
                event->tracker.drive)
            {
                fprintf(out, " [label=cause,style=bold,arrowhead=dot,arrowtail=inv];\n");
            }
            else
            {
                fprintf(out, " [label=cause,style=solid,arrowhead=dot,arrowtail=inv];\n");
            }
            #else
            fprintf(out, " [label=cause,style=solid,arrowhead=dot,arrowtail=inv];\n");
            #endif
        }
        for (i = 0, j = mediator->intermediates.size(); i < j; i++)
        {
            event = (Neuron *)mediator->intermediates[i];
            #ifdef ACTIVITY_TRACKING
            if ((enablementDump && (event->tracker.fire ||
                event->tracker.enable)) ||
                (driveDump && event->tracker.drive))
                ; else continue;
            #endif
            fprintf(out, "\t\"%x\" -> \"%x\"", event, mediator);
            #ifdef ACTIVITY_TRACKING
            if (driveDump && mediator->tracker.drive &&
                event->tracker.drive)
            {
                fprintf(out, " [label=intermediate%d,style=bold,arrowhead=dot,arrowtail=inv];\n",i);
            }
            else
            {
                fprintf(out, " [label=intermediate%d,style=solid,arrowhead=dot,arrowtail=inv];\n",i);
            }
            #else
            fprintf(out, " [label=intermediate%d,style=solid,arrowhead=dot,arrowtail=inv];\n",i);
            #endif
        }
        event = mediator->effect;
        #ifdef ACTIVITY_TRACKING
        if ((enablementDump && (event->tracker.fire ||
            event->tracker.enable)) ||
            (driveDump && event->tracker.drive))
            ; else continue;
        #endif
        fprintf(out, "\t\"%x\" -> \"%x\"", mediator, event);
        #ifdef ACTIVITY_TRACKING
        if (driveDump && mediator->tracker.drive &&
            event->tracker.drive)
        {
            fprintf(out, " [label=effect,style=bold,arrowtail=odot];\n");
        }
        else
        {
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


// Dump mediator in textual or graphical source (Graphviz dot) format.
void
Mona::dumpMediator(int id, enum DUMP_TYPE dumpType, char *title,
FILE *out)
{
    Mediator *mediator;
    list<Neuron *>::iterator neuronItr;
    Neuron *event;
    int i,j,r;
    double e,g;

    // Textual dump?
    if (dumpType == TEXT)
    {
        for (neuronItr = mediators.begin();
            neuronItr != mediators.end(); neuronItr++)
        {
            mediator = (Mediator *)*neuronItr;
            if (mediator->id.value != id) continue;
            fprintf(out, "Mediator{\"%s\" ", mediator->id.description);
            e = mediator->getBaseEnablement() + mediator->getMediatingEnablement();
            r = (int)((e / MAX_ENABLEMENT) * 100.0);
            if (r > 100) r = 100; else if (r < -100) r = -100;
            if (e >= 0.0)
            {
                fprintf(out, "enabled (%d%%)/",r);
            }
            else
            {
                fprintf(out, "disabled (%d%%)/",r);
            }
            if (mediator->enabler)
            {
                fprintf(out, "enabling (");
            }
            else
            {
                fprintf(out, "disabling (");
            }
            for (i = 0, j = mediator->causes.size(); i < j; i++)
            {
                event = (Neuron *)mediator->causes[i];
                fprintf(out, "\"%s\"", event->id.description);
                if (i < j-1) fprintf(out, ",");
            }
            for (i = 0, j = mediator->intermediates.size(); i < j; i++)
            {
                event = (Neuron *)mediator->intermediates[i];
                fprintf(out, "->\"%s\"", event->id.description);
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
        return;
    }

    // Graphical dump
    fprintf(out, "digraph Mediator_%d {\n", id);
    fprintf(out, "\tgraph [size=\"8.5,11\",fontsize=24];\n");
    dumpNeuron(id, out);
    fprintf(out, "\tlabel = \"%s\";\n", title);
    fprintf(out, "}\n");

    return;
}


void
Mona::dumpNeuron(int id, FILE *out)
{
    Receptor *receptor;
    Motor *motor;
    Mediator *mediator;
    list<Neuron *>::iterator neuronItr;
    Neuron *event;
    int i,j,r;
    double e,g;
    char desc[102];
    int linesz = 12;

    for (neuronItr = receptors.begin();
        neuronItr != receptors.end(); neuronItr++)
    {
        receptor = (Receptor *)*neuronItr;
        if (receptor->id.value != id) continue;
        fprintf(out, "\t\"%x\" [label=\"", receptor);
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
        }
        else
        {
            fprintf(out, "%s\",shape=triangle", desc);
        }
        if (receptor->firingStrength > 0.0)
        {
            fprintf(out, ",peripheries=2");
        }
        else
        {
            fprintf(out, ",peripheries=1");
        }
        if (g > 0.0)
        {
            fprintf(out, ",style=filled,color=lightgray");
        }
        fprintf(out, "];\n");
    }

    for (neuronItr = motors.begin();
        neuronItr != motors.end(); neuronItr++)
    {
        motor = (Motor *)*neuronItr;
        if (motor->id.value != id) continue;
        fprintf(out, "\t\"%x\" [label=\"", motor);
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
        if (motor->firingStrength > 0.0)
        {
            fprintf(out, ",peripheries=2");
        }
        else
        {
            fprintf(out, ",peripheries=1");
        }
        if (g > 0.0)
        {
            fprintf(out, ",style=filled,color=lightgray");
        }
        fprintf(out, "];\n");
    }

    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        if (mediator->id.value != id) continue;
        fprintf(out, "\t\"%x\" [label=\"", mediator);
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
        e = mediator->getBaseEnablement() + mediator->getMediatingEnablement();
        r = (int)((e / MAX_ENABLEMENT) * 100.0);
        if (r > 100) r = 100; else if (r < -100) r = -100;
        if (e >= 0.0)
        {
            if (mediator->enabler)
            {
                fprintf(out, "\\nEnabled (%d%%)\",shape=ellipse", r);
            }
            else
            {
                fprintf(out, "\\nEnabled (%d%%)\",shape=box", r);
            }
        }
        else
        {
            if (mediator->enabler)
            {
                fprintf(out, "\\nDisabled (%d%%)\",shape=ellipse", r);
            }
            else
            {
                fprintf(out, "\\nDisabled (%d%%)\",shape=box", r);
            }
        }
        if (mediator->firingStrength > 0.0)
        {
            fprintf(out, ",peripheries=2");
        }
        else
        {
            fprintf(out, ",peripheries=1");
        }
        if (g > 0.0)
        {
            fprintf(out, ",style=filled,color=lightgray");
        }
        fprintf(out, "];\n");
    }

    for (neuronItr = mediators.begin();
        neuronItr != mediators.end(); neuronItr++)
    {
        mediator = (Mediator *)*neuronItr;
        if (mediator->id.value != id) continue;
        for (i = 0, j = mediator->causes.size(); i < j; i++)
        {
            event = (Neuron *)mediator->causes[i];
            dumpNeuron(event->id.value, out);
            fprintf(out, "\t\"%x\" -> \"%x\"", event, mediator);
            fprintf(out, " [label=cause,style=solid,arrowhead=dot,arrowtail=inv];\n");
        }
        for (i = 0, j = mediator->intermediates.size(); i < j; i++)
        {
            event = (Neuron *)mediator->intermediates[i];
            dumpNeuron(event->id.value, out);
            fprintf(out, "\t\"%x\" -> \"%x\"", event, mediator);
            fprintf(out, " [label=intermediate%d,style=solid,arrowhead=dot,arrowtail=inv];\n",i);
        }
        event = mediator->effect;
        dumpNeuron(event->id.value, out);
        fprintf(out, "\t\"%x\" -> \"%x\"", mediator, event);
        fprintf(out, " [label=effect,style=solid,arrowtail=odot];\n");
    }

    return;
}
