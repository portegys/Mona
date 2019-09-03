// Value set functions

#include "valueset.hpp"

// Constructors.
ValueSet::ValueSet()
{
	values = NULL;
	size = 0;
}

ValueSet::ValueSet(int size)
{
	values = NULL;
	this->size = 0;
	alloc(size);
}

// Destructor.
ValueSet::~ValueSet()
{
	clear();
}

// Clear
void
ValueSet::clear()
{
	if (values != NULL) delete values;
	values = NULL;
	size = 0;
}

// Allocate
void
ValueSet::alloc(int size)
{
	clear();
	if (size <= 0) return;
	values = new double[size];
	assert(values != NULL);
	this->size = size;
	zero();
}

// Zero
void
ValueSet::zero()
{
	register int i;

	for (i = 0; i < size; i++) values[i] = 0.0;
}

// Get a specified value.
double
ValueSet::get(int index)
{
	assert(index >= 0 && index < size);
	return(values[index]);
}

// Get sum of values.
double
ValueSet::sum()
{
	register int i;
	double d;

	for (i = 0, d = 0.0; i < size; i++) d += values[i];
	return(d);
}

// Set a specified value.
void
ValueSet::set(int index, double value)
{
	assert(index >= 0 && index < size);
	values[index] = value;
}

// Load given set of values into this.
void
ValueSet::load(class ValueSet *loadSet)
{
	register int i;

	if (loadSet->size != size)
	{
		alloc(loadSet->size);
	}

	for (i = 0; i < size; i++)
	{
		values[i] = loadSet->values[i];
	}
}

// Vector addition.
void
ValueSet::add(class ValueSet *addSet)
{
	register int i;

	assert(size == addSet->size);

	for (i = 0; i < size; i++)
	{
		values[i] += addSet->values[i];
	}
}

// Vector subtraction.
void
ValueSet::subtract(class ValueSet *subSet)
{
	register int i;

	assert(size == subSet->size);

	for (i = 0; i < size; i++)
	{
		values[i] -= subSet->values[i];
	}
}

// Scalar multiplication.
void
ValueSet::scale(double scalar)
{
	register int i;

	for (i = 0; i < size; i++)
	{
		values[i] *= scalar;
	}
}

// Print.
void
ValueSet::print()
{
	register int i;

	for (i = 0; i < size; i++)
	{
		printf("%f ", values[i]);
	}
	printf("\n");
}
