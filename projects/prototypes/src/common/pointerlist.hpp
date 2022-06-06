// Pointer list definitions

// Store and manage a sequential list of pointers.

#ifndef __POINTERLIST__
#define __POINTERLIST__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

class PointerList
{

public:

	PointerList();				// Constructor
	void clear();				// Clear
	void zero();				// Zero entries
	int insert(char *);			// Insert pointer and return its index
	void insert(int, char *);	// Insert pointer by index
	bool remove(char *);		// Remove by pointer
	bool remove(int);			// Remove by index
	int getsize();				// Get number of pointers
	char *index(int);			// Get pointer at given index

private:

	char **list;		// List of pointers
	int size;			// Number of pointers
	int allocation;		// List storage allocation
	void realloc(int);	// Reallocate to new amount

	static int growIncrement;	// List growth parameter
};

typedef class PointerList POINTER_LIST;

#endif
