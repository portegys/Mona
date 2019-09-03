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

// The Mona foraging ant environment functions

#include "environment.hpp"

// Version
void
Environment::version()
{
	const char *antEnvironmentVersion = ANT_ENVIRONMENT_VERSION;
	cout << &antEnvironmentVersion[5] << "\n";
}

// Construct ant environment
Environment::Environment(int width, int height, int length, double trailDirParm)
{
	int i,j,k,x,y,w[4];
	DIR dir,d;

	assert(width > 0 && height > 0 && length >= 0);
	assert(trailDirParm >= 0.0 && trailDirParm <= 1.0);
	this->width = width;
	this->height = height;
	this->trailDirParm = trailDirParm;

	field = new struct Cell *[width];
	for (x = 0; x < this->width; x++)
	{
		field[x] = new struct Cell[height];
		for (y = 0; y < this->height; y++)
		{
			field[x][y].nest = FALSE;
			field[x][y].mark = FALSE;
			field[x][y].food = FALSE;
		}
	}

	this->x = x = width/2;
	this->y = y = height/2;
	field[x][y].nest = field[x][y].mark = TRUE;
	this->dir = dir = (DIR)(rand()%4);
	hasFood = FALSE;
	for (i = 0; i < length; i++)
	{
		for (j = 0; j < 4; j++) w[j] = 1;
		if (RAND_PROB < trailDirParm)
		{
			j = (int)dir;
			w[j] = 0;
			if ((j%2) == 0) { w[j+1] = 0; } else { w[j-1] = 0; }
		}
		for (j = 0; j < 4; j++)
		{
			if (w[j] == 0) continue;
			k = 0;
			d = (DIR)j;
			switch(d)
			{
			case RIGHT:
				if ((x+2) >= width ||
				    ((x+2) < width && field[x+2][y].mark == TRUE))
				{
					k = 1;
				} else if ((y+1) < height && field[x+1][y+1].mark == TRUE)
				{
					k = 1;
				} else if ((y-1) >= 0 && field[x+1][y-1].mark == TRUE)
				{
					k = 1;
				}
				break;
			case LEFT:
				if ((x-2) < 0 ||
				    ((x-2) >= 0 && field[x-2][y].mark == TRUE))
				{
					k = 1;
				} else if ((y+1) < height && field[x-1][y+1].mark == TRUE)
				{
					k = 1;
				} else if ((y-1) > 0 && field[x-1][y-1].mark == TRUE)
				{
					k = 1;
				}
				break;
			case UP:
				if ((y+2) == height ||
				    ((y+2) < height && field[x][y+2].mark == TRUE))
				{
					k = 1;
				} else if ((x+1) < width && field[x+1][y+1].mark == TRUE)
				{
					k = 1;
				} else if ((x-1) >= 0 && field[x-1][y+1].mark == TRUE)
				{
					k = 1;
				}
				break;
			case DOWN:
				if ((y-2) < 0 ||
				    ((y-2) >= 0 && field[x][y-2].mark == TRUE))
				{
					k = 1;
				} else if ((x+1) < width && field[x+1][y-1].mark == TRUE)
				{
					k = 1;
				} else if ((x-1) > 0 && field[x-1][y-1].mark == TRUE)
				{
					k = 1;
				}
				break;
			}
			if (k == 1)
			{
				w[j] = 0;
				if (j == (int)dir)
				{
					if ((j%2) == 0) { w[j+1] = 0; } else { w[j-1] = 0; }
				}
			}
		}
		if (w[(int)dir] == 0)
		{
			for (j = k = 0; j < 4; j++)
			{
				if (w[j] == 1) k++;
			}
			if (k == 0) break;
			while (w[j = (int)(rand()%4)] == 0) {}
			dir = (DIR)j;
		}

		switch(dir)
		{
		case RIGHT:	x++; if (x == width) x = 0; break;
		case LEFT:	x--; if (x < 0) x = width-1; break;
		case UP:	y++; if (y == height) y = 0; break;
		case DOWN:	y--; if (y < 0) y = height-1; break;
		}

		field[x][y].mark = TRUE;
	}
	field[x][y].food = TRUE;
}

// Ant environment input
void
Environment::input(INPUT input)
{
	switch(input)
	{
	case FORWARD:
		x = this->x;
		y = this->y;
		switch(dir)
		{
		case RIGHT:	x++; if (x == this->width) x = 0; break;
		case LEFT:	x--; if (x < 0) x = this->width-1; break;
		case UP:	y++; if (y == this->height) y = 0; break;
		case DOWN:	y--; if (y < 0) y = this->height-1; break;
		}
		this->x = x;
		this->y = y;
		return;
	case BACKWARD:
		x = this->x;
		y = this->y;
		switch(dir)
		{
		case RIGHT:	x--; if (x < 0) x = this->width-1; break;
		case LEFT:	x++; if (x == this->width) x = 0; break;
		case UP:	y--; if (y < 0) y = this->height-1; break;
		case DOWN:	y++; if (y == this->height) y = 0; break;
		}
		this->x = x;
		this->y = y;
		return;
	case ORIENT:
		x = this->x;
		y = this->y;
		switch(dir)
		{
		case RIGHT:
			if (y < this->height-1 && field[x][y+1].mark == TRUE)
			{
				dir = UP;
			} else if (y > 0 && field[x][y-1].mark == TRUE)
			{
				dir = DOWN;
			} else if (x > 0 && field[x-1][y].mark == TRUE)
			{
				dir = LEFT;
			}
			break;
		case LEFT:
			if (y > 0 && field[x][y-1].mark == TRUE)
			{
				dir = DOWN;
			} else if (y < this->height-1 && field[x][y+1].mark == TRUE)
			{
				dir = UP;
			} else if (x < this->width-1 && field[x+1][y].mark == TRUE)
			{
				dir = RIGHT;
			}
			break;
		case UP:
			if (x > 0 && field[x-1][y].mark == TRUE)
			{
				dir = LEFT;
			} else if (x < this->width-1 && field[x+1][y].mark == TRUE)
			{
				dir = RIGHT;
			} else if (y > 0 && field[x][y-1].mark == TRUE)
			{
				dir = DOWN;
			}
			break;
		case DOWN:
			if (x < this->width-1 && field[x+1][y].mark == TRUE)
			{
				dir = RIGHT;
			} else if (x > 0 && field[x-1][y].mark == TRUE)
			{
				dir = LEFT;
			} else if (y < this->height-1 && field[x][y+1].mark == TRUE)
			{
				dir = UP;
			}
			break;
		}
		return;
	case GRAB:
		if (field[x][y].food == TRUE)
		{
			field[x][y].food = FALSE;
			this->hasFood = TRUE;
		}
		return;
	case DROP:
		if (this->hasFood == TRUE)
		{
			field[x][y].food = TRUE;
			this->hasFood = FALSE;
		}
		return;
	}
}

// Ant environment output
Environment::OUTPUT
Environment::output()
{

#ifdef MONA_TRACE
#ifdef NEVER
	int x,y;
	struct Cell *cell;
	char c;

	printf("Environment:\n");
	for (x = 0; x < this->width; x++) printf("#");
	printf("##\n");
	for (y = this->height-1; y >= 0; y--)
	{
		printf("#");
		for (x = 0; x < this->width; x++)
		{
			cell = &(field[x][y]);
			c = ' ';
			if (cell->nest == TRUE) {
				c = 'n';
			} else if (cell->food == TRUE) {
				c = 'f';
			} else if (cell->mark == TRUE) {
				c = 'm';
			}
			if (x == this->x && y == this->y) {
				c = 'a';
			}
			printf("%c", c);
		}
		printf("#\n");
	}
	for (x = 0; x < this->width; x++) printf("#");
	printf("##\n");
#endif
#endif

	return(field[this->x][this->y]);
}

// Print input to stdout
void
Environment::printInput(INPUT input)
{
	switch(input)
	{
	case FORWARD: printf("FORWARD"); break;
	case BACKWARD: printf("BACKWARD"); break;
	case ORIENT: printf("ORIENT"); break;
	case GRAB: printf("GRAB"); break;
	case DROP: printf("DROP"); break;
	}
}

// Print output to stdout
void
Environment::printOutput(OUTPUT output)
{
	if (output.nest) printf("NEST "); else printf("NO_NEST ");
	if (output.mark) printf("MARK "); else printf("NO_MARK ");
	if (output.food) printf("FOOD "); else printf("NO_FOOD ");
}
