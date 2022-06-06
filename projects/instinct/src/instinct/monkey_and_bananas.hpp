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

/*
 * Monkey and Bananas problem.
 * This is the Monkey and Bananas environment. The goal is to
 * obtain a bunch of bananas on an elevated platform by stacking
 * and climbing boxes to reach it.
 *
 * Sensory/motor capabilities:
 * The monkey is able to sense the contents of the cell immediately
 * in front of it. This may be: floor, box, wall, bananas, or air
 * (if standing on a box above a floor). It can also sense whether
 * it is holding a box and the direction it is facing. The monkey
 * can move right and left, climb (one level), and pickup and stack
 * a box.
 *
 * To use graphics display, compile with ALLEGRO_GRAPHICS defined.
 * Graphics requires:
 * Allegro (http://alleg.sourceforge.net), and
 * Glyph Keeper (http://kd.lab.nig.ac.jp/glyph-keeper/)
 */

#ifndef __MONKEY_AND_BANANAS__
#define __MONKEY_AND_BANANAS__

#define MONKEY_AND_BANANAS_VERSION "@(#) Monkey and bananas problem, version 1.0"

#include "instinct.hpp"

class MonkeyInstincts;

// Monkey and Bananas environment.
class MonkeyAndBananas
{
    public:

        // Version.
        static void version();

        // Items.
        typedef enum { MONKEY='m', BANANAS='b', BOX='x' }
        ITEM;

        // Views.
        typedef enum { FLOOR='-', WALL='|', AIR=' ' }
        VIEW;

        // Responses.
        typedef enum { GO_LEFT=0, GO_RIGHT=1, CLIMB=2, PICKUP=3, STACK=4, EAT=5 }
        RESPONSE;

        // Monkey direction.
        typedef enum { LOOK_LEFT=0, LOOK_RIGHT=1 }
        DIRECTION;

        // Display types.
        typedef enum { NO_DISPLAY, TEXT_DISPLAY, GRAPHICS_DISPLAY }
        DISPLAY_TYPE;

        // The world.
        char **world;
        int worldX, worldY;

        // Defaults.
        static const int DEFAULT_MONKEY_LOCATION;
        static const DIRECTION DEFAULT_MONKEY_DIRECTION;
        static const char *DEFAULT_WORLD[];
        static const DISPLAY_TYPE DEFAULT_DISPLAY_TYPE;

        // Display size.
        static const int DISPLAY_WIDTH;
        static const int DISPLAY_HEIGHT;

        // Font.
        static const char *DISPLAY_FONT;

        // Images.
        static const char *MONKEY_IMAGE;
        static const char *BANANAS_IMAGE;

        // Sound.
        static const char *MONKEY_SOUND;

        // Monkey.
        int location;
        DIRECTION direction;
        int numSensors,maxResponse;
        Mona::SENSOR sensors[3];
        Mona::RESPONSE response;
        Mona *brain;
        MonkeyInstincts *instincts;

        // Constructors.
        MonkeyAndBananas(MonkeyInstincts *instincts,
            Mona *brain=NULL);
        MonkeyAndBananas(char **world, int location,
            DIRECTION direction, DISPLAY_TYPE displayType,
            MonkeyInstincts *instincts, Mona *brain=NULL);
        void init(char **world, int location,
            DIRECTION direction, DISPLAY_TYPE displayType,
            MonkeyInstincts *instincts, Mona *brain=NULL);

        // Destructor.
        ~MonkeyAndBananas();

        // Running.
        int run(int steps);
        void step();
        int steps;
        bool foundBananas;

        // Evaluate the world state.
        double evaluate(int steps);

        // Display.
        void setDisplay(DISPLAY_TYPE displayType);
        DISPLAY_TYPE displayType;
        void textDisplay(int step);
        bool graphicsDisplay();

    private:

        char **world2;
        int location2;
        DIRECTION direction2;
        bool tryResponse();
};

// Monkey instincts.
class MonkeyInstincts
{
    public:

        // Parameters.
        static const int DEFAULT_MAX_INSTINCTS;
        static const int DEFAULT_MIN_INSTINCTS;
        static int MAX_INSTINCTS;
        static int MIN_INSTINCTS;
        static const int MIN_INSTINCT_EVENTS;
        static const int MAX_INSTINCT_EVENTS;
        static const int NUM_INSTINCT_NEEDS;
        static const Mona::NEED MAX_NEED_VALUE;
        static const Mona::NEED MIN_NEED_VALUE;
        static const int MAX_NEED_FREQ;
        static const int MIN_NEED_FREQ;
        static const int MAX_NEED_DURATION;
        static const int MIN_NEED_DURATION;

        // Monkey brain.
        Mona *brain;

        // Instincts.
        vector<Instinct *> instincts;

        // Random numbers.
        Random random;

        // Constructor.
        MonkeyInstincts(RANDOM);

        // Destructor.
        ~MonkeyInstincts();

        // Incorporate instincts into brain.
        void init(Mona *brain);

        // Reset instictive needs.
        void reset();

        // Perform instinctive actions.
        void step();

        // Clear mediators.
        void clearMediators();

        // Clone instincts.
        MonkeyInstincts *clone();

        // Load instincts.
        void load(char *filename);
        void load(FILE *);

        // Save instincts.
        void save(char *filename);
        void save(FILE *);

        // Print instincts.
        void print();
};
#endif
