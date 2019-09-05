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

/*
 * Instinct.
 * An instinct modifies a need based on timing and the firing of
 * an associated mediator.
 */

#ifndef __INSTINCT__
#define __INSTINCT__

#include "../mona/mona.hpp"

// Number of sensory stimuli.
#define NUM_INSTINCT_STIMULI 3

// Instinct.
class Instinct
{
    public:

        // Stimulus/response event.
        typedef struct Event
        {
            Mona::SENSOR stimuli[NUM_INSTINCT_STIMULI];
            Mona::RESPONSE response;
        } EVENT;

        vector<EVENT> events;
        int needIndex;
        Mona::NEED needValue;
        int needFrequency;
        int freqTimer;
        int needDuration;
        int durTimer;
        Mona::NEED goalValue;
        Mona::Mediator *mediator;

        // Constructor.
        Instinct();

        // Fire instinct.
        void fire();

        // Induce instinctive need.
        void induce();

        // Is instinct a duplicate?
        bool isDuplicate(Instinct *);

        // Clone instinct.
        Instinct *clone();

        // Load instinct.
        void load(char *filename);
        void load(FILE *);

        // Save instinct.
        void save(char *filename);
        void save(FILE *);

        // Print instinct.
        void print();
};
#endif
