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
Instinct demo server-side starter.

Usage: instinct_demo_starter [<input timeout (secs)>]

Interaction is through standard input/output.

The first string read from the client is the IP address
of the client. This is used to set the DISPLAY environment
variable for X Windows display.

Each "new" command causes a instinct process to be spawned
with a different set of instincts.

To quit:
quit

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

// Commands:
#define ADDRESS 0
#define LOG 1
#define NEW 2
#define QUIT 3
#define UNKNOWN 4

// Processes started.
#define MAX_NEW_PROC 10
int procs[MAX_NEW_PROC];
int procCount = 0;

// Input timeout.
int timeout = -1;

// Log file.
FILE *logfp = NULL;

// Get command
int getCommand()
{
    char command[100];

    if (timeout > 0) alarm(timeout);
    scanf("%s", command);
    if (timeout > 0) alarm(0);
    switch(tolower(command[0]))
    {
        case 'a': return ADDRESS;
        case 'l': return LOG;
        case 'n': return NEW;
        case 'q': return QUIT;
        default: return UNKNOWN;
    }
}


// Wait for children to terminate.
void waitall()
{
    for (int i = 0; i < procCount; i++)
    {
        kill(procs[i], SIGTERM);
        if (wait(NULL) == -1)
        {
            if (logfp != NULL)
            {
                fprintf(logfp, "Wait failed, errno=%d\n", errno);
                fflush(logfp);
            }
            else
            {
                fprintf(stderr, "Wait failed, errno=%d\n", errno);
            }
            exit(1);
        }
    }
    procCount = 0;
}


// Time-out handler.
void alarmcall(int s)
{
    waitall();
    exit(1);
}


int main(int argc, char *argv[])
{
    char address[100],buf[100],*home,path[200],file[200];
    int pid,instinctNum;

    // Get optional timeout.
    if (argc == 2)
    {
        timeout = atoi(argv[1]);
    }
    if (timeout < -1 || timeout == 0 || argc > 2)
    {
        fprintf(stderr, "Usage: %s [<input timeout (secs)>]\n", argv[0]);
        return 1;
    }

    // Set time-out catcher.
    if (timeout > 0)
    {
        signal(SIGALRM, alarmcall);
    }

    // Get home directory.
    if ((home = getenv("MONA_HOME")) == NULL)
    {
        if (logfp != NULL)
        {
            fprintf(logfp, "Cannot get MONA_HOME environment value\n");
            fflush(logfp);
        }
        else
        {
            fprintf(stderr, "Cannot get MONA_HOME environment value\n");
        }
        return 1;
    }

    // Command loop.
    instinctNum = procCount = 0;
    bool done = false;
    while (!done)
    {
        switch(getCommand())
        {
            // Set DISPLAY to talk to xweird (:2).
            case ADDRESS:
                scanf("%s", address);
                if (logfp != NULL)
                {
                    fprintf(logfp, "address: %s\n", address);
                    fflush(logfp);
                }
                sprintf(buf, "DISPLAY=%s:2", address);
                if (putenv(buf) != 0)
                {
                    if (logfp != NULL)
                    {
                        fprintf(logfp, "Cannot set address %s to environment\n", address);
                        fflush(logfp);
                    }
                    else
                    {
                        fprintf(stderr, "Cannot set address %s to environment\n", address);
                    }
                }
                break;
            case NEW:
                // Create new instinct demo process.
                if (procCount >= MAX_NEW_PROC) break;
                switch(pid = fork())
                {
                    case 0:
                        sprintf(path, "%s/bin/instinct.sh", home);
                        sprintf(file, "%s/data/monkey%d.save", home, instinctNum);
                        instinctNum++;
                        if (instinctNum > 4) instinctNum = 0;
                        execl(path, "instinct.sh", "-steps", "200", "-scramble",
                            "-input", file, "-display", "graphics", NULL);
                        if (logfp != NULL)
                        {
                            fprintf(logfp, "Exec failed, errno=%d\n", errno);
                            fflush(logfp);
                        }
                        else
                        {
                            fprintf(stderr, "Exec failed, errno=%d\n", errno);
                        }
                        _exit(1);
                    case -1:
                        if (logfp != NULL)
                        {
                            fprintf(logfp, "Cannot create process, errno=%d\n", errno);
                            fflush(logfp);
                        }
                        else
                        {
                            fprintf(stderr, "Cannot create process, errno=%d\n", errno);
                        }
                        break;
                    default:
                        procs[procCount] = pid;
                        procCount++;
                        break;
                }
                break;
            case LOG:
                scanf("%s", buf);
                if (strcmp(buf, "on") == 0)
                {
                    if (logfp == NULL)
                    {
                        sprintf(buf, "/tmp/instinct%d.log", getpid());
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

    // Wait for children to terminate.
    waitall();

    return 0;
}
