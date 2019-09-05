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
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 */

#pragma comment(lib, "legacy_stdio_definitions.lib")

#include "monkey_and_bananas.hpp"

#ifdef ALLEGRO_GRAPHICS
// Allegro graphics and Glyph Keeper fonts.
#include <allegro.h>
#define GLYPH_TARGET GLYPH_TARGET_ALLEGRO
#include <glyph.h>

// Step timing (ms).
#define MAX_STEP_DELAY 5000
TIME StepDelay;

// Current step.
int Step;

// Take a step.
bool TakeStep;

// Font.
FONT *DisplayFont;
GLYPH_REND *Renderer;

// Images.
BITMAP *MonkeyBmp;
BITMAP *BananasBmp;

// Sound.
SAMPLE *MonkeySound;
#endif

// Version.
void
MonkeyAndBananas::version()
{
    const char *version = MONKEY_AND_BANANAS_VERSION;
    printf("%s\n", &version[5]);
}


// Default monkey.
const int MonkeyAndBananas::DEFAULT_MONKEY_LOCATION = 6;
const MonkeyAndBananas::DIRECTION MonkeyAndBananas::DEFAULT_MONKEY_DIRECTION = LOOK_RIGHT;

// Default world.
const char *MonkeyAndBananas::DEFAULT_WORLD[] =
#ifdef ONE_BOX_WORLD
{
    "b                  ",
    "---|               ",
    "   |        x      ",
    "   ----------------",
    NULL
};
#else
{
    "b                  ",
        "---|               ",
        "   |               ",
        "   |     x   xx    ",
        "   ----------------",
        NULL
};
#endif

// Default display type.
const MonkeyAndBananas::DISPLAY_TYPE MonkeyAndBananas::DEFAULT_DISPLAY_TYPE = NO_DISPLAY;

// Display size.
const int MonkeyAndBananas::DISPLAY_WIDTH = 800;
const int MonkeyAndBananas::DISPLAY_HEIGHT = 300;

// TrueType font.
const char *MonkeyAndBananas::DISPLAY_FONT = "courier-bold.ttf";

// Images.
const char *MonkeyAndBananas::MONKEY_IMAGE = "monkey.bmp";
const char *MonkeyAndBananas::BANANAS_IMAGE = "bananas.bmp";

// Sound.
const char *MonkeyAndBananas::MONKEY_SOUND = "monkey.wav";

// Get descriptions of view.
char *getViewDesc(int view)
{
    switch(view)
    {
        case (int)MonkeyAndBananas::BANANAS: return "Bananas";
        case (int)MonkeyAndBananas::BOX: return "Box";
        case (int)MonkeyAndBananas::FLOOR: return "Floor";
        case (int)MonkeyAndBananas::WALL: return "Wall";
        case (int)MonkeyAndBananas::AIR: return "Air";
        default: return "Dont care";
    }
}


// Get direction description.
char *getDirDesc(int dir)
{
    switch(dir)
    {
        case (int)MonkeyAndBananas::LOOK_LEFT:
            return "Look left";
        case (int)MonkeyAndBananas::LOOK_RIGHT:
            return "Look right";
        default:
            return "Dont care";
    }
}


// Get holding box description.
char *getHoldingDesc(bool holdingBox)
{
    if (holdingBox)
    {
        return "Hold";
    }
    else
    {
        return "No hold";
    }
}


char *getHoldingDesc(int holdingBox)
{
    if (holdingBox == DONT_CARE)
    {
        return "Dont care";
    }
    else
    {
        return getHoldingDesc(holdingBox == 1);
    }
}


// Get description of stimulus.
char *getStimulusDesc(int index, int stimulus)
{
    switch(index)
    {
        case 0: return getViewDesc(stimulus);
        case 1: return getDirDesc(stimulus);
        case 2: return getHoldingDesc(stimulus);
    }
    return NULL;
}


// Get response description.
char *getResponseDesc(int response)
{
    switch(response)
    {
        case (int)MonkeyAndBananas::GO_LEFT: return "Go left";
        case (int)MonkeyAndBananas::GO_RIGHT: return "Go right";
        case (int)MonkeyAndBananas::CLIMB: return "Climb";
        case (int)MonkeyAndBananas::PICKUP: return "Pickup";
        case (int)MonkeyAndBananas::STACK: return "Stack";
        case (int)MonkeyAndBananas::EAT: return "Eat";
        default: return "NA";
    }
}


// Constructors.
MonkeyAndBananas::MonkeyAndBananas(MonkeyInstincts *instincts, Mona *brain)
{
    init((char **)DEFAULT_WORLD, DEFAULT_MONKEY_LOCATION,
        DEFAULT_MONKEY_DIRECTION, DEFAULT_DISPLAY_TYPE,
        instincts, brain);
}


MonkeyAndBananas::MonkeyAndBananas(char **world, int location,
DIRECTION direction, DISPLAY_TYPE displayType,
MonkeyInstincts *instincts, Mona *brain)
{
    init(world, location, direction, displayType, instincts, brain);
}


void MonkeyAndBananas::init(char **world, int location,
DIRECTION direction, DISPLAY_TYPE displayType,
MonkeyInstincts *instincts, Mona *brainIn)
{
    int i;
    Mona::Receptor *receptor;
    char buf[100];

    // Save world.
    assert(world[0] != NULL);
    for (i = 0; world[i] != NULL; i++) {}
    worldY = i;
    worldX = strlen(world[0]);
    this->world = new char*[i + 1];
    assert(this->world != NULL);
    world2 = new char*[i + 1];
    assert(world2 != NULL);
    for (i = 0; world[i] != NULL; i++)
    {
        this->world[i] = new char[strlen(world[i]) + 1];
        assert(this->world[i] != NULL);
        strcpy(this->world[i], world[i]);
        assert(i == 0 || strlen(this->world[i - 1]) == strlen(this->world[i]));
        world2[i] = new char[strlen(world[i]) + 1];
        assert(world2[i] != NULL);
        strcpy(world2[i], world[i]);
    }
    this->world[i] = NULL;
    world2[i] = NULL;

    // Position the monkey.
    this->location = location;
    for (i = 0; this->world[i] != NULL &&
        this->world[i][this->location] == AIR; i++) {}
        assert(i > 0);
    this->world[i - 1][this->location] = MONKEY;
    this->direction = direction;

    // Initialize the monkey brain.
    numSensors = 3;
    maxResponse = EAT;
    if (brainIn != NULL)
    {
        brain = brainIn;
    }
    else
    {
        brain = new Mona(numSensors, maxResponse,
            MonkeyInstincts::NUM_INSTINCT_NEEDS + 1,
            instincts->random.RAND());
        assert(brain != NULL);

        // Assign effect delay timers.
        for (i = 0; i <= Mona::MAX_MEDIATOR_LEVEL; i++)
        {
            brain->eventTimers[i].resize(1);
            if (i == 0)
            {
                brain->eventTimers[i][0] = 1;
            }
            else
            {
                brain->eventTimers[i][0] = strlen(world[0]) * 2;
            }
        }

        // Create receptors.
        // Sensors[0]=scene,[1]=direction,[2]=holding box.
        sensors[0] = FLOOR;
        sensors[1] = DONT_CARE;
        sensors[2] = DONT_CARE;
        sprintf(buf, "%s,%s,%s",
            getViewDesc(sensors[0]), getDirDesc(sensors[1]),
            getHoldingDesc(sensors[2]));
        receptor = brain->newReceptor(sensors, buf);
        sensors[0] = WALL;
        sensors[1] = DONT_CARE;
        sensors[2] = DONT_CARE;
        sprintf(buf, "%s,%s,%s",
            getViewDesc(sensors[0]), getDirDesc(sensors[1]),
            getHoldingDesc(sensors[2]));
        receptor = brain->newReceptor(sensors, buf);
        sensors[0] = BOX;
        sensors[1] = DONT_CARE;
        sensors[2] = DONT_CARE;
        sprintf(buf, "%s,%s,%s",
            getViewDesc(sensors[0]), getDirDesc(sensors[1]),
            getHoldingDesc(sensors[2]));
        receptor = brain->newReceptor(sensors, buf);
        sensors[0] = AIR;
        sensors[1] = DONT_CARE;
        sensors[2] = DONT_CARE;
        sprintf(buf, "%s,%s,%s",
            getViewDesc(sensors[0]), getDirDesc(sensors[1]),
            getHoldingDesc(sensors[2]));
        receptor = brain->newReceptor(sensors, buf);
        sensors[0] = DONT_CARE;
        sensors[1] = LOOK_LEFT;
        sensors[2] = DONT_CARE;
        sprintf(buf, "%s,%s,%s",
            getViewDesc(sensors[0]), getDirDesc(sensors[1]),
            getHoldingDesc(sensors[2]));
        receptor = brain->newReceptor(sensors, buf);
        sensors[0] = DONT_CARE;
        sensors[1] = LOOK_RIGHT;
        sensors[2] = DONT_CARE;
        sprintf(buf, "%s,%s,%s",
            getViewDesc(sensors[0]), getDirDesc(sensors[1]),
            getHoldingDesc(sensors[2]));
        receptor = brain->newReceptor(sensors, buf);
        sensors[0] = DONT_CARE;
        sensors[1] = DONT_CARE;
        sensors[2] = 1;
        sprintf(buf, "%s,%s,%s",
            getViewDesc(sensors[0]), getDirDesc(sensors[1]),
            getHoldingDesc(sensors[2]));
        receptor = brain->newReceptor(sensors, buf);
        sensors[0] = DONT_CARE;
        sensors[1] = DONT_CARE;
        sensors[2] = 0;
        sprintf(buf, "%s,%s,%s",
            getViewDesc(sensors[0]), getDirDesc(sensors[1]),
            getHoldingDesc(sensors[2]));
        receptor = brain->newReceptor(sensors, buf);
        sensors[0] = BANANAS;
        sensors[1] = DONT_CARE;
        sensors[2] = DONT_CARE;
        sprintf(buf, "%s,%s,%s",
            getViewDesc(sensors[0]), getDirDesc(sensors[1]),
            getHoldingDesc(sensors[2]));
        receptor = brain->newReceptor(sensors, buf);

        // Add motor neurons.
        brain->newMotor(GO_LEFT, getResponseDesc(GO_LEFT));
        brain->newMotor(GO_RIGHT, getResponseDesc(GO_RIGHT));
        brain->newMotor(CLIMB, getResponseDesc(CLIMB));
        brain->newMotor(PICKUP, getResponseDesc(PICKUP));
        brain->newMotor(STACK, getResponseDesc(STACK));
        brain->newMotor(EAT, getResponseDesc(EAT));
    }

    // Initialize instincts.
    this->instincts = instincts;
    if (brainIn == NULL)
    {
        this->instincts->init(brain);
    }
    else
    {
        this->instincts->reset();
    }

    // Setup for run.
    steps = 0;
    response = -1;
    foundBananas = false;

    // Display.
    this->displayType = displayType;
}


// Destructor.
MonkeyAndBananas::~MonkeyAndBananas()
{
    for (int i = 0; world[i] != NULL; i++)
    {
        delete world[i];
        delete world2[i];
    }
    delete world;
    delete world2;
    if (brain != NULL) delete brain;
}


// Run.
// Return number of steps to find bananas.
int MonkeyAndBananas::run(int steps)
{
    // Save steps.
    this->steps = steps;

    // For graphics display, gui controls running.
    if (displayType == GRAPHICS_DISPLAY)
    {
        #ifdef ALLEGRO_GRAPHICS
        if (graphicsDisplay() && foundBananas)
        {
            return Step;
        }
        else
        {
            return -1;
        }
        #else
        printf("Graphics unavailable!\n");
        printf("Graphics requires Allegro (http://alleg.sourceforge.net)\n");
        printf("and Glyph Keeper (http://kd.lab.nig.ac.jp/glyph-keeper/)\n");
        printf("Then compile with ALLEGRO_GRAPHICS defined\n");
        #endif
    }
    else
    {

        // Run steps.
        response = -1;
        for (int i = 0; i < steps; i++)
        {
            step();
            if (displayType == TEXT_DISPLAY) textDisplay(i);
            if (foundBananas) return i;
        }
    }
    return -1;
}


// Step.
// Set foundBananas true if found bananas.
void MonkeyAndBananas::step()
{
    int i,x,y,h;

    // Default to failure.
    foundBananas = false;

    // What does the monkey see?
    for (h = 0; world[h] != NULL; h++)
    {
        if (world[h][location] == MONKEY) break;
    }
    if ((direction == LOOK_LEFT && location == 0) ||
        (direction == LOOK_RIGHT && world[h][location + 1] == '\0'))
    {
        sensors[0] = AIR;
    }
    else
    {
        if (direction == LOOK_LEFT) i = -1; else i = 1;
        if (world[h][location + i] != AIR)
        {
            sensors[0] = world[h][location + i];
        } else if (world[h + 1] != NULL &&
            world[h + 1][location + i] != AIR)
        {
            sensors[0] = FLOOR;
        }
        else
        {
            sensors[0] = AIR;
        }
    }

    // Direction.
    sensors[1] = direction;

    // Holding box?
    if (h > 0 && world[h - 1][location] == BOX)
    {
        sensors[2] = 1;
    }
    else
    {
        sensors[2] = 0;
    }

    // Preclude ineffective responses.
    for (response = GO_LEFT; response <= EAT; response++)
    {
        if (tryResponse())
        {
            brain->responseInhibitors[response] = false;
        }
        else
        {
            brain->responseInhibitors[response] = true;
        }
    }

    // Get monkey response to environment.
    response = brain->cycle(sensors);

    #ifdef NEVER
    // Print response potentials.
    printf("Response potentials:\n");
    for (i = 0; i <= EAT; i++)
    {
        switch(i)
        {
            case GO_LEFT: printf("GO_LEFT=%f\n",brain->responsePotentials[i]); break;
            case GO_RIGHT: printf("GO_RIGHT=%f\n",brain->responsePotentials[i]); break;
            case CLIMB: printf("CLIMB=%f\n",brain->responsePotentials[i]); break;
            case PICKUP: printf("PICKUP=%f\n",brain->responsePotentials[i]); break;
            case STACK: printf("STACK=%f\n",brain->responsePotentials[i]); break;
            case EAT: printf("EAT=%f\n",brain->responsePotentials[i]); break;
        }
    }

    // Print needs.
    printf("Needs:\n");
    for (i = 0; i <= instincts->numInstinctNeeds; i++)
    {
        printf("Need[%d]=%f\n", i, brain->getNeed(i));
    }
    #endif

    // Perform instinctive actions.
    instincts->step();

    // Found the bananas?
    if (sensors[0] == BANANAS)
    {
        foundBananas = true;
    }

    // Modify environment based on response.
    if (tryResponse())
    {
        for (y = 0; y < worldY; y++)
        {
            for (x = 0; x < worldX; x++)
            {
                world[y][x] = world2[y][x];
            }
        }
        location = location2;
        direction = direction2;
    }
}


// Try a response using working variables.
// Return true if response is effective.
bool MonkeyAndBananas::tryResponse()
{
    int x,y,h;

    // Copy state.
    for (y = 0; y < worldY; y++)
    {
        for (x = 0; x < worldX; x++)
        {
            world2[y][x] = world[y][x];
        }
    }
    location2 = location;
    direction2 = direction;

    // Falling?
    for (h = 0; world2[h] != NULL; h++)
    {
        if (world2[h][location2] == MONKEY) break;
    }
    if (world2[h + 1] != NULL && world2[h + 1][location2] == AIR)
    {
        // Fall.
        if (sensors[2] == 1)
        {
            world2[h][location2] = world2[h - 1][location2];
            world2[h - 1][location2] = AIR;
        }
        else
        {
            world2[h][location2] = AIR;
        }
        world2[h + 1][location2] = MONKEY;
        if (response == GO_LEFT) direction2 = LOOK_LEFT;
        if (response == GO_RIGHT) direction2 = LOOK_RIGHT;
    }
    else
    {
        switch(response)
        {
            case GO_LEFT:
                direction2 = LOOK_LEFT;
                if (location2 > 0 &&
                    (world2[h][location2 - 1] == FLOOR || world2[h][location2 - 1] == AIR))
                {
                    // Move left.
                    if (sensors[2] == 1)
                    {
                        world2[h - 1][location2 - 1] = world2[h - 1][location2];
                        world2[h - 1][location2] = AIR;
                    }
                    world2[h][location2] = AIR;
                    world2[h][location2 - 1] = MONKEY;
                    location2--;
                }
                break;
            case GO_RIGHT:
                direction2 = LOOK_RIGHT;
                if (world2[h][location2 + 1] != '\0' &&
                    (world2[h][location2 + 1] == FLOOR || world2[h][location2 + 1] == AIR))
                {
                    // Move right.
                    if (sensors[2] == 1)
                    {
                        world2[h - 1][location2 + 1] = world2[h - 1][location2];
                        world2[h - 1][location2] = AIR;
                    }
                    world2[h][location2] = AIR;
                    world2[h][location2 + 1] = MONKEY;
                    location2++;
                }
                break;
            case CLIMB:
                if (direction2 == LOOK_LEFT)
                {
                    if (sensors[0] == WALL || sensors[0] == BOX)
                    {
                        if (h > 0 && location2 > 0 && world2[h - 1][location2 - 1] == AIR)
                        {
                            if (sensors[2] == 1)
                            {
                                if (h > 1)
                                {
                                    world2[h - 2][location2 - 1] = world2[h - 1][location2];
                                    world2[h - 1][location2] = AIR;
                                    world2[h][location2] = AIR;
                                    world2[h - 1][location2 - 1] = MONKEY;
                                    location2--;
                                }
                            }
                            else
                            {
                                world2[h][location2] = AIR;
                                world2[h - 1][location2 - 1] = MONKEY;
                                location2--;
                            }
                        }
                    }
                }
                else
                {
                    if (sensors[0] == WALL || sensors[0] == BOX)
                    {
                        if (h > 0 && world2[h][location2 + 1] != '\0' &&
                            world2[h - 1][location2 + 1] == AIR)
                        {
                            if (sensors[2] == 1)
                            {
                                if (h > 1)
                                {
                                    world2[h - 2][location2 + 1] = world2[h - 1][location2];
                                    world2[h - 1][location2] = AIR;
                                    world2[h][location2] = AIR;
                                    world2[h - 1][location2 + 1] = MONKEY;
                                    location2++;
                                }
                            }
                            else
                            {
                                world2[h][location2] = AIR;
                                world2[h - 1][location2 + 1] = MONKEY;
                                location2++;
                            }
                        }
                    }
                }
                break;
            case PICKUP:
                if (direction2 == LOOK_LEFT)
                {
                    if (sensors[0] == BOX && h > 0 && location2 > 0 && sensors[2] == 0)
                    {
                        if (world2[h - 1][location2 - 1] == AIR)
                        {
                            world2[h - 1][location2] = world2[h][location2 - 1];
                            world2[h][location2 - 1] = AIR;
                        } else if (world2[h - 1][location2 - 1] == BOX)
                        {
                            if (h == 1 || (h > 1 && world2[h - 2][location2 - 1] == AIR))
                            {
                                world2[h - 1][location2] = world2[h - 1][location2 - 1];
                                world2[h - 1][location2 - 1] = AIR;
                            }
                        }
                    }
                }
                else
                {
                    if (sensors[0] == BOX && h > 0 &&
                        world2[h][location2 + 1] != '\0' && sensors[2] == 0)
                    {
                        if (world2[h - 1][location2 + 1] == AIR)
                        {
                            world2[h - 1][location2] = world2[h][location2 + 1];
                            world2[h][location2 + 1] = AIR;
                        } else if (world2[h - 1][location2 + 1] == BOX)
                        {
                            if (h == 1 || (h > 1 && world2[h - 2][location2 + 1] == AIR))
                            {
                                world2[h - 1][location2] = world2[h - 1][location2 + 1];
                                world2[h - 1][location2 + 1] = AIR;
                            }
                        }
                    }
                }
                break;
            case STACK:
                if (direction2 == LOOK_LEFT)
                {
                    if (sensors[0] == FLOOR && location2 > 0 && sensors[2] == 1)
                    {
                        if (world2[h + 1] != NULL && world2[h + 1][location2 - 1] != AIR)
                        {
                            world2[h][location2 - 1] = world2[h - 1][location2];
                            world2[h - 1][location2] = AIR;
                        }
                    } else if ((sensors[0] == WALL || sensors[0] == BOX) &&
                        location2 > 0 && sensors[2] == 1 && h > 0 &&
                        world2[h - 1][location2 - 1] == AIR)
                    {
                        world2[h - 1][location2 - 1] = world2[h - 1][location2];
                        world2[h - 1][location2] = AIR;
                    } else if (world2[h][location2 + 1] == AIR && sensors[2] == 1)
                    {
                        if (location2 == 0 || world2[h][location2 - 1] != AIR)
                        {
                            world2[h][location2] = world2[h - 1][location2];
                            world2[h - 1][location2] = AIR;
                            world2[h][location2 + 1] = MONKEY;
                            location2++;
                        }
                    }
                }
                else
                {
                    if (sensors[0] == FLOOR && world2[h][location2 + 1] != '\0' &&
                        sensors[2] == 1)
                    {
                        if (world2[h + 1] != NULL && world2[h + 1][location2 + 1] != AIR)
                        {
                            world2[h][location2 + 1] = world2[h - 1][location2];
                            world2[h - 1][location2] = AIR;
                        }
                    } else if ((sensors[0] == WALL || sensors[0] == BOX) &&
                        world2[h][location2 + 1] != '\0' && sensors[2] == 1 && h > 0 &&
                        world2[h - 1][location2 + 1] == AIR)
                    {
                        world2[h - 1][location2 + 1] = world2[h - 1][location2];
                        world2[h - 1][location2] = AIR;
                    } else if (location2 > 0 && world2[h][location2 - 1] == AIR &&
                        sensors[2] == 1)
                    {
                        if (world2[h][location2 + 1] != AIR)
                        {
                            world2[h][location2] = world2[h - 1][location2];
                            world2[h - 1][location2] = AIR;
                            world2[h][location2 - 1] = MONKEY;
                            location2--;
                        }
                    }
                }
                break;
            case EAT:
                if (direction2 == LOOK_LEFT)
                {
                    if (sensors[0] == BANANAS)
                    {
                        world2[h][location2 - 1] = AIR;
                    }
                }
                else
                {
                    if (sensors[0] == BANANAS)
                    {
                        world2[h][location2 + 1] = AIR;
                    }
                }
                break;
        }
    }

    // Check state change.
    for (y = 0; y < worldY; y++)
    {
        for (x = 0; x < worldX; x++)
        {
            if (world2[y][x] != world[y][x]) return true;
        }
    }
    if (location2 != location) return true;
    if (direction2 != direction) return true;
    return false;
}


// Evaluate the world state.
double MonkeyAndBananas::evaluate(int steps)
{
    int x,y,mx;
    double value;
    bool foundBananas,stackOK;

    // Assume bananas are eaten.
    foundBananas = true;

    // Assume boxes are not properly stacked.
    stackOK = false;

    value = 0.0;
    for (y = 0; y < worldY; y++)
    {
        for (x = 0; x < worldX; x++)
        {
            switch(world[y][x])
            {
                case BANANAS:
                    foundBananas = false;
                    break;
                case BOX:
                    value += (double)(worldX - x);

                    // Stacked?
                    if (world[y][x - 1] == BOX &&
                        world[y - 1][x - 1] == BOX)
                    {
                        stackOK = true;
                    }
                    break;
                case MONKEY:
                    mx = x;
                    break;
            }
        }
    }

    // If boxes stacked, then reward monkey for going left.
    if (stackOK)
    {
        value += (double)(worldX - mx);
    }

    // If found bananas, also reward quicker time to find them.
    if (foundBananas)
    {
        value += MonkeyInstincts::MAX_NEED_VALUE;
        if (steps != -1) value += (1.0 / (double)(steps + 1));
    }

    return value;
}


// Select world display.
void MonkeyAndBananas::setDisplay(DISPLAY_TYPE type)
{
    displayType = type;
}


// Text world display.
void MonkeyAndBananas::textDisplay(int step)
{
    int i;

    for (i = 0; world[0][i] != '\0'; i++)
    {
        printf("=");
    }
    printf("\n");
    printf("Step: %d/%d ",step,steps);

    printf("Sensors=[");
    printf("%s", getViewDesc(sensors[0]));
    printf(",");
    printf("%s", getDirDesc(sensors[1]));
    printf(",");
    printf("%s", getHoldingDesc(sensors[2]));
    printf("] ");

    if (response != -1)
    {
        printf("Response=%s", getResponseDesc(response));
    }
    printf("\n");

    for (i = 0; world[i] != NULL; i++)
    {
        printf("%s\n", world[i]);
    }
}


#ifdef ALLEGRO_GRAPHICS
// Procedure for displaying and running the world.
int world_proc(int msg, DIALOG *dialog, int c)
{
    int x,y,cw,ch;
    BITMAP *bmp;
    TIME time;
    static TIME timeMark;
    int ret;
    char buf[100];
    MonkeyAndBananas *mb;
    int boxcolor;

    // Access the Monkey and Bananas object.
    mb = (MonkeyAndBananas *)dialog->dp;

    // Process the message.
    ret = D_O_K;
    switch(msg)
    {
        // Initialize.
        case MSG_START:
            Step = 0;
            timeMark = gettime();
            TakeStep = false;
            break;

            // Idle procedure: step world.
        case MSG_IDLE:

            // Time to step?
            time = gettime();
            if (((time - timeMark) >= StepDelay &&
                StepDelay < MAX_STEP_DELAY) || TakeStep)
            {
                timeMark = time;
                mb->step();
                Step++;
                TakeStep = false;
                if (mb->foundBananas)
                {
                    // Play the monkey sound.
                    play_sample(MonkeySound, 255, 128, 1000, FALSE);
                    rest(2000);
                    ret = D_CLOSE;
                }
                if (Step >= mb->steps)
                {
                    ret = D_CLOSE;
                }
                show_mouse(NULL);
                object_message(dialog, MSG_DRAW, 0);
                show_mouse(screen);
            }
            break;

            // Draw the world.
        case MSG_DRAW:

            // Draw onto a temporary memory bitmap to prevent flicker.
            bmp = create_bitmap(dialog->w, dialog->h);
            clear_to_color(bmp, dialog->bg);

            // Show step.
            textprintf_ex(bmp, DisplayFont, 0, 0, dialog->fg, dialog->bg, "Step: %d/%d", Step, mb->steps);

            // Draw the sensors.
            if (Step > 0)
            {
                strcpy(buf, "Sensors=[");
                strcat(buf, getViewDesc(mb->sensors[0]));
                strcat(buf, ",");
                strcat(buf, getDirDesc(mb->sensors[1]));
                strcat(buf, ",");
                strcat(buf, getHoldingDesc(mb->sensors[2]));
                strcat(buf,"]");
                textprintf_ex(bmp, DisplayFont, dialog->w / 4, 0, dialog->fg, dialog->bg, buf);
            }

            // Draw the response.
            if (mb->response != -1)
            {
                strcpy(buf, "Response=");
                strcat(buf, getResponseDesc(mb->response));
                textprintf_ex(bmp, DisplayFont, (3 * dialog->w) / 4, 0, dialog->fg, dialog->bg, buf);
            }

            // Draw the world.
            cw = dialog->w / mb->worldX;
            ch = dialog->h / (mb->worldY + 1);
            boxcolor = makecol(210, 105, 30);
            for (y = 0; y < mb->worldY; y++)
            {
                for (x = 0; x < mb->worldX; x++)
                {
                    switch(mb->world[y][x])
                    {
                        case MonkeyAndBananas::MONKEY:
                            blit(MonkeyBmp, bmp, 0, 0, (x * cw) + 1, ((y + 1) * ch) + 1, cw - 2, ch - 2);
                            break;
                        case MonkeyAndBananas::BANANAS:
                            blit(BananasBmp, bmp, 0, 0, (x * cw) + 1, ((y + 1) * ch) + 1, cw - 2, ch - 2);
                            break;
                        case MonkeyAndBananas::BOX:
                            rectfill(bmp, (x * cw) + 1, (y + 1) * ch, (x + 1) * cw, ((y + 2) * ch) - 1, boxcolor);
                            rect(bmp, (x * cw) + 1, (y + 1) * ch, (x + 1) * cw, ((y + 2) * ch) - 1, dialog->fg);
                            break;
                        case MonkeyAndBananas::FLOOR:
                            rect(bmp, (x * cw) + 1, (y + 1) * ch, (x + 1) * cw, ((y + 2) * ch) - 1, dialog->fg);
                            break;
                        case MonkeyAndBananas::WALL:
                            rect(bmp, (x * cw) + 1, (y + 1) * ch, (x + 1) * cw, ((y + 2) * ch) - 1, dialog->fg);
                            break;
                        case MonkeyAndBananas::AIR: break;
                    }
                }
            }

            // Copy the bitmap onto the screen.
            blit(bmp, screen, 0, 0, dialog->x, dialog->y, dialog->w, dialog->h);
            destroy_bitmap(bmp);
            break;
    }

    return ret;
}


// Speed selection slider.
int speed_slider_proc(int msg, DIALOG *d, int c)
{
    switch (msg)
    {
        case MSG_START:
            // Initialize the slider position.
            StepDelay = d->d2 = d->d1;
            break;

        case MSG_IDLE:
            StepDelay = d->d2;
            break;
    }

    return d_slider_proc(msg, d, c);
}


// Step button.
int step_button_proc(int msg, DIALOG *d, int c)
{
    if (msg == MSG_CLICK)
    {
        TakeStep = true;
        while (true)
        {
            if (mouse_needs_poll()) poll_mouse();
            if (!(mouse_b & 1)) break;
        }
    }

    // Call the parent object.
    return d_ctext_proc(msg, d, c);
}


// Quit button.
int quit_button_proc(int msg, DIALOG *d, int c)
{
    if (msg == MSG_CLICK) exit(0);

    // Call the parent object.
    return d_button_proc(msg, d, c);
}


// The GUI components.
DIALOG gui_dialog[] =
{
    // proc,x,y,w,h,fg,bg,key,flags,d1,d2,dp,dp2,dp3
    {d_clear_proc,0,0,0,0,255,0,0,0,0,0,NULL,NULL,NULL},
    {world_proc,0,0,MonkeyAndBananas::DISPLAY_WIDTH,MonkeyAndBananas::DISPLAY_HEIGHT-40,255,0,0,0,0,0,NULL,NULL,NULL},
    {d_text_proc,5,MonkeyAndBananas::DISPLAY_HEIGHT-30,50,40,255,0,0,0,1000,0,(void *)"Fast",NULL,NULL},
    {speed_slider_proc,55,MonkeyAndBananas::DISPLAY_HEIGHT-30,200,20,255,0,0,0,MAX_STEP_DELAY,0,NULL,NULL,NULL},
    {d_text_proc,260,MonkeyAndBananas::DISPLAY_HEIGHT-30,30,40,255,0,0,0,1000,0,(void *)"Stop",NULL,NULL},
    {d_box_proc,MonkeyAndBananas::DISPLAY_WIDTH/2,MonkeyAndBananas::DISPLAY_HEIGHT-35,80,30,255,0,0,0,0,0,NULL,NULL,NULL},
    {step_button_proc,MonkeyAndBananas::DISPLAY_WIDTH/2,MonkeyAndBananas::DISPLAY_HEIGHT-30,80,30,255,0,0,0,0,0,(void *)"Step",NULL,NULL},
    {quit_button_proc,(3*MonkeyAndBananas::DISPLAY_WIDTH)/4,MonkeyAndBananas::DISPLAY_HEIGHT-35,80,30,255,0,0,D_EXIT,0,0,(void *)"Quit",NULL,NULL},
    {d_yield_proc,0,0,0,0,0,0,0,0,0,0,NULL,NULL,NULL},
    {NULL,0,0,0,0,0,0,0,0,0,0,NULL,NULL,NULL}
};
#endif

// Graphics display.
// World is stepped in world_proc().
bool MonkeyAndBananas::graphicsDisplay()
{
    #ifdef ALLEGRO_GRAPHICS
    int cw,ch;
    GLYPH_FACE *face;
    BITMAP *bmp;
    char path[100],*homePath;

    if (allegro_init() != 0)
    {
        fprintf(stderr, "Unable to initialize graphics\n");
        if (getenv("DISPLAY") == NULL)
        {
            fprintf(stderr,"DISPLAY variable unset!\n");
        }
        return false;
    }
    install_keyboard();
    install_mouse();
    install_timer();
    if (install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL) != 0)
    {
        allegro_message("Error initialising sound system\n%s\n", allegro_error);
        return false;
    }
    if (set_gfx_mode(GFX_AUTODETECT, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, 0) != 0)
    {
        set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
        allegro_message("Unable to set graphics mode\n%s\n", allegro_error);
        allegro_exit();
        return false;
    }
    set_window_title("Monkey and Bananas");

    // Set up colors to match screen color depth (in case it changed).
    for (int i = 0; gui_dialog[i].proc; i++)
    {
        gui_dialog[i].fg = makecol(0, 0, 0);
        gui_dialog[i].bg = makecol(255, 255, 255);
    }

    // Get home environment.
    homePath = getenv("MONA_HOME");

    // Set the display font.
    if (homePath != NULL)
    {
        sprintf(path, "%s/resource/%s", homePath, DISPLAY_FONT);
        face = gk_load_face_from_file(path,0);
    }
    if (homePath == NULL || !face)
    {
        sprintf(path, "../resource/%s", DISPLAY_FONT);
        face = gk_load_face_from_file(path,0);
        if (!face)
        {
            sprintf(path, "../../resource/%s", DISPLAY_FONT);
            face = gk_load_face_from_file(path,0);
            if (!face)
            {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Cannot load the font");
                allegro_exit();
                return false;
            }
        }
    }
    Renderer = gk_create_renderer(face,0);
    if (!Renderer)
    {
        set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
        allegro_message("Cannot create renderer");
        allegro_exit();
        return false;
    }
    gk_rend_set_size_pixels(Renderer,18,18);
    DisplayFont = gk_create_allegro_font(Renderer);
    font = DisplayFont;

    // Load the images.
    if (homePath != NULL)
    {
        sprintf(path, "%s/resource/%s", homePath, MONKEY_IMAGE);
        MonkeyBmp = load_bmp(path, NULL);
    }
    if (homePath == NULL || !MonkeyBmp)
    {
        sprintf(path, "../resource/%s", MONKEY_IMAGE);
        MonkeyBmp = load_bmp(path, NULL);
        if (!MonkeyBmp)
        {
            sprintf(path, "../../resource/%s", MONKEY_IMAGE);
            MonkeyBmp = load_bmp(path, NULL);
            if (!MonkeyBmp)
            {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Cannot load monkey image %s\n", MONKEY_IMAGE);
                destroy_font(DisplayFont);
                allegro_exit();
                return false;
            }
        }
    }
    cw = (DISPLAY_WIDTH / worldX) - 2;
    ch = (DISPLAY_HEIGHT / (worldY + 2)) - 2;
    bmp = create_bitmap(cw, ch);
    stretch_sprite(bmp, MonkeyBmp, 0, 0, cw, ch);
    destroy_bitmap(MonkeyBmp);
    MonkeyBmp = bmp;
    if (homePath != NULL)
    {
        sprintf(path, "%s/resource/%s", homePath, BANANAS_IMAGE);
        BananasBmp = load_bmp(path, NULL);
    }
    if (homePath == NULL || !BananasBmp)
    {
        sprintf(path, "../resource/%s", BANANAS_IMAGE);
        BananasBmp = load_bmp(path, NULL);
        if (!BananasBmp)
        {
            sprintf(path, "../../resource/%s", BANANAS_IMAGE);
            BananasBmp = load_bmp(path, NULL);
            if (!BananasBmp)
            {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Cannot load bananas image %s\n", BANANAS_IMAGE);
                destroy_font(DisplayFont);
                destroy_bitmap(MonkeyBmp);
                allegro_exit();
                return false;
            }
        }
    }
    bmp = create_bitmap(cw, ch);
    stretch_sprite(bmp, BananasBmp, 0, 0, cw, ch);
    destroy_bitmap(BananasBmp);
    BananasBmp = bmp;

    // Load the sound.
    if (homePath != NULL)
    {
        sprintf(path, "%s/resource/%s", homePath, MONKEY_SOUND);
        MonkeySound = load_sample(path);
    }
    if (homePath == NULL || !MonkeySound)
    {
        sprintf(path, "../resource/%s", MONKEY_SOUND);
        MonkeySound = load_sample(path);
        if (!MonkeySound)
        {
            sprintf(path, "../../resource/%s", MONKEY_SOUND);
            MonkeySound = load_sample(path);
            if (!MonkeySound)
            {
                set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
                allegro_message("Error reading WAV file %s\n", MONKEY_SOUND);
                destroy_font(DisplayFont);
                destroy_bitmap(MonkeyBmp);
                destroy_bitmap(BananasBmp);
                allegro_exit();
                return false;
            }
        }
    }

    // Store this object in display entry.
    gui_dialog[1].dp = (void *)this;

    // Start the GUI.
    do_dialog(gui_dialog, -1);

    // Terminate.
    destroy_font(DisplayFont);
    destroy_bitmap(MonkeyBmp);
    destroy_bitmap(BananasBmp);
    destroy_sample(MonkeySound);
    allegro_exit();
    return true;
    #else
    return false;
    #endif
}


// Parameters.
const int MonkeyInstincts::DEFAULT_MAX_INSTINCTS = 20;
const int MonkeyInstincts::DEFAULT_MIN_INSTINCTS = 5;
int MonkeyInstincts::MAX_INSTINCTS = DEFAULT_MAX_INSTINCTS;
int MonkeyInstincts::MIN_INSTINCTS = DEFAULT_MIN_INSTINCTS;
const int MonkeyInstincts::MIN_INSTINCT_EVENTS = 2;
const int MonkeyInstincts::MAX_INSTINCT_EVENTS = 3;
const int MonkeyInstincts::NUM_INSTINCT_NEEDS = 10;
const Mona::NEED MonkeyInstincts::MAX_NEED_VALUE = 10.0;
const Mona::NEED MonkeyInstincts::MIN_NEED_VALUE = 1.0;
const int MonkeyInstincts::MAX_NEED_FREQ = 10;
const int MonkeyInstincts::MIN_NEED_FREQ = 0;
const int MonkeyInstincts::MAX_NEED_DURATION = 50;
const int MonkeyInstincts::MIN_NEED_DURATION = 0;

// Construct instincts.
MonkeyInstincts::MonkeyInstincts(RANDOM randomSeed)
{
    int i,j,k,p,q,numInstincts,numEvents;
    Instinct *instinct;
    Instinct::EVENT event;

    brain = NULL;
    random.SRAND(randomSeed);

    #ifndef HARD_CODED_INSTINCTS
    // Create instincts.
    numInstincts = random.RAND_CHOICE(MAX_INSTINCTS + 1);
    for (i = 0; i < numInstincts; i++)
    {
        instinct = new Instinct();
        assert(instinct != NULL);
        numEvents = random.RAND_CHOICE((MAX_INSTINCT_EVENTS -
            MIN_INSTINCT_EVENTS) + 1) + MIN_INSTINCT_EVENTS;
        for (j = 0; j < numEvents; j++)
        {
            while (true)
            {
                #ifdef NEVER
                switch(random.RAND_CHOICE(6))
                #else
                switch(random.RAND_CHOICE(5))
                #endif
                {
                    case 0: event.stimuli[0] = MonkeyAndBananas::BANANAS; break;
                    case 1: event.stimuli[0] = MonkeyAndBananas::BOX; break;
                    case 2: event.stimuli[0] = MonkeyAndBananas::FLOOR; break;
                    case 3: event.stimuli[0] = MonkeyAndBananas::WALL; break;
                    case 4: event.stimuli[0] = MonkeyAndBananas::AIR; break;
                    case 5: event.stimuli[0] = DONT_CARE; break;
                }
                #ifdef NEVER
                event.stimuli[1] = random.RAND_CHOICE(3);
                #else
                event.stimuli[1] = random.RAND_CHOICE(2);
                #endif
                if (event.stimuli[1] == 2) event.stimuli[1] = DONT_CARE;
                #ifdef NEVER
                event.stimuli[2] = random.RAND_CHOICE(3);
                #else
                event.stimuli[2] = random.RAND_CHOICE(2);
                #endif
                if (event.stimuli[2] == 2) event.stimuli[2] = DONT_CARE;

                // Disallow empty stimuli.
                for (k = 0; k < 3; k++) if (event.stimuli[k] != DONT_CARE) break;
                if (k < 3) break;
            }

            if (j < numEvents - 1)
            {
                event.response = random.RAND_CHOICE(6);
            }
            else
            {
                event.response = -1;
            }
            instinct->events.push_back(event);
        }

        if (NUM_INSTINCT_NEEDS > 0)
        {
            instinct->needIndex = random.RAND_CHOICE(NUM_INSTINCT_NEEDS) + 1;
        }
        else
        {
            instinct->needIndex = -1;
        }
        instinct->needValue =
            (random.RAND_PROB() * (MAX_NEED_VALUE - MIN_NEED_VALUE))
            + MIN_NEED_VALUE;
        instinct->needFrequency =
            random.RAND_CHOICE((MAX_NEED_FREQ - MIN_NEED_FREQ) + 1)
            + MIN_NEED_FREQ;
        instinct->needDuration =
            random.RAND_CHOICE((MAX_NEED_DURATION - MIN_NEED_DURATION) + 1)
            + MIN_NEED_DURATION;
        instinct->goalValue = MAX_NEED_VALUE * random.RAND_PROB();
        if (random.RAND_BOOL())
        {
            instinct->goalValue = -instinct->goalValue;
        }

        // Check for duplicate.
        for (p = 0, q = instincts.size(); p < q; p++)
        {
            if (instincts[p]->isDuplicate(instinct)) break;
        }
        if (p == q)
        {
            instincts.push_back(instinct);
        }
    }
    #else

    // Hard-coded instincts.
    // NUM_INSTINCT_NEEDS = 5;

    // Look right for box.
    instinct = new Instinct();
    assert(instinct != NULL);
    event.stimuli[0] = MonkeyAndBananas::FLOOR;
    event.stimuli[1] = DONT_CARE;
    event.stimuli[2] = 0;
    event.response = MonkeyAndBananas::GO_RIGHT;
    instinct->events.push_back(event);
    event.stimuli[0] = MonkeyAndBananas::BOX;
    event.stimuli[1] = DONT_CARE;
    event.stimuli[2] = 0;
    event.response = -1;
    instinct->events.push_back(event);
    instinct->needIndex = 1;
    instinct->needValue = 4.0;
    instinct->needFrequency = 0;
    instinct->needDuration = 50;
    instinct->goalValue = 0.2;
    instincts.push_back(instinct);

    // Continue looking for box.
    instinct = new Instinct();
    assert(instinct != NULL);
    event.stimuli[0] = MonkeyAndBananas::FLOOR;
    event.stimuli[1] = DONT_CARE;
    event.stimuli[2] = 0;
    event.response = MonkeyAndBananas::GO_RIGHT;
    instinct->events.push_back(event);
    event.stimuli[0] = MonkeyAndBananas::FLOOR;
    event.stimuli[1] = DONT_CARE;
    event.stimuli[2] = 0;
    event.response = MonkeyAndBananas::GO_RIGHT;
    instinct->events.push_back(event);
    event.stimuli[0] = MonkeyAndBananas::BOX;
    event.stimuli[1] = DONT_CARE;
    event.stimuli[2] = 0;
    event.response = -1;
    instinct->events.push_back(event);
    instinct->needIndex = 1;
    instinct->needValue = 4.0;
    instinct->needFrequency = 0;
    instinct->needDuration = 50;
    instinct->goalValue = 0.2;
    instincts.push_back(instinct);

    // Pickup box.
    instinct = new Instinct();
    assert(instinct != NULL);
    event.stimuli[0] = MonkeyAndBananas::BOX;
    event.stimuli[1] = MonkeyAndBananas::LOOK_RIGHT;
    event.stimuli[2] = 0;
    event.response = MonkeyAndBananas::PICKUP;
    instinct->events.push_back(event);
    event.stimuli[0] = MonkeyAndBananas::FLOOR;
    event.stimuli[1] = MonkeyAndBananas::LOOK_RIGHT;
    event.stimuli[2] = 1;
    event.response = -1;
    instinct->events.push_back(event);
    instinct->needIndex = 2;
    instinct->needValue = 1.0;
    instinct->needFrequency = 1;
    instinct->needDuration = 0;
    instinct->goalValue = 0.1;
    instincts.push_back(instinct);

    // Turn left with box.
    instinct = new Instinct();
    assert(instinct != NULL);
    event.stimuli[0] = MonkeyAndBananas::FLOOR;
    event.stimuli[1] = MonkeyAndBananas::LOOK_RIGHT;
    event.stimuli[2] = 1;
    event.response = MonkeyAndBananas::GO_LEFT;
    instinct->events.push_back(event);
    event.stimuli[0] = MonkeyAndBananas::FLOOR;
    event.stimuli[1] = MonkeyAndBananas::LOOK_LEFT;
    event.stimuli[2] = 1;
    event.response = -1;
    instinct->events.push_back(event);
    instinct->needIndex = 2;
    instinct->needValue = 1.0;
    instinct->needFrequency = 1;
    instinct->needDuration = 0;
    instinct->goalValue = 0.1;
    instincts.push_back(instinct);

    // Go left toward bananas.
    instinct = new Instinct();
    assert(instinct != NULL);
    event.stimuli[0] = DONT_CARE;
    event.stimuli[1] = MonkeyAndBananas::LOOK_LEFT;
    event.stimuli[2] = DONT_CARE;
    event.response = MonkeyAndBananas::GO_LEFT;
    instinct->events.push_back(event);
    event.stimuli[0] = MonkeyAndBananas::BANANAS;
    event.stimuli[1] = MonkeyAndBananas::LOOK_LEFT;
    event.stimuli[2] = DONT_CARE;
    event.response = -1;
    instinct->events.push_back(event);
    instinct->needIndex = 2;
    instinct->needValue = 1.0;
    instinct->needFrequency = 1;
    instinct->needDuration = 0;
    instinct->goalValue = 0.1;
    instincts.push_back(instinct);

    // Continue going left.
    instinct = new Instinct();
    assert(instinct != NULL);
    event.stimuli[0] = DONT_CARE;
    event.stimuli[1] = MonkeyAndBananas::LOOK_LEFT;
    event.stimuli[2] = DONT_CARE;
    event.response = MonkeyAndBananas::GO_LEFT;
    instinct->events.push_back(event);
    event.stimuli[0] = DONT_CARE;
    event.stimuli[1] = MonkeyAndBananas::LOOK_LEFT;
    event.stimuli[2] = DONT_CARE;
    event.response = MonkeyAndBananas::GO_LEFT;
    instinct->events.push_back(event);
    event.stimuli[0] = MonkeyAndBananas::BANANAS;
    event.stimuli[1] = MonkeyAndBananas::LOOK_LEFT;
    event.stimuli[2] = DONT_CARE;
    event.response = -1;
    instinct->events.push_back(event);
    instinct->needIndex = 2;
    instinct->needValue = 1.0;
    instinct->needFrequency = 1;
    instinct->needDuration = 0;
    instinct->goalValue = 0.1;
    instincts.push_back(instinct);

    // Stack box on wall.
    instinct = new Instinct();
    assert(instinct != NULL);
    event.stimuli[0] = MonkeyAndBananas::WALL;
    event.stimuli[1] = MonkeyAndBananas::LOOK_LEFT;
    event.stimuli[2] = 1;
    event.response = MonkeyAndBananas::STACK;
    instinct->events.push_back(event);
    event.stimuli[0] = MonkeyAndBananas::BOX;
    event.stimuli[1] = MonkeyAndBananas::LOOK_LEFT;
    event.stimuli[2] = 0;
    event.response = -1;
    instinct->events.push_back(event);
    instinct->needIndex = 3;
    instinct->needValue = 3.0;
    instinct->needFrequency = 0;
    instinct->needDuration = 0;
    instinct->goalValue = 1.0;
    instincts.push_back(instinct);

    // Stack box on box.
    instinct = new Instinct();
    assert(instinct != NULL);
    event.stimuli[0] = MonkeyAndBananas::BOX;
    event.stimuli[1] = MonkeyAndBananas::LOOK_LEFT;
    event.stimuli[2] = 1;
    event.response = MonkeyAndBananas::STACK;
    instinct->events.push_back(event);
    event.stimuli[0] = MonkeyAndBananas::BOX;
    event.stimuli[1] = MonkeyAndBananas::LOOK_LEFT;
    event.stimuli[2] = 0;
    event.response = -1;
    instinct->events.push_back(event);
    instinct->needIndex = 3;
    instinct->needValue = 3.0;
    instinct->needFrequency = 0;
    instinct->needDuration = 0;
    instinct->goalValue = 1.0;
    instincts.push_back(instinct);

    // Prevent stacking box out on floor.
    instinct = new Instinct();
    assert(instinct != NULL);
    event.stimuli[0] = MonkeyAndBananas::FLOOR;
    event.stimuli[1] = DONT_CARE;
    event.stimuli[2] = 1;
    event.response = MonkeyAndBananas::STACK;
    instinct->events.push_back(event);
    event.stimuli[0] = MonkeyAndBananas::BOX;
    event.stimuli[1] = DONT_CARE;
    event.stimuli[2] = 0;
    event.response = -1;
    instinct->events.push_back(event);
    instinct->needIndex = 4;
    instinct->needValue = 0.0;
    instinct->needFrequency = 1;
    instinct->needDuration = 0;
    instinct->goalValue = -10.0;
    instincts.push_back(instinct);

    // Prevent stacking box to right.
    instinct = new Instinct();
    assert(instinct != NULL);
    event.stimuli[0] = MonkeyAndBananas::BOX;
    event.stimuli[1] = MonkeyAndBananas::LOOK_RIGHT;
    event.stimuli[2] = 1;
    event.response = MonkeyAndBananas::STACK;
    instinct->events.push_back(event);
    event.stimuli[0] = MonkeyAndBananas::BOX;
    event.stimuli[1] = MonkeyAndBananas::LOOK_RIGHT;
    event.stimuli[2] = 0;
    event.response = -1;
    instinct->events.push_back(event);
    instinct->needIndex = 4;
    instinct->needValue = 0.0;
    instinct->needFrequency = 1;
    instinct->needDuration = 0;
    instinct->goalValue = -10.0;
    instincts.push_back(instinct);

    // Prevent picking up stacked box.
    instinct = new Instinct();
    assert(instinct != NULL);
    event.stimuli[0] = MonkeyAndBananas::BOX;
    event.stimuli[1] = MonkeyAndBananas::LOOK_LEFT;
    event.stimuli[2] = 0;
    event.response = MonkeyAndBananas::PICKUP;
    instinct->events.push_back(event);
    event.stimuli[0] = MonkeyAndBananas::FLOOR;
    event.stimuli[1] = MonkeyAndBananas::LOOK_LEFT;
    event.stimuli[2] = 1;
    event.response = -1;
    instinct->events.push_back(event);
    instinct->needIndex = 4;
    instinct->needValue = 0.0;
    instinct->needFrequency = 1;
    instinct->needDuration = 0;
    instinct->goalValue = -10.0;
    instincts.push_back(instinct);

    // Climb two boxes.
    instinct = new Instinct();
    assert(instinct != NULL);
    event.stimuli[0] = MonkeyAndBananas::BOX;
    event.stimuli[1] = MonkeyAndBananas::LOOK_LEFT;
    event.stimuli[2] = 0;
    event.response = MonkeyAndBananas::CLIMB;
    instinct->events.push_back(event);
    event.stimuli[0] = DONT_CARE;
    event.stimuli[1] = MonkeyAndBananas::LOOK_LEFT;
    event.stimuli[2] = 0;
    event.response = MonkeyAndBananas::CLIMB;
    instinct->events.push_back(event);
    event.stimuli[0] = MonkeyAndBananas::BANANAS;
    event.stimuli[1] = MonkeyAndBananas::LOOK_LEFT;
    event.stimuli[2] = 0;
    event.response = -1;
    instinct->events.push_back(event);
    instinct->needIndex = 5;
    instinct->needValue = 1.0;
    instinct->needFrequency = 10;
    instinct->needDuration = 0;
    instinct->goalValue = 0.1;
    instincts.push_back(instinct);

    // Climb wall and see floor on platform.
    instinct = new Instinct();
    assert(instinct != NULL);
    event.stimuli[0] = MonkeyAndBananas::WALL;
    event.stimuli[1] = MonkeyAndBananas::LOOK_LEFT;
    event.stimuli[2] = 0;
    event.response = MonkeyAndBananas::CLIMB;
    instinct->events.push_back(event);
    event.stimuli[0] = MonkeyAndBananas::BANANAS;
    event.stimuli[1] = MonkeyAndBananas::LOOK_LEFT;
    event.stimuli[2] = 0;
    event.response = -1;
    instinct->events.push_back(event);
    instinct->needIndex = 5;
    instinct->needValue = 1.0;
    instinct->needFrequency = 10;
    instinct->needDuration = 0;
    instinct->goalValue = 0.1;
    instincts.push_back(instinct);

    // Eat bananas.
    instinct = new Instinct();
    assert(instinct != NULL);
    event.stimuli[0] = MonkeyAndBananas::BANANAS;
    event.stimuli[1] = MonkeyAndBananas::LOOK_LEFT;
    event.stimuli[2] = 0;
    event.response = MonkeyAndBananas::EAT;
    instinct->events.push_back(event);
    event.stimuli[0] = MonkeyAndBananas::FLOOR;
    event.stimuli[1] = MonkeyAndBananas::LOOK_LEFT;
    event.stimuli[2] = 0;
    event.response = -1;
    instinct->events.push_back(event);
    instinct->needIndex = 0;
    instinct->needValue = 10.0;
    instinct->needFrequency = 0;
    instinct->needDuration = 0;
    instinct->goalValue = 10.0;
    instincts.push_back(instinct);
    #endif
}


// Destructor.
MonkeyInstincts::~MonkeyInstincts()
{
    int i,j;

    for (i = 0, j = instincts.size(); i < j; i++)
    {
        delete instincts[i];
    }
    instincts.clear();
    random.RAND_CLEAR();
}


// Incorporate instincts into monkey's brain.
void MonkeyInstincts::init(Mona *brain)
{
    int i,j,k,p,q;
    VALUE_SET *goals;
    Mona::Receptor *receptor;
    Mona::Motor *motor;
    Mona::Mediator *mediator;
    list<Mona::Neuron *>::iterator neuronItr;
    Mona::SENSOR sensors[3];
    char buf[100];

    // Save brain reference.
    this->brain = brain;

    // Add need and goal for bananas.
    brain->initNeed(0, MAX_NEED_VALUE, "Bananas");
    for (neuronItr = brain->receptors.begin();
        neuronItr != brain->receptors.end(); neuronItr++)
    {
        receptor = (Mona::Receptor *)*neuronItr;
        if (receptor->sensorMask[0] == MonkeyAndBananas::BANANAS &&
            receptor->sensorMask[1] == DONT_CARE &&
            receptor->sensorMask[2] == DONT_CARE) break;
    }
    assert(neuronItr != brain->receptors.end());
    goals = new VALUE_SET;
    assert(goals != NULL);
    goals->alloc(NUM_INSTINCT_NEEDS + 1);
    goals->set(0, MAX_NEED_VALUE);
    for (i = 1; i <= NUM_INSTINCT_NEEDS; i++)
    {
        goals->set(i, 0.0);
    }
    receptor->goals.setGoals(*goals, 1.0);

    // Add instinctive needs.
    for (i = 1; i <= NUM_INSTINCT_NEEDS; i++)
    {
        sprintf(buf, "Instinct need %d", i - 1);
        brain->initNeed(i, 0.0, buf);
    }

    // Add instinctive mediators.
    for (i = 0, j = instincts.size(); i < j; i++)
    {
        sprintf(buf, "Instinct %d", i);
        mediator = brain->newMediator(Mona::NEW_ENABLEMENT, true, Mona::NEW_ENABLEMENT, buf);
        mediator->instinct = true;
        instincts[i]->mediator = mediator;
        for (p = 0, q = instincts[i]->events.size(); p < q; p++)
        {
            sensors[0] = instincts[i]->events[p].stimuli[0];
            sensors[1] = instincts[i]->events[p].stimuli[1];
            sensors[2] = instincts[i]->events[p].stimuli[2];
            for (neuronItr = brain->receptors.begin();
                neuronItr != brain->receptors.end(); neuronItr++)
            {
                receptor = (Mona::Receptor *)*neuronItr;
                if (receptor->isDuplicate(sensors))
                {
                    break;
                }
            }
            if (neuronItr == brain->receptors.end())
            {
                sprintf(buf, "%s,%s,%s",
                    getViewDesc(sensors[0]), getDirDesc(sensors[1]),
                    getHoldingDesc(sensors[2]));
                receptor = brain->newReceptor(sensors, buf);
            }
            if (p == 0)
            {
                mediator->addEvent(Mona::CAUSE_EVENT, receptor);
            } else if (p < q - 1)
            {
                mediator->addEvent(Mona::INTERMEDIATE_EVENT, receptor);
            }
            else
            {
                mediator->addEvent(Mona::EFFECT_EVENT, receptor);
            }
            if (instincts[i]->events[p].response != -1)
            {
                for (neuronItr = brain->motors.begin();
                    neuronItr != brain->motors.end(); neuronItr++)
                {
                    motor = (Mona::Motor *)*neuronItr;
                    if (motor->response == instincts[i]->events[p].response) break;
                }
                mediator->addEvent(Mona::INTERMEDIATE_EVENT, motor);
            }
        }

        // Set goal for mediator.
        if (instincts[i]->needIndex != -1)
        {
            for (k = 0; k <= NUM_INSTINCT_NEEDS; k++)
            {
                goals->set(k, 0.0);
            }
            goals->set(instincts[i]->needIndex, instincts[i]->goalValue);
            mediator->goals.setGoals(*goals, 1.0);

            // Immediate need?
            if (instincts[i]->needFrequency <= 1)
            {
                brain->setNeed(instincts[i]->needIndex, instincts[i]->needValue);

                // Set duration timer?
                if (instincts[i]->needDuration > 0)
                {
                    instincts[i]->durTimer = instincts[i]->needDuration;
                }
            }
        }
    }
    delete goals;
}


// Reset instinctive needs.
void MonkeyInstincts::reset()
{
    int i,j;

    // Set need for bananas.
    brain->setNeed(0, MAX_NEED_VALUE);

    // Add instinctive mediators.
    for (i = 0, j = instincts.size(); i < j; i++)
    {
        instincts[i]->freqTimer = 0;
        instincts[i]->durTimer = 0;

        // Set need for mediator.
        if (instincts[i]->needIndex != -1)
        {
            // Immediate need?
            if (instincts[i]->needFrequency <= 1)
            {
                brain->setNeed(instincts[i]->needIndex, instincts[i]->needValue);

                // Set duration timer?
                if (instincts[i]->needDuration > 0)
                {
                    instincts[i]->durTimer = instincts[i]->needDuration;
                }
            }
        }
    }
}


// Perform instinctive actions.
void MonkeyInstincts::step()
{
    int i,j;
    Mona::Receptor *receptor;
    list<Mona::Neuron *>::iterator neuronItr;
    Mona::NEED need;

    // Reward finding bananas.
    for (neuronItr = brain->receptors.begin();
        neuronItr != brain->receptors.end(); neuronItr++)
    {
        receptor = (Mona::Receptor *)*neuronItr;
        if (receptor->sensorMask[0] == MonkeyAndBananas::BANANAS &&
            receptor->sensorMask[1] == DONT_CARE &&
            receptor->sensorMask[2] == DONT_CARE) break;
    }
    if (receptor->firingStrength > 0.0)
    {
        need = brain->getNeed(0);
        need -= MAX_NEED_VALUE;
        if (need < 0.0) need = 0.0;
        brain->setNeed(0, need);
    }

    // Modify needs for firing instincts.
    for (i = 0, j = instincts.size(); i < j; i++)
    {
        instincts[i]->fire();
    }

    // Induce instinctive needs.
    for (i = 0, j = instincts.size(); i < j; i++)
    {
        instincts[i]->induce();
    }
}


// Clear mediators.
void MonkeyInstincts::clearMediators()
{
    int i,j;

    for (i = 0, j = instincts.size(); i < j; i++)
    {
        instincts[i]->mediator = NULL;
    }
}


// Clone instincts.
MonkeyInstincts *MonkeyInstincts::clone()
{
    int i,j;
    MonkeyInstincts *monkeyInstincts;

    monkeyInstincts = new MonkeyInstincts(random.RAND());
    assert(monkeyInstincts != NULL);
    monkeyInstincts->brain = brain;
    for (i = 0, j = instincts.size(); i < j; i++)
    {
        monkeyInstincts->instincts.push_back(instincts[i]->clone());
    }
    random.RAND_CLONE(monkeyInstincts->random);
    return monkeyInstincts;
}


// Load instincts.
void MonkeyInstincts::load(char *filename)
{
    FILE *fp;

    if ((fp = fopen(filename, "r")) == NULL)
    {
        fprintf(stderr, "Cannot load instincts from file %s\n", filename);
        exit(1);
    }
    load(fp);
}


void MonkeyInstincts::load(FILE *fp)
{
    int i,j;
    Instinct *instinct;

    for (i = 0, j = instincts.size(); i < j; i++)
    {
        delete instincts[i];
    }
    instincts.clear();
    random.RAND_CLEAR();

    FREAD_INT(&j, fp);
    for (i = 0; i < j; i++)
    {
        instinct = new Instinct();
        assert(instinct != NULL);
        instinct->load(fp);
        instincts.push_back(instinct);
    }
    random.RAND_LOAD(fp);
}


// Save instincts.
void MonkeyInstincts::save(char *filename)
{
    FILE *fp;

    if ((fp = fopen(filename, "w")) == NULL)
    {
        fprintf(stderr, "Cannot save instincts to file %s\n", filename);
        exit(1);
    }
    save(fp);
}


void MonkeyInstincts::save(FILE *fp)
{
    int i,j;

    j = instincts.size();
    FWRITE_INT(&j, fp);
    for (i = 0; i < j; i++)
    {
        instincts[i]->save(fp);
    }
    random.RAND_SAVE(fp);
}


// Print instincts.
void MonkeyInstincts::print()
{
    int i,j;

    printf("instincts:\n");
    for (i = 0, j = instincts.size(); i < j; i++)
    {
        instincts[i]->print();
    }
}
