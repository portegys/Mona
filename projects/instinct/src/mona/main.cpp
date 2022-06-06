/*
 * This software is provided under the terms of the GNU General
 * Public License as published by the Free Software Foundation.
 *
 * Copyright (c) 2005 Tom Portegys, All Rights Reserved.
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

/*
Mona main interactive driver.

Usage: mona [<input timeout (secs)>]

Interaction is through standard input/output.

Set the primary parameters:
parameters: <number of sensors> <maximum response> <number of needs>

The value of MAX_MEDIATOR_LEVEL is initially output to allow
the optional setting of the event timers as follows:

Set the number of timers for all mediator levels:
settimers: <number of timers>
The initial values will be set to 1.

Set individual values within the timer array:
timer: <mediator level> <timer number> <timer value>

Set the need values:
need: <need number> <need value>

Set goal values:
goals: <receptor sensor values> <goal values>

Run a sensory/response cycle:
cycle: <sensor values>
(output:) <response>

Need changes may be interspersed between cycles.

To override response (for training):
response: <response> (-1 removes override)

To clear working memory:
clear

To reset long term memory:
reset

To quit:
quit

For help:
help

*/

#include "mona.hpp"
#ifdef UNIX
#include <unistd.h>
#include <ctype.h>
#else
#include <windows.h>
#include <process.h>
#endif

// Commands:
#define SET_PARAMETERS 0
#define SET_NUM_TIMERS 1
#define SET_TIMER 2
#define SET_NEED 3
#define SET_GOALS 4
#define CYCLE 5
#define OVERRIDE_RESPONSE 6
#define ERASE 7
#define LOG 8
#define HELP 9
#define QUIT 10
#define UNKNOWN 11

// Input timeout.
int timeout = -1;

#ifndef UNIX
int timer;

// Exit on stdin timeout.
VOID CALLBACK alrm_exit(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime )
{
    exit(1);
}
#endif

// Get command
int getCommand()
{
    char command[50];

    #ifdef UNIX
    if (timeout > 0) alarm(timeout);
    #else
    if (timeout > 0) timer = SetTimer(0, 0, timeout * 1000, (TIMERPROC)alrm_exit);
    #endif
    scanf("%s", command);
    #ifdef UNIX
    if (timeout > 0) alarm(0);
    #else
    if (timeout > 0) KillTimer(0, timer);
    #endif
    switch(tolower(command[0]))
    {
        case 'p': return SET_PARAMETERS;
        case 's': return SET_NUM_TIMERS;
        case 't': return SET_TIMER;
        case 'n': return SET_NEED;
        case 'g': return SET_GOALS;
        case 'c': return CYCLE;
        case 'r': return OVERRIDE_RESPONSE;
        case 'e': return ERASE;
        case 'l': return LOG;
        case 'h': return HELP;
        case 'q': return QUIT;
        default: return UNKNOWN;
    }
}


int main(int argc, char *argv[])
{
    int i,j,n;
    int numSensors, maxResponse, numNeeds;
    Mona *mona;
    Mona::SENSOR *sensors;
    int response;
    Mona::NEED need;
    VALUE_SET *goals;
    Mona::Receptor *receptor;
    list<Mona::Neuron *>::iterator neuronItr;
    char description[100],buf[50];
    FILE *logfp;

    // Get optional timeout.
    if (argc == 2)
    {
        timeout = atoi(argv[1]);
    }
    if (timeout < -1 || timeout == 0 || argc > 2)
    {
        fprintf(stderr, "Usage: %s [<input timeout (secs)>]\n", argv[0]);
        exit(1);
    }

    // Command loop.
    printf("MAX_MEDIATOR_LEVEL=%d\n", Mona::MAX_MEDIATOR_LEVEL);
    fflush(stdout);
    mona = NULL;
    sensors = NULL;
    goals = NULL;
    logfp = NULL;
    bool done = false;
    while (!done)
    {
        switch(getCommand())
        {
            case SET_PARAMETERS:
                scanf("%d %d %d", &numSensors, &maxResponse, &numNeeds);
                if (logfp != NULL)
                {
                    fprintf(logfp, "parameters: %d %d %d\n", numSensors, maxResponse, numNeeds);
                    fflush(logfp);
                }
                if (numSensors <= 0)
                {
                    fprintf(stderr, "Number of sensors must be > 0\n");
                    fflush(stderr);
                    exit(1);
                }
                if (maxResponse <= 0)
                {
                    fprintf(stderr, "Maximum response must be > 0\n");
                    fflush(stderr);
                    exit(1);
                }
                if (numNeeds <= 0)
                {
                    fprintf(stderr, "Number of needs must be > 0\n");
                    fflush(stderr);
                    exit(1);
                }

                // Create mona.
                if (mona != NULL) delete mona;
                mona = new Mona(numSensors, maxResponse, numNeeds);
                if (mona == NULL)
                {
                    fprintf(stderr, "Cannot create mona\n");
                    fflush(stderr);
                    exit(1);
                }
                if (sensors != NULL) delete sensors;
                sensors = new Mona::SENSOR[numSensors];
                if (sensors == NULL)
                {
                    fprintf(stderr, "Cannot create sensors\n");
                    fflush(stderr);
                    exit(1);
                }
                if (goals != NULL) delete goals;
                goals = new VALUE_SET;
                if (goals == NULL)
                {
                    fprintf(stderr, "Cannot create goals\n");
                    fflush(stderr);
                    exit(1);
                }
                goals->alloc(numNeeds);

                // Add motor neurons.
                for (i = 0; i <= maxResponse; i++)
                {
                    sprintf(description, "Motor %d", i);
                    mona->newMotor(i, description);
                }

                // Add needs.
                for (i = 0; i < numNeeds; i++)
                {
                    sprintf(description, "Need %d", i);
                    mona->initNeed(i, 0.0, description);
                }

                // Assign default effect delay timers.
                for (i = 0; i <= Mona::MAX_MEDIATOR_LEVEL; i++)
                {
                    mona->eventTimers[i].resize(1);
                    mona->eventTimers[i][0] = 1;
                }
                break;
            case SET_NUM_TIMERS:
                scanf("%d", &n);
                if (logfp != NULL)
                {
                    fprintf(logfp, "settimers: %d\n", n);
                    fflush(logfp);
                }
                if (mona == NULL) break;
                for (i = 0; i <= Mona::MAX_MEDIATOR_LEVEL; i++)
                {
                    mona->eventTimers[i].resize(n);
                    for (j = 0; j < n; j++)
                    {
                        mona->eventTimers[i][j] = 1;
                    }
                }
                break;
            case SET_TIMER:
                scanf("%d %d %d", &i, &j, &n);
                if (logfp != NULL)
                {
                    fprintf(logfp, "timer: %d %d %d\n", i, j, n);
                    fflush(logfp);
                }
                if (mona == NULL) break;
                mona->eventTimers[i][j] = n;
                break;
            case SET_NEED:
                scanf("%d %s", &i, buf);
                need = atof(buf);
                if (logfp != NULL)
                {
                    fprintf(logfp, "need: %d %f\n", i, need);
                    fflush(logfp);
                }
                if (mona == NULL) break;
                mona->setNeed(i, need);
                break;
            case SET_GOALS:
                if (logfp != NULL)
                {
                    fprintf(logfp, "goals: ");
                }
                if (mona == NULL) break;
                for (i = 0; i < numSensors; i++)
                {
                    scanf("%d", &sensors[i]);
                    if (logfp != NULL)
                    {
                        fprintf(logfp, "%d ", sensors[i]);
                    }
                }
                for (i = 0; i < numNeeds; i++)
                {
                    scanf("%s", buf);
                    goals->set(i, atof(buf));
                    if (logfp != NULL)
                    {
                        fprintf(logfp, "%s ", buf);
                    }
                }
                if (logfp != NULL)
                {
                    fprintf(logfp, "\n");
                    fflush(logfp);
                }
                for (neuronItr = mona->receptors.begin();
                    neuronItr != mona->receptors.end(); neuronItr++)
                {
                    if (((Mona::Receptor *)*neuronItr)->isDuplicate(sensors))
                    {
                        break;
                    }
                }
                if (neuronItr == mona->receptors.end())
                {
                    sprintf(description, "Goal");
                    receptor = mona->newReceptor(sensors, description);
                }
                receptor->goals.setGoals(*goals, 1.0);
                break;
            case CYCLE:
                if (logfp != NULL)
                {
                    fprintf(logfp, "cycle: ");
                }
                if (mona == NULL) break;
                for (i = 0; i < numSensors; i++)
                {
                    scanf("%d", &sensors[i]);
                    if (logfp != NULL)
                    {
                        fprintf(logfp, "%d ", sensors[i]);
                    }
                }
                if (logfp != NULL)
                {
                    fprintf(logfp, "\n");
                    fflush(logfp);
                }
                response = mona->cycle(sensors);
                printf("%d\n", response);
                fflush(stdout);
                if (logfp != NULL)
                {
                    fprintf(logfp, "response: %d\n", response);
                    fflush(logfp);
                }
                break;
            case OVERRIDE_RESPONSE:
                scanf("%d", &response);
                if (logfp != NULL)
                {
                    fprintf(logfp, "response: %d\n", response);
                    fflush(logfp);
                }
                if (mona == NULL) break;
                mona->responseOverride = response;
                break;
            case ERASE:
                scanf("%s", buf);
                if (logfp != NULL)
                {
                    fprintf(logfp, "erase: %s\n", buf);
                    fflush(logfp);
                }
                if (mona == NULL) break;
                mona->clearWorkingMemory();
                if (strcmp(buf, "ltm") != 0) break;
                for (neuronItr = mona->mediators.begin();
                    neuronItr != mona->mediators.end();
                    neuronItr = mona->mediators.begin())
                {
                    mona->deleteNeuron(*neuronItr);
                }
                break;
            case LOG:
                scanf("%s", buf);
                if (strcmp(buf, "on") == 0)
                {
                    if (logfp == NULL)
                    {
                    #ifdef UNIX
                        sprintf(buf, "/tmp/mona%d.log", getpid());
                    #else
                        sprintf(buf, "mona%d.log", _getpid());
                    #endif
                        if ((logfp = fopen(buf, "a")) == NULL)
                        {
                            fprintf(stderr, "Cannot open log file\n");
                            fflush(stderr);
                        }
                        else
                        {
                            fprintf(logfp, "log on\n");
                            fflush(logfp);
                        }
                    }
                }
                else
                {
                    if (logfp != NULL)
                    {
                        fprintf(logfp, "log off\n");
                        fclose(logfp);
                    }
                }
                break;
            case HELP:
                printf("Commands:\n");
                printf("[p]arameters: <number of sensors> <maximum response> <number of needs>\n");
                printf("[s]ettimers: <number of timers>\n");
                printf("[t]imer: <mediator level> <timer number> <timer value>\n");
                printf("[n]eed: <need number> <need value>\n");
                printf("[g]oals: <receptor sensor values> <goal values>\n");
                printf("[c]ycle: <sensor values> (triggers response output)\n");
                printf("[r]esponse: <response override> (-1 removes override)\n");
                printf("[e]rase: stm (short term memory) | ltm (long term memory)\n");
                printf("[h]elp\n");
                printf("[q]uit\n");
                fflush(stdout);
                if (logfp != NULL)
                {
                    fprintf(logfp, "help\n");
                    fprintf(logfp, "Commands:\n");
                    fprintf(logfp, "[p]arameters: <number of sensors> <maximum response> <number of needs>\n");
                    fprintf(logfp, "[s]ettimers: <number of timers>\n");
                    fprintf(logfp, "[t]imer: <mediator level> <timer number> <timer value>\n");
                    fprintf(logfp, "[n]eed: <need number> <need value>\n");
                    fprintf(logfp, "[g]oals: <receptor sensor values> <goal values>\n");
                    fprintf(logfp, "[c]ycle: <sensor values> (triggers response output)\n");
                    fprintf(logfp, "[r]esponse: <response override> (-1 removes override)\n");
                    fprintf(logfp, "[e]rase: stm (short term memory) | ltm (long term memory)\n");
                    fprintf(logfp, "[h]elp\n");
                    fprintf(logfp, "[q]uit\n");
                    fflush(logfp);
                }
                break;
            case QUIT:
                if (logfp != NULL)
                {
                    fprintf(logfp, "quit\n");
                    fclose(logfp);
                }
                done = true;
                break;
            default:
                printf("Unknown command\n");
                fflush(stdout);
                if (logfp != NULL)
                {
                    fprintf(logfp, "Unknown command\n");
                    fflush(logfp);
                }
                break;
        }
    }

    if (mona != NULL) delete mona;
    if (sensors != NULL) delete sensors;
    if (goals != NULL) delete goals;
    return 0;
}
