// Value set definitions

// Store and manage a set of floating point values.

#ifndef __VALUESET__
#define __VALUESET__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

class ValueSet
{

public:

	ValueSet();					// Constructors
	ValueSet(int size);
	~ValueSet();				// Destructor
	void alloc(int size);		// Allocate
	void clear();				// Clear
	void zero();				// Zero values
	double get(int);			// Get a value
	double sum();				// Get sum of values
	void set(int, double);		// Set a value
	void load(ValueSet *);		// Load values from given set
	void add(ValueSet *);		// Vector addition
	void subtract(ValueSet *);	// Vector subtraction
	void scale(double);			// Scalar multiplication
	void print();				// Print

	double *values;	// Value set
	int size;		// Size of set
};

typedef class ValueSet VALUE_SET;

#endif
